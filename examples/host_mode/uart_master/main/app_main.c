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
#include "hal_vic.h"
#include "opl_data_prot.h"
#include "opl_data_hdl.h"
#include "opl_svc.h"
#include "ota_mngr.h"
#include "gap_svc.h"
#include "gatt_svc.h"
#include "log.h"
#include "net_mngr_api.h"
#include "pwr_save.h"
#include "rf_pwr.h"
#include "uart.h"
#include "wifi_mngr_api.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#define HOST_MODE_WAKEUP_PIN                            (GPIO_IDX_04)

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
osTimerId g_tAppSleepSlaveTimer;
osTimerId g_tAppWakeupSlaveTimer;

static T_AppEvtHandlerTbl g_tAppEvtHandlerTbl[] = 
{
    {APP_EVT_BLE_START_ADV,                 APP_EvtHandler_BleStartAdv},
    {APP_EVT_BLE_STOP_ADV,                  APP_EvtHandler_BleStopAdv},
    {APP_EVT_BLE_CONNECTED,                 APP_EvtHandler_BleConnected},
    {APP_EVT_BLE_DISCONNECTED,              APP_EvtHandler_BleDisconnected},
    {APP_EVT_BLE_DATA_IND,                  APP_EvtHandler_BleDataInd},

    {APP_EVT_NETWORK_UP,                    APP_EvtHandler_NetworkUp},
    {APP_EVT_NETWORK_DOWN,                  APP_EvtHandler_NetworkDown},
    {APP_EVT_NETWORK_RESET,                 APP_EvtHandler_NetworkReset},

    {APP_EVT_UART_MSG_SEND,                 APP_EvtHandler_UartMsgSend},
    {APP_EVT_UART_MSG_RECV,                 APP_EvtHandler_UartMsgRecv},
    {APP_EVT_WAKEUP_SLAVE,                  APP_EvtHandler_WakeupSlave},

    {0xFFFFFFFF,                            NULL},
};

// Sec 7: declaration of static function prototype

T_OplErr APP_UartSendingDataProcess(T_UartCmdType tUartCmdType, uint8_t *pu8Data, uint8_t u8DataLen);
T_OplErr APP_UartReceivedDataProcess(uint8_t *pu8UartData);

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
        case NM_USLCTD_EVT_NETWORK_UP:
        {
            APP_SendMessage(APP_EVT_NETWORK_UP, pu8Data, u32DataLen);

            break;
        }
        case NM_USLCTD_EVT_NETWORK_DOWN:
        {
            APP_SendMessage(APP_EVT_NETWORK_DOWN, NULL, 0);

            break;
        }
        case NM_USLCTD_EVT_NETWORK_RESET:
        {
            APP_SendMessage(APP_EVT_NETWORK_RESET, NULL, 0);

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

            break;
        }
        case USLCTED_CB_EVT_BLE_ENT_ADVERTISE:
        {
            APP_SendMessage(APP_EVT_BLE_START_ADV, NULL, 0);

            break;
        }
        case USLCTED_CB_EVT_BLE_EXI_ADVERTISE:
        {
            APP_SendMessage(APP_EVT_BLE_STOP_ADV, NULL, 0);

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

static void APP_SleepSlaveTimerTimeoutHandler(void const *argu)
{
    T_UartData tUartData;

    memset(&tUartData, 0, sizeof(T_UartData));

#if (APP_HOST_MODE_PWR_SAVE_DEMO == 0)
    tUartData.tUartCmdType = UART_CMD_SMART_SLEEP_ENT_REQ;
#else
    tUartData.tUartCmdType = UART_CMD_DEEP_SLEEP_ENT_REQ;
#endif

    tUartData.u8DataLen = 0;

    APP_SendMessage(APP_EVT_UART_MSG_SEND, (uint8_t *)&tUartData, sizeof(T_UartData));
}

static void APP_WakeupSlaveTimerTimeoutHandler(void const *argu)
{
    APP_SendMessage(APP_EVT_WAKEUP_SLAVE, NULL, 0);
}

//////////////////////////////////// 
//// Event handler group
////////////////////////////////////

// add your event handler function here

static void APP_EvtHandler_BleStartAdv(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
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

static void APP_EvtHandler_BleStopAdv(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "BLE stop advertise");
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
}

static void APP_EvtHandler_NetworkDown(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "Network disconnected");
}

static void APP_EvtHandler_NetworkReset(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "Network reset");
}

static void APP_EvtHandler_UartMsgSend(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    T_UartData tUartData = *((T_UartData *)pData);

    if((UART_DATA_RECV_BUF_LEN - 3) < tUartData.u8DataLen)
    {
        // lens invalid
        return;
    }

    // process sending data
    APP_UartSendingDataProcess(tUartData.tUartCmdType, tUartData.u8aData, tUartData.u8DataLen);
}

static void APP_EvtHandler_UartMsgRecv(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    uint8_t u8aUartData[UART_DATA_RECV_BUF_LEN];

    memcpy(u8aUartData, pData, u32DataLen);

    // process received data
    APP_UartReceivedDataProcess(u8aUartData);
}

static void APP_EvtHandler_WakeupSlave(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "Trigger gpio to wakeup slave");
    // trigger GPIO 4 to wakeup slave
    Hal_Vic_GpioOutput(HOST_MODE_WAKEUP_PIN, GPIO_LEVEL_HIGH);
    osDelay(100);
    Hal_Vic_GpioOutput(HOST_MODE_WAKEUP_PIN, GPIO_LEVEL_LOW);
}

T_OplErr APP_UartSendingDataProcess(T_UartCmdType tUartCmdType, uint8_t *pu8Data, uint8_t u8DataLen)
{
    // uart transmit command format
    // --------------------------------------
    // [hdr][cmd][len][data]
    // --------------------------------------
    // hdr : UART_DATA_HEADER (see in uart.h)
    // cmd : T_AppUartCmdType (see in uart.h)
    // len : total lens of payload
    // data : payload

    if(UART_CMD_MAX <= tUartCmdType)
    {
        return OPL_ERR_PARAM_INVALID;
    }

    uint8_t u8UartDataLen = 0;
    uint8_t u8aUartData[UART_DATA_RECV_BUF_LEN];
    memset(u8aUartData, 0, sizeof(u8aUartData));

    u8aUartData[0] = UART_DATA_HEADER;
    u8aUartData[1] = (uint8_t)tUartCmdType;
    u8aUartData[2] = u8DataLen;

    if(0 != u8DataLen)
    {
        memcpy(&u8aUartData[3], pu8Data, u8DataLen);
    }

    u8UartDataLen = 3 + u8DataLen;

    // send uart message
    UART_MessageSend(u8aUartData, u8UartDataLen);

    //     case UART_CMD_WIFI_CONNECT_REQ:
    //     {
    //         uint8_t u8SsidLen = strlen(APP_DEF_AP_SSID);
    //         uint8_t u8PwdLen = strlen(APP_DEF_AP_PWD);

    //         // [data] => [ssid len][ssid 1~32b][pwd len][pwd 8~63b]
    //         pu8aUartData[1] = tAppUartCmdType;
    //         pu8aUartData[2] = 2 + u8SsidLen + u8PwdLen;
    //         pu8aUartData[3] = u8SsidLen;
    //         strcpy((char *)&pu8aUartData[4], APP_DEF_AP_SSID);
    //         pu8aUartData[4 + u8SsidLen] = u8PwdLen;
    //         strcpy((char *)&pu8aUartData[4 + u8SsidLen + 1], APP_DEF_AP_PWD);
    //         break;
    //     }

    //     default:
    //         return OPL_ERR_PARAM_INVALID;
    // }

    // // 3 = header + command + data len
    // *pu8UartDataLen = 3 + pu8aUartData[2];

    return OPL_OK;
}

T_OplErr APP_UartReceivedDataProcess(uint8_t *pu8UartData)
{
    // uart recevie command format
    // --------------------------------------
    // [hdr][cmd][len][data]
    // --------------------------------------

    if(UART_DATA_HEADER != pu8UartData[0])
    {
        OPL_LOG_ERRO(APP, "Uart header incorrect %x", pu8UartData[0]);
    }

    T_UartData tUartData;
    memset(&tUartData, 0, sizeof(T_UartData));

    switch(pu8UartData[1])
    {
        case UART_CMD_SLAVE_WAKEUP_SIGN:
        {
            OPL_LOG_INFO(APP, "Slave in idle");

            // slave wakeup, send ping	
            tUartData.tUartCmdType = UART_CMD_PING;
            tUartData.u8DataLen = 0;

            APP_SendMessage(APP_EVT_UART_MSG_SEND, (uint8_t *)&tUartData, sizeof(T_UartData));

            break;
        }
        case UART_CMD_SMART_SLEEP_ENT_RSP:
        {
            OPL_LOG_INFO(APP, "Slave in smart sleep");

            // slave in sleep, wakeup after 5s
            osTimerStop(g_tAppWakeupSlaveTimer);
            osTimerStart(g_tAppWakeupSlaveTimer, 5000);

            break;
        }
        case UART_CMD_DEEP_SLEEP_ENT_RSP:
        {
            OPL_LOG_INFO(APP, "Slave in deep sleep");

            // slave in sleep, wakeup after 5s
            osTimerStop(g_tAppWakeupSlaveTimer);
            osTimerStart(g_tAppWakeupSlaveTimer, 5000);

            break;
        }
        case UART_CMD_BLE_ADV_START_RSP:
        {
            OPL_LOG_INFO(APP, "Slave BLE advertising");
            break;
        }
        case UART_CMD_BLE_ADV_STOP_RSP:
        {
            OPL_LOG_INFO(APP, "Slave BLE non-advertise");
            break;
        }
        case UART_CMD_WIFI_CONNECT_RSP:
        {
            OPL_LOG_INFO(APP, "Slave WIFI connected");
            break;
        }
        case UART_CMD_WIFI_DISCONNECT_RSP:
        {
            OPL_LOG_INFO(APP, "Slave WIFI disconnected");
            break;
        }
        case UART_CMD_PONG:
        {
            OPL_LOG_INFO(APP, "recv PONG");

            // received pong, send slave to sleep
#if (APP_HOST_MODE_PWR_SAVE_DEMO == 0)
            tUartData.tUartCmdType = UART_CMD_SMART_SLEEP_ENT_REQ;
            tUartData.u8DataLen = 0;

            APP_SendMessage(APP_EVT_UART_MSG_SEND, (uint8_t *)&tUartData, sizeof(T_UartData));
#else
            // start sleep slave timer
            osTimerStart(g_tAppSleepSlaveTimer, 10000);
#endif

            break;
        }
        default:
            return OPL_ERR_PARAM_INVALID;
    }

    return OPL_OK;
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
    UART_Init();
}

void APP_DataInit(void)
{
    // user implement
}

void APP_TaskInit(void)
{
    // create timer
    osTimerDef_t tTimerDef;

    tTimerDef.ptimer = APP_SleepSlaveTimerTimeoutHandler;
    g_tAppSleepSlaveTimer = osTimerCreate(&tTimerDef, osTimerOnce, NULL);
    if(g_tAppSleepSlaveTimer == NULL)
    {
        OPL_LOG_ERRO(APP, "Create sleep slave timer fail");
    }

    tTimerDef.ptimer = APP_WakeupSlaveTimerTimeoutHandler;
    g_tAppWakeupSlaveTimer = osTimerCreate(&tTimerDef, osTimerOnce, NULL);
    if(g_tAppWakeupSlaveTimer == NULL)
    {
        OPL_LOG_ERRO(APP, "Create ping slave timer fail");
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
    Opl_Ble_Init_Req(false);

    // user implement
}

void APP_NetInit(void)
{
    // Network manager initialize (auto-connect enable)
    APP_NmInit(false, &APP_NmUnsolicitedCallback);

    // user implement
}

void APP_CldInit(void)
{
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
    
    // enter smart sleep after 5s
#if (PS_ENABLED == 1)
    PS_EnterSmartSleep(5000);
#endif

#if (APP_HOST_MODE_PWR_SAVE_DEMO == 0)
    // start sleep slave timer
    osTimerStart(g_tAppSleepSlaveTimer, 10000);
#endif
}
