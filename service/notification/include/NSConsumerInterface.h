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

/**
 * @file
 *
 * This file provides APIs of Notification Service for Consumer.
 */

#ifndef _NS_CONSUMER_INTERFACE_H_
#define _NS_CONSUMER_INTERFACE_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include "NSCommon.h"

/**
 * Invoked when the discovered provider is received
 * @param[in] provider  Provider who has the notification resource
 */
typedef void (* NSProviderDiscoveredCallback)(NSProvider *);

/**
 * Invoked when the provider state is changed
 * @param[in] provider  Provider which has the notification resource
 * @param[in] response  Response which has the provider state
 */
typedef void (* NSProviderChangedCallback)(NSProvider *, NSResponse);

/**
 * Invoked when the notification message from provider is received
 * synchronization
 * @param[in] message  Notification message
 */
typedef void (* NSMessageReceivedCallback)(NSMessage *);

/**
 * Invoked when the synchronization data which has notification message
 * read/delete event from provider/consumer is received
 * synchronization
 * @param[in] syncInfo  Synchronization information of the notification message
 */
typedef void (* NSSyncInfoReceivedCallback)(NSSyncInfo *);

typedef struct
{
    NSProviderDiscoveredCallback discoverCb;
    NSProviderChangedCallback changedCb;
    NSMessageReceivedCallback messageCb;
    NSSyncInfoReceivedCallback syncInfoCb;

} NSConsumerConfig;

/**
 * Initialize notification service for consumer
 * @param[in]  config  NSConsumerconfig structure of callback functions
 * @return ::NS_OK or result code of NSResult
 */
NSResult NSStartConsumer(NSConsumerConfig config);

/**
 * Terminate notification service for consumer
 * @return ::NS_OK or result code of NSResult
 */
NSResult NSStopConsumer();

/**
 * Request to discover to remote address as parameter.
 * @param[in]  server address combined with IP address and port number using delimiter :
 * @return ::NS_OK or result code of NSResult
 */
NSResult NSConsumerEnableRemoteService(char *serverAddress);

/**
 * Request discovery manually
 * @return ::NS_OK or result code of NSResult
 */
NSResult NSRescanProvider();

/**
 * Request to subscribe notification message resource of provider
 * @param[in]  provider  Provider who send the notification message
 * @return ::NS_OK or result code of NSResult
 */
NSResult NSSubscribe(NSProvider *provider);

/**
 * Request to unsubscribe in order not to receive notification message from provider
 * @param[in]  provider  Provider who send the notification message
 * @return ::NS_OK or result code of NSResult
 */
NSResult NSUnsubscribe(NSProvider *provider);

/**
 * Send sync type to provider in order to synchronize notification status with other consumers
 * when consumer consumes the notification such as READ, DELETE
 * @param[in]  providerId  Provider id of the Notification message
 * @param[in]  messageId  Notification message id to synchronize the status
 * @param[in]  type  changed notification status from NSSyncType
 * @return ::NS_OK or result code of NSResult
 */
NSResult NSConsumerSendSyncInfo(
        const char * providerId, uint64_t messageId, NSSyncType type);

/**
 * Request NSProvider that is matched by provider id
 * @param[in]  providerId  the id of provider that user wants to get
 * @return NSProvider
 */
NSProvider * NSConsumerGetProvider(const char * providerId);

/**
 * Request NSMessage that is matched by message id
 * @param[in]  messageId  the id of message that user wants to get
 * @return NSMessage
 */
NSMessage * NSConsumerGetMessage(uint64_t messageId);

/**
 * Request NSTopic list that is subscribed from provider
 * @param[in]  provider  the provider that user wants to get
 * @return NSResult
 */
NSResult NSConsumerGetInterestTopics(NSProvider * provider);

/**
 * Select Topic list that is wanted to subscribe from provider
 * @param[in]  provider  the provider that user wants to set
 * @return NSResult
 */
NSResult NSConsumerSelectInterestTopics(NSProvider * provider);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _NS_CONSUMER_INTERFACE_H_
