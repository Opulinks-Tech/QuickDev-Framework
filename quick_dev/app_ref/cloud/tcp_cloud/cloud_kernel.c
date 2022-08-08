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
*  iot_data.c
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

#include "cloud_ctrl.h"
#include "cloud_data.h"
#include "cloud_kernel.h"
#include "cloud_ota_http.h"
#include "evt_group.h"
#include "hal_system.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#define CLOUD_EG_BIT_ONLINE                             (0x00000001U)

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

// cloud kernel event table
static T_CloudTxEvtHandlerTbl g_tCloudTxEvtHandlerTbl[] = 
{
    {CLOUD_EVT_TYPE_INIT,                   Cloud_InitHandler},
    {CLOUD_EVT_TYPE_ESTABLISH,              Cloud_EstablishHandler},
    {CLOUD_EVT_TYPE_DISCONNECT,             Cloud_DisconnectHandler},
    {CLOUD_EVT_TYPE_TIMEOUT,                Cloud_TimeoutHandler},
    {CLOUD_EVT_TYPE_BINDING,                Cloud_BindingHandler},
    {CLOUD_EVT_TYPE_KEEP_ALIVE,             Cloud_KeepAliveHandler},
    {CLOUD_EVT_TYPE_ACK,                    Cloud_AckHandler},
    {CLOUD_EVT_TYPE_POST,                   Cloud_PostHandler},
#if (CLOUD_TX_DATA_BACKUP_ENABLED == 1)
    {CLOUD_EVT_TYPE_POST_BACKUP,            Cloud_PostBackupHandler},
#endif
};

osThreadId g_tCloudTxTaskId;
osThreadId g_tCloudRxTaskId;
osMessageQId g_tCloudTxQueueId;
osTimerId g_tSwResetTimer;

EventGroupHandle_t g_tCloudEventGroup;

// Sec 7: declaration of static function prototype

void Cloud_TxTaskHandler(void *args);
void Cloud_TxTaskInit(void);
void Cloud_RxTaskHandler(void *args);
void Cloud_RxTaskInit(void);

#if (CLOUD_TX_DATA_BACKUP_ENABLED == 1)
void Cloud_RingBufInit(T_CloudRingBuf *ptCloudRingBuf, uint8_t u8QueueMaxCount);
#endif /* CLOUD_TX_DATA_BACKUP_ENABLED */

/***********
C Functions
***********/
// Sec 8: C Functions

/*************************************************************************
* FUNCTION:
*   Cloud_SwResetTimeoutCallback
*
* DESCRIPTION:
*   sw reset timer timeout callback (trigger system reboot)
*
* PARAMETERS
*   argu :          [IN] argument
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_SwResetTimeoutCallback(void const *argu)
{
    tracer_drct_printf("SW RESET!!!\r\n");
    Hal_Sys_SwResetAll();
}

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
void Cloud_OnlineStatusSet(bool blOnline)
{
    EG_StatusSet(g_tCloudEventGroup, CLOUD_EG_BIT_ONLINE, (uint8_t)blOnline);
}

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
bool Cloud_OnlineStatusGet(void)
{
    return (bool)EG_StatusGet(g_tCloudEventGroup, CLOUD_EG_BIT_ONLINE);
}

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
T_OplErr Cloud_OnlineStatusWait(void)
{
    return EG_StatusWait(g_tCloudEventGroup, CLOUD_EG_BIT_ONLINE, 0xFFFFFFFF);
}

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
T_OplErr Cloud_MsgSend(uint32_t u32EventId, uint8_t *pu8Data, uint32_t u32DataLen)
{
    if(NULL == g_tCloudTxQueueId)
    {
        OPL_LOG_WARN(CLOUD, "Queue not init");

        return OPL_ERR_RTOS_QMSG_NOT_INIT;
    }

    if(NULL == g_tCloudTxTaskId)
    {
        OPL_LOG_WARN(CLOUD, "Task not init");
        
        return OPL_ERR_RTOS_TASK_NOT_INIT;
    }

    T_CloudDataMsg *ptCloudDataMsg = (T_CloudDataMsg *)malloc(sizeof(T_CloudDataMsg) + u32DataLen);

    if(NULL == ptCloudDataMsg)
    {
        OPL_LOG_ERRO(CLOUD, "Alloc Tx message fail");

        return OPL_ERR_ALLOC_MEMORY_FAIL;
    }

    // prepare the data
    ptCloudDataMsg->u32EventId = u32EventId;
    ptCloudDataMsg->u32DataLen = u32DataLen;
    if(0 != ptCloudDataMsg->u32DataLen)
    {
        memcpy(ptCloudDataMsg->u8aData, pu8Data, u32DataLen);
    }

    // send message
    if(osOK != osMessagePut(g_tCloudTxQueueId, (uint32_t)ptCloudDataMsg, 0))
    {
        OPL_LOG_ERRO(CLOUD, "Send message fail");

        free(ptCloudDataMsg);

        return OPL_ERR_RTOS_SEND_QMSG_FAIL;
    }

    return OPL_OK;
}

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
void Cloud_Init(void)
{
    // create event group
    EG_Create(&g_tCloudEventGroup);

    // init tx task
    Cloud_TxTaskInit();

    // init rx task
    Cloud_RxTaskInit();

    // init rb
#if (CLOUD_TX_DATA_BACKUP_ENABLED == 1)
    Cloud_BackupRingBufInit();
#endif

    // init ota http task
#if (CLOUD_OTA_ENABLED == 1)
    Cloud_OtaTaskInit();
#endif

    // trigger init handler
    Cloud_MsgSend(CLOUD_EVT_TYPE_INIT, NULL, 0);
}

/*************************************************************************
* FUNCTION:
*   Cloud_TxTaskHandler
*
* DESCRIPTION:
*   handler of tx task
*
* PARAMETERS
*   args :          [IN] arguments
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_TxTaskHandler(void *args)
{
    osEvent tEvent;
    T_CloudDataMsg *ptCloudDataMsg;

    for(;;)
    {
        // wait event
        tEvent = osMessageGet(g_tCloudTxQueueId, osWaitForever);

        if(tEvent.status == osEventMessage)
        {
            ptCloudDataMsg = (T_CloudDataMsg *)tEvent.value.p;

            // start system reset timer
            osTimerStop(g_tSwResetTimer);
            osTimerStart(g_tSwResetTimer, SW_RESET_TIME);

            uint32_t i = 0;
            while(g_tCloudTxEvtHandlerTbl[i].u32EventId != 0xFFFFFFFF)
            {
                // matched
                if(g_tCloudTxEvtHandlerTbl[i].u32EventId == ptCloudDataMsg->u32EventId)
                {
                    g_tCloudTxEvtHandlerTbl[i].fpFunc(ptCloudDataMsg->u32EventId, ptCloudDataMsg->u8aData, ptCloudDataMsg->u32DataLen);
                    break;
                }

                i ++;
            }

            // not match
            if(g_tCloudTxEvtHandlerTbl[i].u32EventId == 0xFFFFFFFF)
            {
                OPL_LOG_WARN(CLOUD, "can't find event in event table");
            }

            if(NULL != ptCloudDataMsg)
            {
                free(ptCloudDataMsg);
            }

            // stop system reset timer
            osTimerStop(g_tSwResetTimer);
        }
    }
}

/*************************************************************************
* FUNCTION:
*   Cloud_TxTaskInit
*
* DESCRIPTION:
*   tx task initiate function
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_TxTaskInit(void)
{
    osThreadDef_t task_def;
    osMessageQDef_t queue_def;
    osTimerDef_t timer_def;

    // create sw reset timer
    timer_def.ptimer = Cloud_SwResetTimeoutCallback;

    g_tSwResetTimer = osTimerCreate(&timer_def, osTimerOnce, NULL);

    if(NULL == g_tSwResetTimer)
    {
        OPL_LOG_ERRO(CLOUD, "SW reset timer create fail");
    }

    // create queue
    queue_def.item_sz = sizeof(T_CloudDataMsg);
    queue_def.queue_sz = CLOUD_TX_MSG_QUEUE_SIZE;

    g_tCloudTxQueueId = osMessageCreate(&queue_def, NULL);

    if(NULL == g_tCloudTxQueueId)
    {
        OPL_LOG_ERRO(CLOUD, "TX queue create fail");
    }

    // create task
    task_def.name = "Cloud TX";
    task_def.stacksize = CLOUD_TX_TASK_STACK_SIZE;
    task_def.tpriority = CLOUD_TX_TASK_PRIORITY;
    task_def.pthread = Cloud_TxTaskHandler;

    g_tCloudTxTaskId = osThreadCreate(&task_def, (void *)NULL);

    if(NULL == g_tCloudTxTaskId)
    {
        OPL_LOG_ERRO(CLOUD, "TX task create fail");
    }

    OPL_LOG_INFO(CLOUD, "TX task create ok");
}

/*************************************************************************
* FUNCTION:
*   Cloud_RxTaskHandler
*
* DESCRIPTION:
*   handler of rx task
*
* PARAMETERS
*   args :          [IN] arguments
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_RxTaskHandler(void *args)
{
    for(;;)
    {
        if(OPL_OK == Cloud_OnlineStatusWait())
        {
            // receive
            Cloud_ReceiveHandler();
        }
    }
}

/*************************************************************************
* FUNCTION:
*   Cloud_RxTaskInit
*
* DESCRIPTION:
*   rx task initiate function
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_RxTaskInit(void)
{
    osThreadDef_t task_def;

    // create task
    task_def.name = "Cloud RX";
    task_def.stacksize = CLOUD_RX_TASK_STACK_SIZE;
    task_def.tpriority = CLOUD_RX_TASK_PRIORITY;
    task_def.pthread = Cloud_RxTaskHandler;

    g_tCloudRxTaskId = osThreadCreate(&task_def, (void *)NULL);

    if(NULL == g_tCloudRxTaskId)
    {
        OPL_LOG_ERRO(CLOUD, "RX task create fail");
    }

    OPL_LOG_INFO(CLOUD, "RX task create ok");

}

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
T_OplErr Cloud_RingBufPush(T_CloudRingBuf *ptCloudRingBuf, T_CloudRingBufData *ptCloudRingBufData)
{
    T_OplErr tEvtRst = OPL_ERR;

    uint32_t u32WriteNext;

    osSemaphoreWait(ptCloudRingBuf->tRbSemaphoreId, osWaitForever);

    if (ptCloudRingBufData == NULL)
    {
        goto done;
    }

    // full, ulWriteIdx + 1 == ulReadIdx
    u32WriteNext = (ptCloudRingBuf->u32WriteIdx + 1) % ptCloudRingBuf->u32QueueMaxCount;

    // Read index always prior to write index
    if (u32WriteNext == ptCloudRingBuf->u32ReadIdx)
    {
        // discard the oldest data, and read index move forware one step.
        free(ptCloudRingBuf->tCloudRingBufData[ptCloudRingBuf->u32ReadIdx].pu8Data);
        ptCloudRingBuf->u32ReadIdx = (ptCloudRingBuf->u32ReadIdx + 1) % ptCloudRingBuf->u32QueueMaxCount;
    }

    // update the temperature data to write index
	memcpy(&(ptCloudRingBuf->tCloudRingBufData[ptCloudRingBuf->u32WriteIdx]), ptCloudRingBufData, sizeof(T_CloudRingBufData));

    ptCloudRingBuf->u32WriteIdx = u32WriteNext;
    ptCloudRingBuf->u16QueueCount++;

    tEvtRst = OPL_OK;

done:
    osSemaphoreRelease(ptCloudRingBuf->tRbSemaphoreId);

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
*   ptCloudRingBuf :
*                   [IN] ring buffer configure
*   ptCloudRingBufData :
*                   [OUT] ring buffer data
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Cloud_RingBufPop(T_CloudRingBuf *ptCloudRingBuf, T_CloudRingBufData *ptCloudRingBufData)
{
    T_OplErr tEvtRst = OPL_ERR;

    osSemaphoreWait(ptCloudRingBuf->tRbSemaphoreId, osWaitForever);

    if (ptCloudRingBufData == NULL)
    {
        goto done;
    }

    // empty, ulWriteIdx == ulReadIdx
    if (ptCloudRingBuf->u32WriteIdx == ptCloudRingBuf->u32ReadIdx)
    {
        goto done;
    }

	memcpy(ptCloudRingBufData, &(ptCloudRingBuf->tCloudRingBufData[ptCloudRingBuf->u32ReadIdx]), sizeof(T_CloudRingBufData));

    tEvtRst = OPL_OK;

done:
    osSemaphoreRelease(ptCloudRingBuf->tRbSemaphoreId);

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
*   ptCloudRingBuf :
*                   [IN] ring buffer configure
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_RingBufReset(T_CloudRingBuf *ptCloudRingBuf)
{
    T_CloudRingBufData ptCloudRingBufData;

    while (false == Cloud_RingBufCheckEmpty(ptCloudRingBuf))
    {
        Cloud_RingBufPop(ptCloudRingBuf, &ptCloudRingBufData);
        Cloud_RingBufReadIdxUpdate(ptCloudRingBuf);

        if(ptCloudRingBufData.pu8Data != NULL)
        {
            free(ptCloudRingBufData.pu8Data);
        }
    }

    ptCloudRingBuf->u16QueueCount = 0;
}

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
void Cloud_RingBufReadIdxUpdate(T_CloudRingBuf *ptCloudRingBuf)
{
    osSemaphoreWait(ptCloudRingBuf->tRbSemaphoreId, osWaitForever);

    ptCloudRingBuf->u32ReadIdx = (ptCloudRingBuf->u32ReadIdx + 1) % ptCloudRingBuf->u32QueueMaxCount;
    ptCloudRingBuf->u16QueueCount --;

    osSemaphoreRelease(ptCloudRingBuf->tRbSemaphoreId);
}

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
bool Cloud_RingBufCheckEmpty(T_CloudRingBuf *ptCloudRingBuf)
{
    bool blRet = false;

    osSemaphoreWait(ptCloudRingBuf->tRbSemaphoreId, osWaitForever);

    // empty, ulWriteIdx == ulReadIdx
    if (ptCloudRingBuf->u32WriteIdx == ptCloudRingBuf->u32ReadIdx)
    {
        blRet = true;
    }

    osSemaphoreRelease(ptCloudRingBuf->tRbSemaphoreId);

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
*   ptCloudRingBuf :
*                   [IN] ring buffer configure
*
* RETURNS
*   bool :          [OUT] true -> ring buffer is full
*                         false -> ring buffer not full
*
*************************************************************************/
bool Cloud_RingBufCheckFull(T_CloudRingBuf *ptCloudRingBuf)
{
    bool blRet = false;
    uint32_t u32WriteNext;

    osSemaphoreWait(ptCloudRingBuf->tRbSemaphoreId, osWaitForever);

    // full, ulWriteIdx + 1 == ulReadIdx
    u32WriteNext = (ptCloudRingBuf->u32WriteIdx + 1) % ptCloudRingBuf->u32QueueMaxCount;

    // Read index always prior to write index
    if(u32WriteNext == ptCloudRingBuf->u32ReadIdx)
    {
        blRet = true;
    }

    osSemaphoreRelease(ptCloudRingBuf->tRbSemaphoreId);

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
*   ptCloudRingBuf :
*                   [IN] ring buffer configure
*   u8QueueCount :  [OUT] queue number of ring buffer
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_RingBufGetQueueCount(T_CloudRingBuf *ptCloudRingBuf, uint16_t *u16QueueCount)
{
    osSemaphoreWait(ptCloudRingBuf->tRbSemaphoreId, osWaitForever);

    *u16QueueCount = ptCloudRingBuf->u16QueueCount;

    osSemaphoreRelease(ptCloudRingBuf->tRbSemaphoreId);
}

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
void Cloud_RingBufInit(T_CloudRingBuf *ptCloudRingBuf, uint8_t u8QueueMaxCount)
{
    osSemaphoreDef_t tSemaphoreDef;

    // create semaphore
    tSemaphoreDef.dummy = 0;
    ptCloudRingBuf->tRbSemaphoreId = osSemaphoreCreate(&tSemaphoreDef, 1);

    if(NULL == ptCloudRingBuf->tRbSemaphoreId)
    {
        OPL_LOG_ERRO(CLOUD, "Create RB sempahore fail");
    }

    osSemaphoreWait(ptCloudRingBuf->tRbSemaphoreId, osWaitForever);

    ptCloudRingBuf->u16QueueCount = 0;
    ptCloudRingBuf->u32QueueMaxCount = u8QueueMaxCount;
    ptCloudRingBuf->u32ReadIdx = 0;
    ptCloudRingBuf->u32WriteIdx = 0;
    ptCloudRingBuf->tCloudRingBufData = (T_CloudRingBufData *)malloc(sizeof(T_CloudRingBufData) * u8QueueMaxCount);
 
    osSemaphoreRelease(ptCloudRingBuf->tRbSemaphoreId);

}
#endif /* CLOUD_Tx_DATA_BACKUP_ENABLED */
