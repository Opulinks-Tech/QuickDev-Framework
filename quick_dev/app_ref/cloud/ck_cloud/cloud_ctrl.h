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

#include "cloud_kernel.h"
#include "cloud_config.h"
#include "evt_group.h"
#include "log.h"
#include "opl_err.h"

#if (CLOUD_TX_DATA_BACKUP_ENABLED == 1)
#include "ring_buffer.h"
#endif /* CLOUD_TX_DATA_BACKUP_ENABLED */

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __CLOUD_CTRL_H__
#define __CLOUD_CTRL_H__

#ifdef __cplusplus
extern "C" {
#endif

// event group bit (0 ~ 23 bits)
// #define IOT_DATA_EVENT_BIT_CLOUD_CONNECTED              0x00000001U
#define IOT_DATA_EVENT_BIT_WAITING_RX_RSP               0x00000002U
#define IOT_DATA_EVENT_BIT_POST_FAIL_RECONNECT          0x00000004U  // trigger by post fail
#define IOT_DATA_EVENT_BIT_LAST_POST_RETRY              0x00000008U  // true: last post fail, then retry
#define IOT_DATA_EVENT_BIT_TIME_QUERY_WHEN_BOOT         0x00000010U
#define IOT_DATA_EVENT_BIT_WIFI_UP                      0x00000020U
#define IOT_DATA_EVENT_BIT_WAITING_TCP_ACK              0x00000040U  // when got the response, need to send back the TCP ACK

// the return value of data post
#define IOT_DATA_POST_RET_CONTINUE_DELETE               (0) // continue post + delete data
#define IOT_DATA_POST_RET_CONTINUE_KEEP                 (1) // continue post + keep data
#define IOT_DATA_POST_RET_STOP_DELETE                   (2) // stop post + delete data
#define IOT_DATA_POST_RET_STOP_KEEP                     (3) // stop post + keep data

#define IOT_DATA_POST_RETRY_MAX                         (1) // the max of post retry times
#define IOT_DATA_LAST_DATA_POST_RETRY_MAX               (2) // the max of last data post retry times
#define IOT_DATA_KEEP_ALIVE_RETRY_MAX                   (1) // the max of keep alive retry times
#define IOT_DATA_KEEP_ALIVE_FAIL_ROUND_MAX              (2) // if keep alive fail round > IOT_DATA_KEEP_ALIVE_FAIL_ROUND_MAX , cloud disconnect
#define IOT_DATA_QUEUE_LAST_DATA_CNT                    (1) // the last data count of queue

#define IOT_DATA_POST_CHECK_CLOUD_CONNECTION            (1)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef enum E_CloudTimerIdx
{
    // user implement
    // *
    CLOUD_TMR_CONN_RETRY = 0,
    CLOUD_TMR_WAIT_RX_RSP,
    CLOUD_TMR_ACK_POST,
    CLOUD_TMR_RSP_TCP_ACK,
    CLOUD_TMR_REQ_DATE,
    // *

    CLOUD_TMR_MAX,
} T_CloudTimerIdx;

typedef enum iot_data_waiting_rsp_type
{
    IOT_DATA_WAITING_TYPE_NONE                 = 0,
    IOT_DATA_WAITING_TYPE_KEEPALIVE            = 1,
    IOT_DATA_WAITING_TYPE_DATA_POST            = 2,

    IOT_DATA_WAITING_TYPE_MAX
} iot_data_waiting_rps_type_e;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

extern T_RingBuf g_stCloudRspQ;
extern T_RingBuf g_stKeepAliveQ;
extern T_RingBuf g_stIotRbData;

extern uint8_t g_u8WaitingRspType;
extern uint8_t g_u8PostRetry_KeepAlive_Cnt;
extern uint8_t g_u8PostRetry_IotRbData_Cnt;
extern uint8_t g_u8CloudRetryIntervalIdx;
extern uint8_t g_u8PostRetry_KeepAlive_Fail_Round;

extern uint16_t g_u16IotDtimTxUse;
extern uint16_t g_u16IotDtimTxCloudAckPost;
extern uint16_t g_u16IotDtimWaitRspAckPost;

extern EventGroupHandle_t g_tIotDataEventGroup;

// Sec 5: declaration of global function prototype

/*************************************************************************
* FUNCTION:
*   Cloud_ConnectRetry
*
* DESCRIPTION:
*   retry connection
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_ConnectRetry(void);

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

#if defined(MAGIC_LED)
void Cloud_SendFirstPostHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen);
#endif

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
