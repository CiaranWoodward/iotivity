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

#ifndef IOTVT_SRM_PSTATR_H
#define IOTVT_SRM_PSTATR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize Pstat resource by loading data from persistent storage.
 *
 * @return ::OC_STACK_OK for Success, otherwise some error value.
 */
OCStackResult InitPstatResource();

/**
 * Perform cleanup for Pstat resources.
 *
 * @return ::OC_STACK_OK for Success, otherwise some error value.
 */
OCStackResult DeInitPstatResource();

/**
 * This method converts PSTAT into the cbor payload.
 *
 * @param pstat pointer to the initialized pstat structure.
 * @param cborPayload pointer to pstat cbor payload.
 * @param size of the cbor payload converted. It is 0 in case of error,
 * else a positive value if succcessful.
 *
 * @return ::OC_STACK_OK for Success, otherwise some error value.
 */
 OCStackResult PstatToCBORPayload(const OicSecPstat_t *pstat, uint8_t **cborPayload,
                                  size_t *cborSize);

/**
 * This method converts cbor into PSTAT data.
 *
 * @param cborPayload is the pstat data in cbor format.
 * @param size of the cborPayload. In case 0 is provided it assigns CBOR_SIZE (255) value.
 * @param pstat pointer to @ref OicSecPstat_t.
  *
  * @return ::OC_STACK_OK for Success, otherwise some error value.
 */
 OCStackResult CBORPayloadToPstat(const uint8_t *cborPayload, const size_t cborSize,
                                  OicSecPstat_t **pstat);

/** This function deallocates the memory for OicSecPstat_t.
 *
 * @param pstat is the pointer to @ref OicSecPstat_t.
 */
void DeletePstatBinData(OicSecPstat_t* pstat);

#ifdef __cplusplus
}
#endif

#endif //IOTVT_SRM_PSTATR_H
