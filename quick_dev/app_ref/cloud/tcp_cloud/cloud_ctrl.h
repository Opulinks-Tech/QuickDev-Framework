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
*  cloud_ctrl.h
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

#include "cloud_config.h"
#include "log.h"
#include "opl_err.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __CLOUD_CTRL_H__
#define __CLOUD_CTRL_H__

#ifdef __cplusplus
extern "C" {
#endif

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef enum E_CloudTimerIdx
{
    // user implement
    // *
    CLOUD_TMR_CONN_RETRY = 0,
    CLOUD_TMR_KEEP_ALIVE,
    CLOUD_TMR_WAIT_ACK_DATA,
    CLOUD_TMR_WAIT_TCP_ACK,
    // *

    CLOUD_TMR_MAX,
} T_CloudTimerIdx;

typedef enum E_CloudStatus
{
    // user implement
    // *
    CLOUD_CB_STA_INIT_DONE = 0,
    CLOUD_CB_STA_INIT_FAIL,
    CLOUD_CB_STA_CONN_DONE,
    CLOUD_CB_STA_CONN_FAIL,
    CLOUD_CB_STA_RECONN_DONE,
    CLOUD_CB_STA_DISCONN,
    CLOUD_CB_STA_RECV_IND,
    // *

    CLOUD_CB_STA_MAX,
} T_CloudStatus;

// cloud status callback prototype
typedef void (* T_CloudStatusCbFp)(T_CloudStatus tCloudStatus, void *pData, uint32_t u32DataLen);

//---- for event use ----//
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
typedef struct S_CloudPayloadFmt
{
    uint8_t u8TopicIndex;                               // tx topic index (range define to CLOUD_TOPIC_NUMBER in cloud_config.h)
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
*   Cloud_StatusCallbackRegister
*
* DESCRIPTION:
*   register cloud status callback function
*
* PARAMETERS
*   tCloudStatusCbFp :
*                   [IN] function pointer
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_StatusCallbackRegister(T_CloudStatusCbFp tCloudStatusCbFp);

/*************************************************************************
* FUNCTION:
*   Cloud_TxTopicRegisterDyn
*
* DESCRIPTION:
*   dynamice way to register tx topic
*
* PARAMETERS
*   tCloudTopicRegInfo :
*                   [IN] topic structure
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Cloud_TxTopicRegisterDyn(T_CloudTopicRegInfo *tCloudTopicRegInfo);

/*************************************************************************
* FUNCTION:
*   Cloud_TxTopicRegisterSta
*
* DESCRIPTION:
*   static way to subscribe tx topic
*
* PARAMETERS
*   tCloudTopicRegInfo :
*                   [IN] topic structure
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_TxTopicRegisterSta(T_CloudTopicRegInfo *tCloudTopicRegInfo);

/*************************************************************************
* FUNCTION:
*   Cloud_TxTopicUnRegisterDyn
*
* DESCRIPTION:
*   un-subscribe tx topic
*
* PARAMETERS
*   u8TopicIndex :  [IN] topic index
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Cloud_TxTopicUnRegisterDyn(uint8_t u8TopicIndex);

/*************************************************************************
* FUNCTION:
*   Cloud_RxTopicRegisterDyn
*
* DESCRIPTION:
*   dynamice way to register rx topic
*
* PARAMETERS
*   tCloudTopicRegInfo :
*                   [IN] topic structure
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Cloud_RxTopicRegisterDyn(T_CloudTopicRegInfo *tCloudTopicRegInfo);

/*************************************************************************
* FUNCTION:
*   Cloud_RxTopicRegisterSta
*
* DESCRIPTION:
*   static way to register rx topic
*
* PARAMETERS
*   tCloudTopicRegInfo :
*                   [IN] topic structure
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_RxTopicRegisterSta(T_CloudTopicRegInfo *tCloudTopicRegInfo);

/*************************************************************************
* FUNCTION:
*   Cloud_RxTopicUnRegisterDyn
*
* DESCRIPTION:
*   un-subscribe rx topic
*
* PARAMETERS
*   u8TopicIndex :  [IN] topic index
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Cloud_RxTopicUnRegisterDyn(uint8_t u8TopicIndex);

/*************************************************************************
* FUNCTION:
*   Cloud_TxTopicGet
*
* DESCRIPTION:
*   get current tx topic global table
*
* PARAMETERS
*   none
*
* RETURNS
*   T_CloudTopicRegInfoPtr :
*                   [OUT] pointer of global tx topic table
*
*************************************************************************/
T_CloudTopicRegInfoPtr Cloud_TxTopicGet(void);

/*************************************************************************
* FUNCTION:
*   Cloud_RxTopicGet
*
* DESCRIPTION:
*   get current rx topic global table
*
* PARAMETERS
*   none
*
* RETURNS
*   T_CloudTopicRegInfoPtr :
*                   [OUT] pointer of global rx topic table
*
*************************************************************************/
T_CloudTopicRegInfoPtr Cloud_RxTopicGet(void);

/*************************************************************************
* FUNCTION:
*   Cloud_GotAckHandler
*
* DESCRIPTION:
*   handle activity after received ack message
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_GotAckHandler(void);

/*************************************************************************
* FUNCTION:
*   Cloud_KeepAliveDurationSet
*
* DESCRIPTION:
*   setup the keep alive duration time, and activate the timing directly if cloud is online
*
* PARAMETERS
*   u32Duration :   [IN] duration time in ms
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_KeepAliveDurationSet(uint32_t u32Duration);

/*************************************************************************
* FUNCTION:
*   Cloud_TimerStart
*
* DESCRIPTION:
*   start timer function
*
* PARAMETERS
*   tTmrIdx :       [IN] timer index (refer to T_CloudTimerIdx enum)
*   u32TimeMs :     [IN] timer time in ms
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_TimerStart(T_CloudTimerIdx tTmrIdx, uint32_t u32TimeMs);

/*************************************************************************
* FUNCTION:
*   Cloud_TimerStop
*
* DESCRIPTION:
*   stop timer function
*
* PARAMETERS
*   tTmrIdx :       [IN] timer index (refer to T_CloudTimerIdx enum)
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_TimerStop(T_CloudTimerIdx tTmrIdx);

/*************************************************************************
* FUNCTION:
*   Cloud_InitHandler
*
* DESCRIPTION:
*   cloud init event handler (CLOUD_EVT_TYPE_INIT event)
*
* PARAMETERS
*   u32EventId :    [IN] event ID
*   pData :         [IN] message data
*   u32DataLen :    [IN] message data lens
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_InitHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen);

/*************************************************************************
* FUNCTION:
*   Cloud_EstablishHandler
*
* DESCRIPTION:
*   establish connection event handler (CLOUD_EVT_TYPE_ESTABLISH event)
*
* PARAMETERS
*   u32EventId :    [IN] event ID
*   pData :         [IN] message data
*   u32DataLen :    [IN] message data lens
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_EstablishHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen);

/*************************************************************************
* FUNCTION:
*   Cloud_DisconnectHandler
*
* DESCRIPTION:
*   cloud disconnection event handler (CLOUD_EVT_TYPE_DISCONNECT event)
*
* PARAMETERS
*   u32EventId :    [IN] event ID
*   pData :         [IN] message data
*   u32DataLen :    [IN] message data lens
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_DisconnectHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen);

/*************************************************************************
* FUNCTION:
*   Cloud_TimeoutHandler
*
* DESCRIPTION:
*   timeout event handler from timer (CLOUD_EVT_TYPE_TIMEOUT event)
*
* PARAMETERS
*   u32EventId :    [IN] event ID
*   pData :         [IN] message data
*   u32DataLen :    [IN] message data lens
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_TimeoutHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen);

/*************************************************************************
* FUNCTION:
*   Cloud_BindingHandler
*
* DESCRIPTION:
*   binding request event handler (CLOUD_EVT_TYPE_BINDING event)
*
* PARAMETERS
*   u32EventId :    [IN] event ID
*   pData :         [IN] message data
*   u32DataLen :    [IN] message data lens
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_BindingHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen);

/*************************************************************************
* FUNCTION:
*   Cloud_KeepAliveHandler
*
* DESCRIPTION:
*   post keep alive event handler (CLOUD_EVT_TYPE_KEEP_ALIVE event)
*
* PARAMETERS
*   u32EventId :    [IN] event ID
*   pData :         [IN] message data
*   u32DataLen :    [IN] message data lens
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_KeepAliveHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen);

/*************************************************************************
* FUNCTION:
*   Cloud_AckHandler
*
* DESCRIPTION:
*   post ack event handler (CLOUD_EVT_TYPE_ACK event)
*
* PARAMETERS
*   u32EventId :    [IN] event ID
*   pData :         [IN] message data
*   u32DataLen :    [IN] message data lens
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_AckHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen);

/*************************************************************************
* FUNCTION:
*   Cloud_PostHandler
*
* DESCRIPTION:
*   post data event handler (CLOUD_EVT_TYPE_POST event)
*
* PARAMETERS
*   u32EventId :    [IN] event ID
*   pData :         [IN] message data
*   u32DataLen :    [IN] message data lens
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_PostHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen);

#if (CLOUD_TX_DATA_BACKUP_ENABLED == 1)
/*************************************************************************
* FUNCTION:
*   Cloud_BackupRingBufInit
*
* DESCRIPTION:
*   init ring buffers here (will be called at runs in Cloud_Init())
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_BackupRingBufInit(void);

/*************************************************************************
* FUNCTION:
*   Cloud_PostBackupHandler
*
* DESCRIPTION:
*   post the back up data event handler (CLOUD_EVT_TYPE_POST_BACKUP event)
*   (CLOUD_TX_DATA_BACKUP_ENABLED must required, and the scenario should be implement)
*
* PARAMETERS
*   u32EventId :    [IN] event ID
*   pData :         [IN] message data
*   u32DataLen :    [IN] message data lens
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_PostBackupHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
#endif /* CLOUD_TX_DATA_BACKUP_ENABLED */

/*************************************************************************
* FUNCTION:
*   Cloud_ReceiveHandler
*
* DESCRIPTION:
*   received data from cloud handler
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_ReceiveHandler(void);

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

#endif /* __CLOUD_CTRL_H__ */
