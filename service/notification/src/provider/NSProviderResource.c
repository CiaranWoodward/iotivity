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

#include "NSProviderResource.h"
#include <string.h>

NSNotificationResource NotificationResource;
NSMessageResource NotificationMessageResource;
NSSyncResource NotificationSyncResource;
NSTopicResource NotificationTopicResource;

#ifdef RD_CLIENT
OCStackApplicationResult NSHandlePublishCb(void *ctx, OCDoHandle handle,
    OCClientResponse *clientResponse)
{
    (void) handle;
    (void) ctx;

    NS_LOG_V(DEBUG, "Publish resource response received code: %d", clientResponse->result);

    return OC_STACK_KEEP_TRANSACTION;
}

NSResult NSPublishResourceToCloud(char *serverAddress)
{

    NS_LOG(DEBUG, "NSPublishResourceToCloud - IN");
    NS_LOG_V(DEBUG, "Remote Server Address: %s", serverAddress);

    OCResourceHandle resourceHandles[1] = {NotificationResource.handle};
    OCStackResult res = OCRDPublish(serverAddress, CT_ADAPTER_TCP, resourceHandles, 1,
            &NSHandlePublishCb, OC_LOW_QOS);

    if (res != OC_STACK_OK)
    {
        NS_LOG(DEBUG, "Unable to publish resources to cloud");
    }

    NS_LOG(DEBUG, "NSPublishResourceToCloud - OUT");
    return NS_OK;
}
#endif

NSResult NSCreateResource(char *uri)
{
    NS_LOG(DEBUG, "NSCreateResource - IN");
    if (!uri)
    {
        NS_LOG(NS_ERROR, "Resource URI cannot be NULL");
        return NS_ERROR;
    }

    if (strcmp(uri, NS_ROOT_URI) == 0)
    {
        NotificationResource.accepter = 0;
        NotificationResource.message_uri = NS_COLLECTION_MESSAGE_URI;
        NotificationResource.sync_uri = NS_COLLECTION_SYNC_URI;
        NotificationResource.topic_uri = NS_COLLECTION_TOPIC_URI;
        NotificationResource.handle = NULL;

        if (OCCreateResource(&NotificationResource.handle, NS_ROOT_TYPE, NS_DEFAULT_INTERFACE,
                NS_ROOT_URI, NSEntityHandlerNotificationCb, NULL, OC_DISCOVERABLE) != OC_STACK_OK)
        {
            NS_LOG(NS_ERROR, "Fail to Create Notification Resource");
            return NS_ERROR;
        }
    }
    else if (strcmp(uri, NS_COLLECTION_MESSAGE_URI) == 0)
    {

        NotificationMessageResource.messageId = 0;

        (NotificationMessageResource.providerId)[0] = '\0';
        NotificationMessageResource.type = 0;
        NotificationMessageResource.dateTime = NULL;
        NotificationMessageResource.ttl = 0;
        NotificationMessageResource.title = NULL;
        NotificationMessageResource.contentText = NULL;
        NotificationMessageResource.sourceName = NULL;
        NotificationMessageResource.mediaContents = NULL;

        if (OCCreateResource(&NotificationMessageResource.handle, NS_COLLECTION_MESSAGE_TYPE,
                NS_DEFAULT_INTERFACE, NS_COLLECTION_MESSAGE_URI, NSEntityHandlerMessageCb, NULL,
                OC_OBSERVABLE) != OC_STACK_OK)
        {
            NS_LOG(NS_ERROR, "Fail to Create Notification Message Resource");
            return NS_ERROR;
        }
    }
    else if (strcmp(uri, NS_COLLECTION_SYNC_URI) == 0)
    {
        NotificationSyncResource.id = NULL;
        NotificationSyncResource.state = NULL;
        NotificationSyncResource.handle = NULL;

        if (OCCreateResource(&(NotificationSyncResource.handle), NS_COLLECTION_SYNC_TYPE,
                NS_DEFAULT_INTERFACE, NS_COLLECTION_SYNC_URI, NSEntityHandlerSyncCb, NULL,
                OC_OBSERVABLE) != OC_STACK_OK)
        {
            NS_LOG(NS_ERROR, "Fail to Create Notification Sync Resource");
            return NS_ERROR;
        }
    }
    else if (strcmp(uri, NS_COLLECTION_TOPIC_URI) == 0)
    {
        (NotificationTopicResource.consumerId)[0] = '\0';
        NotificationTopicResource.TopicList = NULL;
        NotificationTopicResource.handle = NULL;

        if (OCCreateResource(&(NotificationTopicResource.handle), NS_COLLECTION_TOPIC_TYPE,
                NS_DEFAULT_INTERFACE, NS_COLLECTION_TOPIC_URI, NSEntityHandlerTopicCb, NULL,
                OC_DISCOVERABLE) != OC_STACK_OK)
        {
            NS_LOG(NS_ERROR, "Fail to Create Notification Sync Resource");
            return NS_ERROR;
        }
    }
    else
    {
        NS_LOG(ERROR, "Fail to create resource with invalid URI");
        return NS_ERROR;
    }

    NS_LOG(DEBUG, "NSCreateResource - OUT");
    return NS_OK;
}

NSResult NSRegisterResource()
{
    NS_LOG(DEBUG, "NSRegisterResource - IN");

    if (NSCreateResource(NS_COLLECTION_TOPIC_URI) != NS_OK)
    {
        NS_LOG(ERROR, "Fail to register Topic Resource");
        return NS_ERROR;
    }

    if (NSCreateResource(NS_COLLECTION_SYNC_URI) != NS_OK)
    {
        NS_LOG(ERROR, "Fail to register Sync Resource");
        return NS_ERROR;
    }

    if (NSCreateResource(NS_COLLECTION_MESSAGE_URI) != NS_OK)
    {
        NS_LOG(ERROR, "Fail to register Message Resource");
        return NS_ERROR;
    }

    if (NSCreateResource(NS_ROOT_URI) != NS_OK)
    {
        NS_LOG(ERROR, "Fail to register Notification Resource");
        return NS_ERROR;
    }

    NS_LOG(DEBUG, "NSRegisterResource - OUT");
    return NS_OK;
}

NSResult NSUnRegisterResource()
{
    NS_LOG(DEBUG, "NSUnRegisterResource - IN");

    if (OCDeleteResource(NotificationResource.handle) != OC_STACK_OK)
    {
        NS_LOG(ERROR, "Fail to Delete Notification Resource");
        return NS_ERROR;
    }

    if (OCDeleteResource(NotificationMessageResource.handle) != OC_STACK_OK)
    {
        NS_LOG(ERROR, "Fail to Delete Notification Message Resource");
        return NS_ERROR;
    }

    if (OCDeleteResource(NotificationSyncResource.handle) != OC_STACK_OK)
    {
        NS_LOG(ERROR, "Fail to Delete Notification Sync Resource");
        return NS_ERROR;
    }

    if (OCDeleteResource(NotificationTopicResource.handle) != OC_STACK_OK)
    {
        NS_LOG(ERROR, "Fail to Delete Notification Topic Resource");
        return NS_ERROR;
    }

    NotificationResource.handle = NULL;
    NotificationMessageResource.handle = NULL;
    NotificationSyncResource.handle = NULL;
    NotificationTopicResource.handle = NULL;

    NS_LOG(DEBUG, "NSUnRegisterResource - OUT");
    return NS_OK;
}

NSResult NSPutNotificationResource(int accepter, OCResourceHandle * handle)
{
    NS_LOG(DEBUG, "NSPutNotificationResource - IN");

    NotificationResource.accepter = accepter;
    NotificationResource.message_uri = NS_COLLECTION_MESSAGE_URI;
    NotificationResource.sync_uri = NS_COLLECTION_SYNC_URI;
    NotificationResource.topic_uri = NS_COLLECTION_TOPIC_URI;

    *handle = NotificationResource.handle;

    NS_LOG(DEBUG, "NSPutNotificationResource - OUT");
    return NS_OK;
}

NSResult NSPutMessageResource(NSMessage *msg, OCResourceHandle * handle)
{
    NS_LOG(DEBUG, "NSPutMessageResource - IN");

    if(msg != NULL)
    {
        NS_LOG(DEBUG, "NSMessage is valid");

        NotificationMessageResource.messageId = msg->messageId;
        OICStrcpy(NotificationMessageResource.providerId, UUID_STRING_SIZE, msg->providerId);
        NotificationMessageResource.type = msg->type;
        NotificationMessageResource.dateTime = msg->dateTime;
        NotificationMessageResource.ttl = msg->ttl;
        NotificationMessageResource.title = msg->title;
        NotificationMessageResource.contentText = msg->contentText;
        NotificationMessageResource.sourceName = msg->sourceName;
        NotificationMessageResource.mediaContents = msg->mediaContents;
    }
    else
    {
        NS_LOG(ERROR, "NSMessage is NULL");
    }

    *handle = NotificationMessageResource.handle;
    NS_LOG(DEBUG, "NSPutMessageResource - OUT");

    return NS_OK;
}

NSResult NSPutSyncResource(NSSyncInfo *sync, OCResourceHandle * handle)
{
    NS_LOG(DEBUG, "NSPutSyncResource - IN");

    (void) sync;

    *handle = NotificationSyncResource.handle;

    NS_LOG(DEBUG, "NSPutSyncResource - OUT");
    return NS_OK;
}

NSResult NSPutTopicResource(NSTopicList *topicList, OCResourceHandle * handle)
{
    NS_LOG(DEBUG, "NSPutTopicResource - IN");

    (void) topicList;

    *handle = NotificationTopicResource.handle;

    NS_LOG(DEBUG, "NSPutTopicResource - OUT");
    return NS_OK;
}
