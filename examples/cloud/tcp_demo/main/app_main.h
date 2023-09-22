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
#define APP_TASK_STACK_SIZE             (1024)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/// user event range from 0x1000 to 0x1FFF
typedef enum E_AppEvtId
{
    APP_EVT_BEGIN = 0x1000,

    // user event begin here

    APP_EVT_BLE_INIT_IND,
    APP_EVT_BLE_START_ADV,
    APP_EVT_BLE_STOP_IND,
    APP_EVT_BLE_CONNECTED,
    APP_EVT_BLE_DISCONNECTED,
    APP_EVT_BLE_DATA_IND,

    APP_EVT_NETWORK_INIT_IND,
    APP_EVT_NETWORK_UP,
    APP_EVT_NETWORK_DOWN,
    APP_EVT_NETWORK_RESET,

    APP_EVT_CLOUD_CONNECT_IND,
    APP_EVT_CLOUD_DISCONNECT_IND,
    APP_EVT_CLOUD_RECV_IND,

    APP_EVT_SYS_TIMER_TIMEOUT,

    APP_EVT_POST_SET,   // For post data set

#if (OPL_POWER_SLEEP_CONTROL == 1) 
    APP_EVT_PWR_SLEEP_CONTROL,
#endif
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

// For post data set
typedef struct S_PostSetMsg
{
    uint32_t            u32PostDuration;
    uint32_t            u32PostTotalCnt;
    uint32_t            u32PostWaitAck;
} T_PostSetMsg;

#if (OPL_POWER_SLEEP_CONTROL == 1) 
typedef enum E_SlpMode
{
    SLP_MODE_SMART_SLEEP,
    SLP_MODE_TIMER_SLEEP,
    SLP_MODE_DEEP_SLEEP,

    SLP_MODE_NONE,
    SLP_MODE_MAX,
} T_SlpMode;

typedef struct S_SlpCtrlMsg
{
    T_SlpMode           eSlpMode;
    uint32_t            u32SmrtSlpEn;
} T_SlpCtrlMsg;
#endif

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

void APP_TcpSetHostInfo(uint8_t *u8HostIp, uint8_t u8HostIpLen, uint16_t u16HostPort);

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
static void APP_EvtHandler_BleInit(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_BleStartAdv(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_BleStopAdv(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_BleConnected(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_BleDisconnected(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_BleDataInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_NetworkInit(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_NetworkUp(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_NetworkDown(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_NetworkReset(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_CloudConnectInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_CloudDisconnectInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_CloudRecvInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_SysTimerTimeout(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_PostSet(uint32_t u32EventId, void *pData, uint32_t u32DataLen);  // For post data set
#if (OPL_POWER_SLEEP_CONTROL == 1)
static void APP_EvtHandler_PowerSleepControl(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
#endif

/***********
C Functions
***********/
// Sec 8: C Functions

#ifdef __cplusplus
}
#endif

#endif /* __APP_MAIN_H__ */
