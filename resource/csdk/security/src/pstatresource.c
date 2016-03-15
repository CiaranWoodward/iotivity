//******************************************************************
//
// Copyright 2015 Intel Mobile Communications GmbH All Rights Reserved.
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

#include <stdlib.h>
#include <string.h>

#include "ocstack.h"
#include "oic_malloc.h"
#include "ocpayload.h"
#include "payload_logging.h"
#include "resourcemanager.h"
#include "pstatresource.h"
#include "doxmresource.h"
#include "psinterface.h"
#include "srmresourcestrings.h"
#include "srmutility.h"

#define TAG  "SRM-PSTAT"

/** Default cbor payload size. This value is increased in case of CborErrorOutOfMemory.
 * The value of payload size is increased until reaching below max cbor size. */
static const uint8_t CBOR_SIZE = 255;

// Max cbor size payload.
static const uint16_t CBOR_MAX_SIZE = 4400;

// PSTAT Map size - Number of mandatory items
static const uint8_t PSTAT_MAP_SIZE = 7;

static OicSecDpom_t gSm = SINGLE_SERVICE_CLIENT_DRIVEN;
static OicSecPstat_t gDefaultPstat =
{
    false,                                    // bool isOwned
    (OicSecDpm_t)(TAKE_OWNER | BOOTSTRAP_SERVICE | SECURITY_MANAGEMENT_SERVICES |
    PROVISION_CREDENTIALS | PROVISION_ACLS),   // OicSecDpm_t cm
    (OicSecDpm_t)(TAKE_OWNER | BOOTSTRAP_SERVICE | SECURITY_MANAGEMENT_SERVICES |
    PROVISION_CREDENTIALS | PROVISION_ACLS),   // OicSecDpm_t tm
    {.id = {0}},                              // OicUuid_t deviceID
    SINGLE_SERVICE_CLIENT_DRIVEN,             // OicSecDpom_t om */
    1,                                        // the number of elts in Sms
    &gSm,                                     // OicSecDpom_t *sm
    0,                                        // uint16_t commitHash
};

static OicSecPstat_t    *gPstat = NULL;

static OCResourceHandle gPstatHandle = NULL;

void DeletePstatBinData(OicSecPstat_t* pstat)
{
    if (pstat)
    {
        //Clean 'supported modes' field
        OICFree(pstat->sm);

        //Clean pstat itself
        OICFree(pstat);
    }
}

OCStackResult PstatToCBORPayload(const OicSecPstat_t *pstat, uint8_t **payload, size_t *size)
{
    if (NULL == pstat || NULL == payload || NULL != *payload || NULL == size)
    {
        return OC_STACK_INVALID_PARAM;
    }

    size_t cborLen = *size;
    if (0 == cborLen)
    {
        cborLen = CBOR_SIZE;
    }

    *payload = NULL;
    *size = 0;

    OCStackResult ret = OC_STACK_ERROR;

    CborEncoder encoder;
    CborEncoder pstatMap;

    int64_t cborEncoderResult = CborNoError;

    uint8_t *outPayload = (uint8_t *)OICCalloc(1, cborLen);
    VERIFY_NON_NULL(TAG, outPayload, ERROR);
    cbor_encoder_init(&encoder, outPayload, cborLen, 0);

    cborEncoderResult |= cbor_encoder_create_map(&encoder, &pstatMap, PSTAT_MAP_SIZE);
    VERIFY_CBOR_SUCCESS(TAG, cborEncoderResult, "Failed Adding Pstat Map.");

    cborEncoderResult |= cbor_encode_text_string(&pstatMap, OIC_JSON_ISOP_NAME,
        strlen(OIC_JSON_ISOP_NAME));
    VERIFY_CBOR_SUCCESS(TAG, cborEncoderResult, "Failed Adding ISOP Name Tag.");
    cborEncoderResult |= cbor_encode_boolean(&pstatMap, pstat->isOp);
    VERIFY_CBOR_SUCCESS(TAG, cborEncoderResult, "Failed Adding ISOP Name Value.");

    cborEncoderResult |= cbor_encode_text_string(&pstatMap, OIC_JSON_DEVICE_ID_NAME,
        strlen(OIC_JSON_DEVICE_ID_NAME));
    VERIFY_CBOR_SUCCESS(TAG, cborEncoderResult, "Failed Adding Device Id Tag.");
    cborEncoderResult |= cbor_encode_byte_string(&pstatMap, (uint8_t *)pstat->deviceID.id,
                                                sizeof(pstat->deviceID.id));
    VERIFY_CBOR_SUCCESS(TAG, cborEncoderResult, "Failed Adding Device Id Value.");

    cborEncoderResult |= cbor_encode_text_string(&pstatMap, OIC_JSON_COMMIT_HASH_NAME,
        strlen(OIC_JSON_COMMIT_HASH_NAME));
    VERIFY_CBOR_SUCCESS(TAG, cborEncoderResult, "Failed Adding Commit Hash Tag.");
    cborEncoderResult |= cbor_encode_int(&pstatMap, pstat->commitHash);
    VERIFY_CBOR_SUCCESS(TAG, cborEncoderResult, "Failed Adding Commit Hash Value.");

    cborEncoderResult |= cbor_encode_text_string(&pstatMap, OIC_JSON_CM_NAME,
        strlen(OIC_JSON_CM_NAME));
    VERIFY_CBOR_SUCCESS(TAG, cborEncoderResult, "Failed Adding CM Name Tag.");
    cborEncoderResult |= cbor_encode_int(&pstatMap, pstat->cm);
    VERIFY_CBOR_SUCCESS(TAG, cborEncoderResult, "Failed Adding CM Name Value.");

    cborEncoderResult |= cbor_encode_text_string(&pstatMap, OIC_JSON_TM_NAME,
        strlen(OIC_JSON_TM_NAME));
    VERIFY_CBOR_SUCCESS(TAG, cborEncoderResult, "Failed Adding TM Name Tag.");
    cborEncoderResult |= cbor_encode_int(&pstatMap, pstat->tm);
    VERIFY_CBOR_SUCCESS(TAG, cborEncoderResult, "Failed Adding TM Name Value.");

    cborEncoderResult |= cbor_encode_text_string(&pstatMap, OIC_JSON_OM_NAME,
        strlen(OIC_JSON_OM_NAME));
    VERIFY_CBOR_SUCCESS(TAG, cborEncoderResult, "Failed Adding OM Name Tag.");
    cborEncoderResult |= cbor_encode_int(&pstatMap, pstat->om);
    VERIFY_CBOR_SUCCESS(TAG, cborEncoderResult, "Failed Adding OM Name Value.");

    cborEncoderResult |= cbor_encode_text_string(&pstatMap, OIC_JSON_SM_NAME,
        strlen(OIC_JSON_SM_NAME));
    VERIFY_CBOR_SUCCESS(TAG, cborEncoderResult, "Failed Adding SM Name Tag.");
    {
        CborEncoder sm;
        cborEncoderResult |= cbor_encoder_create_array(&pstatMap, &sm, pstat->smLen);
        VERIFY_CBOR_SUCCESS(TAG, cborEncoderResult, "Failed Adding SM Array.");

        for (size_t i = 0; i < pstat->smLen; i++)
        {
            cborEncoderResult |= cbor_encode_int(&sm, pstat->sm[i]);
            VERIFY_CBOR_SUCCESS(TAG, cborEncoderResult, "Failed Adding SM Value in Array.");
        }
        cborEncoderResult |= cbor_encoder_close_container(&pstatMap, &sm);
        VERIFY_CBOR_SUCCESS(TAG, cborEncoderResult, "Failed Closing SM Array.");
    }
    cborEncoderResult |= cbor_encoder_close_container(&encoder, &pstatMap);
    VERIFY_CBOR_SUCCESS(TAG, cborEncoderResult, "Failed Adding Closing PSTAT Map.");

    if (CborNoError == cborEncoderResult)
    {
        *size = encoder.ptr - outPayload;
        *payload = outPayload;
        ret = OC_STACK_OK;
    }
exit:
    if ((CborErrorOutOfMemory == cborEncoderResult) && (cborLen < CBOR_MAX_SIZE))
    {
        // reallocate and try again!
        OICFree(outPayload);
        // Since the allocated initial memory failed, double the memory.
        cborLen += encoder.ptr - encoder.end;
        cborEncoderResult = CborNoError;
        ret = PstatToCBORPayload(pstat, payload, &cborLen);
        if (OC_STACK_OK == ret)
        {
            *size = cborLen;
        }
    }

    if ((CborNoError != cborEncoderResult) || (OC_STACK_OK != ret))
    {
        OICFree(outPayload);
        outPayload = NULL;
        *payload = NULL;
        *size = 0;
        ret = OC_STACK_ERROR;
    }

    return ret;
}

OCStackResult CBORPayloadToPstat(const uint8_t *cborPayload, const size_t size,
                                 OicSecPstat_t **secPstat)
{
    if (NULL == cborPayload || NULL == secPstat || NULL != *secPstat)
    {
        return OC_STACK_INVALID_PARAM;
    }

    OCStackResult ret = OC_STACK_ERROR;
    *secPstat = NULL;

    CborValue pstatCbor;
    CborParser parser;
    CborError cborFindResult = CborNoError;
    int cborLen = size;
    size_t len = 0;
    if (0 == size)
    {
        cborLen = CBOR_SIZE;
    }
    cbor_parser_init(cborPayload, cborLen, 0, &parser, &pstatCbor);
    CborValue pstatMap;

    OicSecPstat_t *pstat = NULL;
    cborFindResult = cbor_value_enter_container(&pstatCbor, &pstatMap);
    VERIFY_CBOR_SUCCESS(TAG, cborFindResult, "Failed Finding PSTAT Map.");

    pstat = (OicSecPstat_t *)OICCalloc(1, sizeof(OicSecPstat_t));
    VERIFY_NON_NULL(TAG, pstat, ERROR);

    cborFindResult = cbor_value_map_find_value(&pstatCbor, OIC_JSON_ISOP_NAME, &pstatMap);
    if (CborNoError == cborFindResult && cbor_value_is_boolean(&pstatMap))
    {
        cborFindResult = cbor_value_get_boolean(&pstatMap, &pstat->isOp);
        VERIFY_CBOR_SUCCESS(TAG, cborFindResult, "Failed Finding isOp Value.");
    }

    cborFindResult = cbor_value_map_find_value(&pstatCbor, OIC_JSON_DEVICE_ID_NAME, &pstatMap);
    if (CborNoError == cborFindResult && cbor_value_is_byte_string(&pstatMap))
    {
        uint8_t *subjectId = NULL;
        cborFindResult = cbor_value_dup_byte_string(&pstatMap, &subjectId, &len, NULL);
        VERIFY_CBOR_SUCCESS(TAG, cborFindResult, "Failed Finding subjectId Value.");
        memcpy(pstat->deviceID.id, subjectId, len);
        OICFree(subjectId);
    }

    cborFindResult = cbor_value_map_find_value(&pstatCbor, OIC_JSON_COMMIT_HASH_NAME, &pstatMap);
    if (CborNoError == cborFindResult && cbor_value_is_integer(&pstatMap))
    {
        cborFindResult = cbor_value_get_int(&pstatMap, (int *) &pstat->commitHash);
        VERIFY_CBOR_SUCCESS(TAG, cborFindResult, "Failed Finding commitHash.");
    }

    cborFindResult = cbor_value_map_find_value(&pstatCbor, OIC_JSON_CM_NAME, &pstatMap);
    if (CborNoError == cborFindResult && cbor_value_is_integer(&pstatMap))
    {
        cborFindResult = cbor_value_get_int(&pstatMap, (int *) &pstat->cm);
        VERIFY_CBOR_SUCCESS(TAG, cborFindResult, "Failed Finding CM.");
    }

    cborFindResult = cbor_value_map_find_value(&pstatCbor, OIC_JSON_OM_NAME, &pstatMap);
    if (CborNoError == cborFindResult && cbor_value_is_integer(&pstatMap))
    {
        cborFindResult = cbor_value_get_int(&pstatMap, (int *) &pstat->om);
        VERIFY_CBOR_SUCCESS(TAG, cborFindResult, "Failed Finding OM.");
    }

    cborFindResult = cbor_value_map_find_value(&pstatCbor, OIC_JSON_SM_NAME, &pstatMap);
    if (CborNoError == cborFindResult && cbor_value_is_array(&pstatMap))
    {
        CborValue sm = { .parser = NULL };
        cborFindResult = cbor_value_get_array_length(&pstatMap, &pstat->smLen);
        VERIFY_CBOR_SUCCESS(TAG, cborFindResult, "Failed Finding Array Len.");

        pstat->sm = (OicSecDpom_t *)OICCalloc(pstat->smLen, sizeof(OicSecDpom_t));
        VERIFY_NON_NULL(TAG, pstat->sm, ERROR);

        cborFindResult = cbor_value_enter_container(&pstatMap, &sm);
        VERIFY_CBOR_SUCCESS(TAG, cborFindResult, "Failed Entering SM.");

        int i = 0;
        while (cbor_value_is_valid(&sm))
        {
            cborFindResult = cbor_value_get_int(&sm, (int *)&pstat->sm[i++]);
            VERIFY_CBOR_SUCCESS(TAG, cborFindResult, "Failed Finding SM.");
            cborFindResult = cbor_value_advance(&sm);
            VERIFY_CBOR_SUCCESS(TAG, cborFindResult, "Failed Closing SM.");
        }
    }

    *secPstat = pstat;
    ret = OC_STACK_OK;

exit:
    if (CborNoError != cborFindResult)
    {
        OIC_LOG(ERROR, TAG, "CBORPayloadToPstat failed");
        DeletePstatBinData(pstat);
        pstat = NULL;
        ret = OC_STACK_ERROR;
    }

    return ret;
}

/**
 * Function to update persistent storage
 */
static bool UpdatePersistentStorage(OicSecPstat_t *pstat)
{
    bool bRet = false;

    size_t size = 0;
    uint8_t *cborPayload = NULL;
    OCStackResult ret = PstatToCBORPayload(pstat, &cborPayload, &size);
    if (OC_STACK_OK == ret)
    {
        if (OC_STACK_OK == UpdateSecureResourceInPS(OIC_JSON_PSTAT_NAME, cborPayload, size))
        {
            bRet = true;
        }
        OICFree(cborPayload);
    }

    return bRet;
}


/**
 * The entity handler determines how to process a GET request.
 */
static OCEntityHandlerResult HandlePstatGetRequest (const OCEntityHandlerRequest * ehRequest)
{
    OIC_LOG(INFO, TAG, "HandlePstatGetRequest  processing GET request");

    // Convert ACL data into CBOR for transmission
    size_t size = 0;
    uint8_t *payload = NULL;
    OCStackResult res = PstatToCBORPayload(gPstat, &payload, &size);

    // A device should always have a default pstat. Therefore, payload should never be NULL.
    OCEntityHandlerResult ehRet = (res == OC_STACK_OK) ? OC_EH_OK : OC_EH_ERROR;

    // Send response payload to request originator
    SendSRMCBORResponse(ehRequest, ehRet, payload, size);
    OICFree(payload);
    return ehRet;
}

/**
 * The entity handler determines how to process a POST request.
 * Per the REST paradigm, POST can also be used to update representation of existing
 * resource or create a new resource.
 * For pstat, it updates only tm and om.
 */
static OCEntityHandlerResult HandlePstatPutRequest(const OCEntityHandlerRequest *ehRequest)
{
    OCEntityHandlerResult ehRet = OC_EH_ERROR;
    OIC_LOG(INFO, TAG, "HandlePstatPutRequest  processing PUT request");
    OicSecPstat_t *pstat = NULL;

    if (ehRequest->resource)
    {
        uint8_t *payload = ((OCSecurityPayload *) ehRequest->payload)->securityData1;
        size_t size = ((OCSecurityPayload *) ehRequest->payload)->payloadSize;
        VERIFY_NON_NULL(TAG, payload, ERROR);

        OCStackResult ret = CBORPayloadToPstat(payload, size, &pstat);
        OICFree(payload);
        VERIFY_NON_NULL(TAG, pstat, ERROR);
        if (OC_STACK_OK == ret)
        {
            if (pstat->tm)
            {
                gPstat->tm = pstat->tm;
                if(0 == pstat->tm && gPstat->commitHash == pstat->commitHash)
                {
                    gPstat->isOp = true;
                    gPstat->cm = NORMAL;
                    OIC_LOG (INFO, TAG, "CommitHash is valid and isOp is TRUE");
                }
                else
                {
                    OIC_LOG(DEBUG, TAG, "CommitHash is not valid");
                }
            }
            if (pstat->om && gPstat)
            {
                /*
                 * Check if the operation mode is in the supported provisioning services
                 * operation mode list.
                 */
                for (size_t i=0; i< gPstat->smLen; i++)
                {
                    if(gPstat->sm[i] == pstat->om)
                    {
                        gPstat->om = pstat->om;
                        break;
                    }
                }
            }
            // Convert pstat data into CBOR for update to persistent storage
            if (UpdatePersistentStorage(gPstat))
            {
                ehRet = OC_EH_OK;
            }
        }
    }
 exit:
    if(OC_EH_OK != ehRet)
    {
        /*
          * If some error is occured while ownership transfer,
          * ownership transfer related resource should be revert back to initial status.
          */
        RestoreDoxmToInitState();
        RestorePstatToInitState();
    }

    //Send payload to request originator
    if(OC_STACK_OK != SendSRMCBORResponse(ehRequest, ehRet, NULL, 0))
    {
        OIC_LOG (ERROR, TAG, "SendSRMResponse failed in HandlePstatPostRequest");
    }
    DeletePstatBinData(pstat);
    return ehRet;
}

/**
 * This internal method is the entity handler for pstat resources.
 */
 OCEntityHandlerResult PstatEntityHandler(OCEntityHandlerFlag flag,
                                          OCEntityHandlerRequest * ehRequest,
                                          void *callbackParam)
{
    (void)callbackParam;
    OCEntityHandlerResult ehRet = OC_EH_ERROR;
    // This method will handle REST request (GET/POST) for /oic/sec/pstat
    if (flag & OC_REQUEST_FLAG)
    {
        OIC_LOG(INFO, TAG, "Flag includes OC_REQUEST_FLAG");
        switch (ehRequest->method)
        {
            case OC_REST_GET:
                ehRet = HandlePstatGetRequest(ehRequest);
                break;
            case OC_REST_PUT:
                ehRet = HandlePstatPutRequest(ehRequest);
                break;
            default:
                ehRet = OC_EH_ERROR;
                SendSRMCBORResponse(ehRequest, ehRet, NULL, 0);
                break;
        }
    }
    return ehRet;
}

/**
 * This internal method is used to create '/oic/sec/pstat' resource.
 */
 OCStackResult CreatePstatResource()
{
    OCStackResult ret = OCCreateResource(&gPstatHandle,
                                         OIC_RSRC_TYPE_SEC_PSTAT,
                                         OIC_MI_DEF,
                                         OIC_RSRC_PSTAT_URI,
                                         PstatEntityHandler,
                                         NULL,
                                         OC_RES_PROP_NONE);

    if (OC_STACK_OK != ret)
    {
        OIC_LOG(FATAL, TAG, "Unable to instantiate pstat resource");
        DeInitPstatResource();
    }
    return ret;
}

/**
 * Get the default value.
 *
 * @return the gDefaultPstat pointer.
 */
static OicSecPstat_t* GetPstatDefault()
{
    return &gDefaultPstat;
}

OCStackResult InitPstatResource()
{
    OCStackResult ret = OC_STACK_ERROR;

    // Read Pstat resource from PS
    uint8_t *data = NULL;
    size_t size = 0;
    ret = GetSecureVirtualDatabaseFromPS(OIC_JSON_PSTAT_NAME, &data, &size);
    // If database read failed
    if (OC_STACK_OK != ret)
    {
        OIC_LOG (DEBUG, TAG, "ReadSVDataFromPS failed");
    }
    if (data)
    {
        // Read ACL resource from PS
        ret = CBORPayloadToPstat(data, size, &gPstat);
    }
    /*
     * If SVR database in persistent storage got corrupted or
     * is not available for some reason, a default pstat is created
     * which allows user to initiate pstat provisioning again.
     */
    if ((OC_STACK_OK != ret) || !data || !gPstat)
    {
        gPstat = GetPstatDefault();
    }
    VERIFY_NON_NULL(TAG, gPstat, FATAL);

    // Instantiate 'oic.sec.pstat'
    ret = CreatePstatResource();

exit:
    OICFree(data);
    if (OC_STACK_OK != ret)
    {
        DeInitPstatResource();
    }
    return ret;
}

OCStackResult DeInitPstatResource()
{
    if (gPstat != &gDefaultPstat)
    {
        DeletePstatBinData(gPstat);
        gPstat = NULL;
    }
    return OCDeleteResource(gPstatHandle);
}

/**
 * Function to restore pstat resurce to initial status.
 * This function will use in case of error while ownership transfer
 */
void RestorePstatToInitState()
{
    if(gPstat)
    {
        OIC_LOG(INFO, TAG, "PSTAT resource will revert back to initial status.");

        gPstat->cm = NORMAL;
        gPstat->tm = NORMAL;
        gPstat->om = SINGLE_SERVICE_CLIENT_DRIVEN;
        if(gPstat->sm && 0 < gPstat->smLen)
        {
            gPstat->sm[0] = SINGLE_SERVICE_CLIENT_DRIVEN;
        }

        if (!UpdatePersistentStorage(gPstat))
        {
            OIC_LOG(ERROR, TAG, "Failed to revert DOXM in persistent storage");
        }
    }
}
