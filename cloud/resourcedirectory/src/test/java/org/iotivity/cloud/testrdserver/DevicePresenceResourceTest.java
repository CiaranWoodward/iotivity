/*
 * //******************************************************************
 * //
 * // Copyright 2016 Samsung Electronics All Rights Reserved.
 * //
 * //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * //
 * // Licensed under the Apache License, Version 2.0 (the "License");
 * // you may not use this file except in compliance with the License.
 * // You may obtain a copy of the License at
 * //
 * //      http://www.apache.org/licenses/LICENSE-2.0
 * //
 * // Unless required by applicable law or agreed to in writing, software
 * // distributed under the License is distributed on an "AS IS" BASIS,
 * // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * // See the License for the specific language governing permissions and
 * // limitations under the License.
 * //
 * //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */
package org.iotivity.cloud.testrdserver;

import static java.util.concurrent.TimeUnit.SECONDS;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertNull;
import static org.mockito.Mockito.mock;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.concurrent.CountDownLatch;
import org.iotivity.cloud.base.device.CoapDevice;
import org.iotivity.cloud.base.protocols.IRequest;
import org.iotivity.cloud.base.protocols.IResponse;
import org.iotivity.cloud.base.protocols.MessageBuilder;
import org.iotivity.cloud.base.protocols.coap.CoapRequest;
import org.iotivity.cloud.base.protocols.coap.CoapResponse;
import org.iotivity.cloud.base.protocols.enums.ContentFormat;
import org.iotivity.cloud.base.protocols.enums.Observe;
import org.iotivity.cloud.base.protocols.enums.RequestMethod;
import org.iotivity.cloud.base.protocols.enums.ResponseStatus;
import org.iotivity.cloud.rdserver.Constants;
import org.iotivity.cloud.rdserver.resources.directory.rd.ResourceDirectoryResource;
import org.iotivity.cloud.rdserver.resources.presence.device.DevicePresenceResource;
import org.iotivity.cloud.util.Cbor;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.stubbing.Answer;

public class DevicePresenceResourceTest {
    private Cbor<ArrayList<Object>>   mCbor                      = new Cbor<>();
    private ResourceDirectoryResource mRDResource                = null;
    private DevicePresenceResource    mockDevicePresenceResource = null;
    private CoapDevice                mockDevice                 = null;
    CountDownLatch                    latch                      = null;
    IResponse                         res;

    @Before
    public void setUp() throws Exception {
        res = null;
        mockDevice = mock(CoapDevice.class);
        latch = new CountDownLatch(1);
        mRDResource = new ResourceDirectoryResource();
        mockDevicePresenceResource = new DevicePresenceResource();
        // callback mock
        Mockito.doAnswer(new Answer<Object>() {
            @Override
            public CoapResponse answer(InvocationOnMock invocation)
                    throws Throwable {
                CoapResponse resp = (CoapResponse) invocation.getArguments()[0];
                latch.countDown();
                res = resp;
                return resp;
            }
        }).when(mockDevice).sendResponse(Mockito.anyObject());
    }

    @After
    public void tearDown() throws Exception {
        RDServerTestUtils.resetRDDatabase();
    }

    private IRequest makePresenceGetRequest(Observe obs) {
        String query = "di=" + RDServerTestUtils.DI;
        IRequest request = null;
        if (obs.compareTo(Observe.SUBSCRIBE) == 0) {
            request = MessageBuilder.createRequest(RequestMethod.GET,
                    RDServerTestUtils.DEVICE_PRS_REQ_URI, query);
        } else if (obs.compareTo(Observe.UNSUBSCRIBE) == 0) {
            ArrayList<String> devices = new ArrayList<>();
            devices.add(RDServerTestUtils.DI);
            HashMap<String, ArrayList<String>> payload = new HashMap<>();
            payload.put(Constants.DEVICE_LIST_KEY, devices);
            Cbor<HashMap<String, Object>> cbor = new Cbor<>();
            request = MessageBuilder.createRequest(RequestMethod.GET,
                    RDServerTestUtils.DEVICE_PRS_REQ_URI, query,
                    ContentFormat.APPLICATION_CBOR,
                    cbor.encodingPayloadToCbor(payload));
        }
        ((CoapRequest) request).setObserve(obs);
        return request;
    }

    @Test
    public void testHandleGetSubscribeRequest_notExistVaule() throws Exception {
        System.out
                .println("\t------testHandleGetSubscribeRequest_notExistVaule");
        IRequest request = makePresenceGetRequest(Observe.SUBSCRIBE);
        mockDevicePresenceResource.onDefaultRequestReceived(mockDevice,
                request);
        // assertion: if the response status is "CONTENT"
        assertTrue(latch.await(2L, SECONDS));
        assertTrue(methodCheck(res, ResponseStatus.CONTENT));
        // assertion : if the payload has "di" and "state"
        assertTrue(arrayHashmapCheck(res, "di"));
        assertTrue(arrayHashmapCheck(res, "state"));
        ArrayList<Object> payloadData = mCbor
                .parsePayloadFromCbor(res.getPayload(), ArrayList.class);
        HashMap<String, Object> mapData = (HashMap<String, Object>) payloadData
                .get(0);
        assertNull(mapData.get("state"));
    }

    @Test
    public void testHandleGetSubscribeRequest() throws Exception {
        System.out.println("\t------testHandleGetSubscribeRequest");
        mRDResource.onDefaultRequestReceived(mockDevice,
                RDServerTestUtils.makePublishRequest());
        IRequest request = makePresenceGetRequest(Observe.SUBSCRIBE);
        mockDevicePresenceResource.onDefaultRequestReceived(mockDevice,
                request);
        // assertion: if the response status is "CONTENT"
        assertTrue(latch.await(2L, SECONDS));
        assertTrue(methodCheck(res, ResponseStatus.CONTENT));
        // assertion : if the payload has "di" and "state"
        assertTrue(arrayHashmapCheck(res, "di"));
        assertTrue(arrayHashmapCheck(res, "state"));
        Cbor<ArrayList<Object>> mCbor = new Cbor<>();
        ArrayList<Object> payloadData = mCbor
                .parsePayloadFromCbor(res.getPayload(), ArrayList.class);
        HashMap<String, Object> mapData = (HashMap<String, Object>) payloadData
                .get(0);
        assertNull(mapData.get("state"));
    }

    @Test
    public void testHandleGetUnsubscribeRequest() throws Exception {
        System.out.println("\t------testHandleGetUnsubscribeRequest");
        IRequest request = makePresenceGetRequest(Observe.UNSUBSCRIBE);
        mRDResource.onDefaultRequestReceived(mockDevice,
                RDServerTestUtils.makePublishRequest());
        mockDevicePresenceResource.onDefaultRequestReceived(mockDevice,
                request);
        // assertion: if the response status is "CONTENT"
        assertTrue(latch.await(2L, SECONDS));
        assertTrue(methodCheck(res, ResponseStatus.CONTENT));
        // assertion : if the payload has "di" and "state"
        assertTrue(arrayHashmapCheck(res, "di"));
        assertTrue(arrayHashmapCheck(res, "state"));
        Cbor<ArrayList<Object>> mCbor = new Cbor<>();
        ArrayList<Object> payloadData = mCbor
                .parsePayloadFromCbor(res.getPayload(), ArrayList.class);
        HashMap<String, Object> mapData = (HashMap<String, Object>) payloadData
                .get(0);
        assertNull(mapData.get("state"));
    }

    @Test
    public void testHandlePostRequest_presenceOn() throws Exception {
        System.out.println("\t------testHandlePostRequest_presenceOn");
        // POST device presence on
        HashMap<String, Object> payload = new HashMap<>();
        Cbor<HashMap<String, Object>> cbor = new Cbor<>();
        payload.put(Constants.DEVICE_ID, RDServerTestUtils.DI);
        payload.put(Constants.PRESENCE_STATE, "on");
        IRequest request = MessageBuilder.createRequest(RequestMethod.POST,
                RDServerTestUtils.DEVICE_PRS_REQ_URI, null,
                ContentFormat.APPLICATION_CBOR,
                cbor.encodingPayloadToCbor(payload));
        mockDevicePresenceResource.onDefaultRequestReceived(mockDevice,
                request);
        // subscribe request (specific device)
        IRequest subRequest = makePresenceGetRequest(Observe.SUBSCRIBE);
        mockDevicePresenceResource.onDefaultRequestReceived(mockDevice,
                subRequest);
        // assertion: if the response status is "CONTENT"
        assertTrue(latch.await(2L, SECONDS));
        assertTrue(methodCheck(res, ResponseStatus.CONTENT));
        // assertion: if the device status is "on"
        assertTrue(arrayHashmapCheck(res, "di"));
        assertTrue(arrayHashmapCheck(res, "state"));
        Cbor<ArrayList<Object>> mCbor = new Cbor<>();
        ArrayList<Object> payloadData = mCbor
                .parsePayloadFromCbor(res.getPayload(), ArrayList.class);
        HashMap<String, Object> mapData = (HashMap<String, Object>) payloadData
                .get(0);
        assertTrue(mapData.get("state").equals("on"));
    }

    @Test
    public void testHandlePostRequest_presenceOff() throws Exception {
        System.out.println("\t------testHandlePostRequest_presenceOff");
        CoapDevice observerDevice = mock(CoapDevice.class);
        CountDownLatch observerLatch = new CountDownLatch(2);
        // callback mock for observer Device
        Mockito.doAnswer(new Answer<Object>() {
            @Override
            public CoapResponse answer(InvocationOnMock invocation)
                    throws Throwable {
                CoapResponse resp = (CoapResponse) invocation.getArguments()[0];
                observerLatch.countDown();
                // assertion for observer device (subscribe response)
                if (observerLatch.getCount() == 1) {
                    assertTrue(methodCheck(resp, ResponseStatus.CONTENT));
                }
                // assertion for observer device (prs off response)
                if (observerLatch.getCount() == 0) {
                    assertTrue(methodCheck(resp, ResponseStatus.CONTENT));
                    Cbor<HashMap<String, Object>> mCbor = new Cbor<>();
                    HashMap<String, Object> payloadData = mCbor
                            .parsePayloadFromCbor(resp.getPayload(),
                                    HashMap.class);
                    assertTrue(payloadData.get("state").equals("off"));
                }

                return null;
            }

        }).when(observerDevice).sendResponse(Mockito.anyObject());
        // subscribe request (specific device)
        IRequest subRequest = makePresenceGetRequest(Observe.SUBSCRIBE);
        mockDevicePresenceResource.onDefaultRequestReceived(observerDevice,
                subRequest);
        // POST device presence off
        HashMap<String, Object> payload = new HashMap<>();
        Cbor<HashMap<String, Object>> cbor = new Cbor<>();
        payload.put(Constants.DEVICE_ID, RDServerTestUtils.DI);
        payload.put(Constants.PRESENCE_STATE, "off");
        IRequest request = MessageBuilder.createRequest(RequestMethod.POST,
                RDServerTestUtils.DEVICE_PRS_REQ_URI, null,
                ContentFormat.APPLICATION_CBOR,
                cbor.encodingPayloadToCbor(payload));
        mockDevicePresenceResource.onDefaultRequestReceived(mockDevice,
                request);
        // assertion for resource server device : responseStatus is "CREATED"
        assertTrue(latch.await(2L, SECONDS));
        assertTrue(observerLatch.await(2L, SECONDS));
        assertTrue(methodCheck(res, ResponseStatus.CREATED));
    }

    private boolean arrayHashmapCheck(IResponse response, String propertyName) {
        Cbor<ArrayList<Object>> mCbor = new Cbor<>();
        ArrayList<Object> payloadData = mCbor
                .parsePayloadFromCbor(response.getPayload(), ArrayList.class);
        HashMap<String, Object> mapData = (HashMap<String, Object>) payloadData
                .get(0);
        if (mapData.containsKey(propertyName))
            return true;
        else
            return false;
    }

    private boolean methodCheck(IResponse response,
            ResponseStatus responseStatus) {
        if (responseStatus == response.getStatus())
            return true;
        else
            return false;
    }

}