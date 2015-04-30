/* ****************************************************************
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

/**
 * @file
 *
 * This file contains the APIs for BT communications.
 */

#ifndef __CA_EDRUTILES_H_
#define __CA_EDRUTILES_H_

#include "cacommon.h"
#include "uthreadpool.h"
#include "uarraylist.h"
#include "jni.h"

#ifdef __cplusplus
extern "C"
{
#endif

jstring CAEDRNativeGetAddressFromDeviceSocket(JNIEnv *env, jobject bluetoothSocketObj);

jstring CAEDRNativeGetLocalDeviceAddress(JNIEnv *env);

jobjectArray CAEDRNativeGetBondedDevices(JNIEnv *env);

jint CAEDRNativeGetBTStateOnInfo(JNIEnv *env);

jboolean CAEDRNativeIsEnableBTAdapter(JNIEnv *env);

jstring CAEDRNativeGetAddressFromBTDevice(JNIEnv *env, jobject bluetoothDevice);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

