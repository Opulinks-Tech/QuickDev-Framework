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
#include "cloud_config.h"
#include "log.h"
#include "opl_err.h"

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

#if (CLOUD_TX_DATA_BACKUP_ENABLED == 1)
// message structure of ring buffer
typedef struct S_CloudRingBufData
{
    uint8_t *pu8Data;
    uint32_t u32DataLen;
} T_CloudRingBufData;

// ring buffer configure
typedef struct S_CloudRingBuf
{
    uint32_t u32ReadIdx;
    uint32_t u32WriteIdx;
    T_CloudRingBufData *tCloudRingBufData;
    osSemaphoreId tRbSemaphoreId;
    uint32_t u32QueueMaxCount;
    uint16_t u16QueueCount;
} T_CloudRingBuf;
#endif /* CLOUD_TX_DATA_BACKUP_ENABLED */

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

#if (CLOUD_TX_DATA_BACKUP_ENABLED == 1)
/*************************************************************************
* FUNCTION:
*   Cloud_RingBufPush
*
* DESCRIPTION:
*   push data into ring buffer
*
* PARAMETERS
*   ptCloudRingBuf :
*                   [IN] ring buffer configure
*   ptCloudRingBufData:
*                   [IN] ring buffer data
*
* RETURNS
*   T_OplErr        see in opl_err.h
*
*************************************************************************/
T_OplErr Cloud_RingBufPush(T_CloudRingBuf *ptCloudRingBuf, T_CloudRingBufData *ptCloudRingBufData);

/*************************************************************************
* FUNCTION:
*   Cloud_RingBufPop
*
* DESCRIPTION:
*   pop data from ring buffer
*
* PARAMETERS
*   ptCloudRingBuf :
*                   [IN] ring buffer configure
*   ptCloudRingBufData :
*                   [OUT] ring buffer data
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Cloud_RingBufPop(T_CloudRingBuf *ptCloudRingBuf, T_CloudRingBufData *ptCloudRingBufData);

/*************************************************************************
* FUNCTION:
*   Cloud_RingBufReset
*
* DESCRIPTION:
*   clear all data in ring buffer
*
* PARAMETERS
*   ptCloudRingBuf :
*                   [IN] ring buffer configure
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_RingBufReset(T_CloudRingBuf *ptCloudRingBuf);

/*************************************************************************
* FUNCTION:
*   Cloud_RingBufReadIdxUpdate
*
* DESCRIPTION:
*   update the read index of ring buffer (follow after ring buffer pop)
*
* PARAMETERS
*   ptCloudRingBuf :
*                   [IN] ring buffer configure
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_RingBufReadIdxUpdate(T_CloudRingBuf *ptCloudRingBuf);

/*************************************************************************
* FUNCTION:
*   Cloud_RingBufCheckEmpty
*
* DESCRIPTION:
*   check ring buffer is empty
*
* PARAMETERS
*   ptCloudRingBuf :
*                   [IN] ring buffer configure
*
* RETURNS
*   bool :          [OUT] true -> ring buffer is empty
*                         false -> ring buffer not empty
*
*************************************************************************/
bool Cloud_RingBufCheckEmpty(T_CloudRingBuf *ptCloudRingBuf);

/*************************************************************************
* FUNCTION:
*   Cloud_RingBufCheckFull
*
* DESCRIPTION:
*   check ring buffer is full
*
* PARAMETERS
*   ptCloudRingBuf :
*                   [IN] ring buffer configure
*
* RETURNS
*   bool :          [OUT] true -> ring buffer is full
*                         false -> ring buffer not full
*
*************************************************************************/
bool Cloud_RingBufCheckFull(T_CloudRingBuf *ptCloudRingBuf);

/*************************************************************************
* FUNCTION:
*   Cloud_RingBufGetQueueCount
*
* DESCRIPTION:
*   get current queue count of ring buffer
*
* PARAMETERS
*   ptCloudRingBuf :
*                   [IN] ring buffer configure
*   u8QueueCount :  [OUT] queue number of ring buffer
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_RingBufGetQueueCount(T_CloudRingBuf *ptCloudRingBuf, uint16_t *u16QueueCount);

/*************************************************************************
* FUNCTION:
*   Cloud_RingBufInit
*
* DESCRIPTION:
*   ring buffer initiate function
*
* PARAMETERS
*   ptCloudRingBuf :
*                   [IN] ring buffer configure
*   u8QueueMaxCount :
*                   [IN] maximum queue number of ring buffer
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_RingBufInit(T_CloudRingBuf *ptCloudRingBuf, uint8_t u8QueueMaxCount);

#endif /* CLOUD_TX_DATA_BACKUP_ENABLED */

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
