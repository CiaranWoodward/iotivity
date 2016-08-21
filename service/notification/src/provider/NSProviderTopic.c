//******************************************************************
//
// Copyright 2016 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "NSProviderTopic.h"

NSResult NSSendTopicUpdation();

NSResult NSInitTopicList()
{
    NS_LOG(DEBUG, "NSInitTopicList - IN");
    consumerTopicList = NSStorageCreate();
    consumerTopicList->cacheType = NS_PROVIDER_CACHE_CONSUMER_TOPIC_NAME;

    registeredTopicList = NSStorageCreate();
    registeredTopicList->cacheType = NS_PROVIDER_CACHE_REGISTER_TOPIC;

    NS_LOG(DEBUG, "NSInitTopicList - OUT");
    return NS_OK;
}

NSResult NSAddTopics(const char * topicName)
{
    NS_LOG(DEBUG, "NSWriteTopicsToStorage()");

    NSCacheTopicData * data = (NSCacheTopicData *)OICMalloc(sizeof(NSCacheTopicData));
    data->topicName = topicName;
    data->state = NS_TOPIC_UNSUBSCRIBED;

    NSCacheElement * element = (NSCacheElement *) OICMalloc(sizeof(NSCacheElement));
    element->data = (void *) data;
    element->next = NULL;

    if(NSStorageWrite(registeredTopicList, element) != NS_OK)
    {
        NS_LOG(DEBUG, "fail to write cache");
    }
    NSSendTopicUpdation();

    NS_LOG(DEBUG, "NSWriteTopicsToStorage() NS_OK");
    return NS_OK;
}

NSResult NSDeleteTopics(const char * topicName)
{
    NS_LOG(DEBUG, "NSDeleteTopics()");

    if(!topicName)
    {
        NS_LOG(ERROR, "topicName is NULL");
        return NS_ERROR;
    }

    NSStorageDelete(registeredTopicList, topicName);
    while(NSStorageDelete(consumerTopicList, topicName) != NS_FAIL);
    return NS_OK;
}

NSResult NSSendTopicUpdation()
{
    NS_LOG(DEBUG, "NSSendTopicUpdation - IN");

    OCRepPayload* payload = OCRepPayloadCreate();

    if (!payload)
    {
        NS_LOG(ERROR, "fail to create playload");
        return NS_ERROR;
    }

    OCResourceHandle rHandle = NULL;
    if (NSPutMessageResource(NULL, &rHandle) != NS_OK)
    {
        NS_LOG(ERROR, "Fail to put message resource");
        return NS_ERROR;
    }

    OCRepPayloadSetUri(payload, NS_COLLECTION_MESSAGE_URI);
    OCRepPayloadSetPropInt(payload, NS_ATTRIBUTE_MESSAGE_ID, NS_TOPIC);
    OCRepPayloadSetPropString(payload, NS_ATTRIBUTE_PROVIDER_ID, NSGetProviderInfo()->providerId);

    OCObservationId obArray[255] = { 0, };
    int obCount = 0;

    NSCacheElement * it = consumerSubList->head;

    while (it)
    {
        NSCacheSubData * subData = (NSCacheSubData *) it->data;

        if (subData->isWhite)
        {
            if(subData->messageObId != 0)
            {
                obArray[obCount++] = subData->messageObId;
            }

#ifdef RD_CLIENT
            if(subData->remote_messageObId != 0)
            {
                obArray[obCount++] = subData->remote_messageObId;
            }
#endif

        }
        it = it->next;
    }

    if(!obCount)
    {
        NS_LOG(ERROR, "observer count is zero");
        return NS_ERROR;
    }

    if (OCNotifyListOfObservers(rHandle, obArray, obCount, payload, OC_HIGH_QOS)
            != OC_STACK_OK)
    {
        NS_LOG(ERROR, "fail to send topic updation");
        OCRepPayloadDestroy(payload);
        return NS_ERROR;

    }
    OCRepPayloadDestroy(payload);

    NS_LOG(DEBUG, "NSSendTopicUpdation - OUT");
    return NS_OK;
}

NSResult NSSendTopicUpdationToConsumer(char *consumerId)
{
    NS_LOG(DEBUG, "NSSendTopicUpdationToConsumer - IN");

    OCRepPayload* payload = OCRepPayloadCreate();

    if (!payload)
    {
        NS_LOG(ERROR, "fail to create playload");
        return NS_ERROR;
    }

    OCResourceHandle rHandle = NULL;
    if (NSPutMessageResource(NULL, &rHandle) != NS_OK)
    {
        NS_LOG(ERROR, "Fail to put message resource");
        return NS_ERROR;
    }

    OCRepPayloadSetUri(payload, NS_COLLECTION_MESSAGE_URI);
    OCRepPayloadSetPropInt(payload, NS_ATTRIBUTE_MESSAGE_ID, NS_TOPIC);
    OCRepPayloadSetPropString(payload, NS_ATTRIBUTE_PROVIDER_ID, NSGetProviderInfo()->providerId);

    NSCacheElement * element = NSStorageRead(consumerSubList, consumerId);

    if(element == NULL)
    {
        NS_LOG(ERROR, "element is NULL");
        return NS_ERROR;
    }

    NSCacheSubData * subData = (NSCacheSubData*) element->data;

    if (OCNotifyListOfObservers(rHandle, (OCObservationId*)&subData->messageObId, 1, payload, OC_HIGH_QOS)
            != OC_STACK_OK)
    {
        NS_LOG(ERROR, "fail to send topic updation");
        OCRepPayloadDestroy(payload);
        return NS_ERROR;

    }

    OCRepPayloadDestroy(payload);

    NS_LOG(DEBUG, "NSSendTopicUpdationToConsumer - OUT");
    return NS_OK;
}

NSResult NSSendTopicList(OCEntityHandlerRequest * entityHandlerRequest)
{
    NS_LOG(DEBUG, "NSSendTopicList - IN");

    char * id = NSGetValueFromQuery(OICStrdup(entityHandlerRequest->query), NS_QUERY_CONSUMER_ID);
    NSTopicLL * topics = NULL;
    NSCacheElement * currList = NULL;

    if(!id)
    {
        NS_LOG(DEBUG, "Send registered topic list");
        topics = NSProviderGetTopicsCacheData(registeredTopicList);
        currList = registeredTopicList->head;
    }
    else
    {
        NS_LOG(DEBUG, "Send subscribed topic list to consumer");
        topics = NSProviderGetConsumerTopicsCacheData(registeredTopicList, consumerTopicList, id);
        currList = consumerTopicList->head;
    }

    if(!currList)
    {
        NS_LOG(DEBUG, "currList is NULL");
        return NS_ERROR;
    }

    // make response for the Get Request
    OCEntityHandlerResponse response;
    response.numSendVendorSpecificHeaderOptions = 0;
    memset(response.sendVendorSpecificHeaderOptions, 0,
            sizeof response.sendVendorSpecificHeaderOptions);
    memset(response.resourceUri, 0, sizeof response.resourceUri);

    OCRepPayload* payload = OCRepPayloadCreate();
    if (!payload)
    {
        NS_LOG(ERROR, "payload is NULL");
        return NS_ERROR;
    }

    // set topics to the array of resource property

    NSCacheElement * iter = currList;
    size_t dimensionSize = (size_t)NSProviderGetListSize(iter);

    NS_LOG_V(DEBUG, "dimensionSize = %d", dimensionSize);

    if(!dimensionSize)
    {
        return NS_ERROR;
    }

    OCRepPayload** payloadTopicArray = (OCRepPayload **) OICMalloc(
            sizeof(OCRepPayload *) * dimensionSize);

    size_t dimensions[3] = {dimensionSize, 0, 0};
    for (int i = 0; i < (int)dimensionSize; i++)
    {
        NSTopicLL * topic = (NSTopicLL *) iter->data;

        NS_LOG_V(DEBUG, "topicName = %s", topic->topicName);
        NS_LOG_V(DEBUG, "topicState = %d",(int) topic->state);

        payloadTopicArray[i] = OCRepPayloadCreate();
        OCRepPayloadSetPropString(payloadTopicArray[i], NS_ATTRIBUTE_TOPIC_NAME, topic->topicName);
        OCRepPayloadSetPropInt(payloadTopicArray[i], NS_ATTRIBUTE_TOPIC_SELECTION,
                (int)topic->state);

        iter = iter->next;
    }

    OCRepPayloadSetUri(payload, NS_COLLECTION_TOPIC_URI);
    if(id)
    {
        OCRepPayloadSetPropString(payload, NS_ATTRIBUTE_CONSUMER_ID, id);
    }
    OCRepPayloadSetPropObjectArray(payload, NS_ATTRIBUTE_TOPIC_LIST,
            (const OCRepPayload**)(payloadTopicArray), dimensions);

    response.requestHandle = entityHandlerRequest->requestHandle;
    response.resourceHandle = entityHandlerRequest->resource;
    response.persistentBufferFlag = 0;
    response.ehResult = OC_EH_OK;
    response.payload = (OCPayload *) payload;

    if (OCDoResponse(&response) != OC_STACK_OK)
    {
        NS_LOG(ERROR, "Fail to response topic list");
        return NS_ERROR;
    }
    OCRepPayloadDestroy(payload);
    NSFreeOCEntityHandlerRequest(entityHandlerRequest);

    NS_LOG(DEBUG, "NSSendTopicList - OUT");
    return NS_OK;
}

NSResult NSPostConsumerTopics(OCEntityHandlerRequest * entityHandlerRequest)
{
    NS_LOG(DEBUG, "NSPostConsumerTopics() - IN");

    char * consumerId = NULL;
    OCRepPayload * payload = entityHandlerRequest->payload;
    OCRepPayloadGetPropString(payload, NS_ATTRIBUTE_CONSUMER_ID, &consumerId);

    if(!consumerId)
    {
        NS_LOG(DEBUG, "Invalid consumer ID");
        return NS_ERROR;
    }

    OCRepPayload ** topicListPayload = NULL;
    OCRepPayloadValue * payloadValue = NULL;
    payloadValue = NSPayloadFindValue(payload, NS_ATTRIBUTE_TOPIC_LIST);
    size_t dimensionSize = calcDimTotal(payloadValue->arr.dimensions);
    size_t dimensions[3] = {dimensionSize, 0, 0};
    OCRepPayloadGetPropObjectArray(payload, NS_ATTRIBUTE_TOPIC_LIST, & topicListPayload, dimensions);

    for(int i = 0; i <(int)dimensionSize; i++)
    {
        char * topicName = NULL;
        int topicState = 0;

        OCRepPayloadGetPropString(topicListPayload[i], NS_ATTRIBUTE_TOPIC_NAME, &topicName);
        OCRepPayloadGetPropInt(topicListPayload[i], NS_ATTRIBUTE_TOPIC_SELECTION, &topicState);
        NS_LOG_V(DEBUG, "Topic Name(state):  %s(%d)", topicName, topicState);

        if(NS_TOPIC_SUBSCRIBED == (NSTopicState)topicState)
        {
            NSCacheTopicSubData * topicSubData =
            (NSCacheTopicSubData *) OICMalloc(sizeof(NSCacheTopicSubData));

            OICStrcpy(topicSubData->id, NS_UUID_STRING_SIZE, consumerId);
            topicSubData->topicName = OICStrdup(topicName);

            NSCacheElement * newObj = (NSCacheElement *) OICMalloc(sizeof(NSCacheElement));
            newObj->data = (NSCacheData *) topicSubData;
            newObj->next = NULL;
            NSStorageWrite(consumerTopicList, newObj);
        }
    }

    NS_LOG(DEBUG, "NSPostConsumerTopics() - OUT");
    return NS_OK;
}

void * NSTopicSchedule(void * ptr)
{
    if (ptr == NULL)
    {
        NS_LOG(DEBUG, "Create NSTopicSchedule");
    }

    while (NSIsRunning[TOPIC_SCHEDULER])
    {
        sem_wait(&NSSemaphore[TOPIC_SCHEDULER]);
        pthread_mutex_lock(&NSMutex[TOPIC_SCHEDULER]);

        if (NSHeadMsg[TOPIC_SCHEDULER] != NULL)
        {
            NSTask *node = NSHeadMsg[TOPIC_SCHEDULER];
            NSHeadMsg[TOPIC_SCHEDULER] = node->nextTask;

            switch (node->taskType)
            {
                case TASK_SEND_TOPICS:
                    NS_LOG(DEBUG, "CASE TASK_SEND_TOPICS : ");
                    NSSendTopicList((OCEntityHandlerRequest*) node->taskData);
                    break;
                case TASK_SUBSCRIBE_TOPIC:
                    NS_LOG(DEBUG, "CASE TASK_SUBSCRIBE_TOPIC : ");
                    NSCacheElement * newObj = (NSCacheElement *) OICMalloc(sizeof(NSCacheElement));
                    newObj->data = node->taskData;
                    newObj->next = NULL;
                    NSStorageWrite(consumerTopicList, newObj);
                    break;
                case TASK_UNSUBSCRIBE_TOPIC:
                    NS_LOG(DEBUG, "CASE TASK_SUBSCRIBE_TOPIC : ");
                    NSProviderDeleteConsumerTopic(consumerTopicList,
                            (NSCacheTopicSubData *) node->taskData);
                    NS_LOG(DEBUG, "CASE TASK_SUBSCRIBE_TOPIC AFter: ");
                    break;
                case TASK_ADD_TOPIC:
                {
                    NS_LOG(DEBUG, "CASE TASK_ADD_TOPIC : ");
                    NSAddTopics((const char *) node->taskData);
                }
                    break;
                case TASK_DELETE_TOPIC:
                {
                    NS_LOG(DEBUG, "CASE_TASK_DELETE_TOPIC : ");
                    NSDeleteTopics((const char *) node->taskData);
                }
                    break;
                case TASK_POST_TOPIC:
                {
                    NS_LOG(DEBUG, "TASK_POST_TOPIC : ");
                    NSPostConsumerTopics((OCEntityHandlerRequest*) node->taskData);
                }
                    break;
                default:
                    break;
            }

            OICFree(node);
        }

        pthread_mutex_unlock(&NSMutex[TOPIC_SCHEDULER]);
    }

    NS_LOG(DEBUG, "Destroy NSTopicSchedule");
    return NULL;
}
