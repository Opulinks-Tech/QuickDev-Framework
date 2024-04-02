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
*  app_at_cmd.c
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
#include "app_at_cmd.h"
#include "app_main.h"
#include "at_cmd.h"
#include "at_cmd_common.h"
#if defined(OPL1000_A2) || defined(OPL1000_A3)
#include "at_cmd_task_patch.h"
#include "at_cmd_data_process_patch.h"
#include "agent_patch.h"
#elif defined(OPL2500_A0)
#include "at_cmd_task.h"
#include "at_cmd_data_process.h"
#include "agent.h"
#endif
#include "mw_fim.h"
#include "mw_fim_default_group12_project.h"
#include "opl_err.h"
#include "pwr_save.h"

#if (WM_ENABLED == 1 && WM_AT_ENABLED == 1)
#include "wifi_mngr_at.h"
#endif

#if (NM_ENABLED == 1 && NM_AT_ENABLED == 1)
#include "net_mngr_at.h"
#endif

#if (WM_AC_ENABLED == 1 && WM_AC_AT_ENABLED == 1)
#include "wifi_autocon_at.h"
#endif

#if (BM_ENABLED == 1 && BM_AT_ENABLED == 1)
#include "ble_mngr_at.h"
#endif

#include <ctype.h>
#include "http_file.h"

#if(AWS_ENABLED == 1)
#include "cloud_ctrl.h"
#include "cloud_config.h"
#include "cloud_kernel.h"
#endif

#include "gap_svc.h"
//#include "devinfo_svc.h"
#include "transfer.h"

#include "ble_mngr_api.h"
#include "net_mngr_api.h"
#include "wifi_mngr_api.h"
//#include "http_ota.h"
#if (OTA_ENABLED == 1)
#include "ota_mngr.h"
#endif

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#define AT_LOG                                          msg_print_uart1
//#define WAKEUP_PIN                                      GPIO_IDX_10
#define HTTPCLIENT_MAX_URL_LEN                          256
#define AT_CLIENT_IDENTIFIER_LENGTH                     (64U)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

// application AT cmd list
SHM_DATA at_command_t g_taAppAtCmd[] =
{
    // user implement
    // *

#if 0
    {"at+writefim",        AT_CmdFimReadHandler,                "Write FIM data" },
    {"at+readfim",         AT_CmdFimWriteHandler,               "Read FIM data" },
#endif
    // * range for BLE commands
    {"at+blestart",        AT_CmdBleStartHandler,               "Start BLE adv"},
    {"at+blestop",         AT_CmdBleStoptHandler,               "Stop BLE adv"},
    //{"at+bleadvdataset",   AT_CmdBleAdvDataHandler,             "BLE adv data set"},
    //{"at+bleappear",       AT_CmdBleAppearanceHandler,          "BLE appearance"},
    //{"at+blefwrev",        AT_CmdBleFwRevHandler,               "BLE FW REV set"},
    //{"at+blemodelnb",      AT_CmdBleModelNbHandler,             "BLE Model NB set"},
    //{"at+bledevmac",       AT_CmdBleDevMacHandler,              "BLE mac address get"},
    //{"at+blenotify",       AT_CmdBleNotifyHandler,              "BLE sending notify"},
    // * end

    // * range for WIFI commands
#if(OPL_DATA_ENABLED == 0)
    {"at+netscan",         AT_CmdWifiScanHandler,               "WIFI scan"},
    {"at+netconnect",      AT_CmdWifiConnectHandler,            "WIFI connect"},
    {"at+netdisconnect",   AT_CmdWifiDisconnectHandler,         "WIFI disconnect"},
#endif
    {"at+netapinfo",       AT_CmdWifiApInfoHandler,             "WIFI ap info get"},
    {"at+netdevmac",       AT_CmdWifiDevMacHandler,             "WIFI mac address get"},
    {"at+netreset",        AT_CmdWifiResetHandler,              "Reset Wi-Fi"},
    // * end

    // * range for Cloud commands
#if(AWS_ENABLED == 1)
    {"at+mqttinit",        AT_CmdMqttInitHandler,               "Mqtt Init"},
    {"at+mqttconnect",     AT_CmdMqttConnectHandler,            "Mqtt Connect"},
    {"at+mqttdisconnect",  AT_CmdMqttDisconnectHandler,         "Mqtt Disconnect"},
    {"at+mqttkpintvl",     AT_CmdMqttKeepAliveIntvlHandler,     "Mqtt Keep alive interval set"},
    {"at+mqttsub",         AT_CmdMqttSubTopicHandler,           "Mqtt Subscribe Topic"},
    {"at+mqttunsub",       AT_CmdMqttUnsubTopicHandler,         "Mqtt UnSubscribe Topic"},
    {"at+mqttpub",         AT_CmdMqttPubDataHandler,            "Mqtt Publish Data"},
    {"at+mqttclientid",    AT_CmdMqttClientIdHandler,           "Mqtt Client ID Set"},
    {"at+mqttlastwill",    AT_CmdMqttLastWillHandler,           "Mqtt Last Will Set"},
    {"at+fulmqttcert",     AT_CmdFulMqttCertHandler,            "Upload MQTT root CA/client cert/priv key file"},
#endif
    // * end

    // * range for File commands
#if(HTTP_ENABLED == 1)
    {"at+httppost",        AT_CmdHttpPostHandler,               "Http Post"},
    {"at+httpget",         AT_CmdHttpGetHandler,                "Http Get"},
#endif
    
    // *end

    // * range for System or else commands
    {"at+sleepmode",       AT_CmdSleepHandler,                 "Enter sleep: 1 = smart sleep, 2 = timer sleep, 3 = deep sleep"},
    //{"at+moduleota",       AT_CmdModuleOtaHandler,              "Module HTTP OTA"},
    // * end

    // * Host wakeup commands
    {"at+hostready",       AT_CmdHostReadyHandler,              "Host ready for message"},
    // * end

    // *

    // wifi manager - wifi_mngr_at.h
#if (WM_ENABLED == 1 && WM_AT_ENABLED == 1)
    {"at+scan",            AT_CmdWmScanHandler,                 "Send scan request to wifi agent"},
    {"at+connect",         AT_CmdWmConnectHandler,              "Send connect request to wifi agent"},
    {"at+disconnect",      AT_CmdWmDisconnectHandler,           "Send disconnect request to wifi agent"},
    {"at+profile",         AT_CmdApProfileHandler,              "Access profile recorder"},
    {"at+wmsta",           AT_CmdWmMngrStatusChkHandler,        "Check WI-FI status(WA)"},
#endif

    // network manager - net_mngr_at.h
#if (NM_ENABLED == 1 && NM_AT_ENABLED == 1)
    {"at+nmscan",          AT_CmdNmScanHandler,                 "Send scan request to network manager"},
    {"at+nmconnect",       AT_CmdNmConnectHandler,              "Send connect request to network manager"},
    {"at+nmqset",          AT_CmdNmQuickConnectSetHandler,      "Send quick connect set request to network manager"},
#endif

    // wifi auto-connect - wifi_autocon_at.h
#if (WM_AC_ENABLED == 1 && WM_AC_AT_ENABLED == 1)
    {"at+acswitch",        AT_CmdWmAcSwitchHandler,             "Switch Auto-Connect procedure"},
    {"at+acsta",           AT_CmdWmAcStatusChkHandler,          "Check WI-FI status(AC)"},
#endif

    // ble manager - ble_mngr_at.h
#if (BM_ENABLED == 1 && BM_AT_ENABLED == 1)
    {"at+bleswitch",       AT_CmdBmAdvSwitchHandler,            "BLE switch start / stop"},
#endif

    // others
    {"at+appfwver",        AT_CmdAppFwVer,                      "Get App firmware version"},

    {NULL,                 NULL,                                NULL},
};

typedef struct
{
    uint32_t u32Id;
    uint16_t u16Index;
    uint16_t u16DataTotalLen;

    uint32_t u32DataRecv;       // Calcuate the receive data
    uint32_t TotalSize;         // user need to input total bytes

    char     u8aReadBuf[8];
    uint8_t  *ResultBuf;
    uint32_t u32StringIndex;       // Indicate the location of reading string
    uint16_t u16Resultindex;       // Indicate the location of result string
    uint8_t  fIgnoreRestString;    // Set a flag for last one comma
    uint8_t  u8aTemp[1];
} T_AtFimParam;

#if(AWS_ENABLED == 1)
typedef struct S_UploadFileStruct
{
    T_UploadFileType    tFileType;
    uint32_t            u32DataLen;
    uint32_t            u32RecvLen;
    uint8_t             pau8DataBuf[];
}T_UploadFileStruct;

typedef struct S_MqttPayloadFmt
{
    T_CloudPayloadFmt tCloudPayload;                      
    uint16_t u16RecvLen;                                // receive data len from MCU
}T_MqttPayloadFmt;
#endif  

typedef struct S_HttpPostFileStruct
{
    uint32_t            u32DataLen;
    uint32_t            u32RecvLen;
    uint32_t            u32UrlLen;
    char                aUrl[HTTPCLIENT_MAX_URL_LEN];
    uint8_t             pau8DataBuf[];
}T_HttpPostFileStruct;

typedef struct S_HttpGetFileStruct
{
    uint32_t            u32DataLen;
    uint32_t            u32RecvLen;
    uint8_t             pau8DataBuf[];
}T_HttpGetFileStruct;

typedef struct S_BleNotifyStruct
{
    uint32_t            u32DataLen;
    uint32_t            u32RecvLen;
    uint8_t             pau8DataBuf[];
}T_BleNotifyStruct;
/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable
//#if 1
#if (HTTP_ENABLED == 1)
extern char *g_pcSslServerCa;
#endif

// Sec 5: declaration of global function prototype

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

static bool AT_CheckAtAccept(void)
{
#if(HTTP_ENABLED == 1)
    if(true == Http_FileInProgress() /*|| true == HTTP_OtaInProgress()*/)
    {
        //if module is progressing OTA or file download, reject the at command
        return false;
    }
#endif
#if(OTA_ENABLED == 1)
    if(true == OTA_InProgress())
    {
        return false;
    }
#endif
    return true;
}

void AT_SmartSleepIoCallback(E_GpioIdx_t tWakeupType)
{
    PS_ExitSmartSleep();

    APP_SendMessage(APP_EVT_IO_EXIT_SMART_SLEEP_IND, NULL, 0);
    
}

void AT_StrToHex(char *pbDest, char *pbSrc, uint8_t u8DataLen)
{
    char h1,h2;
    char s1,s2;
    int i = 0;

    for (; i < u8DataLen/2; i++)
    {
        h1 = pbSrc[2*i];
        h2 = pbSrc[2*i+1];

        s1 = toupper(h1) - 0x30;
        if (s1 > 9)
            s1 -= 7;
        s2 = toupper(h2) - 0x30;
        if (s2 > 9)
            s2 -= 7;

        pbDest[i] = s1*16 + s2;
    }
}

#if(AWS_ENABLED == 1)
SHM_DATA int AT_MqttLastWillCB(uint32_t u32Type, uint8_t *u8aData, uint32_t u32DataLen, void *pParam)
{
    T_MqttPayloadFmt *ptMqttPayloadInfo = (T_MqttPayloadFmt *)pParam;
	int iRet = 1;
    uint32_t i = 0;

    for(; i < u32DataLen; i++)
    {
        ptMqttPayloadInfo->tCloudPayload.u8aPayloadBuf[ptMqttPayloadInfo->u16RecvLen + i] = u8aData[i];
    }

    ptMqttPayloadInfo->u16RecvLen += u32DataLen;

    printf("Topic:%s, Topic len:%d, Qos:%d, Payload: %s, Payload len: %d, Recv len: %d\r\n", ptMqttPayloadInfo->tCloudPayload.u8TopicName
                                                                                        , ptMqttPayloadInfo->tCloudPayload.u16TopicNameLen  
                                                                                        , ptMqttPayloadInfo->tCloudPayload.u8QoS
                                                                                        , ptMqttPayloadInfo->tCloudPayload.u8aPayloadBuf
                                                                                        , ptMqttPayloadInfo->tCloudPayload.u16PayloadLen
                                                                                        , ptMqttPayloadInfo->u16RecvLen);
 
    if(ptMqttPayloadInfo->u16RecvLen == ptMqttPayloadInfo->tCloudPayload.u16PayloadLen)
    {
        //send recv data as payload and send to cloud
        if(OPL_OK != Cloud_MqttLastWill_Set((T_CloudPayloadFmt *) &ptMqttPayloadInfo->tCloudPayload))
        {
            iRet = 0;
            goto done;
        }

        if(NULL != ptMqttPayloadInfo)
        {
            free(ptMqttPayloadInfo);
            ptMqttPayloadInfo = NULL;
        }	
    }
done:

    if(0 == iRet)
    {   
        AT_FAIL(ACK_TAG_CLOUD_MQTT_LASTWILL_SET, NULL, 0);
    }
    else
    {
        AT_OK(ACK_TAG_CLOUD_MQTT_LASTWILL_SET, NULL, 0);
    }
   
    AT_RETURN(iRet);
}

SHM_DATA int AT_MqttPublishCB(uint32_t u32Type, uint8_t *u8aData, uint32_t u32DataLen, void *pParam)
{
    T_MqttPayloadFmt *ptMqttPayloadInfo = (T_MqttPayloadFmt *)pParam;
	int iRet = 1;
    uint32_t i = 0;

    for(; i < u32DataLen; i++)
    {
        ptMqttPayloadInfo->tCloudPayload.u8aPayloadBuf[ptMqttPayloadInfo->u16RecvLen + i] = u8aData[i];
    }

    ptMqttPayloadInfo->u16RecvLen += u32DataLen;

    printf("Topic:%s, Topic len:%d, Qos:%d, Payload: %s, Payload len: %d, Recv len: %d\r\n", ptMqttPayloadInfo->tCloudPayload.u8TopicName
                                                                                           , ptMqttPayloadInfo->tCloudPayload.u16TopicNameLen  
                                                                                           , ptMqttPayloadInfo->tCloudPayload.u8QoS
                                                                                           , ptMqttPayloadInfo->tCloudPayload.u8aPayloadBuf
                                                                                           , ptMqttPayloadInfo->tCloudPayload.u16PayloadLen
                                                                                           , ptMqttPayloadInfo->u16RecvLen);
 
    if(ptMqttPayloadInfo->u16RecvLen == ptMqttPayloadInfo->tCloudPayload.u16PayloadLen)
    {
        //send recv data as payload and send to cloud
        if(OPL_OK != APP_SendMessage(APP_EVT_CLOUD_MQTT_PUB_DATA_REQ, (uint8_t *) &ptMqttPayloadInfo->tCloudPayload, sizeof(ptMqttPayloadInfo->tCloudPayload)))
        {
            AT_FAIL(ACK_TAG_CLOUD_MQTT_PUB_DATA_REQ, NULL, 0);
            iRet = 0;
        }

        if(NULL != ptMqttPayloadInfo)
        {
            free(ptMqttPayloadInfo);
            ptMqttPayloadInfo = NULL;
        }	
    }

    AT_RETURN(iRet);
}

SHM_DATA int AT_UploadFileRecvCB(uint32_t u32Type, uint8_t *u8aData, uint32_t u32DataLen, void *pParam)
{
    T_UploadFileStruct *ptUploadFileStruct = (T_UploadFileStruct *)pParam;

    int8_t iRet = 0;
    uint32_t i = 0;

    for(; i < u32DataLen; i ++)
    {
        #if 0 // Workaround for test, should be turn off in normal case
        if(0x5c==u8aData[i])
        {
            u8aData[i] = 0xA;
        }
        #endif
        ptUploadFileStruct->pau8DataBuf[ptUploadFileStruct->u32RecvLen + i] = u8aData[i];
    }

    ptUploadFileStruct->u32RecvLen += u32DataLen;
    
    if(ptUploadFileStruct->u32RecvLen == ptUploadFileStruct->u32DataLen)
    {
        switch(ptUploadFileStruct->tFileType)
        {
            case UPLOAD_FILE_TYPE_MQTT_ROOT_CA:
            {
                Cloud_MqttFileSet(UPLOAD_FILE_TYPE_MQTT_ROOT_CA, ptUploadFileStruct->pau8DataBuf, ptUploadFileStruct->u32DataLen);
                
                break;
            }

            case UPLOAD_FILE_TYPE_MQTT_CLIENT_CERT:
            {
                Cloud_MqttFileSet(UPLOAD_FILE_TYPE_MQTT_CLIENT_CERT, ptUploadFileStruct->pau8DataBuf, ptUploadFileStruct->u32DataLen);
                
                break;
            }

            case UPLOAD_FILE_TYPE_MQTT_PRIVATE_KEY:
            {
                Cloud_MqttFileSet(UPLOAD_FILE_TYPE_MQTT_PRIVATE_KEY, ptUploadFileStruct->pau8DataBuf, ptUploadFileStruct->u32DataLen);               
                
                break;
            }
//#if 1
#if (HTTP_ENABLED == 1)
            case UPLOAD_FILE_TYPE_HTTPS_CA_CHAIN:
            {
                if(NULL != g_pcSslServerCa)
                {
                    free(g_pcSslServerCa);
                    g_pcSslServerCa = NULL;
                }

                g_pcSslServerCa = (char *) malloc(ptUploadFileStruct->u32DataLen + 1);

                if(g_pcSslServerCa == NULL)
                {
                    goto done;
                }

                memset(g_pcSslServerCa, 0, ptUploadFileStruct->u32DataLen + 1);
                memcpy(g_pcSslServerCa, ptUploadFileStruct->pau8DataBuf, ptUploadFileStruct->u32DataLen);
                g_pcSslServerCa[ptUploadFileStruct->u32DataLen] = '\0';

                break;
//#endif
            }
#endif
            default:            
                goto done; 
            
            
        }

        iRet = 1;
   
done:
        if(0 == iRet)
        {
            AT_FAIL(ACK_TAG_FILE_UPLOAD, &ptUploadFileStruct->tFileType, sizeof(ptUploadFileStruct->tFileType));

        }
        else
        {
            AT_OK(ACK_TAG_FILE_UPLOAD, &ptUploadFileStruct->tFileType, sizeof(ptUploadFileStruct->tFileType));
        }

        if(ptUploadFileStruct != NULL)
        {
            free(ptUploadFileStruct);
					  ptUploadFileStruct = NULL;
        }
    }
    
    AT_RETURN(iRet);
}
#endif

#if(HTTP_ENABLED == 1)
SHM_DATA int AT_HttpPostFileRecvCB(uint32_t u32Type, uint8_t *u8aData, uint32_t u32DataLen, void *pParam)
{
    T_HttpPostFileStruct *ptHttpPostFileStruct = (T_HttpPostFileStruct *)pParam;
    int8_t iRet = 0;
    uint32_t i = 0;
    
    for(; i < u32DataLen; i ++)
    {        
   	    ptHttpPostFileStruct->pau8DataBuf[ptHttpPostFileStruct->u32RecvLen + i] = u8aData[i];        
    }

    ptHttpPostFileStruct->u32RecvLen += u32DataLen;
    
    if(ptHttpPostFileStruct->u32RecvLen == ptHttpPostFileStruct->u32DataLen)
    {
#if(AWS_ENABLED == 1)
        if(false == Cloud_NetworkStatusGet())
        {
            goto done;
        }
#endif
        if(OPL_OK == Http_FilePostSet(ptHttpPostFileStruct->pau8DataBuf, ptHttpPostFileStruct->u32DataLen))
        {
            if(OPL_OK != Http_FileTriggerReq(FILE_EVT_TYPE_HTTP_POST,  (uint8_t *)ptHttpPostFileStruct->aUrl, strlen(ptHttpPostFileStruct->aUrl)))
            {
                goto done;
            }
        }
        else
        {
            goto done;
        }
    
        iRet = 1;
   
done:
        if(0 == iRet)
        {
            AT_FAIL(ACK_TAG_FILE_HTTP_POST_REQ, NULL, 0);

        }
        else
        {
            AT_OK(ACK_TAG_FILE_HTTP_POST_REQ, NULL, 0);
        }

        if(ptHttpPostFileStruct != NULL)
        {  
            free(ptHttpPostFileStruct);
            ptHttpPostFileStruct = NULL;
        }
    }
    
    AT_RETURN(iRet);
}

SHM_DATA int AT_HttpGetFileRecvCB(uint32_t u32Type, uint8_t *u8aData, uint32_t u32DataLen, void *pParam)
{
    T_HttpGetFileStruct *ptHttpGetFileStruct = (T_HttpGetFileStruct *)pParam;
    int8_t iRet = 0;
    uint32_t i = 0;

    for(; i < u32DataLen; i ++)
    {        
        ptHttpGetFileStruct->pau8DataBuf[ptHttpGetFileStruct->u32RecvLen + i] = u8aData[i];        
    }

    ptHttpGetFileStruct->u32RecvLen += u32DataLen;
    
    if(ptHttpGetFileStruct->u32RecvLen == ptHttpGetFileStruct->u32DataLen)
    {
#if(AWS_ENABLED == 1)
        if(false == Cloud_NetworkStatusGet())
        {
            goto done;
        }
#endif
        if(OPL_OK != Http_FileTriggerReq(FILE_EVT_TYPE_HTTP_GET,  (uint8_t *)ptHttpGetFileStruct->pau8DataBuf, ptHttpGetFileStruct->u32DataLen))
        {
            goto done;
        }

    
        iRet = 1;
   
done:
        if(0 == iRet)
        {
            AT_FAIL(ACK_TAG_FILE_HTTP_GET_REQ, NULL, 0);

        }
        else
        {
            AT_OK(ACK_TAG_FILE_HTTP_GET_REQ, NULL, 0);
        }

        if(ptHttpGetFileStruct != NULL)
        {
            free(ptHttpGetFileStruct);
            ptHttpGetFileStruct = NULL;
        }
    }
    
    AT_RETURN(iRet);
}
#endif
/// fim control

#if 0
/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
int AT_CmdFimWrite(uint32_t u32Type, uint8_t *u8aData, uint32_t u32DataLen, void *pParam)
{
    T_AtFimParam *ptParam = (T_AtFimParam *)pParam;

    uint8_t iRet = 0;
    uint8_t u8acmp[] = ",\0";
    uint32_t i = 0;

    ptParam->u32DataRecv += u32DataLen;

    /* If previous segment is error then ignore the rest of segment */
    if (ptParam->fIgnoreRestString)
    {
        goto done;
    }

    for (i=0 ; i<u32DataLen; i++)
    {
        if (u8aData[i] != u8acmp[0])
        {
            if (ptParam->u32StringIndex >= AT_FIM_DATA_LENGTH)
            {
                ptParam->fIgnoreRestString = 1;
                goto done;
            }

            /* compare string. If not comma then store into array. */
            ptParam->u8aReadBuf[ptParam->u32StringIndex] = u8aData[i];
            ptParam->u32StringIndex++;
        }
        else
        {
            /* Convert string into Hex and store into array */
            ptParam->ResultBuf[ptParam->u16Resultindex] = (uint8_t)strtoul(ptParam->u8aReadBuf, NULL, 16);

            /* Result index add one */
            ptParam->u16Resultindex++;

            /* re-count when encounter comma */
            ptParam->u32StringIndex=0;
        }
    }

    /* If encounter the last one comma
       1. AT_FIM_DATA_LENGTH:
       Max character will pick up to compare.

       2. (ptParam->u16DataTotalLen - 1):
       If total length minus 1 is equal (ptParam->u16Resultindex) mean there is no comma at the rest of string.
    */
    if ((ptParam->u16Resultindex == (ptParam->u16DataTotalLen - 1)) && (ptParam->u32StringIndex >= AT_FIM_DATA_LENGTH))
    {
        ptParam->ResultBuf[ptParam->u16Resultindex] = (uint8_t)strtoul(ptParam->u8aReadBuf, NULL, 16);

        /* Result index add one */
        ptParam->u16Resultindex++;
    }

    /* Collect array data is equal to total lengh then write data to fim. */
    if (ptParam->u16Resultindex == ptParam->u16DataTotalLen)
    {
       	if (MW_FIM_OK == MwFim_FileWrite(ptParam->u32Id, ptParam->u16Index, ptParam->u16DataTotalLen, ptParam->ResultBuf))
        {
            msg_print_uart1("OK\r\n");
        }
        else
        {
            ptParam->fIgnoreRestString = 1;
        }
    }
    else
    {
        goto done;
    }

done:
    if (ptParam->TotalSize >= ptParam->u32DataRecv)
    {
        if (ptParam->fIgnoreRestString)
        {
            msg_print_uart1("ERROR\r\n");
        }

        if (ptParam != NULL)
        {
            if (ptParam->ResultBuf != NULL)
            {
                free(ptParam->ResultBuf);
            }
            free(ptParam);
            ptParam = NULL;
        }
    }

    return iRet;
}

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
int AT_CmdFimWriteHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    /* Initialization the value */
    T_AtFimParam *tAtFimParam = (T_AtFimParam*)malloc(sizeof(T_AtFimParam));
    if (tAtFimParam == NULL)
    {
        goto done;
    }
    memset(tAtFimParam, 0, sizeof(T_AtFimParam));

    if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
    {
        goto done;
    }

    if (argc != 4)
    {
        msg_print_uart1("invalid param number\r\n");
        goto done;
    }

    /* save parameters to process uart1 input */
    tAtFimParam->u32Id = (uint32_t)strtoul(argv[1], NULL, 16);
    tAtFimParam->u16Index = (uint16_t)strtoul(argv[2], NULL, 0);
    tAtFimParam->u16DataTotalLen = (uint16_t)strtoul(argv[3], NULL, 0);

    /* If user input data length is 0 then go to error.*/
    if (tAtFimParam->u16DataTotalLen == 0)
    {
        goto done;
    }

    switch (mode)
    {
        case AT_CMD_MODE_SET:
        {
            tAtFimParam->TotalSize = ((tAtFimParam->u16DataTotalLen * AT_FIM_DATA_LENGTH_WITH_COMMA) - 1);

            /* Memory allocate a memory block for pointer */
            tAtFimParam->ResultBuf = (uint8_t *)malloc(tAtFimParam->u16DataTotalLen);
            if (tAtFimParam->ResultBuf == NULL)
                goto done;

            // register callback to process uart1 input
            agent_data_handle_reg(AT_CmdFimWrite, tAtFimParam);

            // redirect uart1 input to callback
#if defined(OPL1000_A2) || defined(OPL1000_A3)
            data_process_lock_patch(LOCK_APP, (tAtFimParam->TotalSize));
#elif defined(OPL2500_A0)
            data_process_lock(LOCK_APP, (tAtFimParam->TotalSize));
#endif

            break;
        }

        default:
            goto done;
    }

    iRet = 1;

done:
    if (iRet)
    {
        msg_print_uart1("OK\r\n");
    }
    else
    {
        msg_print_uart1("ERROR\r\n");
        if (tAtFimParam != NULL)
        {
            if (tAtFimParam->ResultBuf != NULL)
            {
		        free(tAtFimParam->ResultBuf);
            }
            free(tAtFimParam);
            tAtFimParam = NULL;
        }
    }

    return iRet;
}

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
int AT_CmdFimReadHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};
    uint32_t i = 0;
    uint8_t *readBuf = NULL;
    uint32_t u32Id  = 0;
    uint16_t u16Index  = 0;
    uint16_t u16Size  = 0;

    if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
    {
        goto done;
    }

    if (argc != 4)
    {
        AT_LOG("invalid param number\r\n");
        goto done;
    }

    u32Id = (uint32_t)strtoul(argv[1], NULL, 16);
    u16Index = (uint16_t)strtoul(argv[2], NULL, 0);
    u16Size = (uint16_t)strtoul(argv[3], NULL, 0);

    if (u16Size == 0)
    {
        AT_LOG("invalid size[%d]\r\n", u16Size);
        goto done;
    }

    switch (mode)
    {
        case AT_CMD_MODE_SET:
        {
            readBuf = (uint8_t *)malloc(u16Size);
            if (NULL == readBuf)
            {
                AT_LOG("malloc fail\r\n");
                goto done;
            }

            if (MW_FIM_OK == MwFim_FileRead(u32Id, u16Index, u16Size, readBuf))
            {
                msg_print_uart1("%02X", readBuf[0]);
                for (i=1 ; i<u16Size; i++)
                {
                    msg_print_uart1(",%02X", readBuf[i]);
                }
            }
            else
            {
                goto done;
            }

            msg_print_uart1("\r\n");
            break;
        }

        default:
            goto done;
    }

    iRet = 1;

done:
    if (iRet)
    {
        msg_print_uart1("OK\r\n");
    }
    else
    {
        msg_print_uart1("ERROR\r\n");
    }

    if (readBuf != NULL)
        free(readBuf);

    return iRet;
}
#endif

// * range for BLE commands
int AT_CmdBleStartHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_SET:
        {
            if(!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                AT_LOG("at_cmd_buf_to_argc_argv\n");
                goto done;
            }

            if(argc < 2)
            {
                goto done;
            }

            uint8_t u8AutoAdv = (uint8_t)atoi(argv[1]);

            if(OPL_OK != APP_SendMessage(APP_EVT_BLE_START_ADV_REQ, &u8AutoAdv, sizeof(u8AutoAdv)))
            {
                goto done;
            }

            iRet = 1;

            break;
        }
        default:
            break;
    }
    
done:
    if(0 == iRet)
    {
        AT_FAIL(ACK_TAG_BLE_START_ADV_REQ, NULL, 0);
    }

    AT_RETURN(iRet);
}

int AT_CmdBleStoptHandler(char *buf, int len, int mode)
{
    int iRet = 0;

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_EXECUTION:
        {
            if(OPL_OK != APP_SendMessage(APP_EVT_BLE_STOP_REQ, NULL, 0))
            {
                goto done;
            }

            iRet = 1;

            break;
        }
        default:
            break;
    }

done:
    if(0 == iRet)
    { 
        AT_FAIL(ACK_TAG_BLE_STOP_REQ, NULL, 0);
    }

    AT_RETURN(iRet);
}
#if 0
int AT_CmdBleAdvDataHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};
    uint8_t au8StrtoHex[AT_BLE_ADV_DATA_LENGTH_MAX] = {0};
    uint8_t u8StrtoHexLen = 0;

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_SET:
        {
            if(!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                AT_LOG("at_cmd_buf_to_argc_argv\n");
                goto done;
            }

            if(argc < 2)
            {
                goto done;
            }

            // Check ble adv data length
            if(strlen(argv[1]) > AT_BLE_ADV_DATA_LENGTH_MAX)
            {
                goto done;
            }

            if((strlen(argv[1]) % 2) != 0)
            {
                goto done;
            }
            
            u8StrtoHexLen = strlen(argv[1]);

            AT_StrToHex((char *) au8StrtoHex, argv[1], u8StrtoHexLen);

            u8StrtoHexLen = u8StrtoHexLen / 2;

            
            if(OPL_OK != Opl_Ble_Advertise_Data_Set(au8StrtoHex, u8StrtoHexLen))
            {
                goto done;
            }

            iRet = 1;

            break;
        }

        default:
            break;
    }    
done:
    // trigger mcu wakeup
    //APP_TriggerHostWakeUp();

    if(0 == iRet)
    {
        AT_FAIL(ACK_TAG_BLE_ADV_DATA_REQ, NULL, 0);
    }
    else
    {
        AT_OK(ACK_TAG_BLE_ADV_DATA_REQ, NULL, 0);
    }

    AT_RETURN(iRet);
}
#endif

#if 0
int AT_CmdBleAppearanceHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_SET:
        {
            if(!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                AT_LOG("at_cmd_buf_to_argc_argv\n");
                goto done;
            }

            if(argc != 3)
            {
                goto done;
            }

            uint8_t u8AppearancePayload[2] = {0};
            u8AppearancePayload[0] = (uint8_t)strtoul(argv[1], NULL, 16);
            u8AppearancePayload[1] = (uint8_t)strtoul(argv[2], NULL, 16);

            if(OPL_OK != GAP_Svc_Appearance_Set(u8AppearancePayload, 2))
            {
                goto done;
            }

            iRet = 1;

            break;
        }

        default:
            break;
    }

done:
    // trigger mcu wakeup
    //APP_TriggerHostWakeUp();

    if(0 == iRet)
    {
        AT_FAIL(ACK_TAG_BLE_APPEAR_ERQ, NULL, 0);
    }
    else
    {
        AT_OK(ACK_TAG_BLE_APPEAR_ERQ, NULL, 0);
    }

    AT_RETURN(iRet);
}

int AT_CmdBleFwRevHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_SET:
        {
            if(!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                AT_LOG("at_cmd_buf_to_argc_argv\n");
                goto done;
            }

            if(argc < 3)
            {
                goto done;
            }

            uint8_t u8PayloadLen = (uint8_t)atoi(argv[1]);

            if(u8PayloadLen > FW_REV_VAL_MAX_LEN)
            {
                goto done;
            }

            if(OPL_OK != DevInfo_Svc_FirmwareRevision_Set((uint8_t *)argv[2], u8PayloadLen))
            {
                goto done;
            }

            iRet = 1;

            break;
        }

        default:
            break;
    }

done:
    // trigger mcu wakeup
    //APP_TriggerHostWakeUp();

    if(0 == iRet)
    {
        AT_FAIL(ACK_TAG_BLE_FW_REV_REQ, NULL, 0);
    }
    else
    {
        AT_OK(ACK_TAG_BLE_FW_REV_REQ, NULL, 0);
    }

    AT_RETURN(iRet);
}

int AT_CmdBleModelNbHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_SET:
        {
            if(!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                AT_LOG("at_cmd_buf_to_argc_argv\n");
                goto done;
            }

            if(argc < 3)
            {
                goto done;
            }

            uint8_t u8PayloadLen = (uint8_t)atoi(argv[1]);

            if(u8PayloadLen > MODEL_NB_VAL_MAX_LEN)
            {
                goto done;
            }

            if(OPL_OK != DevInfo_Svc_ModelNumber_Set((uint8_t *)argv[2], u8PayloadLen))
            {
                goto done;
            }

            iRet = 1;

            break;
        }

        default:
            break;
    }

done:
    // trigger mcu wakeup
    //APP_TriggerHostWakeUp();

    if(0 == iRet)
    {
        AT_FAIL(ACK_TAG_BLE_MODEL_NB_REQ, NULL, 0);
    }
    else
    {
        AT_OK(ACK_TAG_BLE_MODEL_NB_REQ, NULL, 0);
    }

    AT_RETURN(iRet);
}

SHM_DATA int AT_CmdBleDevMacHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    uint8_t u8aBleMac[6] = {0};

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_READ:
        {
            if(OPL_OK != Opl_Ble_MacAddr_Read(u8aBleMac))
            {
                goto done;
            }
            
            iRet = 1;

            break;
        }

        default:
            break;
    }

done:
    // trigger mcu wakeup
    //APP_TriggerHostWakeUp();

    if(0 == iRet)
    {
        AT_FAIL(ACK_TAG_BLE_GET_DEVICE_MAC_REQ, NULL, 0);
    }
    else
    {
        AT_OK(ACK_TAG_BLE_GET_DEVICE_MAC_REQ, u8aBleMac, sizeof(u8aBleMac));
    }

    AT_RETURN(iRet);
}

SHM_DATA int AT_CmdBleNotifyHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};
    uint32_t u32Datalen = 0;

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_SET:
        {
            if(!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                AT_LOG("at_cmd_buf_to_argc_argv\n");
                goto done;
            }

            if(argc < 2)
            {
                goto done;
            }

            u32Datalen = (uint32_t) atoi(argv[1]);

            if(0 == u32Datalen)
            {
                goto done;
            }

            T_BleNotifyStruct *tBleNotifyStruct = (T_BleNotifyStruct *) malloc(sizeof(T_BleNotifyStruct) + u32Datalen);
            

            if(tBleNotifyStruct == NULL)
            {
                goto done;
            }

            memset(tBleNotifyStruct, 0, sizeof(tBleNotifyStruct) + u32Datalen);
      
            tBleNotifyStruct->u32DataLen = u32Datalen;
            tBleNotifyStruct->u32RecvLen = 0;

            // register callback to process uart1 input
            agent_data_handle_reg(AT_BleNotifyRecvCB, tBleNotifyStruct);

#if defined(OPL1000_A2) || defined(OPL1000_A3)
            data_process_lock_patch(LOCK_APP, (tBleNotifyStruct->u32DataLen));
#elif defined(OPL2500_A0)
            data_process_lock(LOCK_APP, (tBleNotifyStruct->u32DataLen));
#endif

            iRet = 1;

            break;
        }

        default:
            break;
    }

done:
    if(0 == iRet)
    {
        // trigger mcu wakeup
        //APP_TriggerHostWakeUp();

        AT_FAIL(ACK_TAG_BLE_NOTIFY_DATA_REQ, NULL, 0);

    }

    AT_RETURN(iRet);
}
#endif

#if(OPL_DATA_ENABLED == 0)
// * range for WIFI commands
int AT_CmdWifiScanHandler(char *buf, int len, int mode)
{
    int iRet = 0;

    switch(mode)
    {
        case AT_CMD_MODE_EXECUTION:
        {            
            
            if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
            {
                AT_FAIL(ACK_TAG_WIFI_SCAN_REQ, NULL, 0);
                break;
            }

            if(OPL_OK != APP_SendMessage(APP_EVT_WIFI_SCAN_REQ, NULL, 0))
            {
                AT_FAIL(ACK_TAG_WIFI_SCAN_REQ, NULL, 0);
                break;
            }

            iRet = 1;

            break;
        }

        default:
            break;
    }

    AT_RETURN(iRet);
}

static int ConvertMac(char *mac_src, uint8_t *mac_dest)
{
    // mac str fmt : XX:XX:XX:XX:XX:XX
    if(17 != strlen(mac_src))
    {
        return false;
    }

    if(NULL == mac_dest)
    {
        return false;
    }

    // if valid formatting into 6 arg, convert into hex and assign into destination
    return (6 == sscanf( mac_src, "%02X:%02X:%02X:%02X:%02X:%02X"              \
                        ,(uint32_t *)&mac_dest[0]                                \
                        ,(uint32_t *)&mac_dest[1]                                \
                        ,(uint32_t *)&mac_dest[2]                                \
                        ,(uint32_t *)&mac_dest[3]                                \
                        ,(uint32_t *)&mac_dest[4]                                \
                        ,(uint32_t *)&mac_dest[5]));
}

SHM_DATA int AT_CmdWifiConnectHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_SET:
        {
            if(!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                AT_LOG("at_cmd_buf_to_argc_argv\n");
                goto done;
            }

            if(argc < 2)
            {
                goto done;
            }

            T_NmWifiCnctConfig tNmWifiCnctConfig = {0};

            if(true != ConvertMac(argv[1], tNmWifiCnctConfig.u8aBssid))
            {
                if(strlen(argv[1]) > WIFI_MAX_LENGTH_OF_SSID)
                {
                    goto done;
                }

                memset(&tNmWifiCnctConfig.u8aBssid, 0, sizeof(tNmWifiCnctConfig.u8aBssid));

                memcpy(&tNmWifiCnctConfig.u8aSsid, argv[1], strlen(argv[1]));
                tNmWifiCnctConfig.u8SsidLen = strlen(argv[1]);
            }

            if(argc >= 3)
            {
                if (strlen(argv[2]) >= WIFI_LENGTH_PASSPHRASE)
                {
                    goto done;
                }

                memcpy(&tNmWifiCnctConfig.u8aPwd, argv[2], strlen(argv[2]));
                tNmWifiCnctConfig.u8PwdLen = strlen(argv[2]);
            }

            if(OPL_OK != APP_SendMessage(APP_EVT_WIFI_CONNECT_REQ, (uint8_t *)&tNmWifiCnctConfig, sizeof(T_NmWifiCnctConfig)))
            {
                goto done;
            }

            iRet = 1;

            break;
        }

        // case AT_CMD_MODE_EXECUTION:
        // {
        //     if(OPL_OK != APP_SendMessage(APP_EVT_WIFI_CONNECT_REQ, NULL, 0))
        //     {
        //         goto done;
        //     }

        //     iRet = 1;
        
        //     break;
        // }

        default:
            break;
    }

done:
    if(0 == iRet)
    {
        AT_FAIL(ACK_TAG_WIFI_CONNECT_REQ, NULL, 0);
    }


    AT_RETURN(iRet);
}

SHM_DATA int AT_CmdWifiDisconnectHandler(char *buf, int len, int mode)
{
    int iRet = 0;

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_EXECUTION:
        {
            if(OPL_OK != APP_SendMessage(APP_EVT_WIFI_DISCONNECT_REQ, NULL, 0))
            {
                
                goto done;
            }

            iRet = 1;

            break;
        }
    }

done:

    if(0 == iRet)
    {    
        AT_FAIL(ACK_TAG_WIFI_DISCONNECT_REQ, NULL, 0);
    }

    AT_RETURN(iRet);
}
#endif

SHM_DATA int AT_CmdWifiApInfoHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    wifi_ap_record_t *pstApInfo = NULL;

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_READ:
        {
            pstApInfo = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t));

            if(NULL == pstApInfo)
            {
                goto done;
            }

            if(OPL_OK == Opl_Wifi_ApInfo_Get(pstApInfo))
            {  
                if(OPL_OK != Transfer_ScanList(ACK_TAG_WIFI_GET_AP_INFO_REQ,
                                               pstApInfo->bssid,
                                               pstApInfo->ssid,
                                               strlen((char *)pstApInfo->ssid),
                                               pstApInfo->channel,
                                               pstApInfo->rssi,
                                               pstApInfo->auth_mode))
                {
                    goto done;
                }
                
                iRet = 1;
                break;
            }
            else
            {
                goto done;
            }

        }
    }

done:
    if(0 == iRet)
    {   
        AT_FAIL(ACK_TAG_WIFI_GET_AP_INFO_REQ, NULL, 0);
    }

    free(pstApInfo);
    AT_RETURN(iRet);
}

SHM_DATA int AT_CmdWifiDevMacHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    uint8_t u8aMacAddr[6];

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_READ:
        {
            // get wifi mac address
            // get the mac address from flash
            if(OPL_OK != Opl_Wifi_MacAddr_Get((uint8_t *)&u8aMacAddr[0]))
            {
                goto done;
            }

            iRet = 1;

            break;
        }

        default:
            break;
    }

done:

    if(0 == iRet)
    {
        AT_FAIL(ACK_TAG_WIFI_GET_MODULE_MAC_ADDR_REQ, NULL, 0);
    }
    else
    {
        AT_OK(ACK_TAG_WIFI_GET_MODULE_MAC_ADDR_REQ, u8aMacAddr, sizeof(u8aMacAddr));
    }

    AT_RETURN(iRet);
}

SHM_DATA int AT_CmdWifiResetHandler(char *buf, int len, int mode)
{
    int iRet = 0;
	
    switch(mode)
    {
        case AT_CMD_MODE_EXECUTION:
        {
            Opl_Wifi_Profile_Clear();

            Opl_Wifi_Disc_Req(NULL);

            break;
        }
    }
	
    AT_RETURN(iRet);
}
// *

// * range for Cloud commands
#if(AWS_ENABLED == 1)
SHM_DATA int AT_CmdMqttInitHandler(char *buf, int len, int mode)
{
    int iRet = 0;

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_EXECUTION:
        {
            // Before initializing MQTT, first confirm whether RootCA, client cert and private key have been uploaded
            if(OPL_OK != Cloud_MqttFileCheck())
            {
                goto done;
            }

            if(OPL_OK != APP_SendMessage(APP_EVT_CLOUD_MQTT_INIT_REQ, NULL, 0))
            {
                goto done;
            }

            iRet = 1;

            break;
        }

        default:
            break;
    }

done:
    if(0 == iRet)
    {
        AT_FAIL(ACK_TAG_CLOUD_MQTT_INIT_REQ, NULL, 0);
    }

    AT_RETURN(iRet);
}

SHM_DATA int AT_CmdMqttConnectHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_SET:
        {
            if(!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                AT_LOG("at_cmd_buf_to_argc_argv\n");
                goto done;
            }

            if(argc < 3)
            {
                goto done;
            }

            T_CloudConnInfo tCloudConnInfo = {0};

            // default using MBEDTLS_SSL_VERIFY_NONE type
            tCloudConnInfo.u8Security = 0;

            // copy port and host address
            memcpy(tCloudConnInfo.u8aHostAddr, argv[1], strlen(argv[1]));
            tCloudConnInfo.u16HostPort = (uint16_t)strtoul(argv[2], NULL, 0);

            if(argc >= 4)
            {
                // set auto connect flag
                tCloudConnInfo.u8AutoConn = (uint8_t)strtoul(argv[3], NULL, 0);
            }
            
            if(OPL_OK != APP_SendMessage(APP_EVT_CLOUD_MQTT_ESTAB_REQ, (uint8_t *)&tCloudConnInfo, sizeof(T_CloudConnInfo)))
            {
                goto done;
            }

            iRet = 1;

            break;
        }

        default:
            break;
    }

done:
    if(0 == iRet)
    {
        AT_FAIL(ACK_TAG_CLOUD_MQTT_ESTAB_REQ, NULL, 0);
    }

    AT_RETURN(iRet);
}

SHM_DATA int AT_CmdMqttDisconnectHandler(char *buf, int len, int mode)
{
    int iRet = 0;

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_EXECUTION:
        {
            if(OPL_OK != APP_SendMessage(APP_EVT_CLOUD_MQTT_DISCONN_REQ, NULL, 0))
            {
                goto done;
            }

            iRet = 1;

            break;
        }

        default:
            break;
    }

done:
    if(0 == iRet)
    {
        AT_FAIL(ACK_TAG_CLOUD_MQTT_DISCON_REQ, NULL, 0);
    }

    AT_RETURN(iRet);
}

SHM_DATA int AT_CmdMqttKeepAliveIntvlHandler(char *buf, int len, int mode)
{
		int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};
    uint32_t u32KeepAliveIntv = 0;

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_SET:
        {
             if(!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                AT_LOG("at_cmd_buf_to_argc_argv\n");
                goto done;
            }

            if(argc < 1)
            {
                goto done;
            }

            u32KeepAliveIntv = (uint32_t) strtoul(argv[1], NULL, 0);

            if(OPL_OK != Cloud_KeepAliveIntervalSet(u32KeepAliveIntv))
            {
                goto done;
            }
            
            iRet = 1;

        }
    }

done:

    if(0 == iRet)
    {
        AT_FAIL(ACK_TAG_CLOUD_MQTT_KEEP_ALIVE_SET_REQ, (uint8_t *) &u32KeepAliveIntv, sizeof(u32KeepAliveIntv));
    }
    else
    {
        AT_OK(ACK_TAG_CLOUD_MQTT_KEEP_ALIVE_SET_REQ, (uint8_t *) &u32KeepAliveIntv, sizeof(u32KeepAliveIntv));
    }

    AT_RETURN(iRet);
}

SHM_DATA int AT_CmdMqttSubTopicHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};
    uint8_t u8QoS = 2;
    uint16_t u16TopicLen = 0;

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_SET:
        {
            if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                return false;
            }

            if(argc < 3)
            {
                goto done;
            }

            u16TopicLen = strlen(argv[1]);
            if(u16TopicLen == 0 || u16TopicLen > CLOUD_TOPIC_NAME_LEN)
            {
                goto done;
            }

            u8QoS = (uint8_t)atoi(argv[2]);
            if(u8QoS > 1)
            {
                goto done;
            }

            T_CloudTopicRegInfo tCloudTopicRegInfo = {0};
        
            memcpy(tCloudTopicRegInfo.u8TopicName, argv[1], strlen(argv[1]));
            tCloudTopicRegInfo.u16TopicNameLen = strlen(argv[1]);
            tCloudTopicRegInfo.u8QoS = (uint8_t)atoi(argv[2]);

            if(OPL_OK != APP_SendMessage(APP_EVT_CLOUD_MQTT_SUB_TOPIC_REQ, (uint8_t *)&tCloudTopicRegInfo, sizeof(T_CloudTopicRegInfo)))
            {
                goto done;
            }

            iRet = 1;

            break;
        }
    }

done:
    if(0 == iRet)
    {
        AT_FAIL(ACK_TAG_CLOUD_MQTT_SUB_TOPIC_REQ, NULL, 0);
    }

    AT_RETURN(iRet);
}

SHM_DATA int AT_CmdMqttUnsubTopicHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};
    uint8_t u8QoS = 2;
    uint16_t u16TopicLen = 0;

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_SET:
        {
            if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                return false;
            }

            if(argc < 3)
            {
                goto done;
            }

            u16TopicLen = strlen(argv[1]);
            if(u16TopicLen == 0 || u16TopicLen > CLOUD_TOPIC_NAME_LEN)
            {
                goto done;
            }

            u8QoS = (uint8_t)atoi(argv[2]);
            if(u8QoS > 1)
            {
                goto done;
            }

            T_CloudTopicRegInfo tCloudTopicRegInfo = {0};
        
            memcpy(tCloudTopicRegInfo.u8TopicName, argv[1], strlen(argv[1]));
            tCloudTopicRegInfo.u16TopicNameLen = strlen(argv[1]);
            tCloudTopicRegInfo.u8QoS = (uint8_t)atoi(argv[2]);

            if(OPL_OK != APP_SendMessage(APP_EVT_CLOUD_MQTT_UNSUB_TOPIC_REQ, (uint8_t *)&tCloudTopicRegInfo, sizeof(T_CloudTopicRegInfo)))
            {
                goto done;
            }

            iRet = 1;

            break;
        }
    }

done:
    if(0 == iRet)
    {
        AT_FAIL(ACK_TAG_CLOUD_MQTT_UNSUB_TOPIC_REQ, NULL, 0);
    }

    AT_RETURN(iRet);
}

SHM_DATA int AT_CmdMqttPubDataHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};
    uint8_t u8QoS = 2;
    uint16_t u16TopicLen = 0;
    uint16_t u16PayloadLen = 0;

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_SET:
        {
            if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                return false;
            }

            if(argc < 3)
            {
                goto done;
            }

            u16TopicLen = strlen(argv[1]);
            if(u16TopicLen == 0 || u16TopicLen > CLOUD_TOPIC_NAME_LEN)
            {
                goto done;
            }

            u8QoS = (uint8_t)atoi(argv[2]);
            if(u8QoS > 1)
            {
                goto done;
            }

            u16PayloadLen = (uint16_t) atoi(argv[3]);

            /* If user input data length is 0 or beyond the longest length then go to error.*/
            if(0 == u16PayloadLen || u16PayloadLen > CLOUD_PAYLOAD_LEN)
            {
                goto done;
            }

            T_MqttPayloadFmt *tMqttPublishInfo = (T_MqttPayloadFmt *) malloc(sizeof(T_MqttPayloadFmt));

            if(NULL == tMqttPublishInfo)
            {
                goto done;
            }

            memset(tMqttPublishInfo, 0, sizeof(T_MqttPayloadFmt));

            memcpy(tMqttPublishInfo->tCloudPayload.u8TopicName, argv[1], strlen(argv[1]));
            tMqttPublishInfo->tCloudPayload.u16TopicNameLen = strlen(argv[1]);
            tMqttPublishInfo->tCloudPayload.u8QoS = (uint8_t) atoi(argv[2]);
            tMqttPublishInfo->tCloudPayload.u16PayloadLen = u16PayloadLen;
            tMqttPublishInfo->u16RecvLen = 0;
            
            // printf("Recv_len %d, payload_len %d\r\n", tMqttPublishInfo->u16RecvLen, tMqttPublishInfo->tCloudPayload.u16PayloadLen);

            // register callback to process uart1 input
            agent_data_handle_reg(AT_MqttPublishCB, tMqttPublishInfo);

#if defined(OPL1000_A2) || defined(OPL1000_A3)
            data_process_lock_patch(LOCK_APP, ((uint32_t) tMqttPublishInfo->tCloudPayload.u16PayloadLen));
#elif defined(OPL2500_A0)
            data_process_lock(LOCK_APP, ((uint32_t) tMqttPublishInfo->tCloudPayload.u16PayloadLen));
#endif

            iRet = 1;

            break;
        }
    }

done:
    if(0 == iRet)
    {
        AT_FAIL(ACK_TAG_CLOUD_MQTT_PUB_DATA_REQ, NULL, 0);
    }

    AT_RETURN(iRet);
}

int AT_CmdMqttClientIdHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};
    uint8_t u8ClientId[AT_CLIENT_IDENTIFIER_LENGTH] = {0};
    uint16_t u16ClientIdLen = 0;

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

     switch(mode)
    {
        case AT_CMD_MODE_SET:
        {
            if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                return false;
            }

            u16ClientIdLen = strlen(argv[1]);

            if(0 == u16ClientIdLen || u16ClientIdLen > AT_CLIENT_IDENTIFIER_LENGTH)
            {
                goto done;
            }

            memcpy(u8ClientId, argv[1], u16ClientIdLen);

            Cloud_MqttClientId_Set(u8ClientId, u16ClientIdLen);

            iRet = 1;

            break;
        }

        default:
            break;
    }
done:

    if(0 == iRet)
    {
        AT_FAIL(ACK_TAG_CLOUD_MQTT_CLIENTID_SET, NULL, 0);
    }
    else
    {
        AT_OK(ACK_TAG_CLOUD_MQTT_CLIENTID_SET, NULL, 0);
    }

    AT_RETURN(iRet);
}

int AT_CmdMqttLastWillHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};
    uint16_t u16PayloadLen = 0;
    uint16_t u16TopicLen = 0;

  if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_SET:
        {
            if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                return false;
            }

            if(argc < 2)
            {
                goto done;
            }

            u16TopicLen = strlen(argv[1]);
            if(0 == u16TopicLen || u16TopicLen > CLOUD_TOPIC_NAME_LEN)
            {
                goto done;
            }

            u16PayloadLen = (uint16_t) atoi(argv[2]);

            /* If user input data length is 0 or beyond the longest length then go to error.*/
            if(0 == u16PayloadLen || u16PayloadLen > CLOUD_PAYLOAD_LEN)
            {
                goto done;
            }

            T_MqttPayloadFmt *tMqttPublishInfo = (T_MqttPayloadFmt *) malloc(sizeof(T_MqttPayloadFmt));

            if(NULL == tMqttPublishInfo)
            {
                goto done;
            }

            memset(tMqttPublishInfo, 0, sizeof(T_MqttPayloadFmt));

            memcpy(tMqttPublishInfo->tCloudPayload.u8TopicName, argv[1], strlen(argv[1]));
            tMqttPublishInfo->tCloudPayload.u16TopicNameLen = strlen(argv[1]);
            tMqttPublishInfo->tCloudPayload.u8QoS = 1; //QoS need MQTTQoS1 /**< Delivery at least once. */
            tMqttPublishInfo->tCloudPayload.u16PayloadLen = u16PayloadLen;
            tMqttPublishInfo->u16RecvLen = 0;
            
            // printf("Recv_len %d, payload_len %d\r\n", tMqttPublishInfo->u16RecvLen, tMqttPublishInfo->tCloudPayload.u16PayloadLen);

            // register callback to process uart1 input
            agent_data_handle_reg(AT_MqttLastWillCB, tMqttPublishInfo);

#if defined(OPL1000_A2) || defined(OPL1000_A3)
            data_process_lock_patch(LOCK_APP, ((uint32_t) tMqttPublishInfo->tCloudPayload.u16PayloadLen));
#elif defined(OPL2500_A0)
            data_process_lock(LOCK_APP, ((uint32_t) tMqttPublishInfo->tCloudPayload.u16PayloadLen));
#endif

            iRet = 1;

            break;
        }
    }

done:
    if(0 == iRet)
    {
        AT_FAIL(ACK_TAG_CLOUD_MQTT_PUB_DATA_REQ, NULL, 0);
    }

    AT_RETURN(iRet);
}
#endif
// *

#if(HTTP_ENABLED == 1)
//* range for File Commands
SHM_DATA int AT_CmdHttpPostHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};
    uint32_t u32Datalen = 0;

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_SET:
        {
            if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                return false;
            }

            if(argc < 2)
            {
                goto done;
            }

            u32Datalen = (uint32_t)strtoul(argv[1], NULL, 0);

            if(0 == u32Datalen)
            {
                goto done;
            }

            T_HttpPostFileStruct *tHttpPostFileStruct = (T_HttpPostFileStruct *) malloc(sizeof(T_HttpPostFileStruct) + u32Datalen);
            if(tHttpPostFileStruct == NULL)
            {
                goto done;
            }
            memset(tHttpPostFileStruct, 0, sizeof(T_HttpPostFileStruct) + u32Datalen);
            
            tHttpPostFileStruct->u32UrlLen = strlen(argv[2]);
            memcpy(tHttpPostFileStruct->aUrl, argv[2], tHttpPostFileStruct->u32UrlLen);
            tHttpPostFileStruct->u32DataLen = u32Datalen;
            tHttpPostFileStruct->u32RecvLen = 0;

            // register callback to process uart1 input
            agent_data_handle_reg(AT_HttpPostFileRecvCB, tHttpPostFileStruct);

#if defined(OPL1000_A2) || defined(OPL1000_A3)
            data_process_lock_patch(LOCK_APP, (tHttpPostFileStruct->u32DataLen));
#elif defined(OPL2500_A0)
            data_process_lock(LOCK_APP, (tHttpPostFileStruct->u32DataLen));
#endif

            iRet = 1;

            break;
        } 
    }
      
done:
    if(0 == iRet)
    {
        AT_FAIL(ACK_TAG_FILE_HTTP_POST_REQ, NULL, 0);
    }

    AT_RETURN(iRet);
}

SHM_DATA int AT_CmdHttpGetHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};
    uint32_t u32Datalen = 0;

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_SET:
        {
            if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                goto done;
            }

            if(argc < 2)
            {
                goto done;
            }

            u32Datalen = (uint32_t)strtoul(argv[1], NULL, 0);

            if(0 == u32Datalen)
            {
                goto done;
            }

            if(u32Datalen > FILE_DOWNLOAD_HTTP_URL_LEN)
            {
                goto done;
            }

            T_HttpGetFileStruct *tHttpGetFileStruct = (T_HttpGetFileStruct *) malloc(sizeof(T_HttpGetFileStruct) + u32Datalen);

            if(tHttpGetFileStruct == NULL)
            {
                goto done;
            }

            memset(tHttpGetFileStruct, 0, sizeof(T_HttpGetFileStruct) + u32Datalen);

            tHttpGetFileStruct->u32DataLen = u32Datalen;
            tHttpGetFileStruct->u32RecvLen = 0;

            // register callback to process uart1 input
            agent_data_handle_reg(AT_HttpGetFileRecvCB, tHttpGetFileStruct);

#if defined(OPL1000_A2) || defined(OPL1000_A3)
            data_process_lock_patch(LOCK_APP, (tHttpGetFileStruct->u32DataLen));
#elif defined(OPL2500_A0)
            data_process_lock(LOCK_APP, (tHttpGetFileStruct->u32DataLen));
#endif

            iRet = 1;

            break;
        }
    }

done:
    if(0 == iRet)
    {   
        AT_FAIL(ACK_TAG_FILE_HTTP_GET_REQ, NULL, 0);
    }

    AT_RETURN(iRet);
}
#endif

#if(AWS_ENABLED == 1)
SHM_DATA int AT_CmdFulMqttCertHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};
    uint32_t u32Datalen = 0;

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_SET:
        {
            if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                return false;
            }

            if(argc < 3)
            {
                goto done;
            }
            
            u32Datalen = (uint32_t)strtoul(argv[2], NULL, 0);

            if(0 == u32Datalen)
            {
                goto done;
            }

            T_UploadFileStruct *tUploadFileStruct = (T_UploadFileStruct *) malloc(sizeof(T_UploadFileStruct) + u32Datalen);

            if(tUploadFileStruct == NULL)
            {
                goto done;
            }

            memset(tUploadFileStruct, 0, sizeof(T_UploadFileStruct) + u32Datalen);

            tUploadFileStruct->tFileType = (T_UploadFileType)strtoul(argv[1], NULL, 0);
            tUploadFileStruct->u32DataLen = u32Datalen;
            tUploadFileStruct->u32RecvLen = 0;

            // register callback to process uart1 input
            agent_data_handle_reg(AT_UploadFileRecvCB, tUploadFileStruct);

#if defined(OPL1000_A2) || defined(OPL1000_A3)
            data_process_lock_patch(LOCK_APP, (tUploadFileStruct->u32DataLen));
#elif defined(OPL2500_A0)
            data_process_lock(LOCK_APP, (tUploadFileStruct->u32DataLen));
#endif

            iRet = 1;

            break;
        } 
    }
      
done:
    if(0 == iRet)
    {
        AT_FAIL(ACK_TAG_FILE_UPLOAD, NULL, 0);
    }

    AT_RETURN(iRet);
}
#endif

// * range for System or else commands
SHM_DATA int AT_CmdSleepHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
        case AT_CMD_MODE_SET:
        {
            if(!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                AT_LOG("at_cmd_buf_to_argc_argv\n");
                goto done;
            }

            T_AppSleepInfo SleepInfo = {0};
            SleepInfo.u8SleepMode = (uint8_t)atoi(argv[1]);

            //if(SleepInfo.u8SleepMode == 1 || SleepInfo.u8SleepMode == 2)
            if(SleepInfo.u8SleepMode == 2)
            {
                if(argc < 3)
                {
                    goto done;
                }
                SleepInfo.u32SleepTime = (uint32_t)strtoul(argv[2], NULL, 0);
            }
            else
            {
                if(argc < 2)
                {
                    goto done;
                }
                SleepInfo.u32SleepTime = 0;
            }

            APP_SendMessage(APP_EVT_PREPARE_SLEEP, (uint8_t *)&SleepInfo, sizeof(T_AppSleepInfo));

            break;
        }
    }

    iRet = 1;

done:

    if(0 == iRet)
    {
        AT_FAIL(ACK_TAG_SLEEP, NULL, 0);
    }
    
    AT_RETURN(iRet);
}
#if 0
SHM_DATA int AT_CmdModuleOtaHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }

    switch(mode)
    {
       case AT_CMD_MODE_SET:
       {
            if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                goto done;
            }

            HTTP_OtaTriggerReq(HTTP_OTA_EVT_TYPE_DOWNLOAD, (uint8_t *)(argv[1]), strlen(argv[1]));

            iRet = 1;

            break;
       }
    }

done:
    // trigger mcu wakeup
    //APP_TriggerHostWakeUp();

    if(0 == iRet)
    {
        AT_OK(ACK_TAG_MODULE_OTA, NULL, 0);
    }
    else
    {
        AT_FAIL(ACK_TAG_MODULE_OTA, NULL, 0);
    }

   AT_RETURN(iRet);
}
#endif
SHM_DATA int AT_CmdAppFwVer(char *buf, int len, int mode)
{
    int iRet = 0;
    uint8_t u8aAppFwVerStr[] = APP_FW_VER_STR;

    tracer_drct_printf("App fw version [%s]\r\n", u8aAppFwVerStr);

    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        AT_FAIL(ACK_TAG_FW_VER, NULL, 0);
    }
    else
    {
        AT_OK(ACK_TAG_FW_VER, u8aAppFwVerStr, sizeof(u8aAppFwVerStr));
        iRet = 1;
    }

    AT_RETURN(iRet);
}

/*************************************************************************
* FUNCTION:
*   AT_CmdHostReadyHandler
*
* DESCRIPTION:
*   If Host wakeup success.
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
int AT_CmdHostReadyHandler(char *buf, int len, int mode)
{
    int iRet = 0;

#if(OTA_ENABLED == 0)
    if(false == AT_CheckAtAccept()) //Confirm whether the current state can accept at command
    {
        goto done;
    }
#endif
    
    switch(mode)
    {
        case AT_CMD_MODE_EXECUTION:
        {
            APP_SendMessage(APP_EVT_HOST_READY_IND, NULL, 0);
            iRet = 1;

            break;
        }
        default:
            break;
    }

#if(OTA_ENABLED == 0)
done:

    if(0 == iRet) 
    {
        AT_FAIL(ACK_TAG_HOST_READY, NULL, 0);
    }
#endif
    AT_RETURN(iRet);
}

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
SHM_DATA void AT_CmdListAdd(uint8_t u8EnableCrLf)
{
    at_cmd_ext_tbl_reg(g_taAppAtCmd);

    at_cmd_crlf_term_set(u8EnableCrLf);

    set_echo_on(false);

#if (WM_ENABLED == 1 && WM_AT_ENABLED == 1)
    Opl_Wifi_Uslctd_CB_Reg(AT_UnsolicitedCallback);
#endif

}
