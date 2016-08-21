package org.iotivity.cloud.ciserver.resources.proxy.account;

import static java.util.concurrent.TimeUnit.SECONDS;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.mock;

import java.util.HashMap;
import java.util.concurrent.CountDownLatch;

import org.iotivity.cloud.base.connector.ConnectorPool;
import org.iotivity.cloud.base.device.CoapDevice;
import org.iotivity.cloud.base.device.IRequestChannel;
import org.iotivity.cloud.base.protocols.IRequest;
import org.iotivity.cloud.base.protocols.IResponse;
import org.iotivity.cloud.base.protocols.MessageBuilder;
import org.iotivity.cloud.base.protocols.coap.CoapRequest;
import org.iotivity.cloud.base.protocols.enums.ContentFormat;
import org.iotivity.cloud.base.protocols.enums.RequestMethod;
import org.iotivity.cloud.ciserver.DeviceServerSystem;
import org.iotivity.cloud.util.Cbor;
import org.junit.Before;
import org.junit.Test;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.stubbing.Answer;

public class AccountSessionTest {
    private String             di                 = "B371C481-38E6-4D47-8320-7688D8A5B58C";
    public static final String SESSION_URI        = "/.well-known/ocf/account/session";
    private CoapDevice         mockDevice         = mock(CoapDevice.class);
    IResponse                  res                = null;
    IRequest                   req                = null;
    ConnectorPool              connectorPool      = null;
    DeviceServerSystem         deviceServerSystem = new DeviceServerSystem();
    final CountDownLatch       latch              = new CountDownLatch(1);
    @Mock
    IRequestChannel            requestChannel;

    @InjectMocks
    AccountSession             acSessionHandler   = new AccountSession();

    @Before
    public void setUp() throws Exception {
        res = null;
        req = null;
        Mockito.doReturn(di).when(mockDevice).getDeviceId();
        Mockito.doReturn("mockDeviceUser").when(mockDevice).getUserId();
        Mockito.doReturn("1689c70ffa245effc563017fee36d250").when(mockDevice)
                .getAccessToken();
        MockitoAnnotations.initMocks(this);
        deviceServerSystem.addResource(acSessionHandler);
        Mockito.doAnswer(new Answer<Object>() {
            @Override
            public CoapRequest answer(InvocationOnMock invocation)
                    throws Throwable {
                Object[] args = invocation.getArguments();
                CoapRequest request = (CoapRequest) args[0];
                req = request;
                latch.countDown();
                System.out.println(
                        "\t----------payload : " + request.getPayloadString());
                System.out.println(
                        "\t----------uripath : " + request.getUriPath());
                System.out.println(
                        "\t---------uriquery : " + request.getUriQuery());
                return null;
            }
        }).when(requestChannel).sendRequest(Mockito.any(IRequest.class),
                Mockito.any(CoapDevice.class));
    }

    @Test
    public void testAccountSignInOnRequestReceived() throws Exception {
        System.out.println(
                "\t--------------OnRequestReceived Sign In Test------------");
        IRequest request = makeSignInRequest();
        deviceServerSystem.onRequestReceived(mockDevice, request);
        assertTrue(latch.await(1L, SECONDS));
        assertEquals(request, request);
    }

    @Test
    public void testAccountResourceOnRequestReceived() throws Exception {
        System.out.println(
                "\t--------------OnRequestReceived Sign Out Test------------");
        // sign up request from the client
        IRequest request = makeSignOutRequest();
        deviceServerSystem.onRequestReceived(mockDevice, request);
        // assertion : request msg to the AS is identical to the request msg
        // from the client
        assertTrue(latch.await(1L, SECONDS));
        assertTrue(hashmapCheck(req, "uid"));
        assertTrue(hashmapCheck(req, "di"));
        assertTrue(hashmapCheck(req, "accesstoken"));
        assertTrue(hashmapCheck(req, "login"));
    }

    private IRequest makeSignInRequest() {
        Cbor<HashMap<String, Object>> cbor = new Cbor<HashMap<String, Object>>();
        IRequest request = null;
        HashMap<String, Object> payloadData = new HashMap<>();
        payloadData.put("uid", "u0001");
        payloadData.put("di", di);
        payloadData.put("accesstoken", "1689c70ffa245effc563017fee36d250");
        payloadData.put("login", true);
        request = MessageBuilder.createRequest(RequestMethod.POST, SESSION_URI,
                null, ContentFormat.APPLICATION_CBOR,
                cbor.encodingPayloadToCbor(payloadData));
        return request;
    }

    private IRequest makeSignOutRequest() {
        Cbor<HashMap<String, Object>> cbor = new Cbor<HashMap<String, Object>>();
        IRequest request = null;
        HashMap<String, Object> payloadData = new HashMap<>();
        payloadData.put("login", false);
        request = MessageBuilder.createRequest(RequestMethod.POST, SESSION_URI,
                null, ContentFormat.APPLICATION_CBOR,
                cbor.encodingPayloadToCbor(payloadData));
        return request;
    }

    private boolean hashmapCheck(IRequest request, String propertyName) {
        Cbor<HashMap<String, Object>> mCbor = new Cbor<>();
        HashMap<String, Object> payloadData = mCbor
                .parsePayloadFromCbor(request.getPayload(), HashMap.class);
        if (payloadData.get(propertyName) != null)
            return true;
        else
            return false;
    }
}
