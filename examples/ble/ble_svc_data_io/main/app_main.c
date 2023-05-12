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
#include "gap_svc.h"
#include "gatt_svc.h"
#include "log.h"
#include "net_mngr_api.h"
#include "ota_mngr.h"
#include "pwr_save.h"
#include "rf_pwr.h"
#include "ud_svc.h"
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

static T_AppEvtHandlerTbl g_tAppEvtHandlerTbl[] = 
{
    {APP_EVT_BLE_START_ADV,                 APP_EvtHandler_BleStartAdv},
    {APP_EVT_BLE_STOP_ADV,                  APP_EvtHandler_BleStopAdv},
    {APP_EVT_BLE_CONNECTED,                 APP_EvtHandler_BleConnected},
    {APP_EVT_BLE_DISCONNECTED,              APP_EvtHandler_BleDisconnected},
    {APP_EVT_BLE_DATA_IND,                  APP_EvtHandler_BleDataInd},

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
    OPL_LOG_INFO(APP, "Receive ble data..");

    OPL_HEX_DUMP_INFO(APP, pData, u32DataLen);

    // transit data from tx char to rx char
    UD_Svc_RxDataOutNotify(pData, u32DataLen);
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
    // OTA_Init();

    // user implement
}

void APP_DataInit(void)
{
    // user implement
}

void APP_TaskInit(void)
{
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

    // register user define service
    UD_Svc_Init();

    // initialize the ble manager (auto-adv)
    Opl_Ble_Init_Req(true);

    // user implement
}

void APP_NetInit(void)
{
    // Network manager initialize (auto-connect enable)
    // APP_NmInit(true, &APP_NmUnsolicitedCallback);

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
}
