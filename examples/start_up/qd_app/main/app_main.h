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
*  app_main.h
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

#include "opl_err.h"
#include "mw_fim_default_group03.h"
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __APP_MAIN_H__
#define __APP_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#define APP_TASK_PRIORITY               (osPriorityNormal)
#define APP_QUEUE_SIZE                  (20)
#define APP_TASK_STACK_SIZE             (512)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/// user event range from 0x1000 to 0x1FFF
typedef enum E_AppEvtId
{
    APP_EVT_BEGIN = 0x1000,

    // user event begin here

    APP_EVT_BLE_START_ADV,
    APP_EVT_BLE_STOP_ADV,
    APP_EVT_BLE_CONNECTED,
    APP_EVT_BLE_DISCONNECTED,
    APP_EVT_BLE_DATA_IND,
    APP_EVT_BLE_POWER_CONSUMPTION,  
    APP_EVT_NETWORK_UP,
    APP_EVT_NETWORK_DOWN,
    APP_EVT_NETWORK_RESET,

    APP_EVT_WAKEUP_TIMER_START,
    APP_EVT_SYS_TIMER_TIMEOUT,

    // user event end here
    
    APP_EVT_TOTAL,
    APP_EVT_MAX = 0x1FFF,
} T_AppEvtId;

typedef void (*T_AppEvtHandlerFp)(uint32_t u32EventId, void *pData, uint32_t u32DataLen);

typedef struct S_AppEvtHandlerTbl
{
    uint32_t            u32EventId;
    T_AppEvtHandlerFp   fpFunc;
} T_AppEvtHandlerTbl;

typedef struct S_AppMsgStruct
{
    uint32_t            u32EventId;
    uint32_t            u32DataLen;
    uint8_t             pau8Data[];
} T_AppMsgStruct;

typedef enum E_BlePCSleepMode
{
    SMART_SLEEP,
    TIMER_SLEEP,
    DEEP_SLEEP,
    QUERY_STATUS,

    SLEEP_MAX,
}T_BlePCSleepMode;

typedef struct S_BlePCStruct
{
    uint8_t             u8SleepMode;    // 0: smart sleep, 1: timer sleep, 2: deep sleep
    uint8_t             u8BleAdvOnOff;  // BLE AdvertiseMent On/Off for smart sleep
    uint16_t            u16BleAdvIntv;  // BLE AdvertiseMent Interval min for smart sleep when BLE adv on 0.625ms uint min = 100, max = 1500
    uint32_t            u32DtimPeriod;  // Skip dtim setting for smart sleep min = 100ms max = 5000ms 
    uint32_t            u32SleepTime;   // Sleep timer for timer sleep, range min = 15, max = 150000 (ms)
    uint32_t            u32WakeupTime;  // Wake up timer for timer sleep, range min = 15, max = 150000 (ms)      
} T_BlePCStruct;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

//////////////////////////////////// 
//// APP function group
////////////////////////////////////

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
T_OplErr APP_SendMessage(uint32_t u32EventId, uint8_t *pu8Data, uint32_t u32DataLen);

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
T_OplErr APP_EventProcess(T_AppMsgStruct *ptMsg);

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
void APP_Main(void *args);

/*************************************************************************
* FUNCTION:
*   APP_MainInit
*
* DESCRIPTION:
*   APP init
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void APP_MainInit(void);

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype
static void APP_EvtHandler_BleStartAdv(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_BleStopAdv(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_BleConnected(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_BleDisconnected(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_BleDataInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_BleAdvSetInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen); 
static void APP_EvtHandler_BlePowerConsumptionInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen); 
static void APP_EvtHandler_NetworkUp(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_NetworkDown(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_NetworkReset(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_WakeUp(uint32_t u32EventId, void *pData, uint32_t u32DataLen); 
static void APP_EvtHandler_SysTimerTimeout(uint32_t u32EventId, void *pData, uint32_t u32DataLen);

/***********
C Functions
***********/
// Sec 8: C Functions

#ifdef __cplusplus
}
#endif

#endif /* __APP_MAIN_H__ */
