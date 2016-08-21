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

#include "NSProviderSubscription.h"

NSResult NSInitSubscriptionList()
{
    NS_LOG(DEBUG, "NSInitSubscriptionList - IN");

    consumerSubList = NSStorageCreate();
    consumerSubList->cacheType = NS_PROVIDER_CACHE_SUBSCRIBER;

    NS_LOG(DEBUG, "NSInitSubscriptionList - OUT");
    return NS_OK;
}

NSResult NSSetSubscriptionAccessPolicy(bool policy)
{
    NS_LOG(DEBUG, "NSSetSubscriptionAcceptPolicy - IN");

    if (policy == NS_POLICY_PROVIDER)
    {
        NS_LOG(DEBUG, "Place Provider as a subscription accepter");
    }
    else if (policy == NS_POLICY_CONSUMER)
    {
        NS_LOG(DEBUG, "Place Consumer as a subscription accepter");
    }

    NSSetPolicy(policy);

    NS_LOG(DEBUG, "NSSetSubscriptionAcceptPolicy - OUT");
    return NS_OK;
}

NSResult NSSendAccessPolicyResponse(OCEntityHandlerRequest *entityHandlerRequest)
{
    NS_LOG(DEBUG, "NSSendAccessPolicyResponse - IN");

    // put notification resource
    OCResourceHandle notificationResourceHandle = NULL;
    if (NSPutNotificationResource(NSGetPolicy(), &notificationResourceHandle)
            != NS_OK)
    {
        NS_LOG(ERROR, "Fail to put notification resource");
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

    NS_LOG_V(DEBUG, "NS Provider ID: %s", NSGetProviderInfo()->providerId);

    OCRepPayloadSetUri(payload, NS_ROOT_URI);
    OCRepPayloadSetPropString(payload, NS_ATTRIBUTE_PROVIDER_ID, NSGetProviderInfo()->providerId);
    OCRepPayloadSetPropInt(payload, NS_ATTRIBUTE_POLICY, NSGetPolicy());
    OCRepPayloadSetPropString(payload, NS_ATTRIBUTE_MESSAGE, NS_COLLECTION_MESSAGE_URI);
    OCRepPayloadSetPropString(payload, NS_ATTRIBUTE_SYNC, NS_COLLECTION_SYNC_URI);
    OCRepPayloadSetPropString(payload, NS_ATTRIBUTE_TOPIC, NS_COLLECTION_TOPIC_URI);

    response.requestHandle = entityHandlerRequest->requestHandle;
    response.resourceHandle = entityHandlerRequest->resource;
    response.persistentBufferFlag = 0;
    response.ehResult = OC_EH_OK;
    response.payload = (OCPayload *) payload;

    // Send Response
    if (OCDoResponse(&response) != OC_STACK_OK)
    {
        NS_LOG(ERROR, "Fail to AccessPolicy send response");
        return NS_ERROR;
    }
    OCRepPayloadDestroy(payload);
    NSFreeOCEntityHandlerRequest(entityHandlerRequest);

    NS_LOG(DEBUG, "NSSendAccessPolicyResponse - OUT");
    return NS_OK;
}

void NSHandleSubscription(OCEntityHandlerRequest *entityHandlerRequest, NSResourceType resourceType)
{
    NS_LOG(DEBUG, "NSHandleSubscription - IN");

    char * id = NSGetValueFromQuery(OICStrdup(entityHandlerRequest->query), NS_QUERY_CONSUMER_ID);

    if(!id)
    {
        NSFreeOCEntityHandlerRequest(entityHandlerRequest);
        NS_LOG(ERROR, "Invalid ConsumerID");
        return;
    }

    NS_LOG_V(DEBUG, "consumerId = %s", id);
    if (resourceType == NS_RESOURCE_MESSAGE)
    {
        NS_LOG(DEBUG, "resourceType == NS_RESOURCE_MESSAGE");
        NSCacheElement * element = (NSCacheElement *) OICMalloc(sizeof(NSCacheElement));
        NSCacheSubData * subData = (NSCacheSubData *) OICMalloc(sizeof(NSCacheSubData));

        OICStrcpy(subData->id, UUID_STRING_SIZE, id);
        NS_LOG_V(DEBUG, "SubList ID = [%s]", subData->id);

        NS_LOG_V(DEBUG, "Consumer Address: %s", entityHandlerRequest->devAddr.addr);

        subData->remote_messageObId = subData->messageObId = 0;

        bool iSRemoteServer = false;

#ifdef RD_CLIEND
        iSRemoteServer = NSIsRemoteServerAddress(entityHandlerRequest->devAddr.addr);
        if(iSRemoteServer)
        {
            NS_LOG(DEBUG, "Requested by remote server");
            subData->remote_messageObId = entityHandlerRequest->obsInfo.obsId;
            NS_LOG_V(DEBUG, "SubList message observation ID = [%d]", subData->remote_messageObId);
        }
#endif

        if(!iSRemoteServer)
        {
            NS_LOG(DEBUG, "Requested by local consumer");
            subData->messageObId = entityHandlerRequest->obsInfo.obsId;
            NS_LOG_V(DEBUG, "SubList message observation ID = [%d]", subData->messageObId);
        }

        subData->isWhite = false;
        subData->remote_syncObId = 0;
        subData->syncObId = 0;

        element->data = (void*) subData;
        element->next = NULL;

        if (NSStorageWrite(consumerSubList, element) != NS_OK)
        {
            NS_LOG(DEBUG, "fail to write cache");
        }

        bool currPolicy = NSGetPolicy();

        if (currPolicy == NS_POLICY_PROVIDER)
        {
            NS_LOG(DEBUG, "NSGetSubscriptionAccepter == NS_ACCEPTER_PROVIDER");
            NSAskAcceptanceToUser(entityHandlerRequest);
        }
        else if (currPolicy == NS_POLICY_CONSUMER)
        {
            NS_LOG(DEBUG, "NSGetSubscriptionAccepter == NS_ACCEPTER_CONSUMER");
            NSSendSubscriptionResponse(entityHandlerRequest, true);
        }
    }
    else if (resourceType == NS_RESOURCE_SYNC)
    {
        NS_LOG(DEBUG, "resourceType == NS_RESOURCE_SYNC");
        NSCacheElement * element = (NSCacheElement *) OICMalloc(sizeof(NSCacheElement));
        NSCacheSubData * subData = (NSCacheSubData *) OICMalloc(sizeof(NSCacheSubData));

        OICStrcpy(subData->id, UUID_STRING_SIZE, id);
        NS_LOG_V(DEBUG, "SubList ID = [%s]", subData->id);

        NS_LOG_V(DEBUG, "Consumer Address: %s", entityHandlerRequest->devAddr.addr);

        subData->remote_syncObId = subData->syncObId = 0;
        bool isRemoteServer = false;

#ifdef RD_CLIENT
        isRemoteServer = NSIsRemoteServerAddress(entityHandlerRequest->devAddr.addr);
        if(isRemoteServer)
        {
            NS_LOG(DEBUG, "Requested by remote server");
            subData->remote_syncObId = entityHandlerRequest->obsInfo.obsId;
            NS_LOG_V(DEBUG, "SubList sync observation ID = [%d]", subData->remote_syncObId);
        }
#endif

        if(!isRemoteServer)
        {
            NS_LOG(DEBUG, "Requested by local consumer");
            subData->syncObId = entityHandlerRequest->obsInfo.obsId;
            NS_LOG_V(DEBUG, "SubList sync observation ID = [%d]", subData->syncObId);
        }

        subData->isWhite = false;
        subData->messageObId = 0;
        subData->remote_messageObId = 0;

        element->data = (void*) subData;
        element->next = NULL;

        if (NSStorageWrite(consumerSubList, element) != NS_OK)
        {
            NS_LOG(ERROR, "Fail to write cache");
        }

        NSFreeOCEntityHandlerRequest(entityHandlerRequest);
    }

    NS_LOG(DEBUG, "NSHandleSubscription - OUT");
}

void NSHandleUnsubscription(OCEntityHandlerRequest *entityHandlerRequest)
{
    NS_LOG(DEBUG, "NSHandleUnsubscription - IN");

    NSProviderDeleteSubDataFromObId(consumerSubList, entityHandlerRequest->obsInfo.obsId);

    NSFreeOCEntityHandlerRequest(entityHandlerRequest);

    NS_LOG(DEBUG, "NSHandleUnsubscription - OUT");
}

void NSAskAcceptanceToUser(OCEntityHandlerRequest *entityHandlerRequest)
{
    NS_LOG(DEBUG, "NSAskAcceptanceToUser - IN");

    NSPushQueue(CALLBACK_RESPONSE_SCHEDULER, TASK_CB_SUBSCRIPTION, entityHandlerRequest);

    NS_LOG(DEBUG, "NSAskAcceptanceToUser - OUT");
}

NSResult NSSendResponse(const char * id, bool accepted)
{
    NS_LOG(DEBUG, "NSSendResponse - IN");

    OCRepPayload* payload = OCRepPayloadCreate();
    if (!payload)
    {
        NS_LOG(ERROR, "fail to create playload");
        return NS_ERROR;
    }

    OCResourceHandle rHandle = NULL;
    if (NSPutMessageResource(NULL, &rHandle) != NS_OK)
    {
        NS_LOG(ERROR, "Fail to put notification resource");
        return NS_ERROR;
    }

    OCRepPayloadSetUri(payload, NS_COLLECTION_MESSAGE_URI);
    (accepted) ? OCRepPayloadSetPropInt(payload, NS_ATTRIBUTE_MESSAGE_ID, NS_ALLOW)
        : OCRepPayloadSetPropInt(payload, NS_ATTRIBUTE_MESSAGE_ID, NS_DENY);
    OCRepPayloadSetPropString(payload, NS_ATTRIBUTE_PROVIDER_ID, NSGetProviderInfo()->providerId);

    NSCacheElement * element = NSStorageRead(consumerSubList, id);

    if(element == NULL)
    {
        NS_LOG(ERROR, "element is NULL");
        return NS_ERROR;
    }
    NSCacheSubData * subData = (NSCacheSubData*) element->data;

    if (OCNotifyListOfObservers(rHandle, (OCObservationId*)&subData->messageObId, 1, payload, OC_LOW_QOS)
            != OC_STACK_OK)
    {
        NS_LOG(ERROR, "fail to send Acceptance");
        OCRepPayloadDestroy(payload);
        return NS_ERROR;

    }
    OCRepPayloadDestroy(payload);

    NS_LOG(DEBUG, "NSSendResponse - OUT");
    return NS_OK;
}

NSResult NSSendSubscriptionResponse(OCEntityHandlerRequest *entityHandlerRequest, bool accepted)
{
    NS_LOG(DEBUG, "NSSendSubscriptionResponse - IN");

    if (!entityHandlerRequest)
    {
        NS_LOG(ERROR, "Invalid request pointer");
        return OC_EH_ERROR;
    }

    char * id = NSGetValueFromQuery(OICStrdup(entityHandlerRequest->query), NS_QUERY_CONSUMER_ID);

    if(!id)
    {
        NSFreeOCEntityHandlerRequest(entityHandlerRequest);
        NS_LOG(ERROR, "Invalid ConsumerID");
        return NS_ERROR;
    }

    if (accepted)
    {
        NS_LOG(DEBUG, "accepted is true");
        NSCacheElement * element = (NSCacheElement *) OICMalloc(sizeof(NSCacheElement));
        NSCacheSubData * subData = (NSCacheSubData *) OICMalloc(sizeof(NSCacheSubData));

        OICStrcpy(subData->id, UUID_STRING_SIZE, id);

        subData->isWhite = true;
        subData->remote_messageObId = 0;
        subData->remote_syncObId = 0;
        subData->syncObId = 0;
        subData->messageObId = entityHandlerRequest->obsInfo.obsId;

        element->data = (void*) subData;
        element->next = NULL;

        if (NSStorageWrite(consumerSubList, element) != NS_OK)
        {
            NS_LOG(ERROR, "fail to write consumer white list");
        }
    }

    NSSendResponse(id, accepted);

    NSFreeOCEntityHandlerRequest(entityHandlerRequest);

    NS_LOG(DEBUG, "NSSendSubscriptionResponse - OUT");
    return NS_OK;
}

void * NSSubScriptionSchedule(void *ptr)
{
    if (ptr == NULL)
    {
        NS_LOG(DEBUG, "Create NSSubScriptionSchedule");
    }

    while (NSIsRunning[SUBSCRIPTION_SCHEDULER])
    {
        sem_wait(&NSSemaphore[SUBSCRIPTION_SCHEDULER]);
        pthread_mutex_lock(&NSMutex[SUBSCRIPTION_SCHEDULER]);

        if (NSHeadMsg[SUBSCRIPTION_SCHEDULER] != NULL)
        {
            NSTask *node = NSHeadMsg[SUBSCRIPTION_SCHEDULER];
            NSHeadMsg[SUBSCRIPTION_SCHEDULER] = node->nextTask;

            switch (node->taskType)
            {
                case TASK_SEND_POLICY:
                    NS_LOG(DEBUG, "CASE TASK_SEND_POLICY : ");
                    NSSendAccessPolicyResponse((OCEntityHandlerRequest*) node->taskData);
                    break;

                case TASK_RECV_SUBSCRIPTION:
                    NS_LOG(DEBUG, "CASE TASK_RECV_SUBSCRIPTION : ");
                    NSHandleSubscription((OCEntityHandlerRequest*) node->taskData,
                            NS_RESOURCE_MESSAGE);
                    break;

                case TASK_RECV_UNSUBSCRIPTION:
                    NS_LOG(DEBUG, "CASE TASK_RECV_UNSUBSCRIPTION : ");
                    NSHandleUnsubscription((OCEntityHandlerRequest*) node->taskData);
                    break;

                case TASK_SEND_ALLOW:
                {
                    NS_LOG(DEBUG, "CASE TASK_SEND_ALLOW : ");
                    NSConsumer * consumer = (NSConsumer *) node->taskData;

                    NSCacheUpdateSubScriptionState(consumerSubList, consumer->consumerId, true);
                    NSSendResponse(consumer->consumerId, true);
                    NSFreeConsumer(consumer);
                    break;
                }
                case TASK_SEND_DENY:
                {
                    NS_LOG(DEBUG, "CASE TASK_SEND_DENY : ");
                    NSConsumer * consumer = (NSConsumer *) node->taskData;

                    NSCacheUpdateSubScriptionState(consumerSubList, consumer->consumerId, false);
                    NSSendResponse(consumer->consumerId, false);
                    NSFreeConsumer(consumer);

                    break;
                }
                case TASK_SYNC_SUBSCRIPTION:
                    NS_LOG(DEBUG, "CASE TASK_SYNC_SUBSCRIPTION : ");
                    NSHandleSubscription((OCEntityHandlerRequest*) node->taskData,
                            NS_RESOURCE_SYNC);
                    break;
                default:
                    break;

            }
            OICFree(node);
        }

        pthread_mutex_unlock(&NSMutex[SUBSCRIPTION_SCHEDULER]);

    }
    NS_LOG(INFO, "Destroy NSSubScriptionSchedule");
    return NULL;
}
