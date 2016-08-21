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

#include "NSConsumerMemoryCache.h"
#include "oic_malloc.h"
#include "oic_string.h"

pthread_mutex_t * NSGetCacheMutex()
{
    static pthread_mutex_t * g_NSCacheMutex = NULL;
    if (g_NSCacheMutex == NULL)
    {
        g_NSCacheMutex = (pthread_mutex_t *) OICMalloc(sizeof(pthread_mutex_t));
        NS_VERIFY_NOT_NULL(g_NSCacheMutex, NULL);

        pthread_mutex_init(g_NSCacheMutex, NULL);
    }
    return g_NSCacheMutex;
}

NSCacheList * NSStorageCreate()
{
    pthread_mutex_t * mutex = NSGetCacheMutex();
    pthread_mutex_lock(mutex);

    NSCacheList * newList = (NSCacheList *) OICMalloc(sizeof(NSCacheList));
    NS_VERIFY_NOT_NULL_WITH_POST_CLEANING(newList, NULL, pthread_mutex_unlock(mutex));

    newList->head = NULL;
    newList->tail = NULL;

    pthread_mutex_unlock(mutex);

    return newList;
}

NSCacheElement * NSStorageRead(NSCacheList * list, const char * findId)
{
    NS_VERIFY_NOT_NULL(list, NULL);
    NS_VERIFY_NOT_NULL(findId, NULL);

    pthread_mutex_t * mutex = NSGetCacheMutex();
    pthread_mutex_lock(mutex);

    NSCacheElement * iter = list->head;
    NSCacheType type = list->cacheType;

    while (iter)
    {
        if (NSConsumerCompareIdCacheData(type, iter->data, findId))
        {
            pthread_mutex_unlock(mutex);
            return iter;
        }

        iter = iter->next;
    }

    NS_LOG (DEBUG, "No Cache Element");
    pthread_mutex_unlock(mutex);
    return NULL;
}

NSCacheElement * NSGetProviderFromAddr(NSCacheList * list, const char * addr, uint16_t port)
{
    NS_VERIFY_NOT_NULL(list, NULL);
    NS_VERIFY_NOT_NULL(addr, NULL);
    NS_VERIFY_NOT_NULL(
            (list->cacheType != NS_CONSUMER_CACHE_PROVIDER) ? NULL : (void *) 1, NULL);

    pthread_mutex_t * mutex = NSGetCacheMutex();
    pthread_mutex_lock(mutex);

    NSCacheElement * iter = list->head;

    while (iter)
    {
        NSProviderConnectionInfo * connection =
                ((NSProviderConnectionInfo *) iter->data)->next;
        while (connection)
        {
            char * conAddr = connection->addr->addr;
            uint16_t conPort = connection->addr->port;

            if (!strcmp(conAddr, addr) && conPort == port)
            {
                pthread_mutex_unlock(mutex);
                return iter;
            }
            connection = connection->next;
        }

        iter = iter->next;
    }

    NS_LOG (DEBUG, "No Cache Element");
    pthread_mutex_unlock(mutex);
    return NULL;
}

NSResult NSStorageWrite(NSCacheList * list, NSCacheElement * newObj)
{
    NS_VERIFY_NOT_NULL(list, NS_ERROR);
    NS_VERIFY_NOT_NULL(newObj, NS_ERROR);

    NSCacheType type = list->cacheType;
    NS_LOG_V(DEBUG, "cache type : %d", type);

    if (type == NS_CONSUMER_CACHE_MESSAGE)
    {
        return NSConsumerCacheWriteMessage(list, newObj);
    }
    else if (type == NS_CONSUMER_CACHE_PROVIDER)
    {
        return NSConsumerCacheWriteProvider(list, newObj);
    }

    NS_LOG (ERROR, "Not Supported Type");

    return NS_ERROR;
}

NSResult NSStorageDelete(NSCacheList * list, const char * delId)
{
    NS_VERIFY_NOT_NULL(list, NS_ERROR);
    NS_VERIFY_NOT_NULL(delId, NS_ERROR);

    NSCacheType type = list->cacheType;

    pthread_mutex_t * mutex = NSGetCacheMutex();
    pthread_mutex_lock(mutex);

    NSCacheElement * prev = list->head;
    NSCacheElement * del = list->head;
    NS_VERIFY_NOT_NULL_WITH_POST_CLEANING(del, NS_ERROR, pthread_mutex_unlock(mutex));

    if (NSConsumerCompareIdCacheData(type, del->data, delId))
    {
        if (del == list->head)
        {
            if (del == list->tail)
            {
                list->tail = del->next;
            }

            list->head = del->next;

            if (type == NS_CONSUMER_CACHE_MESSAGE)
            {
                NSRemoveMessage((NSMessage *) del->data);
            }
            else if (type == NS_CONSUMER_CACHE_PROVIDER)
            {
                NSRemoveProvider_internal((NSProvider_internal *) del->data);
            }
            NSOICFree(del);
            pthread_mutex_unlock(mutex);

            return NS_OK;
        }
    }

    del = del->next;
    while (del)
    {
        if (NSConsumerCompareIdCacheData(type, del->data, delId))
        {
            if (del == list->tail)
            {
                list->tail = prev;
            }

            prev->next = del->next;
            if (type == NS_CONSUMER_CACHE_MESSAGE)
            {
                NSRemoveMessage((NSMessage *) del->data);
            }
            else if (type == NS_CONSUMER_CACHE_PROVIDER)
            {
                NSRemoveProvider_internal((NSProvider_internal *) del->data);
            }
            NSOICFree(del);
            pthread_mutex_unlock(mutex);

            return NS_OK;
        }

        prev = del;
        del = del->next;
    }
    pthread_mutex_unlock(mutex);
    return NS_OK;
}

void NSConsumerRemoveMessageStore(NSCacheElement * ele, NSStoreMessage * msg)
{
    NSOICFree(ele);
    NSOICFree(msg);
}

NSResult NSConsumerCacheWriteMessage(NSCacheList * list, NSCacheElement * newObj)
{
    NS_VERIFY_NOT_NULL(list, NS_ERROR);
    NS_VERIFY_NOT_NULL(newObj, NS_ERROR);

    pthread_mutex_t * mutex = NSGetCacheMutex();

    NSMessage * newMsgObj = ((NSStoreMessage *)newObj->data)->msg;

    char msgId[NS_DEVICE_ID_LENGTH] = {0, };
    snprintf(msgId, NS_DEVICE_ID_LENGTH, "%lld", (long long int)newMsgObj->messageId);
    NSCacheElement * it = NSStorageRead(list, msgId);

    if (it)
    {
        NS_LOG(DEBUG, "Update message status.");
        pthread_mutex_lock(mutex);
        NSStoreMessage * sMsgObj = (NSStoreMessage *) it->data;
        if(sMsgObj->status == ((NSStoreMessage *)newObj->data)->status)
        {
            NS_LOG (DEBUG, "Already receive message");
            pthread_mutex_unlock(mutex);
            return NS_ERROR;
        }

        sMsgObj->status = ((NSStoreMessage *)newObj->data)->status;
        pthread_mutex_unlock(mutex);
        return NS_OK;
    }

    NS_LOG(DEBUG, "Add message at storage.");
    NSStoreMessage * sMsgObj = (NSStoreMessage *) OICMalloc(sizeof(NSStoreMessage));
    NS_VERIFY_NOT_NULL(sMsgObj, NS_ERROR);

    NSCacheElement * obj = (NSCacheElement *) OICMalloc(sizeof(NSCacheElement));
    NS_VERIFY_NOT_NULL_WITH_POST_CLEANING(obj, NS_ERROR, NSOICFree(sMsgObj));

    sMsgObj->status = NS_SYNC_UNREAD;
    sMsgObj->msg = (void *) NSCopyMessage(newMsgObj);
    NS_VERIFY_NOT_NULL_WITH_POST_CLEANING(sMsgObj->msg, NS_ERROR,
          NSConsumerRemoveMessageStore(obj, sMsgObj));

    obj->next = NULL;
    obj->data = (NSCacheData *) sMsgObj;

    pthread_mutex_lock(mutex);
    if (!list->head)
    {
        list->head = obj;
        list->tail = obj;
        pthread_mutex_unlock(mutex);
        return NS_OK;
    }

    (list->tail)->next = obj;
    list->tail = obj;
    pthread_mutex_unlock(mutex);

    return NS_OK;
}

NSResult NSConsumerCacheWriteProvider(NSCacheList * list, NSCacheElement * newObj)
{
    NS_VERIFY_NOT_NULL(list, NS_ERROR);
    NS_VERIFY_NOT_NULL(newObj, NS_ERROR);

    pthread_mutex_t * mutex = NSGetCacheMutex();

    NS_LOG (DEBUG, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1");
    NSProvider_internal * prov = (NSProvider_internal *)newObj->data;
    NS_LOG_V (DEBUG, "%s", prov->providerId);
    NS_LOG (DEBUG, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1");

    NSProvider_internal * newProvObj = (NSProvider_internal *) newObj->data;

    NSCacheElement * it = NSStorageRead(list, newProvObj->providerId);
    pthread_mutex_lock(mutex);

    if (it)
    {
        NSProvider_internal * provObj = (NSProvider_internal *) it->data;

        NSProviderConnectionInfo * infos = provObj->connection;
        NSProviderConnectionInfo * lastConn = infos->next;
        while(lastConn)
        {
            infos = lastConn;
            lastConn = lastConn->next;
        }
        infos->next = NSCopyProviderConnections(newProvObj->connection);

        pthread_mutex_unlock(mutex);

        return NS_OK;
    }

    NSCacheElement * obj = (NSCacheElement *) OICMalloc(sizeof(NSCacheElement));
    NS_VERIFY_NOT_NULL_WITH_POST_CLEANING(obj, NS_ERROR, pthread_mutex_unlock(mutex));

    NS_LOG_V(DEBUG, "New Object address : %s:%d", newProvObj->connection->addr->addr, newProvObj->connection->addr->port);
    obj->data = (void *) NSCopyProvider_internal(newProvObj);

    NS_LOG (DEBUG, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!2");
    prov = (NSProvider_internal *)obj->data;
    NS_LOG_V (DEBUG, "%s", prov->providerId);
    NS_LOG (DEBUG, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!2");

    if (!obj->data)
    {
        NS_LOG (ERROR, "Failed to CopyProvider");
        pthread_mutex_unlock(mutex);

        return NS_ERROR;
    }
    obj->next = NULL;

    if (!list->head)
    {
        list->head = obj;
        list->tail = obj;

        NS_LOG (DEBUG, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!3");
        prov = (NSProvider_internal *)list->tail->data;
        NS_LOG_V (DEBUG, "%s", prov->providerId);
        NS_LOG (DEBUG, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!3");

        pthread_mutex_unlock(mutex);

        return NS_OK;
    }

    (list->tail)->next = obj;
    list->tail = obj;

    NS_LOG (DEBUG, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!4");
    prov = (NSProvider_internal *)list->tail->data;
    NS_LOG_V (DEBUG, "%s", prov->providerId);
    NS_LOG (DEBUG, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!4");

    pthread_mutex_unlock(mutex);

    return NS_OK;
}

NSCacheElement * NSPopProviderCacheList(NSCacheList * list)
{
    NS_VERIFY_NOT_NULL(list, NULL);

    pthread_mutex_t * mutex = NSGetCacheMutex();
    pthread_mutex_lock(mutex);

    NSCacheElement * head = list->head;
    if (head)
    {
        if (list->tail == head)
        {
            list->tail = NULL;
        }

        list->head = head->next;
        head->next = NULL;
    }

    pthread_mutex_unlock(mutex);
    return head;
}


NSResult NSStorageDestroy(NSCacheList * list)
{
    NS_VERIFY_NOT_NULL(list, NS_ERROR);

    pthread_mutex_t * mutex = NSGetCacheMutex();
    pthread_mutex_lock(mutex);

    NSCacheElement * iter = list->head;
    NSCacheElement * next = NULL;

    NSCacheType type = list->cacheType;

    if (type == NS_CONSUMER_CACHE_MESSAGE)
    {
        while (iter)
        {
            next = (NSCacheElement *) iter->next;

            NSRemoveMessage(((NSStoreMessage *) iter->data)->msg);
            NSOICFree(iter->data);
            NSOICFree(iter);

            iter = next;
        }

        NSOICFree(list);
    }
    else if (type == NS_CONSUMER_CACHE_PROVIDER)
    {
        while (iter)
        {
            next = (NSCacheElement *) iter->next;

            NSRemoveProvider_internal((NSProvider_internal *) iter->data);
            NSOICFree(iter);

            iter = next;
        }

        NSOICFree(list);
    }

    pthread_mutex_unlock(mutex);

    return NS_OK;
}

bool NSConsumerCompareIdCacheData(NSCacheType type, void * data, const char * id)
{
    NS_VERIFY_NOT_NULL(data, false);
    NS_VERIFY_NOT_NULL(id, false);

    if (type == NS_CONSUMER_CACHE_MESSAGE)
    {
        NSMessage * msg = ((NSStoreMessage *) data)->msg;

        char msgId[NS_DEVICE_ID_LENGTH] = {0, };
        snprintf(msgId, NS_DEVICE_ID_LENGTH, "%lld", (long long int)msg->messageId);
        if (!strcmp(msgId, id))
        {
            return true;
        }

        return false;
    }
    else if (type == NS_CONSUMER_CACHE_PROVIDER)
    {
        NSProvider_internal * prov = (NSProvider_internal *) data;
        if (!strcmp(prov->providerId, id))
        {
            return true;
        }

        return false;
    }

    return false;
}
