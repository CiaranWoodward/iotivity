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

// Defining _BSD_SOURCE or _DEFAULT_SOURCE causes header files to expose
// definitions that may otherwise be skipped. Skipping can cause implicit
// declaration warnings and/or bugs and subtle problems in code execution.
// For glibc information on feature test macros,
// Refer http://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html
//
// This file requires #define use due to random() and inet_aton()
// For details on compatibility and glibc support,
// Refer http://man7.org/linux/man-pages/man3/inet.3.html
// Refer http://man7.org/linux/man-pages/man3/random.3.html
#define _DEFAULT_SOURCE
#define _BSD_SOURCE

#include "cawifiinterface.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#include "caadapterutils.h"

/**
 * @def WIFI_CLIENT_TAG
 * @brief Logging tag for module name
 */
#define WIFI_CLIENT_TAG "WIFI_CLIENT"

/**
 * @var g_unicastServerSocketDescClient
 * @brief socket descriptor for unicast server
 */
static int32_t g_unicastServerSocketDescClient = -1;

#ifdef __WITH_DTLS__
/**
 * @var g_unicastServerSecureSocketDescClient
 * @brief socket descriptor for secure unicast server
 */
static int32_t g_unicastServerSecureSocketDescClient = -1;
#endif

static uint32_t CASendData(const char *remoteAddress, const uint16_t port,
                           const void *data, const uint32_t dataLength, int32_t sockfd)
{
    OIC_LOG(DEBUG, WIFI_CLIENT_TAG, "IN");

    VERIFY_NON_NULL_RET(remoteAddress, WIFI_CLIENT_TAG, "IP address is NULL", 0);
    VERIFY_NON_NULL_RET(data, WIFI_CLIENT_TAG, "data is NULL", 0);

    if (dataLength == 0)
    {
        OIC_LOG(ERROR, WIFI_CLIENT_TAG, "Data length is 0 !");
        return 0;
    }

    if (0 > sockfd)
    {
        OIC_LOG(ERROR, WIFI_CLIENT_TAG, "Unicast Server is not running !");
        return 0;
    }

    struct sockaddr_in destAddr;
    memset((char *)&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(port);

    // Conversion from ASCII format to Network format
    if (inet_aton(remoteAddress, &destAddr.sin_addr) == 0)
    {
        OIC_LOG(ERROR, WIFI_CLIENT_TAG, "Failed to convert from ASCII to Network Address");
        return 0;
    }

    int32_t sendDataLength = sendto(sockfd, data, dataLength, 0,
                                    (struct sockaddr *)&destAddr, sizeof(destAddr));
    if (sendDataLength == -1)
    {
        OIC_LOG_V(ERROR, WIFI_CLIENT_TAG, "Failed to Send Data, Error code: %s", strerror(errno));
        return 0;
    }

    OIC_LOG_V(INFO, WIFI_CLIENT_TAG, "Sending data is successful, sent bytes[%d]", sendDataLength);

    OIC_LOG(DEBUG, WIFI_CLIENT_TAG, "OUT");
    return sendDataLength;
}

void CAWiFiSetUnicastSocket(const int32_t socketFD)
{
    OIC_LOG(DEBUG, WIFI_CLIENT_TAG, "IN");

    g_unicastServerSocketDescClient = socketFD;

    OIC_LOG(DEBUG, WIFI_CLIENT_TAG, "OUT");
}

#ifdef __WITH_DTLS__
void CAWiFiSetSecureUnicastSocket(const int32_t socketFD)
{
    OIC_LOG(DEBUG, WIFI_CLIENT_TAG, "IN");

    g_unicastServerSecureSocketDescClient = socketFD;

    OIC_LOG(DEBUG, WIFI_CLIENT_TAG, "OUT");
}
#endif
uint32_t CAWiFiSendData(const char *remoteAddress, const uint16_t port,
                        const void *data, const uint32_t dataLength,
                        bool isMulticast, bool isSecured)
{
    uint32_t len = 0;

#ifdef __WITH_DTLS__
    if (true == isSecured)
    {
        len  = CASendData(remoteAddress, port,
                          data, dataLength, g_unicastServerSecureSocketDescClient);
    }
    else
    {
#endif
        len =  CASendData(remoteAddress, port,
                          data, dataLength, g_unicastServerSocketDescClient);
#ifdef __WITH_DTLS__
    }
#endif
    return len;
}



