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
*  ring_buffer.h
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
#include "opl_err.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

// message structure of ring buffer
typedef struct S_RingBufData
{
    uint8_t *pu8Data;
    uint32_t u32DataLen;
} T_RingBufData;

// ring buffer configure
typedef struct S_RingBuf
{
    uint32_t u32ReadIdx;
    uint32_t u32WriteIdx;
    T_RingBufData *tRingBufData;
    osSemaphoreId tRbSemaphoreId;
    uint32_t u32QueueMaxCount;
    uint16_t u16QueueCount;
} T_RingBuf;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/*************************************************************************
* FUNCTION:
*   RingBuf_Push
*
* DESCRIPTION:
*   push data into ring buffer
*
* PARAMETERS
*   ptRingBuf :     [IN] ring buffer configure
*   ptRingBufData:  [IN] ring buffer data
*
* RETURNS
*   T_OplErr        see in opl_err.h
*
*************************************************************************/
T_OplErr RingBuf_Push(T_RingBuf *ptRingBuf, T_RingBufData *ptRingBufData);

/*************************************************************************
* FUNCTION:
*   RingBuf_Pop
*
* DESCRIPTION:
*   pop data from ring buffer
*
* PARAMETERS
*   ptRingBuf :     [IN] ring buffer configure
*   ptRingBufData : [OUT] ring buffer data
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr RingBuf_Pop(T_RingBuf *ptRingBuf, T_RingBufData *ptRingBufData);

/*************************************************************************
* FUNCTION:
*   RingBuf_Reset
*
* DESCRIPTION:
*   clear all data in ring buffer
*
* PARAMETERS
*   ptRingBuf :     [IN] ring buffer configure
*
* RETURNS
*   none
*
*************************************************************************/
void RingBuf_Reset(T_RingBuf *ptRingBuf);

/*************************************************************************
* FUNCTION:
*   RingBuf_ReadIdxUpdate
*
* DESCRIPTION:
*   update the read index of ring buffer (follow after ring buffer pop)
*
* PARAMETERS
*   ptRingBuf :     [IN] ring buffer configure
*
* RETURNS
*   none
*
*************************************************************************/
void RingBuf_ReadIdxUpdate(T_RingBuf *ptRingBuf);

/*************************************************************************
* FUNCTION:
*   RingBuf_CheckEmpty
*
* DESCRIPTION:
*   check ring buffer is empty
*
* PARAMETERS
*   ptRingBuf :     [IN] ring buffer configure
*
* RETURNS
*   bool :          [OUT] true -> ring buffer is empty
*                         false -> ring buffer not empty
*
*************************************************************************/
bool RingBuf_CheckEmpty(T_RingBuf *ptRingBuf);

/*************************************************************************
* FUNCTION:
*   RingBuf_CheckFull
*
* DESCRIPTION:
*   check ring buffer is full
*
* PARAMETERS
*   ptRingBuf :     [IN] ring buffer configure
*
* RETURNS
*   bool :          [OUT] true -> ring buffer is full
*                         false -> ring buffer not full
*
*************************************************************************/
bool RingBuf_CheckFull(T_RingBuf *ptRingBuf);

/*************************************************************************
* FUNCTION:
*   RingBuf_QueueCount
*
* DESCRIPTION:
*   get current queue count of ring buffer
*
* PARAMETERS
*   ptRingBuf :     [IN] ring buffer configure
*   u8QueueCount :  [OUT] queue number of ring buffer
*
* RETURNS
*   none
*
*************************************************************************/
void RingBuf_QueueCount(T_RingBuf *ptRingBuf, uint16_t *u16QueueCount);

/*************************************************************************
* FUNCTION:
*   RingBuf_Init
*
* DESCRIPTION:
*   ring buffer initiate function
*
* PARAMETERS
*   ptRingBuf :     [IN] ring buffer configure
*   u8QueueMaxCount :
*                   [IN] maximum queue number of ring buffer
*
* RETURNS
*   none
*
*************************************************************************/
void RingBuf_Init(T_RingBuf *ptRingBuf, uint8_t u8QueueMaxCount);

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

#endif /* __RING_BUFFER_H__ */
