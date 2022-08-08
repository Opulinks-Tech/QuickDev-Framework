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
*  cloud_config.h
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

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __CLOUD_CONFIG_H__
#define __CLOUD_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

// enable back up the post data
#ifndef CLOUD_TX_DATA_BACKUP_ENABLED
#define CLOUD_TX_DATA_BACKUP_ENABLED                    (0)
#endif

// WI-FI ota enable (OTA_ENABLE and OTA_Init() must required)
#ifndef CLOUD_OTA_ENABLED
#define CLOUD_OTA_ENABLED                               (1)
#endif

// maximum host url/ip address length
#ifndef CLOUD_HOST_URL_LEN
#define CLOUD_HOST_URL_LEN                              (128)
#endif

// maximum topic name length 
#ifndef CLOUD_TOPIC_NAME_LEN
#define CLOUD_TOPIC_NAME_LEN                            (128)
#endif

// maximum number of topic which can be reigstered
#ifndef CLOUD_TOPIC_NUMBER
#define CLOUD_TOPIC_NUMBER                              (8)
#endif

// maximum paylaod data length
#ifndef CLOUD_PAYLOAD_LEN
#define CLOUD_PAYLOAD_LEN                               (256)
#endif

//---- configuration of MQTT ----//

// keep alive time
#define CLOUD_KEEP_ALIVE_TIME                           (60000) //ms

// tx task watchdog reset time
#define SW_RESET_TIME                                   (300000) //ms

// mqtt macro group
#define MQTT_HOST_URL                                   "broker.emqx.io"
#define MQTT_HOST_PORT                                  (8883)
#define MQTT_AUTO_RECONN_TIME                           (1000)
// #define MQTT_AUTO_RECONN_EN                             (true)
#define MQTT_SSL_HOST_NAME_VERF                         (false)
#define MQTT_COMMAND_TIMEOUT                            (5000)
#define MQTT_TLS_HANDSHAKE_TIMEOUT                      (20000)
#define MQTT_YIELD_TIMEOUT                              (1000)
#define MQTT_YIELD_ROUND_DELAY                          (5000)

#define MQTT_CLIENT_ID                                  ("OPL_MQTT_ID_1")
#define MQTT_CLIENT_ID_LEN                              (15)

#define MQTT_USERNAME                                   ("opulinks")
#define MQTT_PASSWORD                                   ("12345678")

#define BLEWIFI_ENHANCE_AWS
// #define BLEWIFI_ENABLE_KEEPALIVE

// #define BLEWIFI_POST_FAIL_COUNT
// #define BLEWIFI_POST_FAIL_COUNT_MAX                     (3)

#define BLEWIFI_CONNECT_NON_BLOCKING
#define BLEWIFI_CONNECT_TIMEOUT                         (10) //(3) // seconds

// #define BLEWIFI_MQTT_PING_COUNT
// #define BLEWIFI_MQTT_PING_COUNT_MAX                     (3)
// #define BLEWIFI_MQTT_PING_INTERVAL                      (30) // seconds

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

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

#ifdef __cplusplus
}
#endif

#endif /* __CLOUD_CONFIG_H__ */
