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
*  ring_buffer.c
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

#include "ring_buffer.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

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

/*************************************************************************
* FUNCTION:
*   Cloud_RingBufPush
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
T_OplErr RingBuf_Push(T_RingBuf *ptRingBuf, T_RingBufData *ptRingBufData)
{
    T_OplErr tEvtRst = OPL_ERR;

    uint32_t u32WriteNext;

    osSemaphoreWait(ptRingBuf->tRbSemaphoreId, osWaitForever);

    if (ptRingBufData == NULL)
    {
        goto done;
    }

    // full, ulWriteIdx + 1 == ulReadIdx
    u32WriteNext = (ptRingBuf->u32WriteIdx + 1) % ptRingBuf->u32QueueMaxCount;

    // Read index always prior to write index
    if (u32WriteNext == ptRingBuf->u32ReadIdx)
    {
        // discard the oldest data, and read index move forware one step.
        free(ptRingBuf->tRingBufData[ptRingBuf->u32ReadIdx].pu8Data);
        ptRingBuf->u32ReadIdx = (ptRingBuf->u32ReadIdx + 1) % ptRingBuf->u32QueueMaxCount;
    }

    // update the temperature data to write index
	memcpy(&(ptRingBuf->tRingBufData[ptRingBuf->u32WriteIdx]), ptRingBufData, sizeof(T_RingBufData));

    ptRingBuf->u32WriteIdx = u32WriteNext;
    ptRingBuf->u16QueueCount++;

    tEvtRst = OPL_OK;

done:
    osSemaphoreRelease(ptRingBuf->tRbSemaphoreId);

    return tEvtRst;
}

/*************************************************************************
* FUNCTION:
*   Cloud_RingBufPop
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
T_OplErr RingBuf_Pop(T_RingBuf *ptRingBuf, T_RingBufData *ptRingBufData)
{
    T_OplErr tEvtRst = OPL_ERR;

    osSemaphoreWait(ptRingBuf->tRbSemaphoreId, osWaitForever);

    if (ptRingBufData == NULL)
    {
        goto done;
    }

    // empty, ulWriteIdx == ulReadIdx
    if (ptRingBuf->u32WriteIdx == ptRingBuf->u32ReadIdx)
    {
        goto done;
    }

	memcpy(ptRingBufData, &(ptRingBuf->tRingBufData[ptRingBuf->u32ReadIdx]), sizeof(T_RingBufData));

    tEvtRst = OPL_OK;

done:
    osSemaphoreRelease(ptRingBuf->tRbSemaphoreId);

    return tEvtRst;
}

/*************************************************************************
* FUNCTION:
*   Cloud_RingBufReset
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
void RingBuf_Reset(T_RingBuf *ptRingBuf)
{
    T_CloudRingBufData ptRingBufData;

    while (false == Cloud_RingBufCheckEmpty(ptRingBuf))
    {
        Cloud_RingBufPop(ptRingBuf, &ptRingBufData);
        Cloud_RingBufReadIdxUpdate(ptRingBuf);

        if(ptRingBufData.pu8Data != NULL)
        {
            free(ptRingBufData.pu8Data);
        }
    }

    ptRingBuf->u16QueueCount = 0;
}

/*************************************************************************
* FUNCTION:
*   Cloud_RingBufReadIdxUpdate
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
void RingBuf_ReadIdxUpdate(T_RingBuf *ptRingBuf)
{
    osSemaphoreWait(ptRingBuf->tRbSemaphoreId, osWaitForever);

    ptRingBuf->u32ReadIdx = (ptRingBuf->u32ReadIdx + 1) % ptRingBuf->u32QueueMaxCount;
    ptRingBuf->u16QueueCount --;

    osSemaphoreRelease(ptRingBuf->tRbSemaphoreId);
}

/*************************************************************************
* FUNCTION:
*   Cloud_RingBufCheckEmpty
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
bool RingBuf_CheckEmpty(T_RingBuf *ptRingBuf)
{
    bool blRet = false;

    osSemaphoreWait(ptRingBuf->tRbSemaphoreId, osWaitForever);

    // empty, ulWriteIdx == ulReadIdx
    if (ptRingBuf->u32WriteIdx == ptRingBuf->u32ReadIdx)
    {
        blRet = true;
    }

    osSemaphoreRelease(ptRingBuf->tRbSemaphoreId);

    return blRet;
}

/*************************************************************************
* FUNCTION:
*   Cloud_RingBufCheckFull
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
bool RingBuf_CheckFull(T_RingBuf *ptRingBuf)
{
    bool blRet = false;
    uint32_t u32WriteNext;

    osSemaphoreWait(ptRingBuf->tRbSemaphoreId, osWaitForever);

    // full, ulWriteIdx + 1 == ulReadIdx
    u32WriteNext = (ptRingBuf->u32WriteIdx + 1) % ptRingBuf->u32QueueMaxCount;

    // Read index always prior to write index
    if(u32WriteNext == ptRingBuf->u32ReadIdx)
    {
        blRet = true;
    }

    osSemaphoreRelease(ptRingBuf->tRbSemaphoreId);

    return blRet;
}

/*************************************************************************
* FUNCTION:
*   Cloud_RingBufGetQueueCount
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
void RingBuf_QueueCount(T_RingBuf *ptRingBuf, uint16_t *u16QueueCount)
{
    osSemaphoreWait(ptRingBuf->tRbSemaphoreId, osWaitForever);

    *u16QueueCount = ptRingBuf->u16QueueCount;

    osSemaphoreRelease(ptRingBuf->tRbSemaphoreId);
}

/*************************************************************************
* FUNCTION:
*   Cloud_RingBufInit
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
void RingBuf_Init(T_RingBuf *ptRingBuf, uint8_t u8QueueMaxCount)
{
    osSemaphoreDef_t tSemaphoreDef;

    // create semaphore
    tSemaphoreDef.dummy = 0;
    ptRingBuf->tRbSemaphoreId = osSemaphoreCreate(&tSemaphoreDef, 1);

    if(NULL == ptRingBuf->tRbSemaphoreId)
    {
        OPL_LOG_ERRO(CLOUD, "Create RB sempahore fail");
    }

    osSemaphoreWait(ptRingBuf->tRbSemaphoreId, osWaitForever);

    ptRingBuf->u16QueueCount = 0;
    ptRingBuf->u32QueueMaxCount = u8QueueMaxCount;
    ptRingBuf->u32ReadIdx = 0;
    ptRingBuf->u32WriteIdx = 0;
    ptRingBuf->tCloudRingBufData = (T_RingBufData *)malloc(sizeof(T_RingBufData) * u8QueueMaxCount);
 
    osSemaphoreRelease(ptRingBuf->tRbSemaphoreId);

}
