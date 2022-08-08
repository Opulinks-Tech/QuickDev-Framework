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
#include "cmsis_os.h"
#include "log.h"
#include "lwip_helper.h"
#include "net_mngr_api.h"
#include "pwr_save.h"
#include "rf_pwr.h"
#include "wifi_mngr_api.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous
#define INIT_AND_CNCT

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
    {APP_EVT_NETWORK_UP,                    APP_EvtHandler_NetworkUp},
    {APP_EVT_NETWORK_DOWN,                  APP_EvtHandler_NetworkDown},
    {APP_EVT_NETWORK_RESET,                 APP_EvtHandler_NetworkReset},

    {APP_EVT_SYS_TIMER_TIMEOUT,             APP_EvtHandler_SysTimerTimeout},

    {0xFFFFFFFF,                            NULL},
};

// Sec 7: declaration of static function prototype

void APP_SysInit(void);
void APP_DataInit(void);
void APP_TaskInit(void);
void APP_BleInit(void);
void APP_NetInit(void);
void APP_CldInit(void);
void APP_HostModeInit(void);

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

static void APP_SysTimerTimeoutHandler(void const *argu)
{
    APP_SendMessage(APP_EVT_SYS_TIMER_TIMEOUT, NULL, 0);
}

//////////////////////////////////// 
//// Event handler group
////////////////////////////////////

// add your event handler function here

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

static void APP_EvtHandler_SysTimerTimeout(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    // user implement
}

//////////////////////////////////// 
//// APP function group
////////////////////////////////////

// add your application function here

void APP_SysInit(void)
{
    // initialize rf power setting
    RF_PwrSet(RF_CFG_DEF_PWR_SET);

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
    g_tAppSysTimer = osTimerCreate(&tTimerDef, osTimerOnce, NULL);
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
    // user implement
}

void APP_NetInit(void)
{


#ifdef INIT_AND_CNCT
    // Network manager initialize and trigger quick connect
    T_NmWifiCnctConfig tWifiCnctConfig = {0};

    memset(&tWifiCnctConfig, 0, sizeof(T_NmWifiCnctConfig));

    // connect with ssid
    tWifiCnctConfig.u8SsidLen = strlen((const char *)APP_DEF_AP_SSID);
    memcpy(tWifiCnctConfig.u8aSsid, APP_DEF_AP_SSID, tWifiCnctConfig.u8SsidLen);
    // connect with bssid
    // memcpy(tWifiCnctConfig.u8aBssid, APP_DEF_AP_BSSID, WIFI_MAC_ADDRESS_LENGTH);

    // copy password
    if(0 != strlen((const char *)APP_DEF_AP_PWD))
    {
        tWifiCnctConfig.u8PwdLen = strlen((const char *)APP_DEF_AP_PWD);
        memcpy(tWifiCnctConfig.u8aPwd, APP_DEF_AP_PWD, tWifiCnctConfig.u8PwdLen);
    }
#endif
    

#ifdef INIT_AND_CNCT
    if(OPL_OK != APP_NmInitAndCnct(&APP_NmUnsolicitedCallback, &tWifiCnctConfig))
#else
    if(OPL_OK != APP_NmInit(false, &APP_NmUnsolicitedCallback))
#endif
    {
        OPL_LOG_ERRO(APP, "Net manager init fail");

        return;
    }

    OPL_LOG_INFO(APP, "Net manager init ok, try connect to %s..", APP_DEF_AP_SSID);

    // user implement
}

void APP_CldInit(void)
{
    // user implement
}

void APP_HostModeInit(void)
{
    // TODO: host mode implement
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

    APP_HostModeInit();

    // user implement
    
    // enter smart sleep after 5s
#if (PS_ENABLED == 1)
    PS_EnterSmartSleep(5000);
#endif
}
