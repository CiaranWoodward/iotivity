/*
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package oic.simulator.clientcontroller.utils;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.oic.simulator.SimulatorException;

/**
 * This class has common utility methods.
 */
public class Utility {
    public static List<String> convertSetToList(Set<String> typeSet) {
        if (null == typeSet) {
            return null;
        }
        List<String> list = new ArrayList<String>();
        Iterator<String> typeItr = typeSet.iterator();
        while (typeItr.hasNext()) {
            list.add(typeItr.next());
        }
        return list;
    }

    public static String getObservableInString(boolean observable) {
        if (observable) {
            return Constants.YES;
        } else {
            return Constants.NO;
        }
    }

    public static String[] convertListToString(List<String> valueList) {
        String[] strArr;
        if (null != valueList && valueList.size() > 0) {
            strArr = valueList.toArray(new String[1]);
        } else {
            strArr = new String[1];
        }
        return strArr;
    }

    public static Set<String> splitStringByComma(String text) {
        Set<String> tokenSet = null;
        if (null != text) {
            String[] token = text.split(",");
            if (null != token) {
                tokenSet = new HashSet<String>();
                for (String tok : token) {
                    tok = tok.trim();
                    if (tok.length() > 0) {
                        tokenSet.add(tok);
                    }
                }
            }
        }
        return tokenSet;
    }

    public static String getSimulatorErrorString(Exception e, String info) {
        if (null == e) {
            return null;
        }
        String detail;
        if (e instanceof SimulatorException) {
            SimulatorException simEx = (SimulatorException) e;
            detail = simEx.message() + "\n";
            detail += "Exception Type: " + simEx.getClass().getSimpleName()
                    + "\n";
            detail += "Error code: " + simEx.code().toString();
        } else {
            detail = info + "\n";
            detail += "Exception Type: " + e.getClass().getSimpleName() + "\n";
            detail += "Message: " + e.getMessage();
        }
        return detail;
    }
}