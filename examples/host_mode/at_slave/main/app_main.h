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

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/// user event range from 0x1000 to 0x1FFF
typedef enum E_AppEvtId
{
    APP_EVT_BEGIN = 0x1000,

    // user event begin here

    APP_EVT_SYS_TIMER_TIMEOUT,
    APP_EVT_ENT_SLEEP_REQ,

    APP_EVT_OPL_PRV_START_REQ,
    APP_EVT_OPL_PRV_STOP_REQ,
    APP_EVT_OPL_PRV_TIMEOUT_IND,

    APP_EVT_BLE_STATUS_REQ,
    APP_EVT_BLE_START_ADV_IND,
    APP_EVT_BLE_STOP_ADV_IND,
    APP_EVT_BLE_CONNECTED_IND,
    APP_EVT_BLE_DISCONNECTED_IND,
    APP_EVT_BLE_DATA_IND,

    APP_EVT_WIFI_SCAN_REQ,
    APP_EVT_WIFI_CONN_REQ,
    APP_EVT_WIFI_QCONN_REQ,
    APP_EVT_WIFI_STATUS_REQ,

    APP_EVT_WIFI_SCAN_IND,
    APP_EVT_NETWORK_UP_IND,
    APP_EVT_NETWORK_DOWN_IND,
    APP_EVT_NETWORK_RESET_IND,

    APP_EVT_CLOUD_CONNECT_REQ,
    APP_EVT_CLOUD_DISCONNECT_REQ,
    APP_EVT_CLOUD_SEND_REQ,

    APP_EVT_CLOUD_CONNECT_IND,
    APP_EVT_CLOUD_DISCONNECT_IND,
    APP_EVT_CLOUD_RECV_IND,

    APP_EVT_MASTER_WAKEUP,
    APP_EVT_AT_TEST_REQ,

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

// host mode ack command list
typedef enum E_HostModeAckCmdList
{
    AT_CMD_ACK_BOOT_UP = 0,
    AT_CMD_ACK_DEVICE_READY,
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

T_OplErr APP_HostModeResponseAck(T_HostModeAckCmdList tHostModeAckCmdIdx, uint8_t *payload, uint32_t payloadlen);

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
static void APP_EvtHandler_SysTimerTimeout(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_EntSleepReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);

static void APP_EvtHandler_OplPrvSwitchReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_BleEventIndicate(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_BleDataInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen);

static void APP_EvtHandler_WifiCommandReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_WifiScanInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_NetworkUp(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_NetworkDown(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_NetworkReset(uint32_t u32EventId, void *pData, uint32_t u32DataLen);

static void APP_EvtHandler_BleWifiStatusReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);

static void APP_EvtHandler_CloudConnectReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_CloudDisconnectReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_CloudSendReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_CloudConnectInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_CloudDisconnectInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_CloudRecvInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_MasterWakeup(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_AtTestReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);

/***********
C Functions
***********/
// Sec 8: C Functions

#ifdef __cplusplus
}
#endif

#endif /* __APP_MAIN_H__ */
