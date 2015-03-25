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

// Hosting Header
#include "hosting.h"
#include "virtualResource.h"

// External Lib
#include "cJSON.h"

/*
 * internal function
 */
///////////////////////////////////////////////////////////////////////////////////////////////////
static MirrorResourceList *s_mirrorResourceList = NULL;

#define OIC_COORDINATING_FLAG "/hosting"
#define OIC_STRING_MAX_VALUE 100

#define OC_DEFAULT_ADDRESS               "224.0.1.187"
#define OC_DEFAULT_PORT                  "5683"
#define OC_COORDINATING_QUERY            "/oc/core?rt=Resource.Hosting"
#define OC_PRESENCE_URI                  "/oc/presence"
#define DEFAULT_CONTEXT_VALUE 0x99

/*
 * Presence Func for hosting
 */

/**
 *
 * request presence for coordinating
 *
 * @param[in] originResourceAddr - pointer of address string of original resource
 *
 * @return
 *     OC_STACK_OK
 *     OC_STACK_ERROR
 */
OCStackResult requestPresence(char *originResourceAddr);

/**
 *
 * callback function that call when response of presence request received
 *
 * @param[in] originResourceAddr - pointer of address string of original resource
 *
 * @return
 *     OC_STACK_OK
 *     OC_STACK_ERROR
 */
OCStackApplicationResult requestPresenceCB(void *context, OCDoHandle handle,
        OCClientResponse *clientResponse);

/**
 *
 * build mirror resource list by clientResponse
 *
 * @param[in] handle - not using...
 * @param[in] clientResponse - client response that mirror resources are stored
 *
 * @return
 *     pointer of result MirrorResourceList
 */
MirrorResourceList *buildMirrorResourceList(OCDoHandle handle, OCClientResponse *clientResponse);

/**
 *
 * build mirror resource by JSON payload
 *
 * @param[in] ocArray_sub - pointer of json payload string
 *
 * @return
 *     pointer of result MirrorResource
 */
MirrorResource *buildMirrorResource(cJSON *ocArray_sub);

/**
 *
 * This method is used when setting queryUri, registering callback function and starting OCDoResource() Function in order to find Coordinatee Candidate
 *
 * @brief discover coordinatee candidate
 *
 * @return
 *     OC_STACK_OK               - no errors
 *     OC_STACK_INVALID_CALLBACK - invalid callback function pointer
 *     OC_STACK_INVALID_METHOD   - invalid resource method
 *     OC_STACK_INVALID_URI      - invalid required or reference URI
 *     OC_STACK_INVALID_QUERY    - number of resource types specified for filtering presence
 *                                 notifications exceeds @ref MAX_PRESENCE_FILTERS.
 *     OC_STACK_ERROR            - otherwise error(initialized value)
 */
int requestCoordinateeCandidateDiscovery(char *address);

/**
 *
 * This method is used to add a coordinator resource callback method in mirrorResourceList when host resource discovered.
 *
 * @param[in] context
 *              Context for callback method
 * @param[in] handle
 *              Handle to an @ref OCDoResource invocation.
 * @param[in] clientResponse
 *              Response from queries to remote servers. Queries are made by calling the @ref OCDoResource API.
 *
 * @brief callback for receiving response of discoverCoordinateeCandidate()
 *
 * @return
 *     PRINT("Callback Context for DISCOVER query recvd successfully\n")        - context is DEFAULT_CONTEXT_VALUE
 *     call the buildMirrorResource() method                                    - clientResponse is not NULL && clientResponse->result is OC_STACK_OK
 *     OC_STACK_KEEP_TRANSACTION                                                - otherwise case
 */
OCStackApplicationResult requestCoordinateeCandidateDiscoveryCB(void *context, OCDoHandle handle,
        OCClientResponse *clientResponse);

/**
 *
 * This method is used when setting queryUri, registering callback function and starting OCDoResource() Function in order to request resource coordination
 *
 * @brief
 *
 * @param[in] mirrorResource
 *          mirrorResource for using in order to request resource coordination
 *
 * @return
 *     OC_STACK_OK               - no errors
 *     OC_STACK_INVALID_CALLBACK - invalid callback function pointer
 *     OC_STACK_INVALID_METHOD   - invalid resource method
 *     OC_STACK_INVALID_URI      - invalid required or reference URI
 *     OC_STACK_INVALID_QUERY    - number of resource types specified for filtering presence
 *                                 notifications exceeds @ref MAX_PRESENCE_FILTERS.
 *     OC_STACK_ERROR            - otherwise error(initialized value)
 */
OCStackResult requestResourceObservation(MirrorResource *mirrorResource);

/**
 *
 * This method is used to handle callback of requestCoordination method.
 *
 * @param[in] context
 *              Context for callback method
 * @param[in] handle
 *              Handle to update mirror resource and check errorResponse
 * @param[in] clientResponse
 *              Response from queries to remote servers. Queries are made by calling the @ref OCDoResource API.
 *
 * @brief callback when receiving response of coordinating requestion.
 *
 * @todo diverge return value
 *
 * @return
 *
 *     OC_STACK_KEEP_TRANSACTION                                            - otherwise case
 */
OCStackApplicationResult requestResourceObservationCB(void *context, OCDoHandle handle,
        OCClientResponse *clientResponse);

/**
 *
 * This method is used to check resource validation and delete resource if it is not exist(not alive).
 *
 * @brief check mirror resource is alive
 *
 * @param[in] requestHandle
 *              Handle to check mirror resource
 *
 * @return
 *
 *     OC_STACK_DELETE_TRANSACTION                                              - otherwise case
 */
OCStackApplicationResult checkResourceValidation(OCDoHandle requestHandle);

/**
 *
 * register Mirror resource in the base resource list
 *
 * @param[in] requestHandle
 *              Handle to check mirror resource
 *
 * @return
 *     OC_STACK_OK
 *     OC_STACK_ERROR
 */
OCStackResult registerMirrorResource(MirrorResource *node);

/**
 *
 * update resource
 *
 * @param[in] sourceHandle - handle of source resource
 * @param[in] payload - pointer of json payload string that update items stored
 *
 * @return
 *     pointer of mirror resource. return NULL if there is any error.
 */
MirrorResource *updateMirrorResource(OCDoHandle sourceHandle, const char *payload);

/**
 *
 * build response payload
 *
 * @param[in] ehRequest - pointer of handler of entity handler request that to be responded
 *
 * @return
 *     OC_STACK_OK
 *     OC_STACK_ERROR
 */
char *buildResponsePayload (OCEntityHandlerRequest *ehRequest);

/**
 *
 * handle "Get" request
 *
 * @param[in] ehRequest - pointer of handler of entity handler request
 * @param[out] payload - pointer of payload to be responded
 * @param[in] maxPayloadSize - size of payload
 *
 * @return
 *     OC_EH_OK - success to copy response payload
 *     OC_EH_ERROR - error to copy response payload
 */
OCEntityHandlerResult handleGetRequest (OCEntityHandlerRequest *ehRequest,
                                        char *payload, uint16_t maxPayloadSize);

/**
 *
 * handle request for non-existing resource
 *
 * @param[in] ehRequest - pointer of handler of entity handler request
 * @param[out] payload - pointer of payload to be responded
 * @param[in] maxPayloadSize - size of payload
 *
 * @return
 *     OC_EH_RESOURCE_DELETED - resource deleted
 */
OCEntityHandlerResult handleNonExistingResourceRequest(OCEntityHandlerRequest *ehRequest,
        char *payload, uint16_t maxPayloadSize);

/**
 *
 * callback function that called when source resource changed
 *
 * @param[in] flag - entity handler flag
 * @param[in] entityHandlerRequest - pointer of entity handler request
 *
 * @return
 *     OC_EH_OK
 *     OC_EH_ERROR
 */
OCEntityHandlerResult resourceEntityHandlerCB (OCEntityHandlerFlag flag,
        OCEntityHandlerRequest *entityHandlerRequest);

/**
 *
 * request that address is alive
 *
 * @param[in] address - pointer of address string
 *
 * @return
 *     OC_STACK_OK
 *     OC_STACK_ERROR
 */
OCStackResult requestIsAlive(const char *address);

/**
 *
 * get string value of OCStackResult code
 *
 * @param[in] result - OCStringResult code
 *
 * @return
 *     pointer of result string value
 */
const char *getResultString(OCStackResult result);

/*
 * for Lite Device Side
 */

/**
 *
 * register resource as coordinatable
 *
 * @param[in] handle - resource handle
 * @param[in] resourceTypeName - resource type name
 * @param[in] resourceInterfaceName - resource interface name
 * @param[in] resourceUri - resource URI
 * @param[in] entityHandler - entity handler
 * @param[in] resourceProperties - resource properties
 *
 * @return
 *     pointer of result string value
 */
OCStackResult registerResourceAsCoordinatable(OCResourceHandle *handle,
        const char *resourceTypeName, const char *resourceInterfaceName,
        const char *resourceUri, OCEntityHandler entityHandler, uint8_t resourceProperties);

///////////////////////////////////////////////////////////////////////////////////////////////////


OCStackResult registerResourceAsCoordinatable(OCResourceHandle *handle,
        const char *resourceTypeName,
        const char *resourceInterfaceName,
        const char *resourceUri,
        OCEntityHandler entityHandler,
        uint8_t resourceProperties)
{

    char *coordinatingURI = (char *)malloc(sizeof(char) * (strlen(resourceUri) + strlen(
            OIC_COORDINATING_FLAG)));
    sprintf(coordinatingURI, "%s%s", resourceUri, OIC_COORDINATING_FLAG);

    OC_LOG_V(DEBUG, HOSTING_TAG, "requiedUri+coordinatingFlag = %s\n", coordinatingURI);

    OCStackResult result = OCCreateResource(handle, resourceTypeName, resourceInterfaceName,
                                            coordinatingURI, entityHandler, resourceProperties);

    return result;
}

/*
 *  for Hosting Device Side
 */
OCStackResult OICStartCoordinate()
{
    int result = OC_STACK_ERROR;

    s_mirrorResourceList = createMirrorResourceList();
//    result = discoverCoordinateeCandidate(NULL);
    result = requestPresence(OC_DEFAULT_ADDRESS);

    return result;
}

OCStackResult OICStopCoordinate()
{
    OCStackResult result = OC_STACK_ERROR;

    destroyMirrorResourceList(s_mirrorResourceList);

    return result;
}

int requestCoordinateeCandidateDiscovery(char *sourceResourceAddress)
{
    OCStackResult result;
    OCCallbackData cbData;
    OCDoHandle handle;

    /* Start a discovery query*/
    char queryUri[OIC_STRING_MAX_VALUE] = { '\0' };
    if (sourceResourceAddress == NULL)
    {
        sprintf(queryUri, "coap://%s:%s%s", OC_DEFAULT_ADDRESS, OC_DEFAULT_PORT , OC_COORDINATING_QUERY);
    }
    else
    {
        sprintf(queryUri, "coap://%s%s", sourceResourceAddress , OC_COORDINATING_QUERY);
    }

    cbData.cb = requestCoordinateeCandidateDiscoveryCB;
    cbData.context = (void *)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;

    result = OCDoResource(&handle, OC_REST_GET, queryUri, OIC_COORDINATING_FLAG, 0, OC_LOW_QOS, &cbData,
                          NULL, 0);
    if (result != OC_STACK_OK)
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "OCStack resource error\n");
    }
    OC_LOG_V(DEBUG, HOSTING_TAG, "Host Resource Finding...\n");
    return result;
}

OCStackResult requestPresence(char *sourceResourceAddress)
{
    OCStackResult result = OC_STACK_OK;
    OCCallbackData cbData;
    OCDoHandle handle;

    if (sourceResourceAddress == NULL)
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "SourceResourceAddress is not available.\n");
        result = OC_STACK_ERROR;
        return result;
    }

    cbData.cb = requestPresenceCB;
    cbData.context = (void *)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;

    char queryUri[OIC_STRING_MAX_VALUE] = { '\0' };
    sprintf(queryUri, "coap://%s%s", sourceResourceAddress , OC_PRESENCE_URI);
    OC_LOG_V(DEBUG, HOSTING_TAG, "initializePresenceForCoordinating Query : %s\n", queryUri);

    result = OCDoResource(&handle, OC_REST_PRESENCE, queryUri, 0, 0, OC_LOW_QOS, &cbData, NULL, 0);

    if (result != OC_STACK_OK)
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "initializePresenceForCoordinating error\n");
        return result;
    }
    // Need presenceHandle manager

    OC_LOG_V(DEBUG, HOSTING_TAG, "Success initializePresenceForCoordinating\n");

    return result;
}

OCStackApplicationResult requestPresenceCB(void *context, OCDoHandle handle,
        OCClientResponse *clientResponse)
{
    uint8_t remoteIpAddress[4];
    uint16_t remotePortNumber;
    char address[OIC_STRING_MAX_VALUE] = { '\0' };

    if (context == (void *) DEFAULT_CONTEXT_VALUE)
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "\tCallback Context for presence CB recv successfully\n");
    }
    if (clientResponse)
    {
        OCDevAddrToIPv4Addr((OCDevAddr *) clientResponse->addr, remoteIpAddress,
                            remoteIpAddress + 1, remoteIpAddress + 2, remoteIpAddress + 3);
        OCDevAddrToPort((OCDevAddr *) clientResponse->addr, &remotePortNumber);
        OC_LOG_V(DEBUG, HOSTING_TAG, "\tStackResult: %s\n",  getResultString(clientResponse->result));
        OC_LOG_V(DEBUG, HOSTING_TAG, "\tStackResult: %d\n",  clientResponse->result);
        OC_LOG_V(DEBUG, HOSTING_TAG,
                 "\tPresence Device =============> Presence %s @ %d.%d.%d.%d:%d\n",
                 clientResponse->resJSONPayload, remoteIpAddress[0], remoteIpAddress[1],
                 remoteIpAddress[2], remoteIpAddress[3], remotePortNumber);

        sprintf(address, "%d.%d.%d.%d:%d", remoteIpAddress[0], remoteIpAddress[1],
                remoteIpAddress[2], remoteIpAddress[3], remotePortNumber);
        if (clientResponse->result == OC_STACK_OK)
        {
            requestCoordinateeCandidateDiscovery(address);
        }
        //delete logic
//        else if(clientResponse->result == OC_STACK_PRESENCE_STOPPED
        if (clientResponse->result == OC_STACK_PRESENCE_STOPPED
            || clientResponse->result == OC_STACK_PRESENCE_TIMEOUT
            || clientResponse->result == OC_STACK_PRESENCE_DO_NOT_HANDLE)
        {
            requestIsAlive(address);
        }

    }
    return OC_STACK_KEEP_TRANSACTION;
}

OCStackApplicationResult requestCoordinateeCandidateDiscoveryCB(void *ctx, OCDoHandle handle,
        OCClientResponse *clientResponse)
{
    OC_LOG_V(DEBUG, HOSTING_TAG, "Found Host Resource\n");
    OCStackResult ret;

    if (ctx == (void *) DEFAULT_CONTEXT_VALUE)
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "Callback Context for DISCOVER query recvd successfully\n");
    }
    if (clientResponse && clientResponse->result == OC_STACK_OK)
    {
        //TODO: checkError

        MirrorResourceList *vList = buildMirrorResourceList(handle, clientResponse);
        if (vList != NULL)
        {

            if (vList->headerNode == NULL)
            {
                OC_LOG_V(DEBUG, HOSTING_TAG, "This Discover Response is empty\n");
                return OC_STACK_KEEP_TRANSACTION;
            }

            // register All of VirtualResource
            char *address = vList->headerNode->address[OIC_SOURCE_ADDRESS];
            while (vList->headerNode)
            {
                MirrorResource *mirrorResource = vList->headerNode;
                ret = ejectMirrorResource(vList, mirrorResource);
                mirrorResource->next = NULL;
                OC_LOG_V(DEBUG, HOSTING_TAG, "register virtual resource uri : %s\n", mirrorResource->uri);
                if (ret != OC_STACK_OK)
                {
                    continue;
                }

                ret = registerMirrorResource(mirrorResource);
                if (ret != OC_STACK_OK)
                {
                    continue;
                }

                ret = insertMirrorResource(s_mirrorResourceList, mirrorResource);
                if (ret != OC_STACK_OK)
                {
                    OCDeleteResource(mirrorResource->resourceHandle[OIC_MIRROR_HANDLE]);
                    continue;
                }
                printMirrorResourceList(s_mirrorResourceList);

                ret = requestResourceObservation(mirrorResource);
                if (ret != OC_STACK_OK)
                {
                    OCDeleteResource(mirrorResource->resourceHandle[OIC_MIRROR_HANDLE]);
                    deleteMirrorResourceFromList(s_mirrorResourceList, mirrorResource);
                    continue;
                }
            }
            destroyMirrorResourceList(vList);
            if (ret != OC_STACK_OK)
            {
                return OC_STACK_KEEP_TRANSACTION;
            }
        }
    }
    return OC_STACK_KEEP_TRANSACTION;
}

MirrorResourceList *buildMirrorResourceList(OCDoHandle handle, OCClientResponse *clientResponse)
{

    cJSON *discoveryJson = cJSON_CreateObject();
    discoveryJson = cJSON_Parse((char *)clientResponse->resJSONPayload);

    cJSON *ocArray = cJSON_GetObjectItem(discoveryJson, "oc");
    char *ocArray_str = cJSON_PrintUnformatted(ocArray);

    if ( strstr(ocArray_str, "[{}") == ocArray_str )
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "invalid payload : %s\n", ocArray_str);
        cJSON_Delete(discoveryJson);
        return NULL;
    }

    MirrorResourceList *retList = createMirrorResourceList();

    uint8_t remoteIpAddr[4];
    uint16_t remotePortNum;

    OCDevAddrToIPv4Addr((OCDevAddr *) clientResponse->addr, remoteIpAddr,
                        remoteIpAddr + 1, remoteIpAddr + 2, remoteIpAddr + 3);
    OCDevAddrToPort((OCDevAddr *) clientResponse->addr, &remotePortNum);

    char sourceaddr[OIC_STRING_MAX_VALUE] = {'\0'};
    sprintf(sourceaddr, "%d.%d.%d.%d:%d", remoteIpAddr[0], remoteIpAddr[1],
            remoteIpAddr[2], remoteIpAddr[3], remotePortNum);

    OC_LOG_V(DEBUG, HOSTING_TAG, "Host Device =============> Discovered %s @ %s\n",
             clientResponse->resJSONPayload, sourceaddr);

    int i = 0;
    int arraySize = cJSON_GetArraySize(ocArray);
    for (i = 0; i < arraySize; ++i)
    {
        cJSON *ocArray_sub = cJSON_GetArrayItem(ocArray, i);
        MirrorResource *mirrorResource = buildMirrorResource(ocArray_sub);

        if (mirrorResource == NULL)
        {
            continue;
        }
        mirrorResource->address[OIC_SOURCE_ADDRESS] = (char *)malloc(sizeof(char) * OIC_STRING_MAX_VALUE);
        sprintf(mirrorResource->address[OIC_SOURCE_ADDRESS], "%s", sourceaddr);

        mirrorResource->address[OIC_MIRROR_ADDRESS] = (char *)malloc(sizeof(char) * OIC_STRING_MAX_VALUE);
        sprintf(mirrorResource->address[OIC_MIRROR_ADDRESS], "0.0.0.0:00");

        if (OC_STACK_OK != insertMirrorResource(retList, mirrorResource))
        {
            OC_LOG_V(DEBUG, HOSTING_TAG, "buildVirtualResourceList : insert resource fail\n");
        }
    }

    cJSON_Delete(discoveryJson);
    return retList;
}

MirrorResource *buildMirrorResource(cJSON *ocArray_sub)
{

    char temp[OIC_STRING_MAX_VALUE] = {'\0'};
    strcpy(temp, cJSON_GetObjectItem(ocArray_sub, "href")->valuestring);

    if ( strstr(temp, OIC_COORDINATING_FLAG) )
    {

        uint8_t remoteIpAddr[4];
        uint16_t remotePortNum;

        MirrorResource *mirrorResource = createMirrorResource();

        mirrorResource->uri = (char *)malloc(sizeof(char) * OIC_STRING_MAX_VALUE);
        strncpy(mirrorResource->uri, temp, strlen(temp) - strlen(OIC_COORDINATING_FLAG));
        mirrorResource->uri[strlen(temp) - strlen(OIC_COORDINATING_FLAG)] = '\0';
        OC_LOG_V(DEBUG, HOSTING_TAG, "VirtualResource URI : %s\n", mirrorResource->uri);

        cJSON *inArray_sub = cJSON_GetObjectItem(ocArray_sub, "prop");

        cJSON *tmpJSON;
        int sizetemp;
        int k = 0;

        tmpJSON = cJSON_GetObjectItem(inArray_sub, "rt");
        sizetemp = cJSON_GetArraySize(tmpJSON);
        mirrorResource->prop.countResourceType = sizetemp;
        mirrorResource->prop.resourceType = (char **)malloc(sizeof(char *)*sizetemp);
        for (k = 0; k < sizetemp; ++k)
        {
            mirrorResource->prop.resourceType[k] = (char *)malloc(sizeof(char) * OIC_STRING_MAX_VALUE);
            memset(mirrorResource->prop.resourceType[k], '\0', OIC_STRING_MAX_VALUE);
            strcpy(mirrorResource->prop.resourceType[k], cJSON_GetArrayItem(tmpJSON, k)->valuestring);
        }

        tmpJSON = cJSON_GetObjectItem(inArray_sub, "if");
        sizetemp = cJSON_GetArraySize(tmpJSON);
        mirrorResource->prop.countInterface = sizetemp;
        mirrorResource->prop.resourceInterfaceName = (char **)malloc(sizeof(char *)*sizetemp);
        for (k = 0; k < sizetemp; ++k)
        {
            mirrorResource->prop.resourceInterfaceName[k] = (char *)malloc(sizeof(char) * OIC_STRING_MAX_VALUE);
            memset(mirrorResource->prop.resourceInterfaceName[k], '\0', OIC_STRING_MAX_VALUE);
            strcpy(mirrorResource->prop.resourceInterfaceName[k], cJSON_GetArrayItem(tmpJSON, k)->valuestring);
        }
        return mirrorResource;
    }
    else
    {
        return NULL;
    }
}

OCStackResult registerMirrorResource(MirrorResource *mirrorResource)
{
    OCStackResult result = OC_STACK_ERROR;

    MirrorResource *foundMirrorResource = findMirrorResourceUsingAddressAndURI(s_mirrorResourceList,
                                          mirrorResource->address[OIC_MIRROR_ADDRESS], OIC_MIRROR_ADDRESS, mirrorResource->uri);
    if (foundMirrorResource != NULL)
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "Already registered resource\n");
        goto RETURN_ERR;
    }

    result = OCCreateResource(&(mirrorResource->resourceHandle[OIC_MIRROR_HANDLE]),
                              mirrorResource->prop.resourceType[0],
                              mirrorResource->prop.resourceInterfaceName[0],
                              mirrorResource->uri,
                              resourceEntityHandlerCB,
                              OC_DISCOVERABLE | OC_OBSERVABLE);

    printf("mirror resource uri : %s\n", mirrorResource->uri);

    if (result != OC_STACK_OK)
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "error return = %s\n", getResultString(result));
        mirrorResource->next = NULL;
        destroyMirrorResource(mirrorResource);
        goto RETURN_ERR;
    }

    if (mirrorResource->prop.countResourceType > 1)
    {
        int i = 0;
        for (i = 1; i < mirrorResource->prop.countResourceType; ++i)
        {
            result = OCBindResourceTypeToResource(mirrorResource->resourceHandle[OIC_MIRROR_HANDLE],
                                                  mirrorResource->prop.resourceType[i]);
            if (result != OC_STACK_OK)
            {
                OC_LOG_V(DEBUG, HOSTING_TAG, "Virtual Resource Registration Fail : BindResourceType\n");
                OCDeleteResource(mirrorResource->resourceHandle[OIC_MIRROR_HANDLE]);
                mirrorResource->next = NULL;
                destroyMirrorResource(mirrorResource);
                goto RETURN_ERR;
            }
        }
    }

    if (mirrorResource->prop.countInterface > 1)
    {
        int i = 0;
        for (i = 1; i < mirrorResource->prop.countInterface; ++i)
        {
            result = OCBindResourceInterfaceToResource(mirrorResource->resourceHandle[OIC_MIRROR_HANDLE],
                     mirrorResource->prop.resourceInterfaceName[i]);
            if (result != OC_STACK_OK)
            {
                OC_LOG_V(DEBUG, HOSTING_TAG, "Virtual Resource Registration Fail : BindResourceInterfaceName\n");
                OCDeleteResource(mirrorResource->resourceHandle[OIC_MIRROR_HANDLE]);
                mirrorResource->next = NULL;
                destroyMirrorResource(mirrorResource);
                goto RETURN_ERR;
            }
        }
    }

    OC_LOG_V(DEBUG, HOSTING_TAG, "Mirror Resource Registration Success\n");
    OC_LOG_V(DEBUG, HOSTING_TAG, "Mirror Resource uri : %s\n", mirrorResource->uri);
    OC_LOG_V(DEBUG, HOSTING_TAG, "Mirror Resource source address : %s\n",
             mirrorResource->address[OIC_SOURCE_ADDRESS]);
    OC_LOG_V(DEBUG, HOSTING_TAG, "Mirror Resource virtual address : %s\n",
             mirrorResource->address[OIC_MIRROR_ADDRESS]);
    return result;

RETURN_ERR:
    OC_LOG_V(DEBUG, HOSTING_TAG, "Mirror Resource Registration Fail\n");
    return result;
}

OCStackResult requestResourceObservation(MirrorResource *mirrorResource)
{
    OCStackResult result;
    OCCallbackData cbData;

    cbData.cb = requestResourceObservationCB;
    cbData.context = (void *)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;

    char query[OIC_STRING_MAX_VALUE] = {'\0'};
    sprintf(query, "coap://%s%s%s", mirrorResource->address[OIC_SOURCE_ADDRESS], mirrorResource->uri,
            OIC_COORDINATING_FLAG);

    result = OCDoResource(&mirrorResource->resourceHandle[OIC_REQUEST_HANDLE], OC_REST_OBSERVE, query,
                          0, NULL,
                          OC_HIGH_QOS, &cbData, NULL, 0);

    if (result != OC_STACK_OK)
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "OCDoResource returns error %s with method %d\n",
                 getResultString(result), OC_REST_OBSERVE);
    }

    return result;
}

OCStackApplicationResult requestResourceObservationCB(void *context, OCDoHandle handle,
        OCClientResponse *clientResponse)
{
    if (context == (void *)DEFAULT_CONTEXT_VALUE)
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "Callback Context for OBS query recvd successfully\n");
    }

    if (clientResponse && clientResponse->result != OC_STACK_OK)
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "observeCB result error = %s\n",
                 getResultString(clientResponse->result));
        return checkResourceValidation(handle);
    }

    else if (clientResponse && clientResponse->result == OC_STACK_OK)
    {
        OC_LOG_V(DEBUG, HOSTING_TAG,
                 "<=============\nCallback Context for OBSERVE notification recvd successfully\n");
        OC_LOG_V(DEBUG, HOSTING_TAG, "SEQUENCE NUMBER: %d\n", clientResponse->sequenceNumber);
        OC_LOG_V(DEBUG, HOSTING_TAG, "JSON = %s \n=============> Obs Response\n",
                 clientResponse->resJSONPayload);

        MirrorResource *foundMirrorResource = findMirrorResourceUsingHandle(s_mirrorResourceList,
                                              handle, OIC_REQUEST_HANDLE);
        if (foundMirrorResource == NULL)
        {
            OC_LOG_V(DEBUG, HOSTING_TAG, "Cannot found Mirror Resource : Fail\n");
            return OC_STACK_DELETE_TRANSACTION;
        }

        if (foundMirrorResource->isAliveCheck)
        {
            OC_LOG_V(DEBUG, HOSTING_TAG, "This response is Alive Check : Keep resource %s%s\n",
                     foundMirrorResource->address[OIC_SOURCE_ADDRESS], foundMirrorResource->uri);
            deleteMirrorResourceFromList(s_mirrorResourceList, foundMirrorResource);
            return OC_STACK_KEEP_TRANSACTION;
        }

        foundMirrorResource = updateMirrorResource(handle, clientResponse->resJSONPayload);

        if ( OC_STACK_OK != OCNotifyAllObservers(foundMirrorResource->resourceHandle[OIC_MIRROR_HANDLE],
                OC_HIGH_QOS) )
        {
            OC_LOG_V(DEBUG, HOSTING_TAG, "Notify Mirror Resource's Subscriber : Fail\n");
        }
        else
        {
            OC_LOG_V(DEBUG, HOSTING_TAG, "Notify Mirror Resource's Subscriber : Success\n");
        }

        if (clientResponse->sequenceNumber == OC_OBSERVE_REGISTER)
        {
            OC_LOG_V(DEBUG, HOSTING_TAG, "This also serves as a registration confirmation\n");
        }
        else if (clientResponse->sequenceNumber == OC_OBSERVE_DEREGISTER)
        {
            OC_LOG_V(DEBUG, HOSTING_TAG, "This also serves as a deregistration confirmation\n");
            return OC_STACK_DELETE_TRANSACTION;
        }
        else if (clientResponse->sequenceNumber == OC_OBSERVE_NO_OPTION)
        {
            OC_LOG_V(DEBUG, HOSTING_TAG, "This also tells you that registration/deregistration failed\n");
            return OC_STACK_DELETE_TRANSACTION;
        }
    }
    return OC_STACK_KEEP_TRANSACTION;
}

OCStackApplicationResult checkResourceValidation(OCDoHandle handle)
{
    MirrorResource *foundMirrorResource = findMirrorResourceUsingHandle(s_mirrorResourceList,
                                          handle, OIC_REQUEST_HANDLE);

    if (foundMirrorResource == NULL)
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "Not found any error mirror resource.\n");
        return OC_STACK_DELETE_TRANSACTION;
    }

    if (foundMirrorResource->isAliveCheck)
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "This response is Alive Check : Expired resource %s%s\n",
                 foundMirrorResource->address[OIC_SOURCE_ADDRESS], foundMirrorResource->uri);
        MirrorResource *deletedMirrorResource = findMirrorResourceUsingAddressAndURI(s_mirrorResourceList,
                                                foundMirrorResource->uri, OIC_SOURCE_ADDRESS, foundMirrorResource->address[OIC_SOURCE_ADDRESS]);
        if (deletedMirrorResource)
        {
            OCDeleteResource(deletedMirrorResource->resourceHandle[OIC_MIRROR_HANDLE]);
            deleteMirrorResourceFromList(s_mirrorResourceList, deletedMirrorResource);
        }
    }
    else
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "This response is Expired resource %s%s\n",
                 foundMirrorResource->address[OIC_SOURCE_ADDRESS], foundMirrorResource->uri);
        OCDeleteResource(foundMirrorResource->resourceHandle[OIC_MIRROR_HANDLE]);
    }
    deleteMirrorResourceFromList(s_mirrorResourceList, foundMirrorResource);
    return OC_STACK_DELETE_TRANSACTION;
}

MirrorResource *updateMirrorResource(OCDoHandle handle, const char *payload)
{
    MirrorResource *foundMirrorResource = findMirrorResourceUsingHandle(
            s_mirrorResourceList, handle, OIC_REQUEST_HANDLE);

    if (!foundMirrorResource)
    {
        // TODO
        OC_LOG_V(DEBUG, HOSTING_TAG, "Cannot found Mirror Resource.\n");
        return NULL;
    }

    cJSON *observeJson = cJSON_CreateObject();
    observeJson = cJSON_Parse(payload);

    cJSON *ocArray = cJSON_GetObjectItem(observeJson, "oc");
    int arraySize = cJSON_GetArraySize(ocArray);

    cJSON *ocArray_sub = cJSON_GetArrayItem(ocArray, 0);

    cJSON *tempData = cJSON_GetObjectItem(ocArray_sub, "rep");
    char *temp = cJSON_PrintUnformatted(tempData);

    cJSON *repData = cJSON_Parse(temp);

    free(temp);
    cJSON_Delete(observeJson);

    if (foundMirrorResource->rep)
    {
        cJSON_Delete(foundMirrorResource->rep);
        foundMirrorResource->rep = NULL;
    }
    foundMirrorResource->rep = repData;

    cJSON *json = cJSON_CreateObject();

    char nodeData[OIC_STRING_MAX_VALUE] = {'\0'};
    sprintf(nodeData, "%s", foundMirrorResource->uri);
    cJSON_AddStringToObject(json, "href", nodeData);

    cJSON *nodeRep = cJSON_Parse(cJSON_PrintUnformatted(foundMirrorResource->rep));
    cJSON_AddItemToObject(json, "rep", nodeRep);
    OC_LOG_V(DEBUG, HOSTING_TAG, "It will notify resource : %s\n", cJSON_PrintUnformatted(json));

    cJSON_Delete(json);

    return foundMirrorResource;
}

char *buildResponsePayload (OCEntityHandlerRequest *entityHandlerRequest)
{
    MirrorResource *mirrorResource = findMirrorResourceUsingHandle(s_mirrorResourceList,
                                     entityHandlerRequest->resource, OIC_MIRROR_HANDLE);
    if (!mirrorResource)
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "Cannot found Mirror Resource.\n");
        return NULL;
    }

    OC_LOG_V(DEBUG, HOSTING_TAG, "node's uri : %s\n", mirrorResource->uri);
    OC_LOG_V(DEBUG, HOSTING_TAG, "node's source address : %s\n", mirrorResource->address[0]);
    OC_LOG_V(DEBUG, HOSTING_TAG, "node's mirror address : %s\n", mirrorResource->address[1]);
    OC_LOG_V(DEBUG, HOSTING_TAG, "node's rep : %s\n", cJSON_PrintUnformatted(mirrorResource->rep));

    cJSON *jsonObject = cJSON_CreateObject();

    char uriString[OIC_STRING_MAX_VALUE] = {'\0'};
    sprintf(uriString, "%s", mirrorResource->uri);
    cJSON_AddStringToObject(jsonObject, "href", uriString);

    cJSON *itemRep = cJSON_Parse(cJSON_PrintUnformatted(mirrorResource->rep));
    cJSON_AddItemToObject(jsonObject, "rep", itemRep);
    OC_LOG_V(DEBUG, HOSTING_TAG, "Will response resource : %s\n", cJSON_PrintUnformatted(jsonObject));

    char *jsonResponse = cJSON_Print(jsonObject);
    cJSON_Delete(jsonObject);

    return jsonResponse;
}

OCEntityHandlerResult
resourceEntityHandlerCB (OCEntityHandlerFlag entifyHandlerFlag,
                         OCEntityHandlerRequest *entityHandlerRequest)
{
    OC_LOG_V(DEBUG, HOSTING_TAG, "Inside device default entity handler - flags: 0x%x\n",
             entifyHandlerFlag);

    OCEntityHandlerResult entityHandlerResult = OC_EH_OK;
    OCEntityHandlerResponse entityHandlerResponse;
    char payload[MAX_RESPONSE_LENGTH] = {0};

    // Validate pointer
    if (!entityHandlerRequest)
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "Invalid request pointer\n");
        return OC_EH_ERROR;
    }

    // Initialize certain response fields
    entityHandlerResponse.numSendVendorSpecificHeaderOptions = 0;
    memset(entityHandlerResponse.sendVendorSpecificHeaderOptions, 0,
           sizeof entityHandlerResponse.sendVendorSpecificHeaderOptions);
    memset(entityHandlerResponse.resourceUri, 0, sizeof entityHandlerResponse.resourceUri);

    if (entifyHandlerFlag & OC_INIT_FLAG)
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "Flag includes OC_INIT_FLAG\n");
    }
    if (entifyHandlerFlag & OC_REQUEST_FLAG)
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "Flag includes OC_REQUEST_FLAG\n");
        if (entityHandlerRequest->resource == NULL)
        {
            OC_LOG_V(DEBUG, HOSTING_TAG, "Received request from client to a non-existing resource\n");
            entityHandlerResult = handleNonExistingResourceRequest(entityHandlerRequest, payload,
                                  sizeof(payload) - 1);
        }
        else if (OC_REST_GET == entityHandlerRequest->method)
        {
            OC_LOG_V(DEBUG, HOSTING_TAG, "Received OC_REST_GET from client\n");
            entityHandlerResult = handleGetRequest (entityHandlerRequest, payload, sizeof(payload) - 1);
        }
        else if (OC_REST_PUT == entityHandlerRequest->method)
        {
            OC_LOG_V(DEBUG, HOSTING_TAG, "Received OC_REST_PUT from client\n");
        }
        else if (OC_REST_DELETE == entityHandlerRequest->method)
        {
            OC_LOG_V(DEBUG, HOSTING_TAG, "Received OC_REST_DELETE from client\n");
        }
        else
        {
            OC_LOG_V(DEBUG, HOSTING_TAG, "Received unsupported method %d from client\n",
                     entityHandlerRequest->method);
            entityHandlerResult = OC_EH_ERROR;
        }

        // If the result isn't an error or forbidden, send response
        if (!((entityHandlerResult == OC_EH_ERROR) || (entityHandlerResult == OC_EH_FORBIDDEN)))
        {
            // Format the response.  Note this requires some info about the request
            entityHandlerResponse.requestHandle = entityHandlerRequest->requestHandle;
            entityHandlerResponse.resourceHandle = entityHandlerRequest->resource;
            entityHandlerResponse.ehResult = entityHandlerResult;
            entityHandlerResponse.payload = (unsigned char *)payload;
            entityHandlerResponse.payloadSize = strlen(payload);
            // Indicate that response is NOT in a persistent buffer
            entityHandlerResponse.persistentBufferFlag = 0;

            // Handle vendor specific options
            if (entityHandlerRequest->rcvdVendorSpecificHeaderOptions &&
                entityHandlerRequest->numRcvdVendorSpecificHeaderOptions)
            {
                OC_LOG_V(DEBUG, HOSTING_TAG, "Received vendor specific options\n");
                uint8_t i = 0;
                OCHeaderOption *receivedVenderSpecificHeaderOptions =
                    entityHandlerRequest->rcvdVendorSpecificHeaderOptions;
                for ( i = 0; i < entityHandlerRequest->numRcvdVendorSpecificHeaderOptions; i++)
                {
                    if (((OCHeaderOption)receivedVenderSpecificHeaderOptions[i]).protocolID == OC_COAP_ID)
                    {
                        OC_LOG_V(DEBUG, HOSTING_TAG, "Received option with OC_COAP_ID and ID %u with\n",
                                 ((OCHeaderOption)receivedVenderSpecificHeaderOptions[i]).optionID );
                    }
                }
                OCHeaderOption *sendVenderSpecificHeaderOptions =
                    entityHandlerResponse.sendVendorSpecificHeaderOptions;
                uint8_t option2[] = {21, 22, 23, 24, 25, 26, 27, 28, 29, 30};
                uint8_t option3[] = {31, 32, 33, 34, 35, 36, 37, 38, 39, 40};
                sendVenderSpecificHeaderOptions[0].protocolID = OC_COAP_ID;
                sendVenderSpecificHeaderOptions[0].optionID = 2248;
                memcpy(sendVenderSpecificHeaderOptions[0].optionData, option2, sizeof(option2));
                sendVenderSpecificHeaderOptions[0].optionLength = 10;
                sendVenderSpecificHeaderOptions[1].protocolID = OC_COAP_ID;
                sendVenderSpecificHeaderOptions[1].optionID = 2600;
                memcpy(sendVenderSpecificHeaderOptions[1].optionData, option3, sizeof(option3));
                sendVenderSpecificHeaderOptions[1].optionLength = 10;
                entityHandlerResponse.numSendVendorSpecificHeaderOptions = 2;
            }

            // Send the response
            if (OCDoResponse(&entityHandlerResponse) != OC_STACK_OK)
            {
                OC_LOG(ERROR, HOSTING_TAG, "Error sending response");
                entityHandlerResult = OC_EH_ERROR;
            }
        }
    }
    if (entifyHandlerFlag & OC_OBSERVE_FLAG)
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "Flag includes OC_OBSERVE_FLAG\n");
        if (OC_OBSERVE_REGISTER == entityHandlerRequest->obsInfo.action)
        {
            OC_LOG_V(DEBUG, HOSTING_TAG, "Received OC_OBSERVE_REGISTER from client\n");
        }
        else if (OC_OBSERVE_DEREGISTER == entityHandlerRequest->obsInfo.action)
        {
            OC_LOG_V(DEBUG, HOSTING_TAG, "Received OC_OBSERVE_DEREGISTER from client\n");
        }
    }

    return entityHandlerResult;
}
OCEntityHandlerResult handleGetRequest (OCEntityHandlerRequest *entityHandlerRequest,
                                        char *payload, uint16_t maxPayloadSize)
{
    OC_LOG_V(DEBUG, HOSTING_TAG, "ProcessGetRequest in....\n");

    OCEntityHandlerResult entityHandlerResult;
    char *responsePayload = buildResponsePayload(entityHandlerRequest);

    if (maxPayloadSize > strlen ((char *)responsePayload))
    {
        strncpy(payload, responsePayload, strlen((char *)responsePayload));
        entityHandlerResult = OC_EH_OK;
    }
    else
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "Response buffer: %d bytes is too small\n", maxPayloadSize);
        entityHandlerResult = OC_EH_ERROR;
    }

    free(responsePayload);

    return entityHandlerResult;
}
OCEntityHandlerResult handleNonExistingResourceRequest(OCEntityHandlerRequest *entityHandlerRequest,
        char *payload, uint16_t maxPayloadSize)
{
    OC_LOG_V(INFO, HOSTING_TAG, "\n\nExecuting %s ", __func__);

    const char *responsePayload = NULL;
    responsePayload = "{App determines payload: The resource does not exist.}";

    if ( (entityHandlerRequest != NULL) &&
         (maxPayloadSize > strlen ((char *)responsePayload)) )
    {
        strncpy((char *)payload, responsePayload, strlen((char *)responsePayload));
    }
    else
    {
        OC_LOG_V (INFO, HOSTING_TAG, "Response buffer: %d bytes is too small",
                  maxPayloadSize);
    }

    return OC_EH_RESOURCE_DELETED;
}

OCStackResult requestIsAlive(const char *address)
{
    MirrorResourceList *requestMirrorResourceList = findMirrorResourceListUsingAddress(
                s_mirrorResourceList, address, OIC_SOURCE_ADDRESS);

    if (requestMirrorResourceList == NULL)
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "Cannot found any mirror resource1\n");
        return OC_STACK_ERROR;
    }

    if (requestMirrorResourceList->headerNode == NULL)
    {
        OC_LOG_V(DEBUG, HOSTING_TAG, "Cannot found any mirror resource2\n");
        return OC_STACK_ERROR;
    }

    MirrorResource *mirrorResource = requestMirrorResourceList->headerNode;
    while (mirrorResource)
    {
        MirrorResource *currentMirrorResource = mirrorResource;
        mirrorResource = mirrorResource->next;
        if (OC_STACK_OK != ejectMirrorResource(requestMirrorResourceList, currentMirrorResource))
        {
            continue;
        }

        OCCallbackData cbData;

        cbData.cb = requestResourceObservationCB;
        cbData.context = (void *)DEFAULT_CONTEXT_VALUE;
        cbData.cd = NULL;

        char query[OIC_STRING_MAX_VALUE] = {'\0'};
        sprintf(query, "coap://%s%s", address, currentMirrorResource->uri);

        currentMirrorResource->resourceHandle[OIC_REQUEST_HANDLE] = NULL;
        currentMirrorResource->isAliveCheck = 1;
        insertMirrorResource(s_mirrorResourceList, currentMirrorResource);
        OC_LOG_V(DEBUG, HOSTING_TAG, "Check Alive IP : %s, URI : %s\n",
                 currentMirrorResource->address[OIC_SOURCE_ADDRESS], currentMirrorResource->uri);
        OCStackResult result = OCDoResource(&currentMirrorResource->resourceHandle[OIC_REQUEST_HANDLE],
                                            OC_REST_GET, query, 0, NULL, OC_HIGH_QOS, &cbData, NULL, 0);
        if (result != OC_STACK_OK)
        {
            OC_LOG_V(DEBUG, HOSTING_TAG, "OCDoResource returns error %s with method %d\n",
                     getResultString(result), OC_REST_OBSERVE);
            deleteMirrorResourceFromList(s_mirrorResourceList, currentMirrorResource);
        }
    }
    destroyMirrorResourceList(requestMirrorResourceList);

    return OC_STACK_OK;
}

const char *getResultString(OCStackResult result)
{
    switch (result)
    {
        case OC_STACK_OK:
            return "OC_STACK_OK";
        case OC_STACK_RESOURCE_CREATED:
            return "OC_STACK_RESOURCE_CREATED";
        case OC_STACK_RESOURCE_DELETED:
            return "OC_STACK_RESOURCE_DELETED";
        case OC_STACK_INVALID_URI:
            return "OC_STACK_INVALID_URI";
        case OC_STACK_INVALID_QUERY:
            return "OC_STACK_INVALID_QUERY";
        case OC_STACK_INVALID_IP:
            return "OC_STACK_INVALID_IP";
        case OC_STACK_INVALID_PORT:
            return "OC_STACK_INVALID_PORT";
        case OC_STACK_INVALID_CALLBACK:
            return "OC_STACK_INVALID_CALLBACK";
        case OC_STACK_INVALID_METHOD:
            return "OC_STACK_INVALID_METHOD";
        case OC_STACK_NO_MEMORY:
            return "OC_STACK_NO_MEMORY";
        case OC_STACK_COMM_ERROR:
            return "OC_STACK_COMM_ERROR";
        case OC_STACK_INVALID_PARAM:
            return "OC_STACK_INVALID_PARAM";
        case OC_STACK_NOTIMPL:
            return "OC_STACK_NOTIMPL";
        case OC_STACK_NO_RESOURCE:
            return "OC_STACK_NO_RESOURCE";
        case OC_STACK_RESOURCE_ERROR:
            return "OC_STACK_RESOURCE_ERROR";
        case OC_STACK_SLOW_RESOURCE:
            return "OC_STACK_SLOW_RESOURCE";
        case OC_STACK_NO_OBSERVERS:
            return "OC_STACK_NO_OBSERVERS";
        case OC_STACK_VIRTUAL_DO_NOT_HANDLE:
            return "OC_STACK_VIRTUAL_DO_NOT_HANDLE";
        case OC_STACK_PRESENCE_STOPPED:
            return "OC_STACK_PRESENCE_STOPPED";
        case OC_STACK_PRESENCE_TIMEOUT:
            return "OC_STACK_PRESENCE_TIMEOUT";
        case OC_STACK_PRESENCE_DO_NOT_HANDLE:
            return "OC_STACK_PRESENCE_DO_NOT_HANDLE";
        case OC_STACK_ERROR:
            return "OC_STACK_ERROR";
        default:
            return "UNKNOWN";
    }
}

void getJsonArrayPair(cJSON *tempData)
{
    //////////////////////////////////////////////////////////////////////////////////////////////
    // Test
    int countofrep = cJSON_GetArraySize(tempData);
    OC_LOG_V(DEBUG, HOSTING_TAG,
             "//////////////////////////////////////////////////////////////////////////\n");
    OC_LOG_V(DEBUG, HOSTING_TAG, "//Test\n");
    OC_LOG_V(DEBUG, HOSTING_TAG, "rep Size : %d\n", countofrep);
    int i = 0;
    for (i = 0; i < countofrep; ++i)
    {
        cJSON *arrayJSON = cJSON_GetArrayItem(tempData, i);
        OC_LOG_V(DEBUG, HOSTING_TAG, "rep#%d's name : %s\n", i, arrayJSON->string);

        switch (arrayJSON->type)
        {
            case cJSON_False:
            case cJSON_True:
                OC_LOG_V(DEBUG, HOSTING_TAG, "rep#%d's value : %d\n", i, arrayJSON->valueint);
                break;
            case cJSON_Number:
                OC_LOG_V(DEBUG, HOSTING_TAG, "rep#%d's value : %f\n", i, arrayJSON->valuedouble);
                break;
            case cJSON_String:
                OC_LOG_V(DEBUG, HOSTING_TAG, "rep#%d's value : %s\n", i, arrayJSON->valuestring);
                break;
            case cJSON_NULL:
            default:
                OC_LOG_V(DEBUG, HOSTING_TAG, "rep#%d's value : NULL\n", i);
                break;
        }
    }
    OC_LOG_V(DEBUG, HOSTING_TAG,
             "//////////////////////////////////////////////////////////////////////////\n");
    //////////////////////////////////////////////////////////////////////////////////////////////
}
