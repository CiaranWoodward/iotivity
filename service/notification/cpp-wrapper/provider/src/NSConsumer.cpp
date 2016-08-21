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

#include "NSConsumer.h"
#include <cstring>
#include "NSProviderInterface.h"
#include "NSConstants.h"
#include "NSUtils.h"
#include "oic_string.h"

namespace OIC
{
    namespace Service
    {
        ::NSConsumer *NSConsumer::getNSConsumer()
        {
            ::NSConsumer *nsCon = new ::NSConsumer;
            OICStrcpy(nsCon->consumerId, NS_UTILS_UUID_STRING_SIZE, m_consumerId.c_str());
            return nsCon;
        }

        NSConsumer::NSConsumer(::NSConsumer *consumer)
        {
            if (consumer != nullptr)
            {
                m_consumerId.assign(consumer->consumerId, NS_UTILS_UUID_STRING_SIZE);
            }
        }

        std::string NSConsumer::getConsumerId() const
        {
            return m_consumerId;
        }

        int NSConsumer::acceptSubscription(NSConsumer *consumer, bool accepted)
        {
            NS_LOG(DEBUG, "acceptSubscription - IN");
            if (consumer != nullptr)
                return NSAcceptSubscription(consumer->getNSConsumer(), accepted);
            NS_LOG(DEBUG, "acceptSubscription - OUT");
            return NS_ERROR;
        }
    }
}
