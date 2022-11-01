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
*  cloud_ctrl.c
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
#include "cloud_kernel.h"
#include "wifi_agent_api.h"

#if (CLOUD_TX_DATA_BACKUP_ENABLED == 1)
#include "ring_buffer.h"
#endif /* CLOUD_TX_DATA_BACKUP_ENABLED */

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

// timer group
static osTimerId g_tCloudTimer[CLOUD_TMR_MAX] = {NULL};

// Sec 7: declaration of static function prototype

static void Cloud_TimeoutCallback(void const *argu);

/***********
C Functions
***********/
// Sec 8: C Functions

//////////////////////////////////// 
//// User created Functions
////////////////////////////////////

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
void Cloud_StatusCallbackRegister(T_CloudStatusCbFp tCloudStatusCbFp)
{
    g_tCloudStatusCbFp = tCloudStatusCbFp;
}

/*************************************************************************
* FUNCTION:
*   Cloud_StatusCallback
*
* DESCRIPTION:
*   calling cloud status callback function
*
* PARAMETERS
*   tCloudStatus :  [IN] cloud status type
*   pData :         [IN] data
*   u32DataLen :    [IN] data lens
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_StatusCallback(T_CloudStatus tCloudStatus, void *pData, uint32_t u32DataLen)
{
    if(NULL != g_tCloudStatusCbFp)
    {
        g_tCloudStatusCbFp(tCloudStatus, pData, u32DataLen);
    }
}

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
T_OplErr Cloud_TxTopicRegisterDyn(T_CloudTopicRegInfo *tCloudTopicRegInfo)
{
    T_OplErr tEvtRst = OPL_ERR;

    // user implement

    return tEvtRst;    
}

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
void Cloud_TxTopicRegisterSta(T_CloudTopicRegInfo *tCloudTopicRegInfo)
{
    // user implement
}

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
T_OplErr Cloud_TxTopicUnRegisterDyn(uint8_t u8TopicIndex)
{
    T_OplErr tEvtRst = OPL_ERR;

    // user implement
    
    return tEvtRst;
}

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
T_OplErr Cloud_RxTopicRegisterDyn(T_CloudTopicRegInfo *tCloudTopicRegInfo)
{
    T_OplErr tEvtRst = OPL_ERR;

    // user implement

    return tEvtRst;
}

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
void Cloud_RxTopicRegisterSta(T_CloudTopicRegInfo *tCloudTopicRegInfo)
{
    // user implement
}

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
T_OplErr Cloud_RxTopicUnRegisterDyn(uint8_t u8TopicIndex)
{
    T_OplErr tEvtRst = OPL_ERR;

    // user implement

    return tEvtRst;
}

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
T_CloudTopicRegInfoPtr Cloud_TxTopicGet(void)
{
    return g_tTxTopicTab;
}

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
T_CloudTopicRegInfoPtr Cloud_RxTopicGet(void)
{
    return g_tRxTopicTab;
}

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
void Cloud_TimerStart(T_CloudTimerIdx tTmrIdx, uint32_t u32TimeMs)
{
    if(CLOUD_TMR_MAX <= tTmrIdx)
    {
        // over index
        return;
    }

    osTimerStop(g_tCloudTimer[tTmrIdx]);
    osTimerStart(g_tCloudTimer[tTmrIdx], u32TimeMs);
}

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
void Cloud_TimerStop(T_CloudTimerIdx tTmrIdx)
{
    if(CLOUD_TMR_MAX <= tTmrIdx)
    {
        // over index
        return;
    }

    osTimerStop(g_tCloudTimer[tTmrIdx]);
}

//////////////////////////////////// 
//// Callback group
//////////////////////////////////// 

/*************************************************************************
* FUNCTION:
*   Cloud_TimeoutCallback
*
* DESCRIPTION:
*   timer timeout callback
*
* PARAMETERS
*   argu :          [IN] argument
*
* RETURNS
*   none
*
*************************************************************************/
static void Cloud_TimeoutCallback(void const *argu)
{
    T_CloudTimerIdx tCloudTimerIdx = CLOUD_TMR_MAX;
    osTimerId tTmrId = (osTimerId)argu;
    tCloudTimerIdx = (T_CloudTimerIdx)((uint32_t)pvTimerGetTimerID(tTmrId));

    // send message
    Cloud_MsgSend(CLOUD_EVT_TYPE_TIMEOUT, (uint8_t *)&tCloudTimerIdx, sizeof(T_CloudTimerIdx));
}

//////////////////////////////////// 
//// Event handler group
////////////////////////////////////

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
void Cloud_InitHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    osTimerDef_t timer_def;

    // create timers
    timer_def.ptimer = Cloud_TimeoutCallback;
    for(uint32_t i = 0; i < CLOUD_TMR_MAX; i ++)
    {
        g_tCloudTimer[i] = osTimerCreate(&timer_def, osTimerOnce, (void *)i);
        
        if(NULL == g_tCloudTimer[i])
        {
            OPL_LOG_ERRO(CLOUD, "Create cloud timer [%d] fail", i);
        }
    }

    // user implement
}

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
void Cloud_EstablishHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    // user implement
    // 1. establish connection

    // 2. determine the connect status
    // if connect success - set connection status as online
        // Cloud_OnlineStatusSet(true);
    // if connect fail
        // error handle - call retry connection timer
        // osTimerStart(g_tCloudConnectRetryTimer, ConnRetryDuration);

    // 3. if connect success, start keep alive timer
    // Cloud_TimerStart(CLOUD_TMR_KEEP_ALIVE, CLOUD_KEEP_ALIVE_TIME);
}

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
void Cloud_DisconnectHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    // user implement
    // 1. close connection

    // 2. determine the disconnect result and change the connection status
    // if disconnect success - set connection status as offline
        // Cloud_OnlineStatusSet(false);
    // if disconnect fail
        // error handle

    // 3. if disconnect success, stop keep alive timer
    // Cloud_TimerStop(CLOUD_TMR_KEEP_ALIVE);
}

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
void Cloud_TimeoutHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    // user implement
    // 1. timer timeout handle
    T_CloudTimerIdx tCloudTimerIdx = *((T_CloudTimerIdx *)pData);

    // *
    switch(tCloudTimerIdx)
    {
        // case of timer id (refer to T_CloudTimerIdx)
        case CLOUD_TMR_CONN_RETRY:
            OPL_LOG_INFO(CLOUD, "connection retry timeout handle");
            break;

        case CLOUD_TMR_KEEP_ALIVE:
            // Cloud_MsgSend(CLOUD_EVT_TYPE_KEEP_ALIVE, NULL, 0);
            Cloud_KeepAliveHandler(u32EventId, pData, u32DataLen);
            break;

        case CLOUD_TMR_MAX:
        default:
            OPL_LOG_ERRO(CLOUD, "invalid timer index %d", tCloudTimerIdx);
            break;
    }
    // *
}

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
void Cloud_BindingHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    // check connection first
    if(false == Cloud_OnlineStatusGet())
    {   
        OPL_LOG_INFO(CLOUD, "Cloud disconnected");
    }

    // user implement
    // 1. binding process
}

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
void Cloud_KeepAliveHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    // check connection first
    if(false == Cloud_OnlineStatusGet())
    {   
        OPL_LOG_INFO(CLOUD, "Cloud disconnected");
    }

    // user implement
    // 1. post keep alive data

    // 2. restart keep alive timer
    // Cloud_TimerStart(CLOUD_TMR_KEEP_ALIVE, CLOUD_KEEP_ALIVE_TIME);
}

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
void Cloud_AckHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    // check connection first
    if(false == Cloud_OnlineStatusGet())
    {   
        OPL_LOG_INFO(CLOUD, "Cloud disconnected");
    }

    // user implement
    // 1. post ack data
}

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
void Cloud_PostHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    // check connection first
    if(false == Cloud_OnlineStatusGet())
    {   
        OPL_LOG_INFO(CLOUD, "Cloud disconnected");
    }

    // user implement
#if (CLOUD_TX_DATA_BACKUP_ENABLED == 1)
    // 1. create your own scenario to backup data by using RingBuf

    // 2. construct data for post (if required)
    // Cloud_DataConstruct(pData, u32DataLen);

    // 3. post data

    // 4. send event CLOUD_EVT_TYPE_POST_BACKUP if RingBuf not empty
#else
    // 1. construct income data for post (if required)
    // Cloud_DataConstruct(pData, u32DataLen);

    // 2. post data

#endif /* CLOUD_TX_DATA_BACKUP_ENABLED */
}

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
void Cloud_BackupRingBufInit(void)
{
    // user implement
    // 1. initialize ring buffer
    // RingBuf_Init(arg1, arg2);
}

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
void Cloud_PostBackupHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    // 1. create your own scenario to post backup data by using RingBuf

    // 2. construct data for post (if required)

    // 3. post data

    // 4. send event CLOUD_EVT_TYPE_POST_BACKUP again if RingBuf still not empty
}
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
void Cloud_ReceiveHandler(void)
{
    // user implement
    // 1. receive data from cloud

    // 2. determine the receive status
    // if received success - trigger data parser
        // Cloud_DataParser(pData, u32DataLen); //(marked to avoid compiler error)
    // if received fail
        // error handle

    // WARNING: IF DO NOTHING IN RECEIVE HANDLER, THE DELAY MUST EXIST
    // osDelay(1000);
}
