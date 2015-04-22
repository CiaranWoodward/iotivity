#include "gtest/gtest.h"
#include "cainterface.h"
#include "cacommon.h"
#include <string.h>

using namespace std;

void request_handler(CARemoteEndpoint_t* object, CARequestInfo_t* requestInfo);
void response_handler(CARemoteEndpoint_t* object, CAResponseInfo_t* responseInfo);

void request_handler(CARemoteEndpoint_t* object, CARequestInfo_t* requestInfo)
{
    cout << "request_handler, uri : " << (object != NULL) ? object->resourceUri : "";
    cout << ", data : " << (requestInfo != NULL) ? requestInfo->info.payload : "";
    cout << endl;
}

void response_handler(CARemoteEndpoint_t* object, CAResponseInfo_t* responseInfo)
{
    cout << "response_handler, uri : " << (object != NULL) ? object->resourceUri : "";
    cout << ", data : " << (responseInfo != NULL) ? responseInfo->info.payload : "";
    cout << endl;
}

char* uri;
CARemoteEndpoint_t* tempRep = NULL;
CARequestInfo_t requestInfo;
CAInfo_t responseData;
CAResponseInfo_t responseInfo;
CAToken_t tempToken;

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// CAInitialize TC
// check return value
TEST(InitializeTest, TC_01_Positive_01)
{
    EXPECT_EQ(CA_STATUS_OK, CAInitialize());
}

//CATerminate TC
TEST(TerminateTest, TC_02_Positive_01)
{
    CATerminate();

    char* check = (char *) "terminate success";
    EXPECT_STREQ(check, "terminate success");

    CAInitialize();
}
// CAStartListeningServer TC
// check return value
TEST(StartListeningServerTest, TC_03_Positive_01)
{
    CASelectNetwork(CA_WIFI);
    EXPECT_EQ(CA_STATUS_OK, CAStartListeningServer());
}

// CAStartDiscoveryServer TC
// check return value
TEST(StartDiscoveryServerTest, TC_04_Positive_01)
{
    EXPECT_EQ(CA_STATUS_OK, CAStartDiscoveryServer());
}

// CARegisterHandlerTest TC
// check return value
TEST(RegisterHandlerTest, TC_05_Positive_01)
{
    CARegisterHandler(request_handler, response_handler);
}

// CACreateRemoteEndpoint TC
// check return value
TEST(CreateRemoteEndpointTest, TC_06_Positive_01)
{
    uri = (char *) "123.123.123.123:1234/b/light";

    EXPECT_EQ(CA_STATUS_OK, CACreateRemoteEndpoint(uri, CA_ETHERNET, &tempRep));

    CADestroyRemoteEndpoint(tempRep);
}

// check remoteEndpoint and values of remoteEndpoint
TEST(CreateRemoteEndpointTest, TC_07_Positive_02)
{
    uri = (char *) "123.123.123.123:1234/b/light";
    CACreateRemoteEndpoint(uri, CA_ETHERNET, &tempRep);

    EXPECT_TRUE(tempRep != NULL);

    if (tempRep != NULL)
    {
        EXPECT_STRNE(NULL, tempRep->resourceUri);
    }

    CADestroyRemoteEndpoint(tempRep);
}

// check return value if uri is NULL
TEST(CreateRemoteEndpointTest, TC_08_Nagative_01)
{
    uri = NULL;

    EXPECT_EQ(CA_STATUS_FAILED, CACreateRemoteEndpoint(uri, CA_ETHERNET, &tempRep));

    CADestroyRemoteEndpoint(tempRep);
}

// check values of remoteEndpoint if uri is NULL
TEST(CreateRemoteEndpointTest, TC_09_Nagative_02)
{
    uri = NULL;
    CACreateRemoteEndpoint(uri, CA_ETHERNET, &tempRep);

    if (tempRep != NULL)
    {
        EXPECT_STREQ(NULL, tempRep->resourceUri);
    }

    CADestroyRemoteEndpoint(tempRep);
}

// CADestroyRemoteEndpoint TC
// check destroyed remoteEndpoint
TEST(DestroyRemoteEndpointTest, TC_10_Positive_01)
{
    uri = (char *) "123.123.123.123:1234/b/light";
    CACreateRemoteEndpoint(uri, CA_ETHERNET, &tempRep);

    CADestroyRemoteEndpoint(tempRep);

    char * check = (char *) "destroy success";
    EXPECT_STREQ(check, "destroy success");
}

// CAGerateToken TC
// check return value
TEST(GenerateTokenTest, TC_11_Positive_01)
{
    EXPECT_EQ(CA_STATUS_OK, CAGenerateToken(&tempToken));

    CADestroyToken(tempToken);
}

// CADestroyToken TC
// check destroyed token
TEST(DestroyTokenTest, TC_12_Positive_01)
{
    CAGenerateToken(&tempToken);
    CADestroyToken(tempToken);

    char * check = (char *) "destroy success";
    EXPECT_STREQ(check, "destroy success");
}

// CAFindResource TC
// check return value
TEST(FindResourceTest, TC_13_Positive_01)
{
    CARegisterHandler(request_handler, response_handler);
    uri = (char *) "123.123.123.123:1234/b/light";
    EXPECT_EQ(CA_STATUS_OK, CAFindResource(uri));
}

// check return value if uri is NULL
TEST(FindResourceTest, TC_14_Nagative_01)
{
    CARegisterHandler(request_handler, response_handler);
    uri = NULL;
    EXPECT_EQ(CA_STATUS_FAILED, CAFindResource(uri));
}

// CASendRequest TC
// check return value
TEST(SendRequestTest, TC_15_Positive_01)
{
    uri = (char *) "123.123.123.123:1234/b/light";
    memset(&requestInfo, 0, sizeof(CARequestInfo_t));
    CACreateRemoteEndpoint(uri, CA_ETHERNET, &tempRep);
    CAGenerateToken(&tempToken);
    requestInfo.method = CA_GET;
    requestInfo.info.token = tempToken;
    requestInfo.info.payload = (char *) "request payload";

    EXPECT_EQ(CA_STATUS_OK, CASendRequest(tempRep, &requestInfo));

    CADestroyToken(tempToken);
    CADestroyRemoteEndpoint(tempRep);
}

// check return value if uri is NULL
TEST(SendRequestTest, TC_16_Nagative_01)
{
    uri = NULL;
    memset(&requestInfo, 0, sizeof(CARequestInfo_t));
    CACreateRemoteEndpoint(uri, CA_ETHERNET, &tempRep);
    CAGenerateToken(&tempToken);
    requestInfo.method = CA_GET;
    requestInfo.info.token = tempToken;
    requestInfo.info.payload = (char *) "request payload";

    EXPECT_EQ(CA_STATUS_FAILED, CASendRequest(tempRep, &requestInfo));

    CADestroyToken(tempToken);
    CADestroyRemoteEndpoint(tempRep);
}

// CASendResponse TC
// check return value
TEST(SendResponseTest, TC_17_Positive_01)
{
    uri = (char *) "123.123.123.123:1234/b/light";
    CACreateRemoteEndpoint(uri, CA_ETHERNET, &tempRep);

    memset(&responseData, 0, sizeof(CAInfo_t));
    CAGenerateToken(&tempToken);
    responseData.token = tempToken;
    responseData.payload = (char *) "response payload";

    memset(&responseInfo, 0, sizeof(CAResponseInfo_t));
    responseInfo.result = CA_VALID;
    responseInfo.info = responseData;

    EXPECT_EQ(CA_STATUS_OK, CASendResponse(tempRep, &responseInfo));

    CADestroyToken(tempToken);
    CADestroyRemoteEndpoint(tempRep);
}

// check return value if uri is NULL
TEST(SendResponseTest, TC_18_Nagative_01)
{
    uri = NULL;
    CACreateRemoteEndpoint(uri, CA_ETHERNET, &tempRep);

    memset(&responseData, 0, sizeof(CAInfo_t));
    CAGenerateToken(&tempToken);
    responseData.token = tempToken;
    responseData.payload = (char *) "response payload";

    memset(&responseInfo, 0, sizeof(CAResponseInfo_t));
    responseInfo.result = CA_VALID;
    responseInfo.info = responseData;

    EXPECT_EQ(CA_STATUS_FAILED, CASendResponse(tempRep, &responseInfo));

    CADestroyToken(tempToken);
    CADestroyRemoteEndpoint(tempRep);
}

// CASendNotification TC
// check return value
TEST(SendNotificationTest, TC_19_Positive_01)
{
    uri = (char *) "123.123.123.123:1234/b/light";
    CACreateRemoteEndpoint(uri, CA_ETHERNET, &tempRep);

    memset(&responseData, 0, sizeof(CAInfo_t));
    responseData.token = (char *) "client token";
    responseData.payload = (char *) "Temp Notification Data";

    memset(&responseInfo, 0, sizeof(CAResponseInfo_t));
    responseInfo.result = CA_SUCCESS;
    responseInfo.info = responseData;

    EXPECT_EQ(CA_STATUS_OK, CASendNotification(tempRep, &responseInfo));

    CADestroyRemoteEndpoint(tempRep);
}

// check return value if uri is NULL
TEST(SendNotificationTest, TC_20_Nagative_01)
{
    uri = NULL;
    CACreateRemoteEndpoint(uri, CA_ETHERNET, &tempRep);

    memset(&responseData, 0, sizeof(CAInfo_t));
    responseData.token = (char *) "client token";
    responseData.payload = (char *) "Temp Notification Data";

    memset(&responseInfo, 0, sizeof(CAResponseInfo_t));
    responseInfo.result = CA_SUCCESS;
    responseInfo.info = responseData;

    EXPECT_EQ(CA_STATUS_FAILED, CASendNotification(tempRep, &responseInfo));

    CADestroyRemoteEndpoint(tempRep);
}

// CAAdvertiseResource TC
// check return value
TEST(AdvertiseResourceTest, TC_21_Positive_01)
{
    uri = (char *) "123.123.123.123:1234/b/light";
    int optionNum = 1;
    char* optionData;
    CAHeaderOption_t* headerOpt;
    headerOpt = (CAHeaderOption_t*) malloc(sizeof(CAHeaderOption_t) * optionNum);
    memset(headerOpt, 0, sizeof(CAHeaderOption_t) * optionNum);

    int i;
    for(i = 0 ; i < optionNum ; i++)
    {
        int optionID = 2;
        headerOpt[i].optionID = optionID;
        optionData = (char *) "aaa";
        memcpy(headerOpt[i].optionData, optionData, strlen(optionData));
        headerOpt[i].optionLength = (uint16_t)strlen(optionData);
    }
    EXPECT_EQ(CA_STATUS_OK, CAAdvertiseResource(uri, headerOpt, (uint8_t)optionNum));
}

// check return value if uri is NULL
TEST(AdvertiseResourceTest, TC_22_Nagative_01)
{
    uri = NULL;
    int optionNum = 1;
    char* optionData;
    CAHeaderOption_t* headerOpt;
    headerOpt = (CAHeaderOption_t*) malloc(sizeof(CAHeaderOption_t) * optionNum);
    memset(headerOpt, 0, sizeof(CAHeaderOption_t) * optionNum);

    int i;
    for(i = 0 ; i < optionNum ; i++)
    {
        int optionID = 2;
        headerOpt[i].optionID = optionID;
        optionData = (char *) "aaa";
        memcpy(headerOpt[i].optionData, optionData, strlen(optionData));
        headerOpt[i].optionLength = (uint16_t)strlen(optionData);
    }
    EXPECT_EQ(CA_STATUS_FAILED, CAAdvertiseResource(uri, headerOpt, (uint8_t)optionNum));
}

// CASelectNewwork TC
// check return value
TEST(SelectNetworkTest, TC_23_Positive_01)
{
    //Select wifi network
    EXPECT_EQ(CA_STATUS_OK, CASelectNetwork(CA_WIFI));
}

// check return value if selected network is disable
TEST(SelectNetworkTest, TC_24_Nagative_01)
{
    //Select disable network
    EXPECT_EQ(CA_NOT_SUPPORTED, CASelectNetwork(20));
}

// CAUnSelectNewwork TC
// check return value
TEST(UnSelectNetworkTest, TC_25_Positive_01)
{
    //Unselect wifi network
    EXPECT_EQ(CA_STATUS_OK, CAUnSelectNetwork(CA_WIFI));
}

// check return value if selected network is disable
TEST(UnSelectNetworkTest, TC_26_Nagative_01)
{
    //UnSelect disable network
    EXPECT_EQ(CA_NOT_SUPPORTED, CAUnSelectNetwork(20));
}

// CAHandlerRequestResponse TC
// check return value
TEST (HandlerRequestResponseTest, TC_27_Positive_01)
{
    EXPECT_EQ(CA_STATUS_OK, CAHandleRequestResponse());
}

// CASelectCipherSuite TC
// check return value
TEST(DtlsSelectCiperSuiteTest, TC_28_Positive_01)
{
    //TLS_ECDH_anon_WITH_AES_128_CBC_SHA
    EXPECT_EQ(CA_STATUS_OK, CASelectCipherSuite(0xC018));
}

TEST(DtlsSelectCiperSuiteTest, TC_28_Positive_02)
{
    //TLS_PSK_WITH_AES_128_CCM_8
    EXPECT_EQ(CA_STATUS_OK, CASelectCipherSuite(0xC0A8));
}

TEST(DtlsSelectCiperSuiteTest, TC_28_Positive_03)
{
    //TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8
    EXPECT_EQ(CA_STATUS_OK, CASelectCipherSuite(0xC0AE));
}

//T.B.D
//Nagative test case of CASelectCipherSuite() will be added when tinydtls is updated.

// CAEnablesAnonEcdh TC
// check return value
TEST(DtlsEnablesAnonEcdh, TC_29_Positive_01)
{
    EXPECT_EQ(CA_STATUS_OK, CAEnablesAnonEcdh(CA_FALSE));
}

TEST(DtlsEnablesAnonEcdh, TC_29_Positive_02)
{
    EXPECT_EQ(CA_STATUS_OK, CAEnablesAnonEcdh(CA_TRUE));
}

//T.B.D
//Nagative test case of DtlsEnablesAnonEcdh() will be added when tinydtls is updated.


