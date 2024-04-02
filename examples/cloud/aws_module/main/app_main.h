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
#include "transfer.h"
#include "at_cmd_common.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __APP_MAIN_H__
#define __APP_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#define APP_TASK_PRIORITY               (osPriorityNormal)
#define APP_QUEUE_SIZE                  (20)
#define APP_TASK_STACK_SIZE             (512)
//                                   SVN Rev.     year.month.day
#define APP_FW_VER_STR                  "XXXX_YYYYMMDD"
//#define APP_FW_VER_STR                  "TEST_20240319"

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/// user event range from 0x1000 to 0x1FFF
typedef enum E_AppEvtId
{
    APP_EVT_BEGIN = 0x1000,

    // user event begin here

    APP_EVT_BLE_START_ADV_REQ,
    APP_EVT_BLE_STOP_REQ,
    APP_EVT_BLE_INIT_IND,
    APP_EVT_BLE_START_ADV_IND,
    APP_EVT_BLE_STOP_IND,
    APP_EVT_BLE_CONNECTED_IND,
    APP_EVT_BLE_DISCONNECTED_IND,
    APP_EVT_BLE_RECV_DATA_IND, 
    APP_EVT_BLE_DATA_IND,


    APP_EVT_WIFI_SCAN_REQ,
    APP_EVT_WIFI_SCAN_LIST_GET_REQ,
    APP_EVT_WIFI_CONNECT_REQ,
    APP_EVT_WIFI_DISCONNECT_REQ,
    APP_EVT_WIFI_SCAN_DONE_IND,
    APP_EVT_WIFI_NETWORK_INIT_IND,
    APP_EVT_WIFI_NETWORK_UP_IND,
    APP_EVT_WIFI_NETWORK_DOWN_IND,
    APP_EVT_WIFI_NETWORK_RESET_IND,

    APP_EVT_CLOUD_MQTT_INIT_REQ,
    APP_EVT_CLOUD_MQTT_ESTAB_REQ,
    APP_EVT_CLOUD_MQTT_DISCONN_REQ,
    APP_EVT_CLOUD_MQTT_SUB_TOPIC_REQ,
    APP_EVT_CLOUD_MQTT_UNSUB_TOPIC_REQ,
    APP_EVT_CLOUD_MQTT_PUB_DATA_REQ,
    APP_EVT_CLOUD_MQTT_INIT_IND,
    APP_EVT_CLOUD_MQTT_CONNECT_IND,
    APP_EVT_CLOUD_MQTT_DISCONN_IND,
    APP_EVT_CLOUD_MQTT_SUB_TOPIC_IND,
    APP_EVT_CLOUD_MQTT_UNSUB_TOPIC_IND,
    APP_EVT_CLOUD_MQTT_PUB_DATA_IND,
    APP_EVT_CLOUD_RECV_DATA_IND,  
    APP_EVT_OTA_START_IND,
    APP_EVT_OTA_DONE_IND,
    APP_EVT_OTA_FAIL_IND,

    APP_EVT_HOST_READY_IND,
    APP_EVT_SEND_DATA_TO_MCU_IND,

    APP_EVT_TIMER_SLEEP_TIMEOUT_WAKEUP,
    APP_EVT_IO_EXIT_SMART_SLEEP_IND,
    APP_EVT_IO_EXIT_TIMER_SLEEP_IND,
    APP_EVT_PREPARE_SLEEP,
    APP_EVT_WIFI_STOP_SLEEP_CHECK,

    APP_EVT_SYS_TIMER_TIMEOUT,
    APP_EVT_WAIT_HOST_TIMER_TIMEOUT,

    // user event end here
    
    APP_EVT_TOTAL,
    APP_EVT_MAX = 0x1FFF,
} T_AppEvtId;

// enum for mcu state
typedef enum E_AppHostStateList
{
    //APP_HOST_ST_READY,
    APP_HOST_ST_NOT_WAIT,
    APP_HOST_ST_WAIT, // wait for at-command: at+hostready

    APP_HOST_ST_INVALID,
} T_AppHostState;


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

// event linkedlist Node 
typedef struct S_AppAckInfoStruct
{
    T_AckTag    AckTag;
    T_AckStatus AckStatus;
    uint8_t     *AckData;
    uint16_t    AckDataLen;

    uint8_t     *TopicName;
    uint16_t    TopicNameLen;
    uint8_t     *PayloadBuf;
    uint16_t    PayloadLen;

    struct S_AppAckInfoStruct *next;
} T_AppAckInfoStruct;

typedef struct S_AppAckQueue
{
    T_AppAckInfoStruct *AckQueueHead;
    T_AppAckInfoStruct *AckQueueTail;
    size_t AckQueueSize;
} T_AppAckQueue;

typedef struct S_AppSleepInfo
{
    uint8_t     u8SleepMode; // 0: smart sleep, 1: timer sleep, 2: deep sleep
    uint32_t    u32SleepTime;
}T_AppSleepInfo;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

//////////////////////////////////// 
//// APP function group
////////////////////////////////////
/*************************************************************************
* FUNCTION:
*   APP_TriggerHostWakeUp
*
* DESCRIPTION:
*   Trgger MCU Wakeup
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void APP_TriggerHostWakeUp(void);
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

//void APP_NmWifiStopCb(T_OplErr tEvtRst);
/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

static void APP_EvtHandler_BleStateReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_BleAdvDataReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_BleStateRsp(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_BleRecvDataRsp(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_BleDataInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_WifiScanReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_WifiConnectReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_WifiDisconnectReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_WifiApInfoReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_WifiModuleMacReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_WifiNetworkStateRsp(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_CloudMqttInitReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_CloudMqttEstablishReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_CloudMqttDisconnectReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_CloudMqttSubTopicReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_CloudMqttUnSubTopicReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_CloudMqttPubDataReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_CloudMqttStateRsp(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_CloudMqttRecvDataRsp(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_OtaStateRsp(uint32_t u32EventId, void *pData, uint32_t u32DataLen); 
static void APP_EvtHandler_IoExitSmartSleepRsp(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_IoExitTimerSleepRsp(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_HostReadyRsp(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_SendDataToMcu(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_PrepareSleep(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_WifiStopSleepCheckHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_TimerSleepTimeoutWakeup(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_SysTimerTimeout(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_WaitHostTimerTimeout(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
/***********
C Functions
***********/
// Sec 8: C Functions

#ifdef __cplusplus
}
#endif

#endif /* __APP_MAIN_H__ */
