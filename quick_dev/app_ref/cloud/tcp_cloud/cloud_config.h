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

// keep alive time : set 0 will disable the keep alive behavior
#ifndef CLOUD_KEEP_ALIVE_TIME
#define CLOUD_KEEP_ALIVE_TIME                           (120000) //ms
#endif

// tx task watchdog reset time
#ifndef SW_RESET_TIME
#define SW_RESET_TIME                                   (300000) //ms
#endif

//---- configuration of TCP ----//

// TCP server setup
#define TCP_HOST_IP                                     ("192.168.0.100")
#define TCP_HOST_IP_SIZE                                (15)
#define TCP_HOST_PORT                                   (21)

// TCP demo test option (echo the specified data)
#define TCP_RX_DATA_ECHO_ENABLE                         (0)
#define TCP_RX_SPECIFIED_DATA                           ("rcvdata")

// TCP post/recv configuration
#define TCP_TX_POST_TIMEOUT                             (40000)
#define TCP_TX_BUF_SIZE                                 (256)
#define TCP_RX_RECV_TIMEOUT                             (5000)
#define TCP_RX_BUF_SIZE                                 (1024)

// TCP re-connect time
#define TCP_RECONN_TIME                                 (5000)

// TCP ack time
#define TCP_WAIT_TCP_ACK_TIME                           (300)
#define TCP_RECV_WAIT_TCP_ACK_TIME                      (1500)

// TCP host information (backup for retry connection)
typedef struct S_CloudTcpHostInfo
{
    uint16_t u16HostPort;
    uint8_t u8HostIp[TCP_HOST_IP_SIZE];
} T_CloudTcpHostInfo;

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
