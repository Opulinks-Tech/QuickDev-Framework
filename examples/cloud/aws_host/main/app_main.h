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
#define APP_QUEUE_SIZE                  (30)
#define APP_TASK_STACK_SIZE             (2048)

#define HOST_MODE_DATA_LEN              (1800)

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
    APP_EVT_EXAMPLE_MODE,
    APP_EVT_SMART_SLEEP,
    APP_EVT_IO_EXIT_SMART_SLEEP,
    APP_EVT_CLOUD_MQTT_ESTAB_REQ,
    APP_EVT_CLOUD_MQTT_UPLOAD_FILE,
    
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

typedef enum E_UploadFileType
{
    UPLOAD_FILE_TYPE_MQTT_ROOT_CA = 1,
    UPLOAD_FILE_TYPE_MQTT_CLIENT_CERT,
    UPLOAD_FILE_TYPE_MQTT_PRIVATE_KEY,
    UPLOAD_FILE_TYPE_HTTPS_CA_CHAIN,
    UPLOAD_FILE_TYPE_MAX,
} T_UploadFileType;

typedef struct S_CloudConnInfo
{
    uint8_t u8AutoConn;
    uint8_t u8Security;                                 
    char u8aHostAddr[128];      // host ip address or url address
    uint16_t u16HostPort;       // host port
} T_CloudConnInfo;

typedef struct S_UploadFileStruct
{
    T_UploadFileType    tFileType;
    uint32_t            u32DataLen;
    uint32_t            u32RecvLen;
    uint8_t             pau8DataBuf[];
}T_UploadFileStruct;

// host mode req command list
typedef enum E_HostModeReqCmdList
{
    AT_CMD_REQ_FWK_VER = 0,
    AT_CMD_REQ_SLEEP,
    AT_CMD_REQ_BLE_START,
    AT_CMD_REQ_WIFI_SCAN,
    AT_CMD_REQ_WIFI_CONNECT,
    AT_CMD_REQ_WIFI_DISC,

    AT_CMD_REQ_CLOUD_UPLOUD_FILE,
    AT_CMD_REQ_CLOUD_FILE_ROOT_CA,
    AT_CMD_REQ_CLOUD_FILE_CERTIFICATE,
    AT_CMD_REQ_CLOUD_FILE_PRIVATE_KEY,
    AT_CMD_REQ_CLOUD_CILENT_ID,
    AT_CMD_REQ_CLOUD_KEEP_ALIVE,
    AT_CMD_REQ_CLOUD_LAST_WILL,
    AT_CMD_REQ_CLOUD_LAST_WILL_MSG,
    AT_CMD_REQ_CLOUD_INIT,
    AT_CMD_REQ_CLOUD_CONN,
    AT_CMD_REQ_CLOUD_DISC,
    AT_CMD_REQ_CLOUD_SUB_TOPIC,
    AT_CMD_REQ_CLOUD_PUBLISH,
    AT_CMD_REQ_CLOUD_PUBLISH_MSG,

    // add your command here
    AT_CMD_REQ_HOST_READY,

    AT_CMD_REQ_EMPTY,
    AT_CMD_REQ_MAX,
} T_HostModeReqCmdList;

// host mode ack command list
typedef enum E_HostModeAckCmdList
{
    ACK_TAG_BLE_START_ADV_REQ = 1,
    ACK_TAG_BLE_STOP_REQ,
    ACK_TAG_BLE_ADV_DATA_REQ,
    ACK_TAG_BLE_APPEAR_ERQ,
    ACK_TAG_BLE_FW_REV_REQ,
    ACK_TAG_BLE_MODEL_NB_REQ,
    ACK_TAG_BLE_GET_DEVICE_MAC_REQ,
    ACK_TAG_BLE_NOTIFY_DATA_REQ,
    ACK_TAG_BLE_START_ADV_IND,
    ACK_TAG_BLE_STOP_IND,
    ACK_TAG_BLE_CONNECTED_IND,
    ACK_TAG_BLE_DISCONNECTED_IND,
    ACK_TAG_BLE_RECV_DATA_IND,
    // * end

    // * range for WIFI commands
    ACK_TAG_WIFI_SCAN_REQ,
    ACK_TAG_WIFI_SCAN_LIST_GET_REQ,
    ACK_TAG_WIFI_CONNECT_REQ,
    ACK_TAG_WIFI_DISCONNECT_REQ,
    ACK_TAG_WIFI_GET_AP_INFO_REQ,
    ACK_TAG_WIFI_GET_MODULE_MAC_ADDR_REQ,
    ACK_TAG_WIFI_NETWORK_UP_IND,
    ACK_TAG_WIFI_NETWORK_DOWN_IND,
    ACK_TAG_WIFI_NETWORK_RESET_IND,
    // * end

    // * range for Cloud commands
    ACK_TAG_CLOUD_MQTT_INIT_REQ,
    ACK_TAG_CLOUD_MQTT_ESTAB_REQ,
    ACK_TAG_CLOUD_MQTT_DISCON_REQ,
    ACK_TAG_CLOUD_MQTT_KEEP_ALIVE_SET_REQ,
    ACK_TAG_CLOUD_MQTT_SUB_TOPIC_REQ,
    ACK_TAG_CLOUD_MQTT_UNSUB_TOPIC_REQ,
    ACK_TAG_CLOUD_MQTT_PUB_DATA_REQ,
    ACK_TAG_CLOUD_MQTT_INIT_IND,
    ACK_TAG_CLOUD_MQTT_CONNECT_IND,
    ACK_TAG_CLOUD_MQTT_DISCON_IND,
    ACK_TAG_CLOUD_MQTT_SUB_TOPIC_IND,
    ACK_TAG_CLOUD_MQTT_UNSUB_TOPIC_IND,
    ACK_TAG_CLOUD_MQTT_PUB_DATA_IND, //35
    ACK_TAG_CLOUD_MQTT_RECV_DATA_IND,
    // * end

    // * range for System or else commands
    ACK_TAG_FILE_UPLOAD,
    ACK_TAG_SLEEP,
    ACK_TAG_SMART_SLEEP,
    ACK_TAG_TIMER_SLEEP,
    ACK_TAG_DEEP_SLEEP,
    ACK_TAG_MODULE_WAKEUP,
    ACK_TAG_TRIGGER_MCU,
    ACK_TAG_OTA_START,
    ACK_TAG_OTA_DONE,
    ACK_TAG_FILE_HTTP_POST_REQ,
    ACK_TAG_FILE_HTTP_POST_RSP,
    ACK_TAG_FILE_HTTP_GET_REQ,
    ACK_TAG_FILE_HTTP_GET_RSP,
    ACK_TAG_CLOUD_MQTT_CLIENTID_SET,
    ACK_TAG_CLOUD_MQTT_LASTWILL_SET,
    
    // * system
    ACK_TAG_MODULE_READY,
    ACK_TAG_FW_VER,

    // * HOST wakeup
    ACK_TAG_HOST_READY,   
    ACK_TAG_TIMER_SLEEP_WAKEUP,
    
    ACK_TAG_MAX,
    ACK_TAG_INVALID,

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
static void APP_EvtHandler_ExmapleMode(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_SmartSleep(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_IoExitSmartSleep(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_CloudMqttEstablishReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
static void APP_EvtHandler_CloudMqttUploadFile(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
/***********
C Functions
***********/
// Sec 8: C Functions

#ifdef __cplusplus
}
#endif

#endif /* __APP_MAIN_H__ */
