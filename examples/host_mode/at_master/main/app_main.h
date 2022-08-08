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

#include "at_cmd_common.h"
#include "mw_fim_default_group03.h"
#include "opl_err.h"
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

#define HOST_MODE_DATA_LEN              (128)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/// user event range from 0x1000 to 0x1FFF
typedef enum E_AppEvtId
{
    APP_EVT_BEGIN = 0x1000,

    // user event begin here

    APP_EVT_AT_MSG_SEND,
    APP_EVT_AT_MSG_RECV,
    APP_EVT_SLEEP_SLAVE,
    APP_EVT_WAKEUP_SLAVE,

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

// host mode req command list
typedef enum E_HostModeReqCmdList
{
    AT_CMD_REQ_FWK_VER = 0,
    AT_CMD_REQ_SLEEP,
    AT_CMD_REQ_PROVISION_START,
    AT_CMD_REQ_PROVISION_STOP,
    AT_CMD_REQ_BLE_STATUS,
    AT_CMD_REQ_WIFI_SCAN,
    AT_CMD_REQ_WIFI_CONNECT,
    AT_CMD_REQ_WIFI_QCONNECT,
    AT_CMD_REQ_WIFI_STATUS,
    AT_CMD_REQ_CLOUD_CONN,
    AT_CMD_REQ_CLOUD_DISC,
    AT_CMD_REQ_CLOUD_TX_TOPIC,
    AT_CMD_REQ_CLOUD_RX_TOPIC,
    AT_CMD_REQ_CLOUD_TX_POST,

    // add your command here
    
    AT_CMD_REQ_EMPTY,
    AT_CMD_REQ_MAX,
} T_HostModeReqCmdList;

// host mode ack command list
typedef enum E_HostModeAckCmdList
{
    AT_CMD_ACK_DEVICE_READY = 0,
    AT_CMD_ACK_WAKEUP_FROM_SLEEP,
    AT_CMD_ACK_PROVISION,
    AT_CMD_ACK_BLE_STATUS,
    AT_CMD_ACK_WIFI_SCAN,
    AT_CMD_ACK_WIFI_CONN,
    AT_CMD_ACK_WIFI_DISCONN,
    AT_CMD_ACK_NETWORK_UP,
    AT_CMD_ACK_NETWORK_DOWN,
    AT_CMD_ACK_NETWORK_RESET,
    AT_CMD_ACK_WIFI_STATUS,
    AT_CMD_ACK_CLOUD_CONNECT,
    AT_CMD_ACK_CLOUD_DISCONNECT,
    AT_CMD_ACK_CLOUD_KEEPALIVE_INTERVAL,
    AT_CMD_ACK_CLOUD_TX_TOPIC,
    AT_CMD_ACK_CLOUD_RX_TOPIC,
    AT_CMD_ACK_CLOUD_TX_POST,
    AT_CMD_ACK_CLOUD_RX_RECV,
    AT_CMD_ACK_TEST,
    AT_CMD_ACK_NOT_SUPPORT,
    AT_CMD_ACK_OK,
    AT_CMD_ACK_ERROR,

    // add your command here
    
    AT_CMD_ACK_MAX,
} T_HostModeAckCmdList;

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
static void APP_EvtHandler_AtMsgSend(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_AtMsgRecv(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_SleepSlave(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_WakeupSlave(uint32_t u32EventId, void *pData, uint32_t u32DataLen);

/***********
C Functions
***********/
// Sec 8: C Functions

#ifdef __cplusplus
}
#endif

#endif /* __APP_MAIN_H__ */
