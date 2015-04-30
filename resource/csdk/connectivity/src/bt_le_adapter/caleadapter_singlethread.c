/******************************************************************
*
* Copyright 2014 Samsung Electronics All Rights Reserved.
*
*
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
*
******************************************************************/
#include "caleadapter_singlethread.h"

#include "caleinterface_singlethread.h"
#include "cableserver.h"
#include "logger.h"
#include "caadapterutils.h"
#include "camsgparser.h"

#define TAG "LAD"

/**
 * @def MAX_EVENT_COUNT
 * @brief Maximum number of tries to get the event on BLE Shield address.
 */
#define MAX_EVENT_COUNT 20

static CANetworkChangeCallback g_networkCallback = NULL;
static bool g_serverRunning = false;
static CANetworkPacketReceivedCallback g_respCallback;
static char *g_coapBuffer = NULL;
static uint32_t g_dataLen = 0;
static uint32_t g_packetDataLen = 0;

/**
 * @brief API to register for BLE network notification.
 * @param net_callback - network notification callback.
 * @return - Error Code
 */
CAResult_t LERegisterNetworkNotifications(CANetworkChangeCallback netCallback);

/**
 * @brief API to send received data to upper layer.
 * @param[in] data - data received from BLE characteristics.
 * @param[in] dataLen - received data Length.
 * @param[in] senderAdrs - sender Address.
 * @param[in] senderPort - sender port.
 * @return - Error Code
 */
void CANotifyCallback(const void *data, int32_t dataLen, const char *senderAdrs,
                      int32_t senderPort);

/**
 * @brief API to read the data from characteristics and invoke notifyCallback.
 * @return - void
 */
void CACheckData();

/**
 * @brief API to Send the data.
 * @return - Number of bytes sent. -1 on error.
 */
int32_t CASendLEData(const void *data, uint32_t dataLen);

CAResult_t CAInitializeLE(CARegisterConnectivityCallback registerCallback,
                          CANetworkPacketReceivedCallback reqRespCallback,
                          CANetworkChangeCallback netCallback)
{
    OIC_LOG(DEBUG, TAG, "IN");
    if (NULL == registerCallback || NULL == reqRespCallback || NULL == netCallback)
    {
        OIC_LOG(ERROR, TAG, "i/p null");
        return CA_STATUS_INVALID_PARAM;
    }

    CAResult_t result = CALEInitializeNetworkMonitor();
    if (CA_STATUS_OK != result)
    {
        OIC_LOG_V(ERROR, TAG, "n/w init fail: %d", result);
        return CA_STATUS_FAILED;
    }

    g_respCallback = reqRespCallback;
    LERegisterNetworkNotifications(netCallback);
    CAConnectivityHandler_t connHandler;
    connHandler.startAdapter = CAStartLE;
    connHandler.startListenServer = CAStartLEListeningServer;
    connHandler.startDiscoveryServer = CAStartLEDiscoveryServer;
    connHandler.sendData = CASendLEUnicastData;
    connHandler.sendDataToAll = CASendLEMulticastData;
    connHandler.GetnetInfo = CAGetLEInterfaceInformation;
    connHandler.readData = CAReadLEData;
    connHandler.stopAdapter = CAStopLE;
    connHandler.terminate = CATerminateLE;
    registerCallback(connHandler, CA_LE);
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAStartLE()
{
    OIC_LOG(DEBUG, TAG, "IN");
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAStartLEListeningServer()
{
    OIC_LOG(DEBUG, TAG, "IN");
    CAResult_t result = CAInitializeBle();
    if (CA_STATUS_OK != result)
    {
        OIC_LOG_V(ERROR, TAG, "ble init fail: %d", result);
        return CA_STATUS_FAILED;
    }
    uint32_t iter = 0;
    /**
     * Below for loop is to process the BLE Events received from BLE Shield.
     * BLE Events includes BLE Shield Address Added as a patch to RBL Library.
     */
    for (iter = 0; iter < MAX_EVENT_COUNT; iter++)
    {
        CACheckData();
    }

    g_serverRunning = true;
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAStartLEDiscoveryServer()
{
    OIC_LOG(DEBUG, TAG, "IN");
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAStartLENotifyServer()
{
    OIC_LOG(DEBUG, TAG, "IN");
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

uint32_t CASendLENotification(const CARemoteEndpoint_t *endpoint, const void *data,
                              uint32_t dataLen)
{
    OIC_LOG(DEBUG, TAG, "IN");
    OIC_LOG(DEBUG, TAG, "OUT");
    return 1;
}

int32_t CASendLEUnicastData(const CARemoteEndpoint_t *remoteEndpoint, const void *data,
                            uint32_t dataLen)
{
    OIC_LOG(DEBUG, TAG, "IN");
    if (NULL == remoteEndpoint || NULL == data || dataLen == 0)
    {
        OIC_LOG(ERROR, TAG, "i/p null");
        return -1;
    }
    OIC_LOG(DEBUG, TAG, "OUT");
    return CASendLEData(data, dataLen);
}

int32_t CASendLEMulticastData(const void *data, uint32_t dataLen)
{
    OIC_LOG(DEBUG, TAG, "IN");
    if (NULL == data || 0 == dataLen)
    {
        OIC_LOG(ERROR, TAG, "i/p null");
        return -1;
    }

    OIC_LOG(DEBUG, TAG, "OUT");
    return CASendLEData(data, dataLen);
}

CAResult_t CAGetLEInterfaceInformation(CALocalConnectivity_t **info, uint32_t *size)
{
    OIC_LOG(DEBUG, TAG, "IN");

    if (NULL == info || NULL == size)
    {
        OIC_LOG(ERROR, TAG, "i/p null");
        return CA_STATUS_INVALID_PARAM;
    }

    char *leAddress = NULL;
    CAGetLEAddress(&leAddress);
    OIC_LOG_V(DEBUG, TAG, "leAddress = %s", leAddress);

    /**
     * Create local endpoint using util function
     */
    (*info) = CAAdapterCreateLocalEndpoint(CA_LE, leAddress);
    if (NULL == (*info))
    {
        OIC_LOG(ERROR, TAG, "malloc fail");
        return CA_MEMORY_ALLOC_FAILED;
    }

    (*size) = 1;
    if (*leAddress)
    {
        OICFree(leAddress);
    }
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAReadLEData()
{
    if (true == g_serverRunning)
    {
        CACheckData();
    }
    return CA_STATUS_OK;
}

CAResult_t CAStopLE()
{
    OIC_LOG(DEBUG, TAG, "IN");
    CAStopBleGattServer();
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

void CATerminateLE()
{
    OIC_LOG(DEBUG, TAG, "IN");
    g_respCallback = NULL;
    LERegisterNetworkNotifications(NULL);
    CAResult_t result = CATerminateBle();
    if (CA_STATUS_OK != result)
    {
        OIC_LOG(ERROR, TAG, "ble terminate fail");
        return;
    }

    CALETerminateNetworkMonitor();
    g_serverRunning = false;
    OIC_LOG(DEBUG, TAG, "OUT");
    return;
}

CAResult_t LERegisterNetworkNotifications(CANetworkChangeCallback netCallback)
{
    OIC_LOG(DEBUG, TAG, "IN");
    g_networkCallback = netCallback;
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAStartBleGattServer()
{
    OIC_LOG(DEBUG, TAG, "IN");
    // Done at time of setup i.e. in initializeBle api
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAStopBleGattServer()
{
    OIC_LOG(DEBUG, TAG, "IN");
    // There is no server running to stop.
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

void CANotifyCallback(const void *data, int32_t dataLen, const char *senderAdrs, int32_t senderPort)
{
    OIC_LOG(DEBUG, TAG, "IN");
    if (g_respCallback)
    {

        /* Cannot get Address as of now */
        CARemoteEndpoint_t endPoint;
        endPoint.resourceUri = "";     // will be filled by upper layer
        endPoint.connectivityType = CA_LE;

        g_respCallback(&endPoint, data, dataLen);
    }
    OIC_LOG(DEBUG, TAG, "OUT");
}

void CACheckData()
{
    CABleDoEvents();

    if (CAIsBleDataAvailable())
    {
        // Allocate Memory for COAP Buffer and do ParseHeader
        if (NULL == g_coapBuffer)
        {
            OIC_LOG(DEBUG, TAG, "IN");
            char headerArray[CA_HEADER_LENGTH] = "";
            while (CAIsBleDataAvailable() && g_dataLen < CA_HEADER_LENGTH)
            {
                headerArray[g_dataLen++] = CAReadBleData();
            }

            g_packetDataLen = CAParseHeader(headerArray);

            if (g_packetDataLen > COAP_MAX_PDU_SIZE)
            {
                OIC_LOG(ERROR, TAG, "len > pdu_size");
                return;
            }

            g_coapBuffer = (char *)OICCalloc(g_packetDataLen, sizeof(char));
            if (NULL == g_coapBuffer)
            {
                OIC_LOG(ERROR, TAG, "malloc");
                return;
            }

            OIC_LOG(DEBUG, TAG, "OUT");
            g_dataLen = 0;
        }

        OIC_LOG(DEBUG, TAG, "IN");
        while (CAIsBleDataAvailable())
        {
            OIC_LOG(DEBUG, TAG, "In While loop");
            g_coapBuffer[g_dataLen++] = CAReadBleData();
            if (g_dataLen == g_packetDataLen)
            {
                OIC_LOG(DEBUG, TAG, "Read Comp BLE Pckt");
                g_coapBuffer[g_dataLen] = '\0';
                if (g_dataLen > 0)
                {
                    OIC_LOG_V(DEBUG, TAG, "recv dataLen=%d", g_dataLen);
                    CANotifyCallback((void *)g_coapBuffer, g_dataLen, "", 0);
                }
                g_dataLen = 0;
                OICFree(g_coapBuffer);
                g_coapBuffer = NULL;
                break;
            }
        }
        OIC_LOG(DEBUG, TAG, "OUT");
    }
    else
    {
        OIC_LOG(DEBUG, TAG, "NoData");
    }
    return;
}

int32_t CASendLEData(const void *data, uint32_t dataLen)
{
    OIC_LOG(DEBUG, TAG, "IN");
    char header[CA_HEADER_LENGTH] = {0};

    CAResult_t result = CAGenerateHeader(header, dataLen);

    if (CA_STATUS_OK != result)
    {
        return -1;
    }

    if (!CAIsBleConnected())
    {
        OIC_LOG(DEBUG, TAG, "le not conn");
        return -1;
    }

    CAUpdateCharacteristicsInGattServer(header, CA_HEADER_LENGTH);
    int32_t dataLimit = dataLen / CA_SUPPORTED_BLE_MTU_SIZE;
    int32_t iter = 0;
    for (iter = 0; iter < dataLimit; iter++)
    {
        CAUpdateCharacteristicsInGattServer((data + (iter * CA_SUPPORTED_BLE_MTU_SIZE)),
                                                CA_SUPPORTED_BLE_MTU_SIZE);
        CABleDoEvents();
    }

    CAUpdateCharacteristicsInGattServer((data + (dataLimit * CA_SUPPORTED_BLE_MTU_SIZE)),
                                            dataLen % CA_SUPPORTED_BLE_MTU_SIZE);
    CABleDoEvents();
    OIC_LOG(DEBUG, TAG, "writebytes done");
    OIC_LOG(DEBUG, TAG, "OUT");
    // Arduino BLEWrite doesnot return value. So, Return the received DataLength
    return dataLen;
}

