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
*  http_ota.c
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
// Sec 0: Comment block of the file

// Sec 1: Include File

#include "cmsis_os.h"
#include "http_ota.h"
#include "controller_wifi.h"
#include "controller_wifi_com.h"
#include "lwip/etharp.h"
#include "ota_mngr.h"
#include "qd_config.h"
#include "qd_module.h"
#include "wifi_mngr_api.h"

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

osThreadId g_tHttpOtaTaskId;
osMessageQId g_tHttpOtaQueueId;

static HTTPCLIENT_T g_tHttpOtaHttpClient = {0};

static uint16_t g_u16OtaDtimId = 0;
static uint16_t g_u16OtaSeqId = 0;
static bool g_blOtaProgress = false;


// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

/*************************************************************************
* FUNCTION:
*   HTTP_OtaInProgress
*
* DESCRIPTION:
*   check is in ota progress
*
* PARAMETERS
*   none
*
* RETURNS
*   bool :          [OUT] true: in OTA progress
*                         false: in idle
*
*************************************************************************/
bool HTTP_OtaInProgress(void)
{
    return g_blOtaProgress;
}

/*************************************************************************
* FUNCTION:
*   _HTTP_OtaProcessTimeoutHandler
*
* DESCRIPTION:
*   timeout handler function pointer which been called from OTA manager
*   (ota process timeout time configure in qd_module.h -> OTA_PROCESS_PERIOD_TIMEOUT)
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void _HTTP_OtaProcessTimeoutHandler(void)
{
    // user implement

    // *

    OPL_LOG_WARN(HTTPOTA, "OTA timeout, reboot after 3s");

    OTA_TriggerReboot(3000);

    // *
}

/*************************************************************************
* FUNCTION:
*   _HTTP_OtaHttpRetrieveOffset
*
* DESCRIPTION:
*   retrieving offset
*
* PARAMETERS
*   client :        [IN] http client
*   client_data :   [IN] http client data
*   offset :        [IN] offset index
*   parse_hdr :     [IN] request 
*
* RETURNS
*   int32_t :       [OUT] result
*
*************************************************************************/
int32_t _HTTP_OtaHttpRetrieveOffset(HTTPCLIENT_T *client, HTTPCLIENT_DATA_T *client_data, int offset, int parse_hdr)
{
#if defined(HTTPCLIENT_EXP)
    int8_t i8Ret = HTTPCLIENT_ERROR_CONN_T;
#else
    int8_t i8Ret = HTTPCLIENT_ERROR_CONN;
#endif
    uint8_t u8ParseHdr = parse_hdr;
    T_MwOtaFlashHeader *ptOtaHdr = NULL;

    do
    {
        // receive response from server
#if defined(HTTPCLIENT_EXP)
        i8Ret = httpclient_exp_recv_response(client, client_data);
#else
        i8Ret = httpclient_recv_response(client, client_data);
#endif

        if(0 > i8Ret)
        {
            OPL_LOG_ERRO(HTTPOTA, "http client recv response error (ret = %d)", i8Ret);
            return i8Ret;
        }

        if(1 == u8ParseHdr)
        {
            OPL_HEX_DUMP_DEBG(HTTPOTA, (uint8_t *)client_data->response_buf, HTTP_OTA_HTTP_DATA_LEN - 1);

            ptOtaHdr = (T_MwOtaFlashHeader *)client_data->response_buf;

            OPL_LOG_DEBG(HTTPOTA, "pid %d, cid %d, fid %d, sum %d, total len %d", ptOtaHdr->uwProjectId,
                                                                                ptOtaHdr->uwChipId,
                                                                                ptOtaHdr->uwFirmwareId,
                                                                                ptOtaHdr->ulImageSum,
                                                                                ptOtaHdr->ulImageSize);

            if(OTA_UpgradeBegin(&g_u16OtaSeqId, ptOtaHdr, _HTTP_OtaProcessTimeoutHandler) != OPL_OK)
            {
                OPL_LOG_ERRO(HTTPOTA, "OTA begin upgrade fail");
                return -1;
            }

            u8ParseHdr = 0;
        }

        if((client_data->response_content_len - client_data->retrieve_len) == offset)
        {
            break;
        }

#if defined(HTTPCLIENT_EXP)
    } while (i8Ret == HTTPCLIENT_RETRIEVE_MORE_DATA_T);

    return HTTPCLIENT_OK_T;
#else
    } while (i8Ret == HTTPCLIENT_RETRIEVE_MORE_DATA);

    return HTTPCLIENT_OK;
#endif
}

/*************************************************************************
* FUNCTION:
*   _HTTP_OtaHttpRetrieveGet
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
int32_t _HTTP_OtaHttpRetrieveGet(HTTPCLIENT_T *client, char *url, char *buf, uint32_t len)
{
#if defined(HTTPCLIENT_EXP)
    int8_t i8Ret = HTTPCLIENT_ERROR_CONN_T;
#else
    int8_t i8Ret = HTTPCLIENT_ERROR_CONN;
#endif

    HTTPCLIENT_DATA_T tHttpOtaHttpClientData = {0};

    uint32_t u32WriteCount = 0;
    uint32_t u32DataRecv = 0;
    uint32_t u32RecvTemp = 0;
    uint32_t u32DataLen = 0;

    tHttpOtaHttpClientData.response_buf = buf;
    tHttpOtaHttpClientData.response_buf_len = len;

    // send request to server
#if defined(HTTPCLIENT_EXP)
    i8Ret = httpclient_exp_send_request(client, url, HTTPCLIENT_GET_T, &tHttpOtaHttpClientData);
#else
    i8Ret = httpclient_send_request(client, url, HTTPCLIENT_GET, &tHttpOtaHttpClientData);
#endif

    if(0 > i8Ret)
    {
        OPL_LOG_ERRO(HTTPOTA, "http client fail to send request");
        return i8Ret;
    }

    // response body start

    // skip 2nd boot agent, 0x00000000 ~ 0x00003000 : 12KB
    i8Ret = _HTTP_OtaHttpRetrieveOffset(client, &tHttpOtaHttpClientData, MW_OTA_HEADER_ADDR_1, 0);
    if(0 > i8Ret)
    {
        OPL_LOG_ERRO(HTTPOTA, "http retrieve offset error, i8Ret = %d", i8Ret);
        return i8Ret;
    }

    // parse 1st OTA header, 0x00003000 ~ 0x00004000 : 4KB
    i8Ret = _HTTP_OtaHttpRetrieveOffset(client, &tHttpOtaHttpClientData, MW_OTA_HEADER_ADDR_2, 1);
    if(0 > i8Ret)
    {
        OPL_LOG_ERRO(HTTPOTA, "http retrieve offset error, i8Ret = %d", i8Ret);
        return i8Ret;
    }

    // skip 2nd OTA header, 0x00004000 ~ 0x00005000 : 4KB
    i8Ret = _HTTP_OtaHttpRetrieveOffset(client, &tHttpOtaHttpClientData, MW_OTA_IMAGE_ADDR_1, 0);
    if(0 > i8Ret)
    {
        OPL_LOG_ERRO(HTTPOTA, "http retrieve offset error, i8Ret = %d", i8Ret);
        return i8Ret;
    }

    u32RecvTemp = tHttpOtaHttpClientData.retrieve_len;
    u32DataRecv = tHttpOtaHttpClientData.response_content_len - tHttpOtaHttpClientData.retrieve_len;

    do
    {
        // receive response from server
#if defined(HTTPCLIENT_EXP)
        i8Ret = httpclient_exp_recv_response(client, &tHttpOtaHttpClientData);
#else
        i8Ret = httpclient_recv_response(&client, &tHttpOtaHttpClientData);
#endif
        if (i8Ret < 0)
        {
            OPL_LOG_ERRO(HTTPOTA, "http client recv response error, i8Ret = %d", i8Ret);
            return i8Ret;
        }

        u32DataLen = u32RecvTemp - tHttpOtaHttpClientData.retrieve_len;

        if(OTA_WriteData(g_u16OtaSeqId, (uint8_t *)tHttpOtaHttpClientData.response_buf, u32DataLen) != OPL_OK)
        {
            OPL_LOG_ERRO(HTTPOTA, "OTA write data fail");
            return -1;
        }

        u32WriteCount += u32DataLen;
        u32DataRecv += u32DataLen;
        u32RecvTemp = tHttpOtaHttpClientData.retrieve_len;

        OPL_LOG_INFO(HTTPOTA, "written image length %d", u32WriteCount);
        OPL_LOG_INFO(HTTPOTA, "download progress %u", u32DataRecv * 100 / tHttpOtaHttpClientData.response_content_len);

#if defined(HTTPCLIENT_EXP)
    } while (i8Ret == HTTPCLIENT_RETRIEVE_MORE_DATA_T);
#else
    } while (i8Ret == HTTPCLIENT_RETRIEVE_MORE_DATA);
#endif

    OPL_LOG_INFO(HTTPOTA, "total write data length: %d", u32WriteCount);
    OPL_LOG_INFO(HTTPOTA, "data received length: %d", u32DataRecv);

#if defined(HTTPCLIENT_EXP)
    if(u32DataRecv != tHttpOtaHttpClientData.response_content_len || httpclient_exp_get_response_code(client) != 200)
#else
    if(u32DataRecv != tHttpOtaHttpClientData.response_content_len || httpclient_get_response_code(client) != 200)
#endif
    {
        OPL_LOG_ERRO(HTTPOTA, "data received not completed, or invalid error code");
        return -1;
    }
    else if(u32DataRecv == 0)
    {
        OPL_LOG_ERRO(HTTPOTA, "receive length is zero, file not found");
        return -2;
    }
    else
    {
        OPL_LOG_INFO(HTTPOTA, "download success");
        return i8Ret;
    }
}

/*************************************************************************
* FUNCTION:
*   HTTP_OtaHttpDownload
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
int32_t HTTP_OtaHttpDownload(char *url, int len)
{
    char get_url[HTTP_OTA_HTTP_URL_LEN];
    char *buf;

#if defined(HTTPCLIENT_EXP)
    int8_t i8Ret = HTTPCLIENT_ERROR_CONN_T;
#else
    int8_t i8Ret = HTTPCLIENT_ERROR_CONN;
#endif
    uint32_t u16UrlLen = strlen(url);
    uint16_t u16Pid, u16Cid, u16Fid;
    uint8_t u8RetryCount = 0;

    if(1 > u16UrlLen)
    {
        return -1;
    }

    memset(get_url, 0, HTTP_OTA_HTTP_URL_LEN);
    strncpy(get_url, url, len);

    OPL_LOG_DEBG(HTTPOTA, "OTA url %s (len: %d)", get_url, u16UrlLen);

    buf = malloc(HTTP_OTA_HTTP_DATA_LEN);

    if(NULL == buf)
    {
        OPL_LOG_ERRO(HTTPOTA, "Alloc buf fail");

        return -1;
    }

    lwip_auto_arp_enable(1, 0);

    g_tHttpOtaHttpClient.timeout_ms = HTTP_OTA_HTTP_CONNECTION_TIMEOUT;

    // connect to http server
    do
    {
#if defined(HTTPCLIENT_EXP)
        i8Ret = httpclient_exp_connect(&g_tHttpOtaHttpClient, get_url);
#else
        i8Ret = httpclient_connect(&g_tHttpOtaHttpClient, get_url);
#endif
        if(!i8Ret)
        {
            OPL_LOG_INFO(HTTPOTA, "Connect to http server");
            break;
        }
        else
        {
            osDelay(1000);
            u8RetryCount ++;
            OPL_LOG_WARN(HTTPOTA, "Connect to http server fail, retry count (%d)", u8RetryCount);
        }
    } while(u8RetryCount < HTTP_OTA_HTTP_CONNECTION_RETRY_COUNT);

    // get current ota version
    OTA_CurrentVersionGet(&u16Pid, &u16Cid, &u16Fid);

    OPL_LOG_DEBG(HTTPOTA, "pid=%d, cid=%d, fid=%d", u16Pid, u16Cid, u16Fid);

    i8Ret = _HTTP_OtaHttpRetrieveGet(&g_tHttpOtaHttpClient, get_url, buf, HTTP_OTA_HTTP_DATA_LEN);

    if(0 > i8Ret)
    {
        if(OPL_OK != OTA_UpgradeGiveUp(g_u16OtaSeqId))
        {
            OPL_LOG_ERRO(HTTPOTA, "OTA give up fail");
        }
    }
    else
    {
        if(OPL_OK != OTA_UpgradeFinish(g_u16OtaSeqId))
        {
            OPL_LOG_ERRO(HTTPOTA, "OTA finish fail");
        }
    }

    OPL_LOG_INFO(HTTPOTA, "download result = %d", i8Ret);

    // close http connection
#if defined(HTTPCLIENT_EXP)
    httpclient_exp_close(&g_tHttpOtaHttpClient);
#else
    httpclient_close(&g_tHttpOtaHttpClient);
#endif

    free(buf);
    buf = NULL;

    return i8Ret;
}

/*************************************************************************
* FUNCTION:
*   HTTP_OtaTriggerReq
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
T_OplErr HTTP_OtaTriggerReq(uint32_t u32EventId, uint8_t *pu8Data, uint32_t u32DataLen)
{
    if(NULL == g_tHttpOtaQueueId)
    {
        OPL_LOG_WARN(HTTPOTA, "Queue not init");

        return OPL_ERR_RTOS_QMSG_NOT_INIT;
    }

    T_HttpOtaMsg *ptHttpOtaMsg = (T_HttpOtaMsg *)malloc(sizeof(T_HttpOtaMsg) + u32DataLen);

    if(NULL == ptHttpOtaMsg)
    {
        OPL_LOG_ERRO(HTTPOTA, "Alloc Tx message fail");

        return OPL_ERR_ALLOC_MEMORY_FAIL;
    }

    // prepare the data
    ptHttpOtaMsg->u32EventId = u32EventId;
    ptHttpOtaMsg->u32DataLen = u32DataLen;
    if(0 != ptHttpOtaMsg->u32DataLen)
    {
        memcpy(ptHttpOtaMsg->u8aData, pu8Data, u32DataLen);
    }

    // send message
    if(osOK != osMessagePut(g_tHttpOtaQueueId, (uint32_t)ptHttpOtaMsg, 0))
    {
        OPL_LOG_ERRO(HTTPOTA, "Send message fail");

        free(ptHttpOtaMsg);

        return OPL_ERR_RTOS_SEND_QMSG_FAIL;
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION:
*   HTTP_OtaTaskHandler
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
void HTTP_OtaTaskHandler(void *args)
{
    osEvent tEvent;
    T_HttpOtaMsg *ptHttpOtaMsg;

    if(OPL_OK != Opl_Wifi_Skip_Dtim_Module_Reg(&g_u16OtaDtimId))
    {
        OPL_LOG_WARN(HTTPOTA, "OTA Dtim ID reg fail");
    }

    for(;;)
    {
        // wait event
        tEvent = osMessageGet(g_tHttpOtaQueueId, osWaitForever);

        if(tEvent.status == osEventMessage)
        {
            ptHttpOtaMsg = (T_HttpOtaMsg *)tEvent.value.p;

            switch(ptHttpOtaMsg->u32EventId)
            {
                case HTTP_OTA_EVT_TYPE_DOWNLOAD:
                {
                    // disable skip dtim
                    Opl_Wifi_Skip_Dtim_Set(g_u16OtaDtimId, false);

                    lwip_one_shot_arp_enable();

#if defined(OPL1000_A2) || defined(OPL1000_A3)
                    CtrlWifi_PsStateForce(STA_PS_AWAKE_MODE, 0);
#elif defined(OPL2500_A0)
                    CtrlWifi_PsStateForce(STA_MODE_PERFORMANCE, 0);
#endif
                    g_blOtaProgress = true;

                    int32_t i32Ret = HTTP_OtaHttpDownload((char *)ptHttpOtaMsg->u8aData, (int)ptHttpOtaMsg->u32DataLen);

                    g_blOtaProgress = false;

                    // enable skip dtim
                    Opl_Wifi_Skip_Dtim_Set(g_u16OtaDtimId, true);

#if defined(OPL1000_A2) || defined(OPL1000_A3)
                    CtrlWifi_PsStateForce(STA_PS_NONE, 0);
#elif defined(OPL2500_A0)
                    CtrlWifi_PsStateForce(STA_MODE_LIGHT_PS, 0);
#endif

                    if(0 != i32Ret)
                    {
                        OPL_LOG_ERRO(HTTPOTA, "OTA proc fail");
                    }
                    else
                    {
                        // restart the system
                        OTA_TriggerReboot(3000);
                    }

                    break;
                }

                case HTTP_OTA_EVT_TYPE_GET_VERSION:
                {
                    break;
                }

                default:
                    break;

            }

            if(NULL != ptHttpOtaMsg)
            {
                free(ptHttpOtaMsg);
            }
        }
    }
}

/*************************************************************************
* FUNCTION:
*   HTTP_OtaTaskInit
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
void HTTP_OtaTaskInit(void)
{
#if (OTA_ENABLED == 1)
    osThreadDef_t task_def;
    osMessageQDef_t queue_def;

    // create queue
    queue_def.item_sz = sizeof(T_HttpOtaMsg);
    queue_def.queue_sz = HTTP_OTA_MSG_QUEUE_SIZE;

    g_tHttpOtaQueueId = osMessageCreate(&queue_def, NULL);

    if(NULL == g_tHttpOtaQueueId)
    {
        OPL_LOG_ERRO(HTTPOTA, "OTA queue create fail");
    }

    // create task
    task_def.name = "HTTP OTA";
    task_def.stacksize = HTTP_OTA_TASK_STACK_SIZE;
    task_def.tpriority = HTTP_OTA_TASK_PRIORITY;
    task_def.pthread = HTTP_OtaTaskHandler;

    g_tHttpOtaTaskId = osThreadCreate(&task_def, (void *)NULL);

    if(NULL == g_tHttpOtaTaskId)
    {
        OPL_LOG_ERRO(HTTPOTA, "OTA task create fail");
    }

    OPL_LOG_INFO(HTTPOTA, "OTA task create ok");

#else
    OPL_LOG_WARN(HTTPOTA, "OTA mngr not enable");
#endif
}
