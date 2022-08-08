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
*  cloud_ota_http.c
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
#include "cloud_ota_http.h"
#include "cloud_kernel.h"
#include "cloud_config.h"
#include "controller_wifi.h"
#include "controller_wifi_com.h"
#include "httpclient.h"
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

osThreadId g_tCloudOtaTaskId;
osMessageQId g_tCloudOtaQueueId;

static httpclient_t g_tCloudOtaHttpClient = {0};

static uint16_t g_u16OtaDtimId = 0;
static uint16_t g_u16OtaSeqId = 0;

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

/*************************************************************************
* FUNCTION:
*   _Cloud_OtaProcessTimeoutHandler
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
void _Cloud_OtaProcessTimeoutHandler(void)
{
    // user implement

    // *

    OPL_LOG_WARN(CLOUD, "OTA timeout, reboot after 3s");

    OTA_TriggerReboot(3000);

    // *
}

/*************************************************************************
* FUNCTION:
*   _Cloud_OtaHttpRetrieveOffset
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
int32_t _Cloud_OtaHttpRetrieveOffset(httpclient_t *client, httpclient_data_t *client_data, int offset, int parse_hdr)
{
    int8_t i8Ret = HTTPCLIENT_ERROR_CONN;
    uint8_t u8ParseHdr = parse_hdr;
    T_MwOtaFlashHeader *ptOtaHdr = NULL;

    do
    {
        // receive response from server
        i8Ret = httpclient_recv_response(client, client_data);

        if(0 > i8Ret)
        {
            OPL_LOG_ERRO(CLOUD, "http client recv response error (ret = %d)", i8Ret);
            return i8Ret;
        }

        if(1 == u8ParseHdr)
        {
            OPL_HEX_DUMP_DEBG(CLOUD, (uint8_t *)client_data->response_buf, CLOUD_OTA_HTTP_DATA_LEN - 1);

            ptOtaHdr = (T_MwOtaFlashHeader *)client_data->response_buf;

            OPL_LOG_DEBG(CLOUD, "pid %d, cid %d, fid %d, sum %d, total len %d", ptOtaHdr->uwProjectId,
                                                                                ptOtaHdr->uwChipId,
                                                                                ptOtaHdr->uwFirmwareId,
                                                                                ptOtaHdr->ulImageSum,
                                                                                ptOtaHdr->ulImageSize);

            if(OTA_UpgradeBegin(&g_u16OtaSeqId, ptOtaHdr, _Cloud_OtaProcessTimeoutHandler) != OPL_OK)
            {
                OPL_LOG_ERRO(CLOUD, "OTA begin upgrade fail");
                return -1;
            }

            u8ParseHdr = 0;
        }

        if((client_data->response_content_len - client_data->retrieve_len) == offset)
        {
            break;
        }

    } while (i8Ret == HTTPCLIENT_RETRIEVE_MORE_DATA);

    return HTTPCLIENT_OK;
}

/*************************************************************************
* FUNCTION:
*   _Cloud_OtaHttpRetrieveGet
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
int32_t _Cloud_OtaHttpRetrieveGet(char *url, char *buf, uint32_t len)
{
    int8_t i8Ret = HTTPCLIENT_ERROR_CONN;

    httpclient_data_t tCloudOtaHttpClientData = {0};

    uint32_t u32WriteCount = 0;
    uint32_t u32DataRecv = 0;
    uint32_t u32RecvTemp = 0;
    uint32_t u32DataLen = 0;

    tCloudOtaHttpClientData.response_buf = buf;
    tCloudOtaHttpClientData.response_buf_len = len;

    // send request to server
    i8Ret = httpclient_send_request(&g_tCloudOtaHttpClient, url, HTTPCLIENT_GET, &tCloudOtaHttpClientData);

    if(0 > i8Ret)
    {
        OPL_LOG_ERRO(CLOUD, "http client fail to send request");
        return i8Ret;
    }

    // response body start

    // skip 2nd boot agent, 0x00000000 ~ 0x00003000 : 12KB
    i8Ret = _Cloud_OtaHttpRetrieveOffset(&g_tCloudOtaHttpClient, &tCloudOtaHttpClientData, MW_OTA_HEADER_ADDR_1, 0);
    if(0 > i8Ret)
    {
        OPL_LOG_ERRO(CLOUD, "http retrieve offset error, i8Ret = %d", i8Ret);
        return i8Ret;
    }

    // parse 1st OTA header, 0x00003000 ~ 0x00004000 : 4KB
    i8Ret = _Cloud_OtaHttpRetrieveOffset(&g_tCloudOtaHttpClient, &tCloudOtaHttpClientData, MW_OTA_HEADER_ADDR_2, 1);
    if(0 > i8Ret)
    {
        OPL_LOG_ERRO(CLOUD, "http retrieve offset error, i8Ret = %d", i8Ret);
        return i8Ret;
    }

    // skip 2nd OTA header, 0x00004000 ~ 0x00005000 : 4KB
    i8Ret = _Cloud_OtaHttpRetrieveOffset(&g_tCloudOtaHttpClient, &tCloudOtaHttpClientData, MW_OTA_IMAGE_ADDR_1, 0);
    if(0 > i8Ret)
    {
        OPL_LOG_ERRO(CLOUD, "http retrieve offset error, i8Ret = %d", i8Ret);
        return i8Ret;
    }

    u32RecvTemp = tCloudOtaHttpClientData.retrieve_len;
    u32DataRecv = tCloudOtaHttpClientData.response_content_len - tCloudOtaHttpClientData.retrieve_len;

    do
    {
        // receive response from server
        i8Ret = httpclient_recv_response(&g_tCloudOtaHttpClient, &tCloudOtaHttpClientData);
        if (i8Ret < 0)
        {
            OPL_LOG_ERRO(CLOUD, "http client recv response error, i8Ret = %d", i8Ret);
            return i8Ret;
        }

        u32DataLen = u32RecvTemp - tCloudOtaHttpClientData.retrieve_len;

        if(OTA_WriteData(g_u16OtaSeqId, (uint8_t *)tCloudOtaHttpClientData.response_buf, u32DataLen) != OPL_OK)
        {
            OPL_LOG_ERRO(CLOUD, "OTA write data fail");
            return -1;
        }

        u32WriteCount += u32DataLen;
        u32DataRecv += u32DataLen;
        u32RecvTemp = tCloudOtaHttpClientData.retrieve_len;

        OPL_LOG_INFO(CLOUD, "written image length %d", u32WriteCount);
        OPL_LOG_INFO(CLOUD, "download progress %u", u32DataRecv * 100 / tCloudOtaHttpClientData.response_content_len);

    } while (i8Ret == HTTPCLIENT_RETRIEVE_MORE_DATA);

    OPL_LOG_INFO(CLOUD, "total write data length: %d", u32WriteCount);
    OPL_LOG_INFO(CLOUD, "data received length: %d", u32DataRecv);

    if(u32DataRecv != tCloudOtaHttpClientData.response_content_len || httpclient_get_response_code(&g_tCloudOtaHttpClient) != 200)
    {
        OPL_LOG_ERRO(CLOUD, "data received not completed, or invalid error code");
        return -1;
    }
    else if(u32DataRecv == 0)
    {
        OPL_LOG_ERRO(CLOUD, "receive length is zero, file not found");
        return -2;
    }
    else
    {
        OPL_LOG_INFO(CLOUD, "download success");
        return i8Ret;
    }
}

/*************************************************************************
* FUNCTION:
*   Cloud_OtaHttpDownload
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
int32_t Cloud_OtaHttpDownload(char *url, int len)
{
    char get_url[CLOUD_OTA_HTTP_URL_LEN];
    char *buf;

    int8_t i8Ret = HTTPCLIENT_ERROR_CONN;
    uint32_t u16UrlLen = strlen(url);
    uint16_t u16Pid, u16Cid, u16Fid;
    uint8_t u8RetryCount = 0;

    if(1 > u16UrlLen)
    {
        return -1;
    }

    memset(get_url, 0, CLOUD_OTA_HTTP_URL_LEN);
    strncpy(get_url, url, len);

    OPL_LOG_DEBG(CLOUD, "OTA url %s (len: %d)", get_url, u16UrlLen);

    buf = malloc(CLOUD_OTA_HTTP_DATA_LEN);

    if(NULL == buf)
    {
        OPL_LOG_ERRO(CLOUD, "Alloc buf fail");

        return -1;
    }

    lwip_auto_arp_enable(1, 0);

    g_tCloudOtaHttpClient.timeout_ms = CLOUD_OTA_HTTP_CONNECTION_TIMEOUT;

    // connect to http server
    do
    {
        i8Ret = httpclient_connect(&g_tCloudOtaHttpClient, get_url);
        if(!i8Ret)
        {
            OPL_LOG_INFO(CLOUD, "Connect to http server");
            break;
        }
        else
        {
            osDelay(1000);
            u8RetryCount ++;
            OPL_LOG_WARN(CLOUD, "Connect to http server fail, retry count (%d)", u8RetryCount);
        }
    } while(u8RetryCount < CLOUD_OTA_HTTP_CONNECTION_RETRY_COUNT);

    // get current ota version
    OTA_CurrentVersionGet(&u16Pid, &u16Cid, &u16Fid);

    OPL_LOG_DEBG(CLOUD, "pid=%d, cid=%d, fid=%d", u16Pid, u16Cid, u16Fid);

    i8Ret = _Cloud_OtaHttpRetrieveGet(get_url, buf, CLOUD_OTA_HTTP_DATA_LEN);

    if(0 > i8Ret)
    {
        if(OPL_OK != OTA_UpgradeGiveUp(g_u16OtaSeqId))
        {
            OPL_LOG_ERRO(CLOUD, "OTA give up fail");
        }
    }
    else
    {
        if(OPL_OK != OTA_UpgradeFinish(g_u16OtaSeqId))
        {
            OPL_LOG_ERRO(CLOUD, "OTA finish fail");
        }
    }

    OPL_LOG_INFO(CLOUD, "download result = %d", i8Ret);

    // close http connection
    httpclient_close(&g_tCloudOtaHttpClient);

    free(buf);
    buf = NULL;

    return i8Ret;
}

/*************************************************************************
* FUNCTION:
*   Cloud_OtaTriggerReq
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
T_OplErr Cloud_OtaTriggerReq(uint32_t u32EventId, uint8_t *pu8Data, uint32_t u32DataLen)
{
    if(NULL == g_tCloudOtaQueueId)
    {
        OPL_LOG_WARN(CLOUD, "Queue not init");

        return OPL_ERR_RTOS_QMSG_NOT_INIT;
    }

    T_CloudDataMsg *ptCloudOtaMsg = (T_CloudDataMsg *)malloc(sizeof(T_CloudDataMsg) + u32DataLen);

    if(NULL == ptCloudOtaMsg)
    {
        OPL_LOG_ERRO(CLOUD, "Alloc Tx message fail");

        return OPL_ERR_ALLOC_MEMORY_FAIL;
    }

    // prepare the data
    ptCloudOtaMsg->u32EventId = u32EventId;
    ptCloudOtaMsg->u32DataLen = u32DataLen;
    if(0 != ptCloudOtaMsg->u32DataLen)
    {
        memcpy(ptCloudOtaMsg->u8aData, pu8Data, u32DataLen);
    }

    // send message
    if(osOK != osMessagePut(g_tCloudOtaQueueId, (uint32_t)ptCloudOtaMsg, 0))
    {
        OPL_LOG_ERRO(CLOUD, "Send message fail");

        free(ptCloudOtaMsg);

        return OPL_ERR_RTOS_SEND_QMSG_FAIL;
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION:
*   Cloud_OtaTaskHandler
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
void Cloud_OtaTaskHandler(void *args)
{
    osEvent tEvent;
    T_CloudDataMsg *ptCloudOtaMsg;

    for(;;)
    {
        // wait event
        tEvent = osMessageGet(g_tCloudOtaQueueId, osWaitForever);

        if(tEvent.status == osEventMessage)
        {
            ptCloudOtaMsg = (T_CloudDataMsg *)tEvent.value.p;

            switch(ptCloudOtaMsg->u32EventId)
            {
                case CLOUD_OTA_EVT_TYPE_DOWNLOAD:
                {
                    if(OPL_OK != Opl_Wifi_Skip_Dtim_Module_Reg(&g_u16OtaDtimId))
                    {
                        OPL_LOG_WARN(CLOUD, "OTA Dtim ID reg fail");
                    }

                    // disable skip dtim
                    Opl_Wifi_Skip_Dtim_Set(g_u16OtaDtimId, false);

                    lwip_one_shot_arp_enable();

#if defined(OPL1000_A2) || defined(OPL1000_A3)
                    CtrlWifi_PsStateForce(STA_PS_AWAKE_MODE, 0);
#elif defined(OPL2500_A0)
                    CtrlWifi_PsStateForce(STA_MODE_PERFORMANCE, 0);
#endif

                    int32_t i32Ret = Cloud_OtaHttpDownload((char *)ptCloudOtaMsg->u8aData, (int)ptCloudOtaMsg->u32DataLen);

                    // enable skip dtim
                    Opl_Wifi_Skip_Dtim_Set(g_u16OtaDtimId, true);

#if defined(OPL1000_A2) || defined(OPL1000_A3)
                    CtrlWifi_PsStateForce(STA_PS_NONE, 0);
#elif defined(OPL2500_A0)
                    CtrlWifi_PsStateForce(STA_MODE_LIGHT_PS, 0);
#endif

                    if(0 != i32Ret)
                    {
                        OPL_LOG_ERRO(CLOUD, "OTA proc fail");
                    }
                    else
                    {
                        // restart the system
                        OTA_TriggerReboot(3000);
                    }

                    break;
                }

                case CLOUD_OTA_EVT_TYPE_GET_VERSION:
                {
                    break;
                }

                default:
                    break;

            }

            if(NULL != ptCloudOtaMsg)
            {
                free(ptCloudOtaMsg);
            }
        }
    }
}

/*************************************************************************
* FUNCTION:
*   Cloud_OtaTaskInit
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
void Cloud_OtaTaskInit(void)
{
#if (OTA_ENABLED == 1)
    osThreadDef_t task_def;
    osMessageQDef_t queue_def;

    // create queue
    queue_def.item_sz = sizeof(T_CloudDataMsg);
    queue_def.queue_sz = CLOUD_OTA_MSG_QUEUE_SIZE;

    g_tCloudOtaQueueId = osMessageCreate(&queue_def, NULL);

    if(NULL == g_tCloudOtaQueueId)
    {
        OPL_LOG_ERRO(CLOUD, "OTA queue create fail");
    }

    // create task
    task_def.name = "Cloud OTA";
    task_def.stacksize = CLOUD_OTA_TASK_STACK_SIZE;
    task_def.tpriority = CLOUD_OTA_TASK_PRIORITY;
    task_def.pthread = Cloud_OtaTaskHandler;

    g_tCloudOtaTaskId = osThreadCreate(&task_def, (void *)NULL);

    if(NULL == g_tCloudOtaTaskId)
    {
        OPL_LOG_ERRO(CLOUD, "OTA task create fail");
    }

    OPL_LOG_INFO(CLOUD, "OTA task create ok");

#else
    OPL_LOG_WARN(CLOUD, "OTA not enable");
#endif
}
