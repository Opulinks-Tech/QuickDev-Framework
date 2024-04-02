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
*  transfer.h
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
#include "log.h"
#include "qd_config.h"
// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __TRANSFER_H__
#define __TRANSFER_H__

#ifdef __cplusplus
extern "C" {
#endif

// module log
// #if (OTA_LOG_ENABLED == 1)
// #define OTA_LOG_DEBG(...)                               OPL_LOG_DEBG(OTA, __VA_ARGS__)
// #define OTA_LOG_INFO(...)                               OPL_LOG_INFO(OTA, __VA_ARGS__)
// #define OTA_LOG_WARN(...)                               OPL_LOG_WARN(OTA, __VA_ARGS__)
// #define OTA_LOG_ERRO(...)                               OPL_LOG_ERRO(OTA, __VA_ARGS__)
// #else
// #define OTA_LOG_DEBG(...)
// #define OTA_LOG_INFO(...)
// #define OTA_LOG_WARN(...)
// #define OTA_LOG_ERRO(...)
// #endif

typedef enum E_AckTag
{
    // * range for BLE commands
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
    ACK_TAG_CLOUD_MQTT_PUB_DATA_IND,
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

} T_AckTag;

typedef enum E_AckStatus
{
    ACK_STATUS_OK = 0,
    ACK_STATUS_FAIL,
    ACK_STATUS_BEGIN,
    ACK_STATUS_PROGRESS,
    ACK_STATUS_END,
} T_AckStatus;

typedef struct S_AckHdr
{
    uint8_t ack_tag;    // T_AckTag
    uint8_t ack_status; // T_AckStatus;
} T_AckHdr;

typedef struct S_AckPayload
{
    uint16_t payload_len;
    uint8_t *payload;
    uint16_t sum;       // check sum
}T_AckPayload;

typedef struct S_AckMqttPayload
{
    uint16_t topic_name_len;
    uint8_t *topic_name;
    T_AckPayload ack_payload;
} T_AckMqttPayload;

typedef struct S_TransferFileFormat
{
    T_AckHdr ack_hdr;
    uint8_t reserve;    // 0
    uint32_t file_total_len;
    T_AckPayload ack_payload;
} T_TransferFileFormat;

typedef struct S_TransferCommandAckFormat
{
    T_AckHdr ack_hdr;
    T_AckPayload ack_payload;
} T_TransferCommandAckFormat;

typedef struct S_TransferMqttAckFormat
{
    T_AckHdr ack_hdr;
    T_AckMqttPayload mqtt_payload;
} T_TransferMqttAckFormat;

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/*************************************************************************
* FUNCTION:
*   Transfer_File
*
* DESCRIPTION:
*   transfer file
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
T_OplErr Transfer_File(int method, uint8_t *pu8Data, uint16_t u16Len, uint16_t u16FileTotalLen);

/*************************************************************************
* FUNCTION:
*   Transfer_AckCommand
*
* DESCRIPTION:
*   transfer ack command
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Transfer_AckCommand(T_AckTag tAckTagId, T_AckStatus tAckStatus, uint8_t *pu8Data, uint16_t u16Len);

/*************************************************************************
* FUNCTION:
*   Transfer_MqttRecivedData
*
* DESCRIPTION:
*   transfer mqtt received data
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
T_OplErr Transfer_MqttRecivedData(uint8_t *pu8TopicName, uint16_t u16TopicNameLen, uint8_t *pu8Data, uint16_t u16Len);

/*************************************************************************
* FUNCTION:
*   Transfer_ScanList
*
* DESCRIPTION:
*   transfer scan list
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
T_OplErr Transfer_ScanList(T_AckTag tAckTagId, uint8_t *pu8Bssid, uint8_t *pu8Ssid, uint8_t u8SsidLen, uint8_t u8Channel, int8_t i8Rssi, uint8_t u8AuthMode);

/*************************************************************************
* FUNCTION:
*   Transfer_ScanNum
*
* DESCRIPTION:
*   transfer scan ap number
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
T_OplErr Transfer_ScanNum(T_AckTag tAckTagId, uint8_t ApNum);

/*************************************************************************
* FUNCTION:
*   Transfer_Init
*
* DESCRIPTION:
*   initialize transfer module
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Transfer_Init(void);

/***********
C Functions
***********/
// Sec 8: C Functions

#ifdef __cplusplus
}
#endif

#endif /* __TRANSFER_H__ */
