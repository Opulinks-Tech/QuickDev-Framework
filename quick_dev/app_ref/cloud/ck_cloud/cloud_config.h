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
#define CLOUD_TX_DATA_BACKUP_ENABLED                    (1)
#endif

// tx task watchdog reset time
#define SW_RESET_TIME                                   (300000) //ms

// ring buffer size
#if defined(MAGIC_LED)
#define IOT_RB_COUNT                                    (3)
#else
#define IOT_RB_COUNT                                    (32)
#endif
#define IOT_RB_KEEP_ALIVE_COUNT                         (2)
#define IOT_RB_CLOUD_RSP_COUNT                          (10)

// iot version
#define IOT_VERSION                                     (8)

// cloud http
#define CLOUD_HTTP_POST_ENHANCEMENT                     (1)

// keep alive time
#define CLOUD_KEEP_ALIVE_TIME                           (60000)

// wait response timeout
#define CLOUD_TX_WAIT_TIMEOUT                           (40000) //(20000) //ms

// ack post timeout
#define CLOUD_ACK_POST_TIMEOUT                          (1500)  //ms

// response tcp ack timeout
#define CLOUD_RSP_TCP_ACK_TIMEOUT                       (100)   //ms

// rx receive buffer size
#if defined(MAGIC_LED)
#define CLOUD_RCV_BUFFER_SIZE                           (1664)
#else
#define CLOUD_RCV_BUFFER_SIZE                           (768)   //ms
#endif

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
