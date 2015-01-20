//******************************************************************
//
// Copyright 2014 Samsung Electronics All Rights Reserved.
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

#ifndef __OC_ACTIONSET__
#define __OC_ACTIONSET__

#include <string>
#include <vector>

using namespace std;

namespace OIC{
	class Capability
    {
    public:
        std::string capability;
        std::string status;
    };

    class Action
    {
    public:
        Action() :
                target("")
        {
        }
        ~Action()
        {
            listOfCapability.clear();
        }
        std::string target;

        std::vector< Capability* > listOfCapability;
    };

    class ActionSet
    {
    public:
        ActionSet() :
                actionsetName("")
        {
        }
        ~ActionSet()
        {
            listOfAction.clear();
        }
        std::string actionsetName;

        std::vector< Action* > listOfAction;
    };
}
#endif 
