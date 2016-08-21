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
package org.iotivity.service.ns.provider;
import org.iotivity.service.ns.common.*;
/**
  * @class   Consumer
  * @brief   This class provides implementation of Notification Consumer object.
  */
public class Consumer
{
    public native int  nativeAcceptSubscription(Consumer consumer, boolean accepted);

    public String mConsumerId;

    public Consumer(final String consumerId)
    {
        mConsumerId = consumerId;
    }
    public String getConsumerId( )
    {
        return mConsumerId;
    }
    public int AcceptSubscription(Consumer consumer, boolean accepted) throws NSException
    {
        if (consumer != null)
            return nativeAcceptSubscription(consumer, accepted);
        return -1;
    }

}