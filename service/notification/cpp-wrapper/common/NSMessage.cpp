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

#include "NSMessage.h"
#include "string.h"
#include "NSUtils.h"
#include <cstdint>

namespace OIC
{
    namespace Service
    {
        NSMessage::NSMessage(::NSMessage *msg)
        {
            m_messageId = 0;
            m_type = NSMessage::NSMessageType::NS_MESSAGE_ALERT;
            m_ttl = 0;
            m_mediaContents = new NSMediaContents();
            
            if (msg != nullptr)
            {
                m_messageId = msg->messageId;
                m_providerId.assign(msg->providerId, NS_UTILS_UUID_STRING_SIZE);

                m_type = (NSMessageType)msg->type;

                if ((msg->dateTime != nullptr) && strlen(msg->dateTime))
                    m_time.assign(msg->dateTime, strlen(msg->dateTime));

                m_ttl =  msg->ttl;

                if ((msg->title != nullptr) && strlen(msg->title))
                    m_title.assign(msg->title, strlen(msg->title));

                if ((msg->contentText != nullptr) && strlen(msg->contentText))
                    m_contentText.assign(msg->contentText, strlen(msg->contentText));

                if ((msg->sourceName != nullptr) && strlen(msg->sourceName))
                    m_sourceName.assign(msg->sourceName, strlen(msg->sourceName));

                if (msg->mediaContents != nullptr)
                    if ((msg->mediaContents->iconImage != nullptr) && strlen(msg->mediaContents->iconImage))
                        m_mediaContents->setIconImage(msg->mediaContents->iconImage);

            }
        }

        NSMessage::~NSMessage()
        {
            if(m_mediaContents != nullptr)
                delete m_mediaContents;
        }

        uint64_t NSMessage::getMessageId() const
        {
            return m_messageId;
        }

        std::string NSMessage::getProviderId() const
        {
            return m_providerId;
        }

        NSMessage::NSMessageType NSMessage::getType() const
        {
            return m_type;
        }

        std::string NSMessage::getTime() const
        {
            return m_time;
        }

        uint64_t NSMessage::getTTL() const
        {
            return m_ttl;
        }

        std::string NSMessage::getTitle() const
        {
            return m_title;
        }

        std::string NSMessage::getContentText() const
        {
            return m_contentText;
        }

        std::string NSMessage::getSourceName() const
        {
            return m_sourceName;
        }

        NSMediaContents *NSMessage::getMediaContents() const
        {
            return m_mediaContents;
        }

        void NSMessage::setType(const NSMessageType &type)
        {
            m_type = type;
        }

        void NSMessage::setTime(const std::string &time)
        {
            m_time = time;
        }

        void NSMessage::setTTL(const uint64_t &ttl)
        {
            m_ttl = ttl;
        }

        void NSMessage::setTitle(const std::string &title)
        {
            m_title = title;
        }

        void NSMessage::setContentText(const std::string &contextText)
        {
            m_contentText = contextText;
        }

        void NSMessage::setSourceName(const std::string &sourceName)
        {
            m_sourceName = sourceName;
        }

        void NSMessage::setMediaContents(NSMediaContents *mediaContents)
        {
            m_mediaContents = mediaContents;
        }
    }
}
