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

#include "NSConsumerService.h"
#include <cstring>
#include "NSConsumerInterface.h"
#include "NSMessage.h"
#include "NSCommon.h"
#include "NSConstants.h"
#include "oic_string.h"

namespace OIC
{
    namespace Service
    {
        void onNSProviderDiscovered(::NSProvider *provider)
        {
            NS_LOG(DEBUG, "onNSProviderDiscovered - IN");
            NSProvider *nsProvider = new NSProvider(provider);
            if (NSConsumerService::getInstance()->getConsumerConfig().m_discoverCb != NULL)
                NSConsumerService::getInstance()->getConsumerConfig().m_discoverCb(nsProvider);
            delete nsProvider;
            NS_LOG(DEBUG, "onNSProviderDiscovered - OUT");
        }

        void onNSProviderChanged(::NSProvider *provider, ::NSResponse response)
        {
            NS_LOG(DEBUG, "onNSProviderChanged - IN");
            NS_LOG_V(DEBUG, "provider Id : %s",provider->providerId);
            NS_LOG_V(DEBUG, "response : %d",(int)response);
            
            NSProvider *nsProvider = new NSProvider(provider);
            auto changeCallback = NSConsumerService::getInstance()->getConsumerConfig().m_changedCb;
            if(response == NS_ALLOW)
            {
                NSConsumerService::getInstance()->getAcceptedProviders().push_back(nsProvider);
                if (changeCallback != NULL)
                    changeCallback(nsProvider, (NSResponse) response);
            }
            else if(response == NS_DENY)
            {
                NSConsumerService::getInstance()->getAcceptedProviders().remove(nsProvider);
                if (changeCallback != NULL)
                    changeCallback(nsProvider, (NSResponse) response);
                delete nsProvider;
            }
            NS_LOG(DEBUG, "onNSProviderChanged - OUT");
        }

        void onNSMessageReceived(::NSMessage *message)
        {
            NS_LOG(DEBUG, "onNSMessageReceived - IN");
            NSMessage *nsMessage = new NSMessage(message);
            for (auto it : NSConsumerService::getInstance()->getAcceptedProviders())
            {
                if (it->getProviderId() == nsMessage->getProviderId())
                {
                    auto callback = it->getMessageReceivedCb();
                    if (callback != NULL)
                    {
                        callback(nsMessage);
                    }
                    break;
                }
            }
            delete nsMessage;
            NS_LOG(DEBUG, "onNSMessageReceived - OUT");
        }

        void onNSSyncInfoReceived(::NSSyncInfo *syncInfo)
        {
            NS_LOG(DEBUG, "onNSSyncInfoReceived - IN");
            NSSyncInfo *nsSyncInfo = new NSSyncInfo(syncInfo);
            for (auto it : NSConsumerService::getInstance()->getAcceptedProviders())
            {
                if (it->getProviderId() == nsSyncInfo->getProviderId())
                {
                    auto callback = it->getSyncInfoReceivedCb();
                    if (callback != NULL)
                    {
                        callback(nsSyncInfo);
                    }
                    break;
                }
            }
            delete nsSyncInfo;
            NS_LOG(DEBUG, "onNSSyncInfoReceived - OUT");
        }

        NSConsumerService::NSConsumerService()
        {
            m_config.m_discoverCb = NULL;
            m_config.m_changedCb = NULL;
        }

        NSConsumerService::~NSConsumerService()
        {
            for (auto it : getAcceptedProviders())
            {
                delete it;
            }
            getAcceptedProviders().clear();
        }
        NSConsumerService *NSConsumerService::getInstance()
        {
            static  NSConsumerService s_instance;
            return &s_instance;
        }

        void NSConsumerService::Start(NSConsumerService::ConsumerConfig config)
        {
            NS_LOG(DEBUG, "Start - IN");
            m_config = config;
            NSConsumerConfig nsConfig;
            nsConfig.discoverCb = onNSProviderDiscovered;
            nsConfig.changedCb= onNSProviderChanged;
            nsConfig.messageCb = onNSMessageReceived;
            nsConfig.syncInfoCb = onNSSyncInfoReceived;

            NSStartConsumer(nsConfig);
            NS_LOG(DEBUG, "Start - OUT");
            return;
        }

        void NSConsumerService::Stop()
        {
            NS_LOG(DEBUG, "Stop - IN");
            NSStopConsumer();
            NS_LOG(DEBUG, "Stop - OUT");
            return;
        }

        NSResult NSConsumerService::EnableRemoteService(const std::string &serverAddress)
        {
            NS_LOG(DEBUG, "EnableRemoteService - IN");
            NSResult result = NSResult::ERROR;
#ifdef WITH_CLOUD
            result = (NSResult) NSConsumerEnableRemoteService(OICStrdup(serverAddress.c_str()));
#else
            NS_LOG(ERROR, "Remote Services feature is not enabled in the Build");
#endif
            NS_LOG(DEBUG, "EnableRemoteService - OUT");
            return result;
        }

        void NSConsumerService::RescanProvider()
        {
            NS_LOG(DEBUG, "RescanProvider - IN");
            NSRescanProvider();
            NS_LOG(DEBUG, "RescanProvider - OUT");
            return;
        }

        NSProvider *NSConsumerService::getProvider(const std::string &id)
        {
            for (auto it : getAcceptedProviders())
            {
                if (it->getProviderId() == id)
                    return it;
            }
            return NULL;
        }

        NSConsumerService::ConsumerConfig NSConsumerService::getConsumerConfig()
        {
            return m_config;
        }

        std::list<NSProvider *> NSConsumerService::getAcceptedProviders()
        {
            return m_acceptedProviders;
        }
    }
}
