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
package org.iotivity.cloud.base.device;

import java.util.Date;

import org.iotivity.cloud.base.connector.CoapClient;
import org.iotivity.cloud.base.protocols.IResponse;
import org.iotivity.cloud.util.Log;

import io.netty.channel.ChannelHandlerContext;

public class CoapDevice extends Device {
    private CoapClient       mCoapClient    = null;
    private String           mUserId        = null;
    private String           mDeviceId      = null;
    private String           mAccessToken   = null;
    private Date             mIssuedTime    = null;
    private int              mExpiredPolicy = 0;

    private static final int INFINITE_TIME  = -1;

    public CoapDevice(ChannelHandlerContext ctx) {
        super(ctx);
    }

    public void updateDevice(String did, String uid, String accesstoken) {
        mDeviceId = did;
        mUserId = uid;
        mAccessToken = accesstoken;
    }

    @Override
    public String getDeviceId() {
        return mDeviceId;
    }

    @Override
    public String getUserId() {
        return mUserId;
    }

    @Override
    public String getAccessToken() {
        return mAccessToken;
    }

    public Date getIssuedTime() {
        return mIssuedTime;
    }

    public int getExpiredPolicy() {
        return mExpiredPolicy;
    }

    public void setExpiredPolicy(int expiredPolicy) {
        mIssuedTime = new Date();
        this.mExpiredPolicy = expiredPolicy;
    }

    public void setUerId(String userId) {
        this.mUserId = userId;
    }

    public void setAccessToken(String accessToken) {
        this.mAccessToken = accessToken;
    }

    // This is called by cloud resource model
    @Override
    public void sendResponse(IResponse response) {
        // This message must converted to CoapResponse
        ctx.channel().writeAndFlush(response);
    }

    public IRequestChannel getRequestChannel() {
        if (mCoapClient == null) {
            mCoapClient = new CoapClient(ctx.channel());
        }

        return mCoapClient;
    }

    public boolean isExpiredTime() {

        if (mExpiredPolicy == INFINITE_TIME) {
            return false;
        }

        Date currentTime = new Date();
        long difference = currentTime.getTime() - mIssuedTime.getTime();
        long remainTime = mExpiredPolicy - difference / 1000;

        if (remainTime < 0) {

            Log.w("accessToken is expired..");
            return true;
        }

        return false;
    }

    @Override
    public void onConnected() {
    }

    @Override
    public void onDisconnected() {
    }

}
