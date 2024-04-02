/******************************************************************************
*  Copyright 2022, Opulinks Technology Ltd.
*  ----------------------------------------------------------------------------
*  Statement:
*  ----------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Opulinks Technology Ltd. (C) 2022
******************************************************************************/

/******************************************************************************
*  Filename:
*  ---------
*  file_download.c
*
*  Project:
*  --------
*  
*
*  Description:
*  ------------
*  
*
*  Author:
*  -------
*  AE Team
*
******************************************************************************/
/***********************
Head Block of The File
***********************/
#include "qd_config.h"
#include "qd_module.h"
#if (HTTP_ENABLED == 1)
// Sec 0: Comment block of the file

// Sec 1: Include File

#include "cmsis_os.h"
#include "http_file.h"
#include "controller_wifi.h"
#include "controller_wifi_com.h"
#include "lwip/etharp.h"

#include "wifi_mngr_api.h"
#include "log.h"
#include "transfer.h"
// #include "httpclient.h"
#include "app_main.h"
#include "app_at_cmd.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

osThreadId g_tFileDownloadHttpTaskId;
osMessageQId g_tFileDownloadHttpQueueId;

SHM_DATA static HTTPCLIENT_T g_tFileDownloadHttpClient = {0};

static uint16_t g_u16FileDownloadDtimId = 0;

bool g_blHttpFileProgress = false;
uint8_t *g_pu8HttpPost = NULL;
uint32_t g_PostLen = 0;
#if 1
char *g_pcSslServerCa = NULL;
#else
const char ssl_server_ca[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\r\n"
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\r\n"
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\r\n"
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\r\n"
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\r\n"
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\r\n"
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\r\n"
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\r\n"
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\r\n"
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\r\n"
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\r\n"
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\r\n"
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\r\n"
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\r\n"
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\r\n"
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\r\n"
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\r\n"
"rqXRfboQnoZsG4q5WTP468SQvvG5\r\n"
"-----END CERTIFICATE-----\r\n"
"-----BEGIN CERTIFICATE-----\r\n"
"MIIFQTCCAymgAwIBAgITBmyf0pY1hp8KD+WGePhbJruKNzANBgkqhkiG9w0BAQwF\r\n"
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\r\n"
"b24gUm9vdCBDQSAyMB4XDTE1MDUyNjAwMDAwMFoXDTQwMDUyNjAwMDAwMFowOTEL\r\n"
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\r\n"
"b3QgQ0EgMjCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK2Wny2cSkxK\r\n"
"gXlRmeyKy2tgURO8TW0G/LAIjd0ZEGrHJgw12MBvIITplLGbhQPDW9tK6Mj4kHbZ\r\n"
"W0/jTOgGNk3Mmqw9DJArktQGGWCsN0R5hYGCrVo34A3MnaZMUnbqQ523BNFQ9lXg\r\n"
"1dKmSYXpN+nKfq5clU1Imj+uIFptiJXZNLhSGkOQsL9sBbm2eLfq0OQ6PBJTYv9K\r\n"
"8nu+NQWpEjTj82R0Yiw9AElaKP4yRLuH3WUnAnE72kr3H9rN9yFVkE8P7K6C4Z9r\r\n"
"2UXTu/Bfh+08LDmG2j/e7HJV63mjrdvdfLC6HM783k81ds8P+HgfajZRRidhW+me\r\n"
"z/CiVX18JYpvL7TFz4QuK/0NURBs+18bvBt+xa47mAExkv8LV/SasrlX6avvDXbR\r\n"
"8O70zoan4G7ptGmh32n2M8ZpLpcTnqWHsFcQgTfJU7O7f/aS0ZzQGPSSbtqDT6Zj\r\n"
"mUyl+17vIWR6IF9sZIUVyzfpYgwLKhbcAS4y2j5L9Z469hdAlO+ekQiG+r5jqFoz\r\n"
"7Mt0Q5X5bGlSNscpb/xVA1wf+5+9R+vnSUeVC06JIglJ4PVhHvG/LopyboBZ/1c6\r\n"
"+XUyo05f7O0oYtlNc/LMgRdg7c3r3NunysV+Ar3yVAhU/bQtCSwXVEqY0VThUWcI\r\n"
"0u1ufm8/0i2BWSlmy5A5lREedCf+3euvAgMBAAGjQjBAMA8GA1UdEwEB/wQFMAMB\r\n"
"Af8wDgYDVR0PAQH/BAQDAgGGMB0GA1UdDgQWBBSwDPBMMPQFWAJI/TPlUq9LhONm\r\n"
"UjANBgkqhkiG9w0BAQwFAAOCAgEAqqiAjw54o+Ci1M3m9Zh6O+oAA7CXDpO8Wqj2\r\n"
"LIxyh6mx/H9z/WNxeKWHWc8w4Q0QshNabYL1auaAn6AFC2jkR2vHat+2/XcycuUY\r\n"
"+gn0oJMsXdKMdYV2ZZAMA3m3MSNjrXiDCYZohMr/+c8mmpJ5581LxedhpxfL86kS\r\n"
"k5Nrp+gvU5LEYFiwzAJRGFuFjWJZY7attN6a+yb3ACfAXVU3dJnJUH/jWS5E4ywl\r\n"
"7uxMMne0nxrpS10gxdr9HIcWxkPo1LsmmkVwXqkLN1PiRnsn/eBG8om3zEK2yygm\r\n"
"btmlyTrIQRNg91CMFa6ybRoVGld45pIq2WWQgj9sAq+uEjonljYE1x2igGOpm/Hl\r\n"
"urR8FLBOybEfdF849lHqm/osohHUqS0nGkWxr7JOcQ3AWEbWaQbLU8uz/mtBzUF+\r\n"
"fUwPfHJ5elnNXkoOrJupmHN5fLT0zLm4BwyydFy4x2+IoZCn9Kr5v2c69BoVYh63\r\n"
"n749sSmvZ6ES8lgQGVMDMBu4Gon2nL2XA46jCfMdiyHxtN/kHNGfZQIG6lzWE7OE\r\n"
"76KlXIx3KadowGuuQNKotOrN8I1LOJwZmhsoVLiJkO/KdYE+HvJkJMcYr07/R54H\r\n"
"9jVlpNMKVv/1F2Rs76giJUmTtt8AF9pYfl3uxRuw0dFfIRDH+fO6AgonB8Xx1sfT\r\n"
"4PsJYGw=\r\n"
"-----END CERTIFICATE-----\r\n"
"-----BEGIN CERTIFICATE-----\r\n"
"MIIBtjCCAVugAwIBAgITBmyf1XSXNmY/Owua2eiedgPySjAKBggqhkjOPQQDAjA5\r\n"
"MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6b24g\r\n"
"Um9vdCBDQSAzMB4XDTE1MDUyNjAwMDAwMFoXDTQwMDUyNjAwMDAwMFowOTELMAkG\r\n"
"A1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJvb3Qg\r\n"
"Q0EgMzBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABCmXp8ZBf8ANm+gBG1bG8lKl\r\n"
"ui2yEujSLtf6ycXYqm0fc4E7O5hrOXwzpcVOho6AF2hiRVd9RFgdszflZwjrZt6j\r\n"
"QjBAMA8GA1UdEwEB/wQFMAMBAf8wDgYDVR0PAQH/BAQDAgGGMB0GA1UdDgQWBBSr\r\n"
"ttvXBp43rDCGB5Fwx5zEGbF4wDAKBggqhkjOPQQDAgNJADBGAiEA4IWSoxe3jfkr\r\n"
"BqWTrBqYaGFy+uGh0PsceGCmQ5nFuMQCIQCcAu/xlJyzlvnrxir4tiz+OpAUFteM\r\n"
"YyRIHN8wfdVoOw==\r\n"
"-----END CERTIFICATE-----\r\n"
"-----BEGIN CERTIFICATE-----\r\n"
"MIIB8jCCAXigAwIBAgITBmyf18G7EEwpQ+Vxe3ssyBrBDjAKBggqhkjOPQQDAzA5\r\n"
"MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6b24g\r\n"
"Um9vdCBDQSA0MB4XDTE1MDUyNjAwMDAwMFoXDTQwMDUyNjAwMDAwMFowOTELMAkG\r\n"
"A1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJvb3Qg\r\n"
"Q0EgNDB2MBAGByqGSM49AgEGBSuBBAAiA2IABNKrijdPo1MN/sGKe0uoe0ZLY7Bi\r\n"
"9i0b2whxIdIA6GO9mif78DluXeo9pcmBqqNbIJhFXRbb/egQbeOc4OO9X4Ri83Bk\r\n"
"M6DLJC9wuoihKqB1+IGuYgbEgds5bimwHvouXKNCMEAwDwYDVR0TAQH/BAUwAwEB\r\n"
"/zAOBgNVHQ8BAf8EBAMCAYYwHQYDVR0OBBYEFNPsxzplbszh2naaVvuc84ZtV+WB\r\n"
"MAoGCCqGSM49BAMDA2gAMGUCMDqLIfG9fhGt0O9Yli/W651+kI0rz2ZVwyzjKKlw\r\n"
"CkcO8DdZEv8tmZQoTipPNU0zWgIxAOp1AE47xDqUEpHJWEadIRNyp4iciuRMStuW\r\n"
"1KyLa2tJElMzrdfkviT8tQp21KW8EA==\r\n"
"-----END CERTIFICATE-----\r\n"
"-----BEGIN CERTIFICATE-----\r\n"
"MIID7zCCAtegAwIBAgIBADANBgkqhkiG9w0BAQsFADCBmDELMAkGA1UEBhMCVVMx\r\n"
"EDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNjb3R0c2RhbGUxJTAjBgNVBAoT\r\n"
"HFN0YXJmaWVsZCBUZWNobm9sb2dpZXMsIEluYy4xOzA5BgNVBAMTMlN0YXJmaWVs\r\n"
"ZCBTZXJ2aWNlcyBSb290IENlcnRpZmljYXRlIEF1dGhvcml0eSAtIEcyMB4XDTA5\r\n"
"MDkwMTAwMDAwMFoXDTM3MTIzMTIzNTk1OVowgZgxCzAJBgNVBAYTAlVTMRAwDgYD\r\n"
"VQQIEwdBcml6b25hMRMwEQYDVQQHEwpTY290dHNkYWxlMSUwIwYDVQQKExxTdGFy\r\n"
"ZmllbGQgVGVjaG5vbG9naWVzLCBJbmMuMTswOQYDVQQDEzJTdGFyZmllbGQgU2Vy\r\n"
"dmljZXMgUm9vdCBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkgLSBHMjCCASIwDQYJKoZI\r\n"
"hvcNAQEBBQADggEPADCCAQoCggEBANUMOsQq+U7i9b4Zl1+OiFOxHz/Lz58gE20p\r\n"
"OsgPfTz3a3Y4Y9k2YKibXlwAgLIvWX/2h/klQ4bnaRtSmpDhcePYLQ1Ob/bISdm2\r\n"
"8xpWriu2dBTrz/sm4xq6HZYuajtYlIlHVv8loJNwU4PahHQUw2eeBGg6345AWh1K\r\n"
"Ts9DkTvnVtYAcMtS7nt9rjrnvDH5RfbCYM8TWQIrgMw0R9+53pBlbQLPLJGmpufe\r\n"
"hRhJfGZOozptqbXuNC66DQO4M99H67FrjSXZm86B0UVGMpZwh94CDklDhbZsc7tk\r\n"
"6mFBrMnUVN+HL8cisibMn1lUaJ/8viovxFUcdUBgF4UCVTmLfwUCAwEAAaNCMEAw\r\n"
"DwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYEFJxfAN+q\r\n"
"AdcwKziIorhtSpzyEZGDMA0GCSqGSIb3DQEBCwUAA4IBAQBLNqaEd2ndOxmfZyMI\r\n"
"bw5hyf2E3F/YNoHN2BtBLZ9g3ccaaNnRbobhiCPPE95Dz+I0swSdHynVv/heyNXB\r\n"
"ve6SbzJ08pGCL72CQnqtKrcgfU28elUSwhXqvfdqlS5sdJ/PHLTyxQGjhdByPq1z\r\n"
"qwubdQxtRbeOlKyWN7Wg0I8VRw7j6IPdj/3vQQF3zCepYoUz8jcI73HPdwbeyBkd\r\n"
"iEDPfUYd/x7H4c7/I9vG+o1VTqkC50cRRj70/b17KSa7qWFiNyi2LSr2EIZkyXCn\r\n"
"0q23KXB56jzaYyWf/Wi3MOxw+3WKt21gZ7IeyLnp2KhvAotnDU0mV3HaIPzBSlCN\r\n"
"sSi6\r\n"
"-----END CERTIFICATE-----\r\n";
#endif

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions
/*************************************************************************
* FUNCTION:
*   Http_FilePostSet
*
* DESCRIPTION:
*   check is in file download progress
*
* PARAMETERS
*   none
*
* RETURNS
*   bool :          [OUT] true: in OTA progress
*                         false: in idle
*
*************************************************************************/
T_OplErr Http_FilePostSet(uint8_t *u8Data, uint32_t u32DataLen)
{
    T_OplErr tRet = OPL_ERR;
    uint8_t **u8HttpPostFile = NULL;

    if(NULL == u8Data || 0 == u32DataLen)
    {
        return tRet;
    }

    if(g_pu8HttpPost != NULL)
    {
        free(g_pu8HttpPost);
        g_pu8HttpPost = NULL;
    }

    u8HttpPostFile = &g_pu8HttpPost;

    *u8HttpPostFile = (uint8_t *) malloc(u32DataLen);
        
    if(NULL == u8HttpPostFile)
    {
        return tRet;
    }

    g_PostLen = u32DataLen;
    memset(*u8HttpPostFile, 0, g_PostLen);
    memcpy(*u8HttpPostFile, u8Data, g_PostLen);
    tRet = OPL_OK;
    return tRet;
}

/*************************************************************************
* FUNCTION:
*   Http_FileInProgress
*
* DESCRIPTION:
*   check is in file download progress
*
* PARAMETERS
*   none
*
* RETURNS
*   bool :          [OUT] true: in OTA progress
*                         false: in idle
*
*************************************************************************/
bool Http_FileInProgress(void)
{
    return g_blHttpFileProgress;
}

/*************************************************************************
* FUNCTION:
*   Http_FileRetrirveGet
*
* DESCRIPTION:
*   process http data retrieve
*
* PARAMETERS
*   url :           [IN] host url
*   buf :           [IN] buffer for carried downlaoded data
*   len :           [IN] size of buffer for carried download data
*
* RETURNS
*   int32_t :       [OUT] result
*
*************************************************************************/
SHM_DATA int32_t Http_FileRetrirveGet(HTTPCLIENT_T *client, char *url, char *buf, uint32_t len, int method)
{
#if defined(HTTPCLIENT_EXP)
    int8_t i8Ret = HTTPCLIENT_ERROR_CONN_T;
#else
    int8_t i8Ret = HTTPCLIENT_ERROR_CONN;
#endif

    HTTPCLIENT_DATA_T tFileDownloadHttpClientData = {0};

    tFileDownloadHttpClientData.response_buf = buf;
    tFileDownloadHttpClientData.response_buf_len = len;

    uint32_t u32RecvDataOnce = 0;
    uint32_t u32RecvDataTotal = 0;

    if(HTTPCLIENT_POST_T == method)
    {
        tFileDownloadHttpClientData.post_buf_len = g_PostLen;
        tFileDownloadHttpClientData.post_buf = (char *)g_pu8HttpPost;
    }

    // send request to server
#if defined(HTTPCLIENT_EXP)
    i8Ret = httpclient_exp_send_request(client, url, method, &tFileDownloadHttpClientData);
#else
    i8Ret = httpclient_send_request(client, url, HTTPCLIENT_GET, &tFileDownloadHttpClientData);
#endif

    if(0 > i8Ret)
    {
        OPL_LOG_ERRO(FILEDL, "http client fail to send request");
        return i8Ret;
    }

    do
    {        
        // receive response from server
#if defined(HTTPCLIENT_EXP)
        i8Ret = httpclient_exp_recv_response(client, &tFileDownloadHttpClientData);
#else
        i8Ret = httpclient_recv_response(&client, &tFileDownloadHttpClientData);
#endif
        if (i8Ret < 0)
        {
            OPL_LOG_ERRO(FILEDL, "http client recv response error, i8Ret = %d", i8Ret);
            return i8Ret;
        }

        u32RecvDataOnce = tFileDownloadHttpClientData.response_content_len - (tFileDownloadHttpClientData.retrieve_len + u32RecvDataTotal);

        printf("content len %d; retrieve len %d; recvData %d; recvDataTotal %d\n", tFileDownloadHttpClientData.response_content_len, tFileDownloadHttpClientData.retrieve_len, u32RecvDataOnce, u32RecvDataTotal);

        if(u32RecvDataOnce)
        {
            Transfer_File(method,(uint8_t *)tFileDownloadHttpClientData.response_buf, u32RecvDataOnce, tFileDownloadHttpClientData.response_content_len);

            //If total len <= 1024, inform the MCU that the message has been transmitted
            if(tFileDownloadHttpClientData.response_content_len < FILE_DOWNLOAD_HTTP_DATA_LEN)
            {
                Transfer_File(method, NULL, 0, tFileDownloadHttpClientData.response_content_len);
            }

            u32RecvDataTotal += u32RecvDataOnce;

            osDelay(50);
        }

#if defined(HTTPCLIENT_EXP)
    } while (i8Ret == HTTPCLIENT_RETRIEVE_MORE_DATA_T);
#else
    } while (i8Ret == HTTPCLIENT_RETRIEVE_MORE_DATA);
#endif

// #if defined(HTTPCLIENT_EXP)
//     if(u32DataRecv != tFileDownloadHttpClientData.response_content_len || httpclient_exp_get_response_code(client) != 200)
// #else
//     if(u32DataRecv != tFileDownloadHttpClientData.response_content_len || httpclient_get_response_code(client) != 200)
// #endif
//     {
//         OPL_LOG_ERRO(FILEDL, "data received not completed, or invalid error code");
//         return -1;
//     }
//     else if(u32DataRecv == 0)
//     {
//         OPL_LOG_ERRO(FILEDL, "receive length is zero, file not found");
//         return -2;
//     }
//     else
//     {
        OPL_LOG_INFO(FILEDL, "download success %d", i8Ret);
        return i8Ret;
//     }
}

/*************************************************************************
* FUNCTION:
*   Http_FileRequest
*
* DESCRIPTION:
*   process http download and OTA
*
* PARAMETERS
*   url :           [IN] host url
*   len :           [IN] host url lens
*
* RETURNS
*   int32_t :       [OUT] result
*
*************************************************************************/
SHM_DATA int32_t Http_FileRequest(char *url, int len, int method)
{
    char get_url[FILE_DOWNLOAD_HTTP_URL_LEN+1];
    char *buf;

#if defined(HTTPCLIENT_EXP)
    int8_t i8Ret = HTTPCLIENT_ERROR_CONN_T;
#else
    int8_t i8Ret = HTTPCLIENT_ERROR_CONN;
#endif
    uint32_t u16UrlLen = strlen(url);
    uint8_t u8RetryCount = 0;

    if(1 > u16UrlLen)
    {
        return -1;
    }

    memset(get_url, 0, FILE_DOWNLOAD_HTTP_URL_LEN+1);
    strncpy(get_url, url, len);

    OPL_LOG_DEBG(FILEDL, "File download url %s (len: %d)", get_url, u16UrlLen);

    buf = malloc(FILE_DOWNLOAD_HTTP_DATA_LEN);

    if(NULL == buf)
    {
        OPL_LOG_ERRO(FILEDL, "Alloc buf fail");

        return -1;
    }

    lwip_auto_arp_enable(1, 0);

    g_tFileDownloadHttpClient.timeout_ms = FILE_DOWNLOAD_HTTP_CONNECTION_TIMEOUT;
#if 1
    g_tFileDownloadHttpClient.server_cert = g_pcSslServerCa;
    if(NULL != g_pcSslServerCa)
    {
        g_tFileDownloadHttpClient.server_cert_len = strlen(g_pcSslServerCa) + 1;
    }
    else
    {
        g_tFileDownloadHttpClient.server_cert_len = 0;
    }
#else
    g_tFileDownloadHttpClient.server_cert = ssl_server_ca;
    g_tFileDownloadHttpClient.server_cert_len = sizeof(ssl_server_ca);
#endif
    // connect to http server
    do
    {
#if defined(HTTPCLIENT_EXP)
        i8Ret = httpclient_exp_connect(&g_tFileDownloadHttpClient, get_url);
#else
        i8Ret = httpclient_connect(&g_tFileDownloadHttpClient, get_url);
#endif
        if(!i8Ret)
        {
            OPL_LOG_INFO(FILEDL, "Connect to http server");
            break;
        }
        else
        {
            osDelay(1000);
            u8RetryCount ++;
            OPL_LOG_WARN(FILEDL, "Connect to http server fail, retry count (%d)", u8RetryCount);

            // close http connection
#if defined(HTTPCLIENT_EXP)
            httpclient_exp_close(&g_tFileDownloadHttpClient);
#else
            httpclient_close(&g_tFileDownloadHttpClient);
#endif
            if(HTTPCLIENT_ERROR_PRTCL_T == i8Ret)   // mbedtls_ssl_handshake certificate verification failed, shoud not retry
            {
                break;
            }
        }
    } while(u8RetryCount < FILE_DOWNLOAD_HTTP_CONNECTION_RETRY_COUNT);


    if(!i8Ret)
    {
        i8Ret = Http_FileRetrirveGet(&g_tFileDownloadHttpClient, get_url, buf, FILE_DOWNLOAD_HTTP_DATA_LEN, method);

        OPL_LOG_INFO(FILEDL, "download result = %d", i8Ret);

        // close http connection
#if defined(HTTPCLIENT_EXP)
        httpclient_exp_close(&g_tFileDownloadHttpClient);
#else
        httpclient_close(&g_tFileDownloadHttpClient);
#endif
    }


    free(buf);
    buf = NULL;

    return i8Ret;
}

/*************************************************************************
* FUNCTION:
*   Http_FileTriggerReq
*
* DESCRIPTION:
*   send OTA request to OTA task
*
* PARAMETERS
*   u32EventId :    [IN] event ID
*   pu8Data :       [IN] message data
*   u32DataLen :    [IN] messgae data lens
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Http_FileTriggerReq(uint32_t u32EventId, uint8_t *pu8Data, uint32_t u32DataLen)
{
    if(NULL == g_tFileDownloadHttpQueueId)
    {
        OPL_LOG_WARN(FILEDL, "Queue not init");

        return OPL_ERR_RTOS_QMSG_NOT_INIT;
    }

    T_FileDownloadHttpMsg *ptFileDownloadMsg = (T_FileDownloadHttpMsg *)malloc(sizeof(T_FileDownloadHttpMsg) + u32DataLen);

    if(NULL == ptFileDownloadMsg)
    {
        OPL_LOG_ERRO(FILEDL, "Alloc Tx message fail");

        return OPL_ERR_ALLOC_MEMORY_FAIL;
    }

    // prepare the data
    ptFileDownloadMsg->u32EventId = u32EventId;
    ptFileDownloadMsg->u32DataLen = u32DataLen;
    if(0 != ptFileDownloadMsg->u32DataLen)
    {
        memcpy(ptFileDownloadMsg->u8aData, pu8Data, u32DataLen);
    }

    // send message
    if(osOK != osMessagePut(g_tFileDownloadHttpQueueId, (uint32_t)ptFileDownloadMsg, 0))
    {
        OPL_LOG_ERRO(FILEDL, "Send message fail");

        free(ptFileDownloadMsg);

        return OPL_ERR_RTOS_SEND_QMSG_FAIL;
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION:
*   File_DownloadTaskHandler
*
* DESCRIPTION:
*   handler of OTA task
*
* PARAMETERS
*   args :          [IN] arguments
*
* RETURNS
*   none
*
*************************************************************************/
SHM_DATA void File_DownloadTaskHandler(void *args)
{
    osEvent tEvent;
    T_FileDownloadHttpMsg *ptFileDownloadMsg;
    uint8_t u8Method;

    if(OPL_OK != Opl_Wifi_Skip_Dtim_Module_Reg(&g_u16FileDownloadDtimId))
    {
        OPL_LOG_WARN(FILEDL, "File Download Dtim ID reg fail");
    }

    for(;;)
    {
        // wait event
        tEvent = osMessageGet(g_tFileDownloadHttpQueueId, osWaitForever);

        if(tEvent.status == osEventMessage)
        {
            ptFileDownloadMsg = (T_FileDownloadHttpMsg *)tEvent.value.p;

            switch(ptFileDownloadMsg->u32EventId)
            {
                case FILE_EVT_TYPE_HTTP_GET:
                {
                    u8Method = HTTPCLIENT_GET_T;
                    break;
                }

                case FILE_EVT_TYPE_HTTP_POST:
                {
                    u8Method = HTTPCLIENT_POST_T;
                    break;
                }

                default:
                    break;
            }
            
            // disable skip dtim
            Opl_Wifi_Skip_Dtim_Set(g_u16FileDownloadDtimId, false);

            lwip_one_shot_arp_enable();

#if defined(OPL1000_A2) || defined(OPL1000_A3)
            CtrlWifi_PsStateForce(STA_PS_AWAKE_MODE, 0);
#elif defined(OPL2500_A0)
            CtrlWifi_PsStateForce(STA_MODE_PERFORMANCE, 0);
            sys_cfg_clk_set((T_SysCfgClkIdx) SYS_CFG_CLK_DECI_160_MHZ);
#endif
            g_blHttpFileProgress = true;

            int32_t i32Ret = Http_FileRequest((char *)ptFileDownloadMsg->u8aData, (int)ptFileDownloadMsg->u32DataLen, (int)u8Method);

            g_blHttpFileProgress = false;

            // enable skip dtim
            Opl_Wifi_Skip_Dtim_Set(g_u16FileDownloadDtimId, true);

#if defined(OPL1000_A2) || defined(OPL1000_A3)
            CtrlWifi_PsStateForce(STA_PS_NONE, 0);
#elif defined(OPL2500_A0)
            CtrlWifi_PsStateForce(STA_MODE_LIGHT_PS, 0);
            sys_cfg_clk_set(SYS_CFG_CLK_RATE);
#endif

            if(0 != i32Ret)
            {
                if(FILE_EVT_TYPE_HTTP_GET == ptFileDownloadMsg->u32EventId)
                {
                    AT_FAIL(ACK_TAG_FILE_HTTP_GET_RSP, NULL, 0);
                }
                else
                if(FILE_EVT_TYPE_HTTP_POST == ptFileDownloadMsg->u32EventId)
                {
                    AT_FAIL(ACK_TAG_FILE_HTTP_POST_RSP, NULL, 0);
                }

                OPL_LOG_ERRO(FILEDL, "File download fail");
            }
            
            if(NULL != ptFileDownloadMsg)
            {
                free(ptFileDownloadMsg);
            }
        }
    }
}

/*************************************************************************
* FUNCTION:
*   File_DownloadTaskInit
*
* DESCRIPTION:
*   OTA task initiate function
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
SHM_DATA void File_DownloadTaskInit(void)
{
    osThreadDef_t task_def;
    osMessageQDef_t queue_def;

    // create queue
    queue_def.item_sz = sizeof(T_FileDownloadHttpMsg);
    queue_def.queue_sz = FILE_DOWNLOAD_HTTP_MSG_QUEUE_SIZE;

    g_tFileDownloadHttpQueueId = osMessageCreate(&queue_def, NULL);

    if(NULL == g_tFileDownloadHttpQueueId)
    {
        OPL_LOG_ERRO(FILEDL, "File Download queue create fail");
    }

    // create task
    task_def.name = "FILEDL";
    task_def.stacksize = FILE_DOWNLOAD_HTTP_TASK_STACK_SIZE;
    task_def.tpriority = FILE_DOWNLOAD_HTTP_TASK_PRIORITY;
    task_def.pthread = File_DownloadTaskHandler;

    g_tFileDownloadHttpTaskId = osThreadCreate(&task_def, (void *)NULL);

    if(NULL == g_tFileDownloadHttpTaskId)
    {
        OPL_LOG_ERRO(FILEDL, "File Download task create fail");
    }

    OPL_LOG_INFO(FILEDL, "File Download task create ok");
}

#endif 
