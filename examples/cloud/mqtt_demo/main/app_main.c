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
*  app_main.c
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

#include "app_main.h"
#include "app_at_cmd.h"
#include "ble_mngr.h"
#include "cmsis_os.h"
#include "cloud_ctrl.h"
#include "cloud_kernel.h"
#include "gap_svc.h"
#include "gatt_svc.h"
#include "log.h"
#include "net_mngr_api.h"
#include "opl_data_hdl.h"
#include "opl_data_prot.h"
#include "opl_svc.h"
#include "ota_mngr.h"
#include "pwr_save.h"
#include "rf_pwr.h"
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

osThreadId g_tAppTaskId;
osMessageQId g_tAppQueueId;
osTimerId g_tAppSysTimer;

static T_AppEvtHandlerTbl g_tAppEvtHandlerTbl[] = 
{
    {APP_EVT_BLE_INIT_IND,                  APP_EvtHandler_BleInit},
    {APP_EVT_BLE_START_ADV,                 APP_EvtHandler_BleStartAdv},
    {APP_EVT_BLE_STOP_IND,                  APP_EvtHandler_BleStopAdv},
    {APP_EVT_BLE_CONNECTED,                 APP_EvtHandler_BleConnected},
    {APP_EVT_BLE_DISCONNECTED,              APP_EvtHandler_BleDisconnected},
    {APP_EVT_BLE_DATA_IND,                  APP_EvtHandler_BleDataInd},

    {APP_EVT_NETWORK_INIT_IND,              APP_EvtHandler_NetworkInit},
    {APP_EVT_NETWORK_UP,                    APP_EvtHandler_NetworkUp},
    {APP_EVT_NETWORK_DOWN,                  APP_EvtHandler_NetworkDown},
    {APP_EVT_NETWORK_RESET,                 APP_EvtHandler_NetworkReset},

    {APP_EVT_CLOUD_CONNECT_DONE_IND,        APP_EvtHandler_CloudConnectDoneInd},
    {APP_EVT_CLOUD_CONNECT_FAIL_IND,        APP_EvtHandler_CloudConnectFailInd},
    {APP_EVT_CLOUD_RECONNECT_DONE_IND,      APP_EvtHandler_CloudReconnectDoneInd},
    {APP_EVT_CLOUD_DISCONNECT_IND,          APP_EvtHandler_CloudDisconnectInd},

    {APP_EVT_SYS_TIMER_TIMEOUT,             APP_EvtHandler_SysTimerTimeout},

    {0xFFFFFFFF,                            NULL},
};

// Sec 7: declaration of static function prototype

void APP_BleAdvDataInit(void);
void APP_BleScanRspDataInit(void);
void APP_SysInit(void);
void APP_DataInit(void);
void APP_TaskInit(void);
void APP_BleInit(void);
void APP_NetInit(void);
void APP_CldInit(void);
void APP_UserAtInit(void);

/***********
C Functions
***********/
// Sec 8: C Functions

// indicate callback for each type request

//////////////////////////////////// 
//// Callback group
//////////////////////////////////// 

// add your callback function here

void APP_NmUnsolicitedCallback(T_NmUslctdEvtType tEvtType, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // tEvtType refer to net_mngr_api.h
    switch(tEvtType)
    {
        case NM_USLCTD_EVT_NETWORK_INIT:
        {
            APP_SendMessage(APP_EVT_NETWORK_INIT_IND, NULL, 0);

            break;
        }
        case NM_USLCTD_EVT_NETWORK_UP:
        {
            APP_SendMessage(APP_EVT_NETWORK_UP, pu8Data, u32DataLen);

            // set network up (for cloud to checking wifi connection)
            Cloud_NetworkStatusSet(true);

            break;
        }
        case NM_USLCTD_EVT_NETWORK_DOWN:
        {
            APP_SendMessage(APP_EVT_NETWORK_DOWN, NULL, 0);

            // set network down (for cloud to checking wifi connection)
            Cloud_NetworkStatusSet(false);

            break;
        }
        case NM_USLCTD_EVT_NETWORK_RESET:
        {
            APP_SendMessage(APP_EVT_NETWORK_RESET, NULL, 0);

            // set network down (for cloud to checking wifi connection)
            Cloud_NetworkStatusSet(false);

            break;
        }
        default:
        {
            // should not be here

            break;
        }
    }
}

void APP_BleUnsolicitedCallback(uint16_t u16EvtType, T_OplErr tEvtRst, uint8_t *pu8Data, uint32_t u32DataLen)
{
    switch(u16EvtType)
    {
        case USLCTED_CB_EVT_BLE_INIT:
        {       
            // initialize ble advertise data
            APP_BleAdvDataInit();

            // initialize ble scan response data
            APP_BleScanRspDataInit();

            APP_SendMessage(APP_EVT_BLE_INIT_IND, NULL, 0);

            break;
        }
        case USLCTED_CB_EVT_BLE_ENT_ADVERTISE:
        {
            APP_SendMessage(APP_EVT_BLE_START_ADV, &tEvtRst, sizeof(T_OplErr));

            break;
        }
        case USLCTED_CB_EVT_BLE_STOP:
        {
            APP_SendMessage(APP_EVT_BLE_STOP_IND, &tEvtRst, sizeof(T_OplErr));

            break;
        }
        case USLCTED_CB_EVT_BLE_CONNECTED:
        {
            APP_SendMessage(APP_EVT_BLE_CONNECTED, pu8Data, u32DataLen);

            break;
        }
        case USLCTED_CB_EVT_BLE_DISCONNECT:
        {
            APP_SendMessage(APP_EVT_BLE_DISCONNECTED, NULL, 0);

            break;
        }
        default:
        {
            // should not be here
         
            break;
        }
    }
}

void APP_CloudStatusCallback(T_CloudStatus tCloudStatus, void *pData, uint32_t u32DataLen)
{
    switch(tCloudStatus)
    {
        case CLOUD_CB_STA_INIT_DONE:
        case CLOUD_CB_STA_INIT_FAIL:
            break;
        case CLOUD_CB_STA_CONN_DONE:
        {
            APP_SendMessage(APP_EVT_CLOUD_CONNECT_DONE_IND, NULL, 0);
            break;
        }
        case CLOUD_CB_STA_CONN_FAIL:
        {
            APP_SendMessage(APP_EVT_CLOUD_CONNECT_FAIL_IND, NULL, 0);        
            break;
        }
        case CLOUD_CB_STA_RECONN_DONE:
        {
            APP_SendMessage(APP_EVT_CLOUD_RECONNECT_DONE_IND, NULL, 0);
            break;
        }
        case CLOUD_CB_STA_DISCONN:
        {
            APP_SendMessage(APP_EVT_CLOUD_DISCONNECT_IND, NULL, 0);
            break;
        }
        case CLOUD_CB_STA_SUB_DONE:
        case CLOUD_CB_STA_SUB_FAIL:
        case CLOUD_CB_STA_UNSUB_DONE:
        case CLOUD_CB_STA_UNSUB_FAIL:
        case CLOUD_CB_STA_PUB_DONE:
        case CLOUD_CB_STA_PUB_FAIL:
        default:
            break;
    }
}

static void APP_SysTimerTimeoutHandler(void const *argu)
{
    APP_SendMessage(APP_EVT_SYS_TIMER_TIMEOUT, NULL, 0);
}

void APP_MqttSubTest1IndicateCallback(AWS_IoT_Client *pClient, char *pTopicName, uint16_t topicNameLen,
									  IoT_Publish_Message_Params *pParams, void *pClientData)
{   
    OPL_LOG_INFO(APP, "sub1: %.*s\t%.*s", topicNameLen, pTopicName, (int)pParams->payloadLen, (char *)pParams->payload);
}

void APP_MqttSubTest2IndicateCallback(AWS_IoT_Client *pClient, char *pTopicName, uint16_t topicNameLen,
									  IoT_Publish_Message_Params *pParams, void *pClientData)
{
    OPL_LOG_INFO(APP, "sub2: %.*s\t%.*s", topicNameLen, pTopicName, (int)pParams->payloadLen, (char *)pParams->payload);
}

//////////////////////////////////// 
//// Event handler group
////////////////////////////////////

// add your event handler function here

static void APP_EvtHandler_BleInit(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "BLE ready");
}

static void APP_EvtHandler_BleStartAdv(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    T_OplErr eOplErr = *((T_OplErr *)pData);

    if(OPL_OK == eOplErr)
    {
        uint8_t u8aBleMac[6] = {0};
        Opl_Ble_MacAddr_Read(u8aBleMac);

        OPL_LOG_INFO(APP, "BLE advertising...Device mac [%0X:%0X:%0X:%0X:%0X:%0X]", u8aBleMac[0],
                                                                                    u8aBleMac[1],
                                                                                    u8aBleMac[2],
                                                                                    u8aBleMac[3],
                                                                                    u8aBleMac[4],
                                                                                    u8aBleMac[5]);
    }
    else
    {
        OPL_LOG_WARN(APP, "Start BLE advertising fail!");
    }
}

static void APP_EvtHandler_BleStopAdv(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    T_OplErr eOplErr = *((T_OplErr *)pData);

    if(OPL_OK == eOplErr)
    {
        OPL_LOG_INFO(APP, "BLE stop success");
    }
    else
    {
        OPL_LOG_WARN(APP, "BLE stop fail");
    }
}

static void APP_EvtHandler_BleConnected(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    uint8_t u8aBlePeerMac[6] = {0};
    memcpy(u8aBlePeerMac, pData, sizeof(u8aBlePeerMac));

    OPL_LOG_INFO(APP, "BLE connected...Peer mac [%0X:%0X:%0X:%0X:%0X:%0X]", u8aBlePeerMac[0],
                                                                            u8aBlePeerMac[1],
                                                                            u8aBlePeerMac[2],
                                                                            u8aBlePeerMac[3],
                                                                            u8aBlePeerMac[4],
                                                                            u8aBlePeerMac[5]);
}

static void APP_EvtHandler_BleDisconnected(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "BLE disconnected");
}

static void APP_EvtHandler_BleDataInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_DataRecvHandler(pData, (uint16_t)u32DataLen);
}

static void APP_EvtHandler_NetworkInit(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "WiFi ready");
}

static void APP_EvtHandler_NetworkUp(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "Network connected");

    // find ip number in network interface
    uint32_t u32Ip = *((uint32_t*)pData);
    uint8_t u8aIp[4] = {0};

    u8aIp[0] = (u32Ip >> 0) & 0xFF;
    u8aIp[1] = (u32Ip >> 8) & 0xFF;
    u8aIp[2] = (u32Ip >> 16) & 0xFF;
    u8aIp[3] = (u32Ip >> 24) & 0xFF;

    OPL_LOG_INFO(APP, "WI-FI IP [%d.%d.%d.%d]", u8aIp[0], u8aIp[1], u8aIp[2], u8aIp[3]);

    // register topics
    T_CloudTopicRegInfo tCloudTopicRegInfo;

    // register subscribe topic 1
    memset(&tCloudTopicRegInfo, 0, sizeof(T_CloudTopicRegInfo));
    tCloudTopicRegInfo.u8TopicIndex = 1;
    tCloudTopicRegInfo.fpFunc = APP_MqttSubTest1IndicateCallback;
    strcpy((char *)tCloudTopicRegInfo.u8aTopicName, "QD_FWK/MQTT_DEMO/SUB_Test/1");

    Cloud_RxTopicRegisterSta(&tCloudTopicRegInfo);

    // register publish topic
    memset(&tCloudTopicRegInfo, 0, sizeof(T_CloudTopicRegInfo));
    tCloudTopicRegInfo.u8TopicIndex = 1;
    tCloudTopicRegInfo.fpFunc = NULL;
    strcpy((char *)tCloudTopicRegInfo.u8aTopicName, "QD_FWK/MQTT_DEMO/PUB_Test/1");

    Cloud_TxTopicRegisterSta(&tCloudTopicRegInfo);

    Cloud_MsgSend(CLOUD_EVT_TYPE_ESTABLISH, NULL, 0);
}

static void APP_EvtHandler_NetworkDown(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "Network disconnected");

    // Cloud_MsgSend(CLOUD_EVT_TYPE_DISCONNECT, NULL, 0);
}

static void APP_EvtHandler_NetworkReset(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "Network reset");
}

static void APP_EvtHandler_CloudConnectDoneInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    osDelay(1000);

    // register topics
    T_CloudTopicRegInfo tCloudTopicRegInfo;

    // register subscribe topic 1
    memset(&tCloudTopicRegInfo, 0, sizeof(T_CloudTopicRegInfo));
    tCloudTopicRegInfo.u8TopicIndex = 2;
    tCloudTopicRegInfo.fpFunc = APP_MqttSubTest2IndicateCallback;
    strcpy((char *)tCloudTopicRegInfo.u8aTopicName, "QD_FWK/MQTT_DEMO/SUB_Test/2");

    Cloud_RxTopicRegisterDyn(&tCloudTopicRegInfo);

    // start periodic timer to publish data
#if (MQTT_DEMO_PERI_POST_EN == 1)
    osTimerStart(g_tAppSysTimer, MQTT_DEMO_PERI_POST_INTERVAL);
#endif

    // stop ble advertise
    Opl_Ble_Stop_Req();
}

static void APP_EvtHandler_CloudConnectFailInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    // retry connection
    Cloud_MsgSend(CLOUD_EVT_TYPE_ESTABLISH, NULL, 0);
}

static void APP_EvtHandler_CloudReconnectDoneInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    // start periodic timer to publish data
#if (MQTT_DEMO_PERI_POST_EN == 1)
    osTimerStart(g_tAppSysTimer, MQTT_DEMO_PERI_POST_INTERVAL);
#endif
}

static void APP_EvtHandler_CloudDisconnectInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    // stop periodic timer
#if (MQTT_DEMO_PERI_POST_EN == 1)
    osTimerStop(g_tAppSysTimer);
#endif
}

static void APP_EvtHandler_SysTimerTimeout(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "Systimer timeout");

    // publish mqtt data
    T_CloudPayloadFmt tCloudPayloadFmt;

    // ready the payload to send
    char u8Payload[9] = "HelloMQTT";

    tCloudPayloadFmt.u8TopicIndex = 1;
    tCloudPayloadFmt.u32PayloadLen = strlen(u8Payload);
    memcpy(tCloudPayloadFmt.u8aPayloadBuf, u8Payload, tCloudPayloadFmt.u32PayloadLen);

    Cloud_MsgSend(CLOUD_EVT_TYPE_POST, (uint8_t *)&tCloudPayloadFmt, sizeof(T_CloudPayloadFmt));
}

//////////////////////////////////// 
//// APP function group
////////////////////////////////////

// add your application function here

void APP_BleAdvDataInit(void)
{
    // ble advertise data inititate

    // user modify
    // *

    uint8_t u8Len;
    uint8_t au8BleAdvertData[] =
    {
        0x02,
        GAP_ADTYPE_FLAGS,
        GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
        // connection interval range
        0x05,
        GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE, 
        UINT16_LO(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
        UINT16_HI(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
        UINT16_LO(DEFAULT_DESIRED_MAX_CONN_INTERVAL),
        UINT16_HI(DEFAULT_DESIRED_MAX_CONN_INTERVAL),
        0x02,
        GAP_ADTYPE_POWER_LEVEL,
        0,
        0x11,
        GAP_ADTYPE_128BIT_COMPLETE,
        0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00
    };

    // error handle
    u8Len = sizeof(au8BleAdvertData);

    if (u8Len > BLE_ADV_SCAN_BUF_SIZE)
    {
        u8Len = BLE_ADV_SCAN_BUF_SIZE;
    }

    Opl_Ble_Advertise_Data_Set(au8BleAdvertData, u8Len);

    // *
}

void APP_BleScanRspDataInit(void)
{
    // ble scan response data inititate

    // user modify
    // *

    char u8aBleName[31];
    uint8_t u8BleNameLen = 0;
    uint8_t au8BleScanRspData[BLE_ADV_SCAN_BUF_SIZE];
    uint8_t u8aBleMac[6] = {0};

    Opl_Ble_MacAddr_Read(u8aBleMac);

    sprintf(u8aBleName, "%s_%0X:%0X:%0X:%0X", BLE_GAP_PF_DEVICE_NAME, 
                                              u8aBleMac[2],
                                              u8aBleMac[3],
                                              u8aBleMac[4],
                                              u8aBleMac[5]);

    u8BleNameLen = strlen(u8aBleName);

    au8BleScanRspData[0] = (u8BleNameLen + 1);
    au8BleScanRspData[1] = GAP_ADTYPE_LOCAL_NAME_COMPLETE;

    MemCopy((au8BleScanRspData + 2), u8aBleName, u8BleNameLen);

    if(OPL_OK != Opl_Ble_ScanRsp_Data_Set(au8BleScanRspData, (u8BleNameLen + 2)))
    {
        OPL_LOG_ERRO(APP, "Scan Rsp Data Set Fail");
    }

    // *
}

void APP_SysInit(void)
{
#if (EXT_PA_ENABLED == 1)
    // Do not overwrite RF power setting if external PA enable
#else
    // initialize rf power setting
    RF_PwrSet(RF_CFG_DEF_PWR_SET);
#endif

    // initialize ota manager
    OTA_Init();

    // user implement
}

void APP_DataInit(void)
{
    // user implement
}

void APP_TaskInit(void)
{
    // create timer
    osTimerDef_t tTimerDef;

    tTimerDef.ptimer = APP_SysTimerTimeoutHandler;
    g_tAppSysTimer = osTimerCreate(&tTimerDef, osTimerPeriodic, NULL);
    if(g_tAppSysTimer == NULL)
    {
        OPL_LOG_ERRO(APP, "Create sys timer fail");
    }

    // create message queue
    osMessageQDef_t tQueueDef;

    tQueueDef.item_sz = sizeof(T_AppMsgStruct);
    tQueueDef.queue_sz = APP_QUEUE_SIZE;
    g_tAppQueueId = osMessageCreate(&tQueueDef, NULL);

    if(g_tAppQueueId == NULL)
    {
        OPL_LOG_ERRO(APP, "Create queue fail");
    }

    // create task
    osThreadDef_t tTaskDef;
    
    tTaskDef.name = "App Main";
    tTaskDef.stacksize = APP_TASK_STACK_SIZE;
    tTaskDef.tpriority = APP_TASK_PRIORITY;
    tTaskDef.pthread = APP_Main;
    g_tAppTaskId = osThreadCreate(&tTaskDef, NULL);

    if(g_tAppTaskId == NULL)
    {
        OPL_LOG_ERRO(APP, "Create task fail");
    }

    OPL_LOG_INFO(APP, "Create task ok");

    // user implement
}

void APP_BleInit(void)
{
    // assign unsolicited callback function
    Opl_Ble_Uslctd_CB_Reg(&APP_BleUnsolicitedCallback);

    // register service
    GAP_Svc_Init();
    GATT_Svc_Init();

    // register opl service
    OPL_Svc_Init();

    // initialize the ble manager (auto-adv)
    Opl_Ble_Init_Req(true);

    // user implement
}

void APP_NetInit(void)
{
    // Network manager initialize (auto-connect enable)
    APP_NmInit(true, &APP_NmUnsolicitedCallback);

    // user implement
}

void APP_CldInit(void)
{
    // initialize cloud status callback
    Cloud_StatusCallbackRegister(APP_CloudStatusCallback);

    // initialize cloud connect information
    T_CloudConnInfo tCloudConnInfo;

    tCloudConnInfo.u8AutoConn = 1;
    tCloudConnInfo.u8Security = 0;
    tCloudConnInfo.u16HostPort = MQTT_HOST_PORT;
    strcpy((char *)tCloudConnInfo.u8aHostAddr, MQTT_HOST_URL);

    Cloud_Init(&tCloudConnInfo);

    // user implement
}

void APP_UserAtInit(void)
{
    // add at cmd and enable CR/LF
    AT_CmdListAdd(1);  // #define CRLF_ENABLE (1)
}

//////////////////////////////////// 
//// APP task group
//////////////////////////////////// 
/*************************************************************************
* FUNCTION:
*   APP_SendMessage
*
* DESCRIPTION:
*   Send message queue to APP task
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
T_OplErr APP_SendMessage(uint32_t u32EventId, uint8_t *pu8Data, uint32_t u32DataLen)
{
    if(NULL == g_tAppQueueId)
    {
        OPL_LOG_WARN(APP, "Queue not init");

        return OPL_ERR_RTOS_QMSG_NOT_INIT;
    }

    T_AppMsgStruct *ptMsg = (T_AppMsgStruct *)malloc(sizeof(T_AppMsgStruct) + u32DataLen);

    if(NULL == ptMsg)
    {
        OPL_LOG_ERRO(APP, "Alloc WM message fail");
        return OPL_ERR_ALLOC_MEMORY_FAIL;
    }

    // prepare the data
    ptMsg->u32EventId = u32EventId;
    ptMsg->u32DataLen = u32DataLen;

    if(0 != ptMsg->u32DataLen)
    {
        memcpy(ptMsg->pau8Data, pu8Data, u32DataLen);
    }

    // send message
    if(osOK != osMessagePut(g_tAppQueueId, (uint32_t)ptMsg, 0))
    {
        OPL_LOG_ERRO(APP, "Send message fail");
        free(ptMsg);

        return OPL_ERR_RTOS_SEND_QMSG_FAIL;
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION:
*   APP_EventProcess
*
* DESCRIPTION:
*   Message processor
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
T_OplErr APP_EventProcess(T_AppMsgStruct *ptMsg)
{
    // event in user app define list
    if((APP_EVT_BEGIN <= ptMsg->u32EventId) && (ptMsg->u32EventId <= APP_EVT_TOTAL))
    {   
        uint32_t i = 0;
        while(g_tAppEvtHandlerTbl[i].u32EventId != 0xFFFFFFFF)
        {
            // matched
            if(g_tAppEvtHandlerTbl[i].u32EventId == ptMsg->u32EventId)
            {
                g_tAppEvtHandlerTbl[i].fpFunc(ptMsg->u32EventId, ptMsg->pau8Data, ptMsg->u32DataLen);
                break;
            }

            i ++;
        }

        // not match
        if(g_tAppEvtHandlerTbl[i].u32EventId == 0xFFFFFFFF)
        {
            OPL_LOG_WARN(APP, "can't find event in event table");

            return OPL_ERR_RTOS_EVT_NOT_FOUND;
        }

        return OPL_OK;
    }
    else
    {
        return OPL_ERR_RTOS_EVT_NOT_FOUND;
    }
}

/*************************************************************************
* FUNCTION:
*   AppInit
*
* DESCRIPTION:
*   the initial of application
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void APP_Main(void *args)
{
    osEvent tEvent;
    T_AppMsgStruct *ptMsg;

    for(;;)
    {
        // wait event
        tEvent = osMessageGet(g_tAppQueueId, osWaitForever);

        if (tEvent.status == osEventMessage)
        {
            ptMsg = (T_AppMsgStruct *)tEvent.value.p;

            if(OPL_ERR_RTOS_EVT_NOT_FOUND == APP_EventProcess(ptMsg))
            {
#if (NM_ENABLED == 1)
                // while event not found in user define, try network manager
                APP_NmEventProc(ptMsg->u32EventId, ptMsg->pau8Data, ptMsg->u32DataLen);
#endif
            }

            if(ptMsg != NULL)
            {
                free(ptMsg);
            }
        }
    }
}

/*************************************************************************
* FUNCTION:
*   AppInit
*
* DESCRIPTION:
*   the initial of application
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void APP_MainInit(void)
{
    // create main task
    OPL_LOG_INFO(APP, "App Main Init");

    APP_SysInit();

    APP_DataInit();

    APP_TaskInit();

    APP_BleInit();

    APP_NetInit();

    APP_CldInit();

    APP_UserAtInit();
 
#if 0   // Workaround for after several auto-connect rounds will cause SDK hang
    printf("Warning: ice mode!!!\r\n");
    reg_write(0x40001100, 0x12);    // software ice mode workaround for mantis 3226 
#endif

    // enter smart sleep after 5s
#if (PS_ENABLED == 1)
    PS_EnterSmartSleep(5000);
#endif

}
