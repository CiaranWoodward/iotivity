//******************************************************************
//
// Copyright 2016 Samsung Electronics All Rights Reserved.
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

#include "JniNotificationProvider.h"
#include "NSProviderService.h"

static JavaVM *g_jvm = NULL;

static jobject g_obj_subscriptionListener = NULL;
static jobject g_obj_syncListener = NULL;

jclass g_cls_Message;
jclass g_cls_Message_Type;
jclass g_cls_Consumer;
jclass g_cls_SyncInfo;
jclass g_cls_SyncType;
jclass g_cls_MediaContents;

static JNIEnv *GetJNIEnv(jint *ret)
{
    JNIEnv *env = NULL;

    *ret = g_jvm->GetEnv((void **) &env, JNI_CURRENT_VERSION);
    switch (*ret)
    {
        case JNI_OK:
            return env;
        case JNI_EDETACHED:
            if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK)
            {
                LOGE ("Failed to get the environment");
                return NULL;
            }
            else
            {
                return env;
            }
        case JNI_EVERSION:
            LOGE ("JNI version is not supported");
        default:
            LOGE ("Failed to get the environment");
            return NULL;
    }
}

void onSubscribeListenerCb(OIC::Service::NSConsumer *consumer)
{
    LOGI("JNIProviderService_onSubscribeListenerCb - IN");

    jint envRet;
    JNIEnv *env = GetJNIEnv(&envRet);
    if (NULL == env) return ;

    jobject jSubscriptionListener = (jobject) env->NewLocalRef(g_obj_subscriptionListener);
    if (!jSubscriptionListener)
    {
        LOGE ("Failed to Get jSubscriptionListener");
        if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
        return ;
    }

    LOGI("consumer ID : %s\n", consumer->getConsumerId().c_str());

    jstring jConsumerId = env->NewStringUTF( consumer->getConsumerId().c_str());

    jclass cls_consumer = (jclass) (env->NewLocalRef(g_cls_Consumer));
    if (!cls_consumer)
    {
        LOGE ("Failed to Get ObjectClass for Consumer");
        if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
        return ;
    }

    jmethodID mid_consumer = env->GetMethodID(
                                 cls_consumer,
                                 "<init>",
                                 "(JLjava/lang/String;Lorg/iotivity/service/ns/provider/Consumer)V");
    if (!mid_consumer)
    {
        LOGE ("Failed to Get MethodID for Consumer<init>");
        if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
        return ;
    }
    jobject obj_consumer = env->NewObject( cls_consumer, mid_consumer, jConsumerId);

    jclass cls = env->GetObjectClass( jSubscriptionListener);
    if (!cls)
    {
        LOGE("Failed to Get ObjectClass of jSubscriptionListener");
        if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
        return;
    }
    jmethodID mid = env->GetMethodID(
                        cls,
                        "onConsumerSubscribed",
                        "(Lorg/iotivity/service/ns/provider/Consumer;)V");
    if (!mid)
    {
        LOGE("Failed to Get MethodID of onConsumerSubscribed");
        if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
        return;
    }

    env->CallVoidMethod( jSubscriptionListener, mid, obj_consumer);
    env->DeleteLocalRef(jSubscriptionListener);
    env->DeleteLocalRef(cls_consumer);
    if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
    LOGI("JNIProviderService_onSubscribeListenerCb - OUT");
    return;
}

void onSyncInfoListenerCb(OIC::Service::NSSyncInfo *sync)
{
    LOGI("JNIProviderService_onSyncInfoListenerCb - IN");

    jint envRet;
    JNIEnv *env = GetJNIEnv(&envRet);
    if (NULL == env) return ;

    jobject jSyncListener = (jobject) env->NewLocalRef(g_obj_syncListener);
    if (!jSyncListener)
    {
        LOGE ("Failed to Get jSyncListener");
        if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
        return ;
    }

    LOGI("Sync ID : %ld\n", (long) sync->getMessageId());
    LOGI("Sync STATE : %d\n", (int) sync->getState());

    jlong jMessageId = (long)  sync->getMessageId();
    jstring jProviderId = env->NewStringUTF(sync->getProviderId().c_str());
    jobject syncType;

    jclass cls_SyncType = (jclass) (env->NewLocalRef(g_cls_SyncType));
    if (!cls_SyncType)
    {
        LOGE ("Failed to Get ObjectClass for SyncType");
        if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
        return ;
    }
    switch (sync->getState())
    {
        case OIC::Service::NSSyncInfo::NSSyncType::NS_SYNC_UNREAD:
            {
                static jfieldID fieldID = env->GetStaticFieldID(cls_SyncType,
                                          "UNREAD", "Lorg/iotivity/service/ns/common/SyncInfo$SyncType;");
                syncType = env->GetStaticObjectField(cls_SyncType, fieldID);
            }
        case OIC::Service::NSSyncInfo::NSSyncType::NS_SYNC_READ :
            {
                static jfieldID fieldID = env->GetStaticFieldID(cls_SyncType,
                                          "READ", "Lorg/iotivity/service/ns/common/SyncInfo$SyncType;");
                syncType = env->GetStaticObjectField(cls_SyncType, fieldID);
            }
        case OIC::Service::NSSyncInfo::NSSyncType::NS_SYNC_DELETED :
            {
                static jfieldID fieldID = env->GetStaticFieldID(cls_SyncType,
                                          "DELETED", "Lorg/iotivity/service/ns/common/SyncInfo$SyncType;");
                syncType = env->GetStaticObjectField(cls_SyncType, fieldID);
            }

    }

    jclass cls_SyncInfo = (jclass) (env->NewLocalRef(g_cls_SyncInfo));
    if (!cls_SyncInfo)
    {
        LOGE ("Failed to Get ObjectClass for SyncInfo");
        if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
        return ;
    }
    jmethodID mid_syncInfo = env->GetMethodID(
                                 cls_SyncInfo,
                                 "<init>",
                                 "(JLjava/lang/String;Lorg/iotivity/service/ns/common/SyncInfo$SyncType)V");
    if (!mid_syncInfo)
    {
        LOGE ("Failed to Get MethodID for SyncInfo");
        if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
        return ;
    }

    jobject obj_syncInfo = env->NewObject( cls_SyncInfo, mid_syncInfo, jMessageId, jProviderId,
                                           syncType);

    jclass cls = env->GetObjectClass( jSyncListener);
    if (!cls)
    {
        LOGE("Failed to Get ObjectClass");
        if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
        return;
    }
    jmethodID mid = env->GetMethodID( cls, "onMessageSynchronized",
                                      "(Lorg/iotivity/service/ns/common/SyncInfo)V");
    if (!mid)
    {
        LOGE("Failed to Get MethodID");
        if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
        return;
    }
    env->CallVoidMethod( jSyncListener, mid, obj_syncInfo);

    env->DeleteLocalRef(jSyncListener);
    env->DeleteLocalRef(cls_SyncInfo);
    env->DeleteLocalRef(cls_SyncType);
    if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();

    LOGI("JNIProviderService: OnSyncInfoListenerCb - OUT");
    return;

}

OIC::Service::NSMessage *getMessage(JNIEnv *env, jobject jMsg)
{
    LOGI("JNIProviderService: getMessage - IN");

    jclass cls = env->GetObjectClass( jMsg);

    // Message type
    jclass cls_messageType = (jclass) (env->NewLocalRef(g_cls_Message_Type));
    if (!cls_messageType)
    {
        LOGE ("Failed to Get ObjectClass for Message Type");
        return nullptr;
    }
    jmethodID mid = env->GetMethodID(cls_messageType, "ordinal", "()I");
    jfieldID fid_type = env->GetFieldID( cls, "mType",
                                         "Lorg/iotivity/service/ns/common/Message$MessageType;");
    if (fid_type == NULL)
    {
        LOGE("Error: jfieldID for message type  is null");
        return nullptr;
    }
    jobject jobj = env->GetObjectField( jMsg, fid_type);
    if (jobj == NULL)
    {
        LOGE("Error: object of field  Message Type is null");
        return nullptr;
    }
    jint jtype = env->CallIntMethod(jobj, mid);
    OIC::Service::NSMessage::NSMessageType  type = (OIC::Service::NSMessage::NSMessageType) jtype;

    LOGI("Message Type: %ld\n", (long )type);

    // Message Time
    jfieldID fid_tm = env->GetFieldID( cls, "mTime", "Ljava/lang/String;");
    if (fid_tm == NULL)
    {
        LOGE("Error: jfieldID for message time is null");
        return nullptr;
    }
    jstring jtime = (jstring)env->GetObjectField( jMsg, fid_tm);
    const char *time = "";
    if (jtime)
    {
        time = env->GetStringUTFChars( jtime, NULL);
    }
    else
    {
        LOGI("Info: messageTitle is null");
    }
    LOGI("Message Time: %s\n", time);

    // Message TTL
    jfieldID fid_ttl = env->GetFieldID( cls, "mTTL", "J");
    if (fid_ttl == NULL)
    {
        LOGE("Error: jfieldID for message ttl is null");
        return nullptr;
    }
    jlong jttl = (jlong) env->GetObjectField( jMsg, fid_ttl);
    uint64_t  ttl = jttl;

    LOGI("Message ID: %lld\n", ttl);

    // Message Title
    jfieldID fid_title = env->GetFieldID( cls, "mTitle", "Ljava/lang/String;");
    if (fid_title == NULL)
    {
        LOGE("Error: jfieldID for message title is null");
        return nullptr;
    }
    jstring jmsgTitle = (jstring)env->GetObjectField( jMsg, fid_title);
    const char *messageTitle = "";
    if (jmsgTitle)
    {
        messageTitle = env->GetStringUTFChars( jmsgTitle, NULL);
    }
    else
    {
        LOGI("Info: messageTitle is null");
    }
    LOGI("Message Title: %s\n", messageTitle);

    // Message Content Text
    jfieldID fid_body = env->GetFieldID( cls, "mContentText", "Ljava/lang/String;");
    if (fid_body == NULL)
    {
        LOGE("Error: jfieldID for message context Text is null");
        return nullptr;
    }
    jstring jmsgBody = (jstring)env->GetObjectField( jMsg, fid_body);
    const char *messageBody = "";
    if (jmsgBody)
    {
        messageBody = env->GetStringUTFChars( jmsgBody, NULL);
    }
    else
    {
        LOGI("Info: messageBody is null");
    }
    LOGI("Message Body: %s\n", messageBody);

    // Message Source
    jfieldID fid_source = env->GetFieldID( cls, "mSourceName", "Ljava/lang/String;");
    if (fid_source == NULL)
    {
        LOGE("Error: jfieldID for message source is null");
        return nullptr;
    }
    jstring jmsgSource = (jstring)env->GetObjectField( jMsg, fid_source);
    const char *messageSource = "";
    if (jmsgSource)
    {
        messageSource = env->GetStringUTFChars( jmsgSource, NULL);
    }
    else
    {
        LOGI("Info: messageSource is null");
    }
    LOGI("Message Source: %s\n", messageSource);

    // Message MediaContents
    jfieldID fid_media = env->GetFieldID( cls, "mMediaContents",
                                          "Lorg/iotivity/service/ns/common/MediaContents;");
    if (fid_media == NULL)
    {
        LOGE("Error: jfieldID for MediaContents is null");
        return nullptr;
    }
    jobject jmedia = env->GetObjectField( jMsg, fid_media);
    if (jmedia == NULL)
    {
        LOGE("Error: jmedia object of MediaContents inside Message is null");
        return nullptr;
    }
    jclass cls_MediaContents = (jclass) (env->NewLocalRef(g_cls_MediaContents));
    if (!cls_MediaContents)
    {
        LOGE ("Failed to Get ObjectClass for class MediaContents");
        return nullptr;
    }
    jfieldID fid_icon = env->GetFieldID( cls_MediaContents, "mIconImage", "Ljava/lang/String;");
    if (fid_icon == NULL)
    {
        LOGE("Error: jfieldID for iconImage is null");
        return nullptr;
    }
    jstring jiconImage = (jstring)env->GetObjectField( jmedia, fid_icon);
    const char *iconImage = "";
    if (jiconImage)
    {
        iconImage = env->GetStringUTFChars( jiconImage, NULL);
    }
    else
    {
        LOGI("Info: iconImage is null");
    }

    LOGI("iconImage: %s\n", iconImage);

    OIC::Service::NSMediaContents *media = new OIC::Service::NSMediaContents(std::string(iconImage));
    OIC::Service::NSMessage *nsMsg = OIC::Service::NSProviderService::getInstance()->CreateMessage();

    nsMsg->setType(type);
    nsMsg->setTime(std::string(time));
    nsMsg->setTTL(ttl);
    nsMsg->setTitle(std::string(messageTitle));
    nsMsg->setContentText(std::string(messageBody));
    nsMsg->setSourceName(std::string(messageSource));
    nsMsg->setMediaContents(media);

    env->DeleteLocalRef(cls_messageType);
    env->DeleteLocalRef(cls_MediaContents);

     if (jtime)
    {
        env->ReleaseStringUTFChars(jtime, time);
    }
     if (jmsgTitle)
    {
        env->ReleaseStringUTFChars(jmsgTitle, messageTitle);
    }
     if (jmsgBody)
    {
        env->ReleaseStringUTFChars(jmsgBody, messageBody);
    }
     if (jmsgSource)
    {
        env->ReleaseStringUTFChars(jmsgSource, messageSource);
    }
    if (jiconImage)
    {
        env->ReleaseStringUTFChars(jiconImage, iconImage);
    }

    LOGI("JNIProviderService: getMessage - OUT");
    return nsMsg;

}

JNIEXPORT jint JNICALL Java_org_iotivity_service_ns_provider_ProviderService_nativeStart(
    JNIEnv *env, jobject jObj, jboolean jPolicy, jobject jSubscriptionListener,
    jobject jSyncListener)
{
    LOGI("JNIProviderService: nativeStart - IN");
    if (!jSubscriptionListener || !jSyncListener)
    {
        LOGE("Fail to set listeners");
        ThrowNSException(NS_ERROR, "Listener cannot be null");
        return (jint) OIC::Service::NSResult::ERROR;
    }

    if (g_obj_subscriptionListener != NULL)
    {
        env->DeleteGlobalRef(g_obj_subscriptionListener);
    }
    if (g_obj_syncListener != NULL)
    {
        env->DeleteGlobalRef(g_obj_syncListener);
    }

    g_obj_subscriptionListener = (jobject)  env->NewGlobalRef(jSubscriptionListener);
    g_obj_syncListener = (jobject)  env->NewGlobalRef(jSyncListener);

    // check access policy

    OIC::Service::NSProviderService::ProviderConfig cfg;
    cfg.m_subscribeRequestCb =  onSubscribeListenerCb;
    cfg.m_syncInfoCb =  onSyncInfoListenerCb;
    cfg.policy = (bool) jPolicy;

    OIC::Service::NSResult result = OIC::Service::NSProviderService::getInstance()->Start(cfg);
    if (result != OIC::Service::NSResult::OK)
    {
        LOGE("Fail to start NSProviderService");

    }

    LOGI("JNIProviderService: nativeStart - OUT");
    return (jint) result;
}

JNIEXPORT jint JNICALL Java_org_iotivity_service_ns_provider_ProviderService_nativeStop(
    JNIEnv *env, jobject jObj)
{
    LOGI("JNIProviderService: nativeStop - IN");

    OIC::Service::NSResult result = OIC::Service::NSProviderService::getInstance()->Stop();
    if (result !=  OIC::Service::NSResult::OK)
    {
        LOGI("Fail to stop NSProvider service");
        return (jint) result;
    }

    env->DeleteGlobalRef( g_obj_subscriptionListener);
    env->DeleteGlobalRef( g_obj_syncListener);
    g_obj_subscriptionListener = NULL;
    g_obj_syncListener = NULL;

    LOGI("JNIProviderService: nativeStop - OUT");
    return (jint) result;
}

JNIEXPORT jint JNICALL Java_org_iotivity_service_ns_provider_ProviderService_nativeSendMessage(
    JNIEnv *env, jobject jObj, jobject jMsg)
{
    LOGI("JNIProviderService: nativeSendMessage - IN");
    if (!jMsg)
    {
        LOGI("Fail to send notification - Message is null");
        ThrowNSException(NS_ERROR, "Message cannot be null");
        return (jint) OIC::Service::NSResult::ERROR;
    }
    OIC::Service::NSMessage *nsMsg = getMessage(env, jMsg);
    if (nsMsg == nullptr)
    {
        ThrowNSException(NS_ERROR, "Message didn't have a field ID ");
        return (jint) OIC::Service::NSResult::ERROR;
    }

    OIC::Service::NSResult result = OIC::Service::NSProviderService::getInstance()->SendMessage(nsMsg);
    if (result !=  OIC::Service::NSResult::OK)
    {
        LOGI("Fail to send NSProvider Message");
    }
    LOGI("JNIProviderService: nativeSendMessage - OUT");
    return (jint) result;
}

JNIEXPORT void JNICALL Java_org_iotivity_service_ns_provider_ProviderService_nativeSendSyncInfo(
    JNIEnv *env, jobject jObj, jlong messageId , jint syncState)
{
    LOGI("JNIProviderService: nativeSendSyncInfo - IN");
    OIC::Service::NSProviderService::getInstance()->SendSyncInfo( messageId,
            (OIC::Service::NSSyncInfo::NSSyncType) syncState);
    LOGI("JNIProviderService: nativeSendSyncInfo - OUT");
    return;
}

JNIEXPORT jint JNICALL
Java_org_iotivity_service_ns_provider_ProviderService_nativeEnableRemoteService(JNIEnv *env,
        jobject jObj, jstring jstr)
{
    LOGI("JNIProviderService: nativeEnableRemoteService - IN");
    if (!jstr)
    {
        ThrowNSException(NS_ERROR, "Server Address Can't be NULL");
        return (jint) OIC::Service::NSResult::ERROR;
    }

    const char *address = env->GetStringUTFChars( jstr, NULL);
    std::string servAddress(address);
    OIC::Service::NSResult result  = OIC::Service::NSProviderService::getInstance()->EnableRemoteService(
                                       servAddress);
    if (result !=  OIC::Service::NSResult::OK)
    {
        LOGE("Fail to Enable Remote Service");
    }
    env->ReleaseStringUTFChars(jstr, address);
    LOGI("JNIProviderService: nativeEnableRemoteService - OUT");
    return (jint) result;
}

JNIEXPORT jint JNICALL
Java_org_iotivity_service_ns_provider_ProviderService_nativeDisableRemoteService(JNIEnv *env,
        jobject jObj, jstring jstr)
{
    LOGI("JNIProviderService: nativeDisableRemoteService - IN");
    if (!jstr)
    {
        ThrowNSException(NS_ERROR, "Server Address Can't be NULL");
        return (jint) OIC::Service::NSResult::ERROR;
    }

    const char *address = env->GetStringUTFChars( jstr, NULL);
    std::string servAddress(address);
    OIC::Service::NSResult result  = OIC::Service::NSProviderService::getInstance()->DisableRemoteService(
                                       servAddress);
    if (result !=  OIC::Service::NSResult::OK)
    {
        LOGE("Fail to Disable Remote Service");
    }
    env->ReleaseStringUTFChars(jstr, address);
    LOGI("JNIProviderService: nativeDisableRemoteService - OUT");
    return (jint) result;
}

JNIEXPORT jint JNICALL Java_org_iotivity_service_ns_provider_Consumer_nativeAcceptSubscription(
    JNIEnv *env,
    jobject jObj, jobject jConsumer, jboolean jAccepted)
{
    LOGD("JNIProviderService: nativeAcceptSubscription - IN");

    jclass consumerClass =  env->GetObjectClass( jConsumer);
    if (!consumerClass)
    {
        ThrowNSException(NS_ERROR, "Failed to Get ObjectClass for Consumer");
        return (jint) OIC::Service::NSResult::ERROR;
    }

    // Consumer ID
    jfieldID fid_id =  env->GetFieldID(consumerClass, "mConsumerId",  "Ljava/lang/String;");
    if (fid_id == NULL)
    {
        LOGE("Error: jfieldID for mConsumerId  is null");
        ThrowNSException(NS_ERROR, "ConsumerId not found");
        return (jint) OIC::Service::NSResult::ERROR;
    }

    jstring jconId = (jstring)env->GetObjectField( jConsumer, fid_id);
    const char *conId = "";
    if (conId)
    {
        conId = env->GetStringUTFChars( jconId, NULL);
    }
    else
    {
        LOGI("Info: Consumer Id  is null");
    }
    std::string consumerId(conId);
    env->ReleaseStringUTFChars(jconId, conId);

    LOGI("Consumer ID: %s\n", consumerId.c_str());

    OIC::Service::NSConsumer *consumer = new OIC::Service::NSConsumer(consumerId);
    int result =  consumer->acceptSubscription(consumer,  (bool)jAccepted);
    if (jAccepted)
    {
        LOGI("Subscription Accepted");
    }
    else
    {
        LOGI("Subscription Denied");
    }

    LOGD("JNIProviderService: nativeAcceptSubscription - OUT");
    return result;
}

// JNI OnLoad
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    LOGD("ProviderService_JNI_OnLoad");
    g_jvm = jvm;

    JNIEnv *env;
    if (jvm->GetEnv((void **)&env, JNI_CURRENT_VERSION) != JNI_OK)
    {
        LOGE("Failed to get the environment using GetEnv()");
        return JNI_ERR;
    }

    jclass localMessage = env->FindClass(
                              "org/iotivity/service/ns/common/Message");
    if (!localMessage)
    {
        LOGE("Failed to get local Message class");
        return JNI_ERR;
    }
    g_cls_Message = (jclass) (env->NewGlobalRef(localMessage));
    if (!g_cls_Message)
    {
        LOGE("Failed to set Global Message reference");
        return JNI_ERR;
    }

    jclass localMessageType = env->FindClass(
                                  "org/iotivity/service/ns/common/Message$MessageType");
    if (!localMessageType)
    {
        LOGE("Failed to get local Message Type class");
        return JNI_ERR;
    }
    g_cls_Message_Type = (jclass) (env->NewGlobalRef(localMessageType));
    if (!g_cls_Message_Type)
    {
        LOGE("Failed to set Global Message Type reference");
        return JNI_ERR;
    }

    jclass localConsumer = env->FindClass(
                               "org/iotivity/service/ns/provider/Consumer");
    if (!localConsumer)
    {
        LOGE("Failed to get local Provider class");
        return JNI_ERR;
    }
    g_cls_Consumer = (jclass) (env->NewGlobalRef(localConsumer));
    if (!g_cls_Consumer)
    {
        LOGE("Failed to set Global Provider reference");
        return JNI_ERR;
    }

    jclass localSyncInfo = env->FindClass(
                               "org/iotivity/service/ns/common/SyncInfo");
    if (!localSyncInfo)
    {
        LOGE("Failed to get local SyncInfo class");
        return JNI_ERR;
    }
    g_cls_SyncInfo = (jclass) (env->NewGlobalRef(localSyncInfo));
    if (!g_cls_SyncInfo)
    {
        LOGE("Failed to set Global SyncInfo reference");
        return JNI_ERR;
    }

    jclass localSyncType = env->FindClass(
                               "org/iotivity/service/ns/common/SyncInfo$SyncType");
    if (!localSyncType)
    {
        LOGE("Failed to get local SyncType enum");
        return JNI_ERR;
    }
    g_cls_SyncType = (jclass) (env->NewGlobalRef(localSyncType));
    if (!g_cls_SyncType)
    {
        LOGE("Failed to set Global SyncType reference");
        return JNI_ERR;
    }

    jclass localMediaContents = env->FindClass(
                                    "org/iotivity/service/ns/common/MediaContents");
    if (!localMediaContents)
    {
        LOGE("Failed to get local MediaContents class");
        return JNI_ERR;
    }
    g_cls_MediaContents = (jclass) (env->NewGlobalRef(localMediaContents));
    if (!g_cls_MediaContents)
    {
        LOGE("Failed to set Global MediaContents reference");
        return JNI_ERR;
    }

    env->DeleteLocalRef(localMessage);
    env->DeleteLocalRef(localMessageType);
    env->DeleteLocalRef(localConsumer);
    env->DeleteLocalRef(localSyncInfo);
    env->DeleteLocalRef(localSyncType);
    env->DeleteLocalRef(localMediaContents);

    return NSExceptionInit(env);
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *jvm, void *reserved)
{
    LOGI("ProviderService_JNI_OnUnload");
    JNIEnv *env;

    if (jvm->GetEnv((void **)&env, JNI_CURRENT_VERSION) != JNI_OK)
    {
        LOGE("Failed to get the environment using GetEnv()");
        return ;
    }

    env->DeleteGlobalRef(g_cls_Message);
    env->DeleteGlobalRef(g_cls_Consumer);
    env->DeleteGlobalRef(g_cls_SyncInfo);
    env->DeleteGlobalRef(g_cls_SyncType);
    env->DeleteGlobalRef(g_cls_MediaContents);
    env->DeleteGlobalRef(g_cls_Message_Type);
}
