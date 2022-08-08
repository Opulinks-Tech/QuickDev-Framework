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

#include "app_main.h"
#include "cmsis_os.h"
#include "cloud_data.h"
#include "cloud_ctrl.h"
#include "cloud_kernel.h"
#include "tcp_client.h"
#include "wifi_mngr_api.h"

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

osSemaphoreId g_tCloudSemaphoreId = NULL;

// timer group
static osTimerId g_tCloudTimer[CLOUD_TMR_MAX] = {NULL};

static int32_t g_i32TcpHdlId = -1;
static int32_t g_i32TcpTxId = -1;
static int32_t g_i32TcpRxId = -1;
static uintptr_t g_ptrCloudTcpHdlId = 0;

uint16_t g_u16CloudTcpSkipDtimId = 0;
uint32_t g_u32CloudKeepAliveDuration = CLOUD_KEEP_ALIVE_TIME;

// static uint32_t g_u32CloudKeepAliveCount = 0;

// global cloud configure information
T_CloudConnInfo g_tCloudConnInfo = 
{
    .u8AutoConn = false,
    .u8Security = 0,
    .u8aHostAddr = TCP_HOST_IP,
    .u16HostPort = TCP_HOST_PORT,
};
T_CloudTopicRegInfo g_tTxTopicTab[CLOUD_TOPIC_NUMBER];
T_CloudTopicRegInfo g_tRxTopicTab[CLOUD_TOPIC_NUMBER];

// Sec 7: declaration of static function prototype

static void Cloud_TimeoutCallback(void const *argu);
static void Cloud_PostData(uint8_t *pu8Data, uint32_t u32DataLen);

/***********
C Functions
***********/
// Sec 8: C Functions

//////////////////////////////////// 
//// User created Functions
////////////////////////////////////

T_OplErr Cloud_TxTopicRegister(T_CloudTopicRegInfo *tCloudTopicRegInfo, void *vCallback)
{
    T_OplErr tEvtRst = OPL_ERR;

    uint8_t u8TopicIndex = tCloudTopicRegInfo->u8TopicIndex;

    if(CLOUD_TOPIC_NUMBER <= u8TopicIndex)
    {
        return OPL_ERR_PARAM_INVALID;
    }

    // register topic to cloud
    // *
    // tcp no need the tx topic registeration
    // copy the tx topic register information to global registeration table
    tEvtRst = OPL_OK;
    // *

    if(OPL_OK == tEvtRst)
    {
        memcpy(&g_tTxTopicTab[u8TopicIndex], tCloudTopicRegInfo, sizeof(T_CloudTopicRegInfo));
        g_tTxTopicTab[u8TopicIndex].u8IsTopicRegisted = true;
    }

		return tEvtRst;
}

T_OplErr Cloud_RxTopicRegister(T_CloudTopicRegInfo *tCloudTopicRegInfo, void *vCallback)
{
    T_OplErr tEvtRst = OPL_ERR;

    uint8_t u8TopicIndex = tCloudTopicRegInfo->u8TopicIndex;

    if(CLOUD_TOPIC_NUMBER <= u8TopicIndex)
    {
        return OPL_ERR_PARAM_INVALID;
    }

    tracer_drct_printf("reg rx: %s (%d)\r\n", tCloudTopicRegInfo->u8aTopicName, strlen((char *)tCloudTopicRegInfo->u8aTopicName));

    memcpy(&g_tRxTopicTab[u8TopicIndex], tCloudTopicRegInfo, sizeof(T_CloudTopicRegInfo));

    // register topic to cloud
    // *
    // tcp no need the rx topic registeration
    // copy the rx topic register information to global registeration table
    tEvtRst = OPL_OK;
    // *

    if(OPL_OK == tEvtRst)
    {
        g_tRxTopicTab[u8TopicIndex].u8IsTopicRegisted = true;
    }
    else
    {
        memset(&g_tRxTopicTab[u8TopicIndex], 0, sizeof(T_CloudTopicRegInfo));
    }

    return tEvtRst;
}

T_CloudTopicRegInfoPtr Cloud_TxTopicGet(void)
{
    return g_tTxTopicTab;
}

T_CloudTopicRegInfoPtr Cloud_RxTopicGet(void)
{
    return g_tRxTopicTab;
}

/*************************************************************************
* FUNCTION:
*   Cloud_PostData
*
* DESCRIPTION:
*   post the data to cloud
*
* PARAMETERS
*   pu8Data :       [IN] post data
*   u32DataLen :    [IN] post data lens
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_PostData(uint8_t *pu8Data, uint32_t u32DataLen)
{
    int32_t i32Ret = 0;

    Opl_Wifi_Skip_Dtim_Set(g_u16CloudTcpSkipDtimId, false);

    osSemaphoreWait(g_tCloudSemaphoreId, osWaitForever);

    if(g_i32TcpTxId != g_i32TcpHdlId)
    {
        g_i32TcpTxId = g_i32TcpHdlId;
    }

    osSemaphoreRelease(g_tCloudSemaphoreId);

    i32Ret = TCP_Send(g_ptrCloudTcpHdlId, (const char *)pu8Data, u32DataLen, TCP_TX_POST_TIMEOUT);

    if(i32Ret <= 0)
    {
        OPL_LOG_ERRO(CLOUD, "post fail (ret %d)", i32Ret);

        osSemaphoreWait(g_tCloudSemaphoreId, osWaitForever);

        if(((uintptr_t)-1 != g_ptrCloudTcpHdlId) &&
            (g_i32TcpHdlId == g_i32TcpTxId) &&
            (true == Cloud_OnlineStatusGet()))
        {
            uint8_t u8ReConnect = true;
            Cloud_MsgSend(CLOUD_EVT_TYPE_DISCONNECT, &u8ReConnect, sizeof(u8ReConnect));

            g_ptrCloudTcpHdlId = (uintptr_t)-1;
            g_i32TcpHdlId = -1;
            g_i32TcpTxId = -1;
        }

        osSemaphoreRelease(g_tCloudSemaphoreId);
    }

    OPL_LOG_INFO(CLOUD, "post: %s (%d)", pu8Data, u32DataLen);
    
    Opl_Wifi_Skip_Dtim_Set(g_u16CloudTcpSkipDtimId, true);
}

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
void Cloud_KeepAliveDurationSet(uint32_t u32Duration)
{
    // check connection first
    if(false == Cloud_OnlineStatusGet())
    {   
        OPL_LOG_INFO(CLOUD, "cloud disconnected");
    }
    else
    {
        g_u32CloudKeepAliveDuration = u32Duration;

        Cloud_TimerStart(CLOUD_TMR_KEEP_ALIVE, g_u32CloudKeepAliveDuration);
    }
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
    osSemaphoreDef_t sema_def;

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

    // create semaphore
    sema_def.dummy = 0;
    g_tCloudSemaphoreId = osSemaphoreCreate(&sema_def, 1);
    if (g_tCloudSemaphoreId == NULL)
    {
        OPL_LOG_ERRO(CLOUD, "Create semaphore fail");
    }

    // register skip DTIM id
    if(OPL_OK != Opl_Wifi_Skip_Dtim_Module_Reg(&g_u16CloudTcpSkipDtimId))
    {
        OPL_LOG_ERRO(CLOUD, "Reg cloud tcp skip DTIM id fail");
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
    if(u32DataLen != 0)
    {
        memset(&g_tCloudConnInfo, 0, sizeof(T_CloudConnInfo));
        g_tCloudConnInfo = *((T_CloudConnInfo *)pData);
    }

    if(false == Cloud_OnlineStatusGet())
    {
        osSemaphoreWait(g_tCloudSemaphoreId, osWaitForever);

        Opl_Wifi_Skip_Dtim_Set(g_u16CloudTcpSkipDtimId, false);

        g_ptrCloudTcpHdlId = TCP_Establish((char *)g_tCloudConnInfo.u8aHostAddr, g_tCloudConnInfo.u16HostPort);

        // 2. determine the connect status
        if((uintptr_t)-1 == g_ptrCloudTcpHdlId)
        {
            osSemaphoreRelease(g_tCloudSemaphoreId);
            
            OPL_LOG_WARN(CLOUD, "tcp connect fail, retry connection..");

            Cloud_TimerStart(CLOUD_TMR_CONN_RETRY, 5000);
        }
        else
        {
            Cloud_OnlineStatusSet(true);

            g_i32TcpHdlId ++;
            g_i32TcpHdlId = g_i32TcpHdlId & (0xFF);

            osSemaphoreRelease(g_tCloudSemaphoreId);

            // notify to application
            APP_SendMessage(APP_EVT_CLOUD_CONNECT_IND, NULL, 0);

            Cloud_TimerStart(CLOUD_TMR_KEEP_ALIVE, g_u32CloudKeepAliveDuration);

            OPL_LOG_INFO(CLOUD, "tcp connect pass, start keep alive..");
        }

        Opl_Wifi_Skip_Dtim_Set(g_u16CloudTcpSkipDtimId, true);
    }
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
    uint8_t u8ReConnect = *((uint8_t *)pData);

    // user implement
    osSemaphoreWait(g_tCloudSemaphoreId, osWaitForever);

    Opl_Wifi_Skip_Dtim_Set(g_u16CloudTcpSkipDtimId, false);

    Cloud_TimerStop(CLOUD_TMR_CONN_RETRY);
    Cloud_TimerStop(CLOUD_TMR_KEEP_ALIVE);
    Cloud_TimerStop(CLOUD_TMR_DATA_POST);

    Cloud_OnlineStatusSet(false);

    // 1. close connection
    int32_t i32Ret = TCP_Disconnect(g_ptrCloudTcpHdlId);

    // 2. determine the disconnect result and change the connection status
    if(i32Ret < 0)
    {
        OPL_LOG_ERRO(CLOUD, "tcp disconnect fail (ret %d)", i32Ret);
    }
    else
    {
        OPL_LOG_INFO(CLOUD, "tcp disconnected");

        // notify to application
        APP_SendMessage(APP_EVT_CLOUD_DISCONNECT_IND, NULL, 0);
    }

    if(u8ReConnect == true)
    {
        Cloud_MsgSend(CLOUD_EVT_TYPE_ESTABLISH, NULL, 0);
    }

    Opl_Wifi_Skip_Dtim_Set(g_u16CloudTcpSkipDtimId, true);

    osSemaphoreRelease(g_tCloudSemaphoreId);
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
            Cloud_EstablishHandler(u32EventId, NULL, 0);
            break;

        case CLOUD_TMR_KEEP_ALIVE:
            Cloud_KeepAliveHandler(u32EventId, pData, u32DataLen);
            break;

        case CLOUD_TMR_DATA_POST:
        {
            uint8_t u8ReConnect = true;

            OPL_LOG_WARN(CLOUD, "post data timeout");

            Cloud_MsgSend(CLOUD_EVT_TYPE_DISCONNECT, &u8ReConnect, sizeof(u8ReConnect));
            break;
        }

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
        OPL_LOG_INFO(CLOUD, "cloud disconnected");
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
        OPL_LOG_INFO(CLOUD, "cloud disconnected");
    }
    else
    {
        // user implement
        // 1. post keep alive data
        uint8_t u8KeepAliveData[4] = "ping";
        uint8_t u8PostData[TCP_TX_BUF_SIZE] = {0};
        uint32_t u32PostDataLen = 0;

        // u32PostDataLen += sprintf((char *)u8PostData, "%s%d", u8KeepAliveData, g_u32CloudKeepAliveCount++);
        u32PostDataLen += sprintf((char *)u8PostData, "%s", u8KeepAliveData);

        Cloud_PostData(u8PostData, u32PostDataLen);

        Cloud_TimerStart(CLOUD_TMR_KEEP_ALIVE, g_u32CloudKeepAliveDuration);
    }
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
        OPL_LOG_INFO(CLOUD, "cloud disconnected");
    }
    else
    {
        // user implement

        // 1. construct income data for post
        uint8_t u8AckData[TCP_TX_BUF_SIZE] = {0};
        uint32_t u32AckDataLen = 0;
        
        Cloud_AckDataConstruct(pData, u32DataLen, u8AckData, &u32AckDataLen);

        // 2. post ack data
        Cloud_PostData(u8AckData, u32AckDataLen);
    }
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
        OPL_LOG_INFO(CLOUD, "cloud disconnected");
    }
    else
    {
        // user implement
#if (CLOUD_TX_DATA_BACKUP_ENABLED == 1)
        // 1. create your own scenario to backup data by using RingBuf (Cloud_RingBuf___)

        // 2. construct data for post (if required)

        // 3. post data

        // 4. send event CLOUD_EVT_TYPE_POST_BACKUP if RingBuf not empty
#else
        // 1. construct income data for post (if required)
        uint8_t u8PostData[TCP_TX_BUF_SIZE] = {0};
        uint32_t u32PostDataLen = 0;

        T_CloudPayloadFmt tCloudPayloadFmt = *((T_CloudPayloadFmt *)pData);

        Cloud_DataConstruct(tCloudPayloadFmt.u8aPayloadBuf, tCloudPayloadFmt.u32PayloadLen, u8PostData, &u32PostDataLen);

        // 2. post data
        Cloud_PostData(u8PostData, u32PostDataLen);

        // start data post timer
        // Cloud_TimerStart(CLOUD_TMR_DATA_POST, TCP_TX_POST_TIMEOUT);

#endif /* CLOUD_TX_DATA_BACKUP_ENABLED */
    }
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
    // 1. create your own scenario to post backup data by using RingBuf (Cloud_RingBuf___)

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
    int8_t i8Ret = -1;

    // user implement
    // 1. receive data from cloud
    char databuf[TCP_RX_BUF_SIZE] = {0};

    osSemaphoreWait(g_tCloudSemaphoreId, osWaitForever);

    if(g_i32TcpRxId != g_i32TcpHdlId)
    {
        g_i32TcpRxId = g_i32TcpHdlId;
    }

    osSemaphoreRelease(g_tCloudSemaphoreId);

    i8Ret = TCP_Recv(g_ptrCloudTcpHdlId, (char *)databuf, TCP_RX_BUF_SIZE, TCP_RX_RECV_TIMEOUT);
    
    // 2. determine the receive status
    if(i8Ret > 0)
    {
        OPL_LOG_INFO(CLOUD, "recv: %s (%d)", databuf, strlen(databuf));

        Cloud_DataParser((uint8_t *)databuf, strlen(databuf));

        APP_SendMessage(APP_EVT_CLOUD_RECV_IND, (uint8_t *)databuf, strlen(databuf));
    }
    else if(i8Ret < 0)
    {
        OPL_LOG_ERRO(CLOUD, "recv fail (ret %d)", i8Ret);

        osSemaphoreWait(g_tCloudSemaphoreId, osWaitForever);

        if(((uintptr_t)-1 != g_ptrCloudTcpHdlId) &&
            (g_i32TcpHdlId == g_i32TcpRxId) &&
            (true == Cloud_OnlineStatusGet()))
        {
            uint8_t u8ReConnect = true;
            Cloud_MsgSend(CLOUD_EVT_TYPE_DISCONNECT, &u8ReConnect, sizeof(u8ReConnect));

            g_ptrCloudTcpHdlId = (uintptr_t)-1;
            g_i32TcpHdlId = -1;
            g_i32TcpRxId = -1;
        }

        osSemaphoreRelease(g_tCloudSemaphoreId);
    }

    // WARNING: IF DO NOTHING IN RECEIVE HANDLER, THE DELAY MUST EXIST
    // osDelay(1000);
}
