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

package oic.simulator.clientcontroller.remoteresource;

import java.util.HashMap;
import java.util.Map;

import org.oic.simulator.InvalidArgsException;
import org.oic.simulator.SimulatorResourceAttribute;
import org.oic.simulator.SimulatorResourceModel;

public class ResourceRepresentation {
    private Map<String, AttributeElement> mAttributes = new HashMap<String, AttributeElement>();

    public ResourceRepresentation(SimulatorResourceModel resourceModel) {
        if (resourceModel != null && resourceModel.size() > 0) {
            for (Map.Entry<String, SimulatorResourceAttribute> entry : resourceModel
                    .getAttributes().entrySet())
                mAttributes.put(entry.getKey(), new AttributeElement(this,
                        entry.getValue()));
        }
    }

    public Map<String, AttributeElement> getAttributes() {
        return mAttributes;
    }

    public void update(SimulatorResourceModel resourceModel,
            boolean ramlUploaded) {
        for (Map.Entry<String, SimulatorResourceAttribute> entry : resourceModel
                .getAttributes().entrySet()) {
            AttributeElement attributeElement = mAttributes.get(entry.getKey());
            if (attributeElement != null) {
                if (ramlUploaded)
                    attributeElement.updateForRAMLUpload(entry.getValue());
                else
                    attributeElement.update(entry.getValue());
            } else // Display new attribute in UI
            {
                AttributeElement newAttribute = new AttributeElement(this,
                        entry.getValue());
                mAttributes.put(entry.getKey(), newAttribute);
            }
        }
    }

    public SimulatorResourceModel getModel() {
        if (null == mAttributes || mAttributes.isEmpty()) {
            return null;
        }
        SimulatorResourceModel model = new SimulatorResourceModel();
        for (Map.Entry<String, AttributeElement> entry : mAttributes.entrySet()) {
            AttributeElement attributeElement = mAttributes.get(entry.getKey());
            if (attributeElement != null) {
                try {
                    model.addAttribute(attributeElement
                            .getSimulatorResourceAttribute());
                } catch (InvalidArgsException e) {
                    e.printStackTrace();
                }
            }
        }
        return model;
    }

    public SimulatorResourceModel getSelectedModel() {
        if (null == mAttributes || mAttributes.isEmpty()) {
            return null;
        }
        SimulatorResourceModel model = new SimulatorResourceModel();
        for (Map.Entry<String, AttributeElement> entry : mAttributes.entrySet()) {
            AttributeElement attributeElement = mAttributes.get(entry.getKey());
            if (attributeElement != null && attributeElement.getPostState()) {
                try {
                    model.addAttribute(attributeElement
                            .getSimulatorResourceAttribute());
                } catch (InvalidArgsException e) {
                    e.printStackTrace();
                }
            }
        }
        return model;
    }
}