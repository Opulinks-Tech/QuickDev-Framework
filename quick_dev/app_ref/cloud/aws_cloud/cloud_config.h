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

// host url/ip address
#ifndef CLOUD_HOST_URL_IP
#define CLOUD_HOST_URL_IP                               "your.aws.url"
#endif

// maximum host url/ip address length
#ifndef CLOUD_HOST_URL_IP_LEN
#define CLOUD_HOST_URL_IP_LEN                           ((uint16_t)(sizeof(CLOUD_HOST_URL_IP) - 1))
#endif

// host port number
#ifndef CLOUD_HOST_PORT
#define CLOUD_HOST_PORT                                 (8883)
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
#define CLOUD_PAYLOAD_LEN                               (1024)
#endif

// cloud reconnection time
#ifndef CLOUD_RECONN_TIME
#define CLOUD_RECONN_TIME                               (5000) //ms
#endif

// keep alive time : set 0 will disable the keep alive behavior
#ifndef CLOUD_KEEP_ALIVE_TIME
#define CLOUD_KEEP_ALIVE_TIME                           (600000) //ms
#endif

#ifndef CLOUD_KEEP_ALIVE_TIME_MAX
#define CLOUD_KEEP_ALIVE_TIME_MAX                       (600000) //ms
#endif

#ifndef CLOUD_KEEP_ALIVE_TIME_MIN
#define CLOUD_KEEP_ALIVE_TIME_MIN                       (60000) //ms
#endif

// wait ack timeout
#ifndef CLOUD_WAIT_ACK_TIME
#define CLOUD_WAIT_ACK_TIME                             (60000) //ms
#endif

// delay time to postpone enable skip dtim
#ifndef CLOUD_DELAY_TO_EN_SKIP_DTIM_TIME
#define CLOUD_DELAY_TO_EN_SKIP_DTIM_TIME                (100) //ms
#endif

// tx task watchdog reset time
#ifndef SW_RESET_TIME
#define SW_RESET_TIME                                   (300000) //ms
#endif

//---- configuration for AWS MQTT ----//

#define MQTT_CLIENT_ID_TAG                              "OPL_MQTT"

#define MQTT_USERNAME                                   "opulinks"

#define MQTT_PASSWORD                                   "12345678"

/*---- below definition are requires to exist ----*/

// timeout in milliseconds for processing
#define MQTT_PROCESS_TIMEOUT_MS                         (3000U)

// buffer size used to declaire the buffer for passing data to MQTT library
//#define MQTT_NETWORK_BUFFER_SIZE                        (512U)
#define MQTT_NETWORK_BUFFER_SIZE                        (1024U)

// keep alive interval in sec
#define MQTT_KEEP_ALIVE_INTERVAL_SEC                    (CLOUD_KEEP_ALIVE_TIME / 1000)

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
