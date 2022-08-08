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
*  iot_data.h
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

#include "cmsis_os.h"
#include "cloud_ctrl.h"
#include "cloud_config.h"
#include "cloud_data.h"
#include "cloud_ota_http.h"
#include "log.h"
#include "opl_err.h"
#if (CLOUD_TX_DATA_BACKUP_ENABLED == 1)
#include "ring_buffer.h"
#endif /* CLOUD_TX_DATA_BACKUP_ENABLED */

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __IOT_DATA_H__
#define __IOT_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CLOUD_TX_TASK_STACK_SIZE                        (2048)
#define CLOUD_TX_TASK_PRIORITY                          (osPriorityNormal)
#define CLOUD_TX_MSG_QUEUE_SIZE                         (48)
#define CLOUD_RX_TASK_STACK_SIZE                        (1792)
#define CLOUD_RX_TASK_PRIORITY                          (osPriorityNormal)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

//---- for cloud kernel use ----//
// cloud kernel event list
typedef enum E_CloudTxEvt
{
    CLOUD_EVT_TYPE_INIT,
    CLOUD_EVT_TYPE_ESTABLISH,
    CLOUD_EVT_TYPE_DISCONNECT,
    CLOUD_EVT_TYPE_TIMEOUT,
    CLOUD_EVT_TYPE_BINDING,
    CLOUD_EVT_TYPE_KEEP_ALIVE,
    CLOUD_EVT_TYPE_ACK,
    CLOUD_EVT_TYPE_POST,
#if (CLOUD_TX_DATA_BACKUP_ENABLED == 1)
    CLOUD_EVT_TYPE_POST_BACKUP,
#endif
} T_CloudTxEvt;

// prototype of cloud kernel event handler
typedef void (*T_CloudTxEvtHandlerFp)(uint32_t u32EventId, void *pData, uint32_t u32DataLen);

// cloud kernel event table
typedef struct S_CloudTxEvtHandlerTbl
{
    uint32_t u32EventId;
    T_CloudTxEvtHandlerFp fpFunc;
} T_CloudTxEvtHandlerTbl;

// cloud kernel event message
typedef struct S_CloudDataMsg
{
    uint32_t u32EventId;
    uint32_t u32DataLen;
    uint8_t u8aData[];
} T_CloudDataMsg;

//---- for event activate use ----//
// cloud connction information
typedef struct S_CloudConnInfo
{
    uint8_t u8AutoConn;                                 // cloud auto-connect after unsolicited disconnect event
                                                        // <0> NOT do auto-connect after cloud disconnect (DEFAULT)
                                                        // <1> do auto-connect after cloud disconnect

    uint8_t u8Security;                                 // TLS or SSL connection security type (only activate if connection is a security type)
                                                        // <0> MBEDTLS_SSL_VERIFY_NONE (DEFAULT)
                                                        // <1> MBEDTLS_SSL_VERIFY_OPTIONAL
                                                        // <2> MBEDTLS_SSL_VERIFY_REQUIRED
                                                        // <3> MBEDTLS_SSL_VERIFY_UNSET

    uint8_t u8aHostAddr[CLOUD_HOST_URL_LEN];            // host ip address or url address

    uint16_t u16HostPort;                               // host port
} T_CloudConnInfo;

// cloud topic register information
typedef struct S_CloudTopicRegInfo
{
    uint8_t u8TopicIndex;                               // topic index (range define to CLOUD_TOPIC_NUMBER in cloud_config.h)
    uint8_t u8IsTopicRegisted;                          // register status
    uint8_t u8aTopicName[CLOUD_TOPIC_NAME_LEN];         // topic name in string type
} T_CloudTopicRegInfo;

// cloud topic register information pointer
typedef T_CloudTopicRegInfo *T_CloudTopicRegInfoPtr;

// cloud payload format
typedef S_CloudPayloadFmt
{
    uint8_t u8TopicIndex;                               // topic index (range define to CLOUD_TOPIC_NUMBER in cloud_config.h)
    uint32_t u32PayloadLen;                             // payload lens
    uint8_t u8aPayloadBuf[CLOUD_PAYLOAD_LEN];           // payload
} T_CloudPayloadFmt;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/*************************************************************************
* FUNCTION:
*   Cloud_OnlineStatusSet
*
* DESCRIPTION:
*   set cloud connection status
*
* PARAMETERS
*   blOnline :      [IN] true -> cloud online
*                        false -> cloud offline
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_OnlineStatusSet(bool blOnline);

/*************************************************************************
* FUNCTION:
*   Cloud_OnlineStatusGet
*
* DESCRIPTION:
*   get cloud connection status
*
* PARAMETERS
*   none
*
* RETURNS
*   bool :          [OUT] true -> cloud online
*                         false -> cloud offline
*
*************************************************************************/
bool Cloud_OnlineStatusGet(void);

/*************************************************************************
* FUNCTION:
*   Cloud_OnlineStatusWait
*
* DESCRIPTION:
*   waiting cloud connection
*
* PARAMETERS
*   none
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Cloud_OnlineStatusWait(void);

/*************************************************************************
* FUNCTION:
*   Cloud_MsgSend
*
* DESCRIPTION:
*   send event message to cloud kernel
*
* PARAMETERS
*   u32EventId :    [IN] event ID
*   pu8Data :       [IN] message data
*   u32DataLen :    [IN] message data lens
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Cloud_MsgSend(uint32_t u32EventId, uint8_t *pu8Data, uint32_t u32DataLen);

/*************************************************************************
* FUNCTION:
*   Cloud_Init
*
* DESCRIPTION:
*   cloud initiate function
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_Init(void);

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

#endif /* __IOT_DATA_H__ */
