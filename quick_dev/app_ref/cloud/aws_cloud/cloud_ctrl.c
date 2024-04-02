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
#include "app_at_cmd.h"
// #include "aws_mqtt_helper.h"
#include "aws_mqtt_cert.h"
#include "ble_mngr_api.h"
#include "evt_group.h"
#include "cmsis_os.h"
#include "cloud_ctrl.h"
#include "cloud_kernel.h"
#include "wifi_mngr_api.h"
#include "transfer.h"

#if (CLOUD_TX_DATA_BACKUP_ENABLED == 1)
#include "ring_buffer.h"
#endif /* CLOUD_TX_DATA_BACKUP_ENABLED */

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#define CLOUD_MQTT_EG_BIT_RX_PROC                       (0x00000001U)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

struct NetworkContext
{
    MbedtlsOplContext_t *tMbedtlsOplContext;
    MbedtlsOplCredentials_t *tMbedtlsOplCredentials;
};

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable
uint8_t *g_pu8MqttRootCA = NULL;
uint8_t *g_pu8MqttCert = NULL;
uint8_t *g_pu8MqttPriKey = NULL;
T_CloudPayloadFmt *g_pMqttLastWill = NULL;
// Sec 5: declaration of global function prototype

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable
extern osSemaphoreId g_tYieldSemaphoreId;

// timer group
static osTimerId g_tCloudTimer[CLOUD_TMR_MAX] = {NULL};

// dtim id
static uint16_t g_u16CloudDtimId = 0;
// static uint16_t g_u16CloudDtimId[CLOUD_SKIP_DTIM_MAX] = {0};

// mqtt context
static MQTTContext_t g_tMqttContext = {0};

// network context
static NetworkContext_t g_tNetworkContext = {0};

// mbedtls context
static MbedtlsOplContext_t g_tMbedtlsOplContext = {0};

// mbedtls credentials
static MbedtlsOplCredentials_t g_tMbedtlsOplCredentials = {0};

// event group of mqtt control
EventGroupHandle_t g_tCloudMqttCtrl;

// global cloud status callback fp
T_CloudStatusCbFp g_tCloudStatusCbFp = NULL;

// global cloud configure information
static T_CloudConnInfo g_tCloudConnInfo = {0};
// T_CloudTopicRegInfo g_tTxTopicTab[CLOUD_TOPIC_NUMBER];
// T_CloudTopicRegInfo g_tRxTopicTab[CLOUD_TOPIC_NUMBER];

static uint16_t g_SubscribePacketAckIdentifier = 0U;
static uint16_t g_UnsubscribePacketAckIdentifier = 0U;
static uint16_t g_PublishPacketAckIdentifier = 0U;

uint32_t g_u32KeepAliveInterval = CLOUD_KEEP_ALIVE_TIME;
static uint8_t g_u8RegisteredTopicNumber = 0;
static bool g_blCloudTimerAndDtimInitFlag = false;

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
*   Cloud_MqttFileCheck
*
* DESCRIPTION:
*   check rootca, client cert and private key
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
T_OplErr Cloud_MqttFileCheck(void)
{
    if(NULL == g_pu8MqttRootCA || NULL == g_pu8MqttCert || NULL ==g_pu8MqttPriKey)
    {
        return OPL_ERR;
    }
    else
    {
        return OPL_OK;
    }
}

/*************************************************************************
* FUNCTION:
*   Cloud_MqttFileSet
*
* DESCRIPTION:
*   set rootca, client cert and private key
*
* PARAMETERS
*   u8FileType:     [IN] one of the three file types: root ca, client cert or private key
*   u8Data:         [IN] file buffer 
*   u32DataLen:     [IN] file len
*   
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Cloud_MqttFileSet(uint8_t u8FileType, uint8_t *u8Data, uint32_t u32DataLen)
{
    T_OplErr tRet = OPL_ERR;
    uint8_t **u8MqttFile = NULL;

    if(NULL == u8Data || 0 == u32DataLen)
    {
        return tRet;
    }

    switch(u8FileType)
    {
        case UPLOAD_FILE_TYPE_MQTT_ROOT_CA:
        {
            if(g_pu8MqttRootCA != NULL)
            {
                Cloud_MqttFileFree(g_pu8MqttRootCA);
            }

            u8MqttFile = &g_pu8MqttRootCA;
            
            break;
        }

        case UPLOAD_FILE_TYPE_MQTT_CLIENT_CERT:
        {
            if(g_pu8MqttCert != NULL)
            {
                Cloud_MqttFileFree(g_pu8MqttCert);
            }

            u8MqttFile = &g_pu8MqttCert;

            break;
        }

        case UPLOAD_FILE_TYPE_MQTT_PRIVATE_KEY:
        {
            if(g_pu8MqttPriKey != NULL)
            {
                Cloud_MqttFileFree(g_pu8MqttPriKey);
            }

            u8MqttFile = &g_pu8MqttPriKey;

            break;
        }

        default:
            break;
    }

    *u8MqttFile =(uint8_t *) malloc(u32DataLen);
        
    if(NULL == u8MqttFile)
    {
        return tRet;
    }

    memset(*u8MqttFile, 0, u32DataLen);
    memcpy(*u8MqttFile, u8Data, u32DataLen);

    tRet = OPL_OK;
    return tRet;
}

/*************************************************************************
* FUNCTION:
*   Cloud_MqttClientId_Set
*
* DESCRIPTION:
*   Set AWS MQTT ClientID
*
* PARAMETERS
*   u8ClientId:             [IN] AWS Client ID 
*   u16ClientIdLen:         [IN] AWS Client ID Len
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_MqttClientId_Set(uint8_t *u8ClientId, uint16_t u16ClientIdLen)
{
    AWS_MqttHelperClientIdSet(u8ClientId, u16ClientIdLen);
}

/*************************************************************************
* FUNCTION:
*   Cloud_MqttLastWill_Set
*
* DESCRIPTION:
*   LastWill Set
*
* PARAMETERS
*   u8Data:         [IN] file buffer 
*   u32DataLen:     [IN] file len
*   
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Cloud_MqttLastWill_Set(T_CloudPayloadFmt *tData)
{
    T_OplErr tRet = OPL_ERR;
    T_CloudPayloadFmt **tMqttLastWill = NULL;

    if(NULL == tData)
    {
        return tRet;
    }

    if(g_pMqttLastWill != NULL)
    {
        Cloud_MqttFileFree((uint8_t *)g_pMqttLastWill);
    }

    tMqttLastWill = &g_pMqttLastWill;

    *tMqttLastWill =(T_CloudPayloadFmt *) malloc(sizeof(T_CloudPayloadFmt));
        
    if(NULL == tMqttLastWill)
    {
        return tRet;
    }

    memset(*tMqttLastWill, 0, sizeof(T_CloudPayloadFmt));
    memcpy(*tMqttLastWill, tData, sizeof(T_CloudPayloadFmt));

    tRet = OPL_OK;
    return tRet;

}

/*************************************************************************
* FUNCTION:
*   Cloud_MqttFileFree
*
* DESCRIPTION:
*   Free rootca, client cert and private key 
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_MqttFileFree(uint8_t *u8Data)
{
    free(u8Data);
    u8Data = NULL;
}

/*************************************************************************
* FUNCTION:
*   Cloud_MqttSkipDtimInit
*
* DESCRIPTION:
*   initiate mqtt module index to skip dtim controller
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_MqttSkipDtimInit(void)
{
    if(OPL_OK != Opl_Wifi_Skip_Dtim_Module_Reg(&g_u16CloudDtimId))
    {
        OPL_LOG_ERRO(CLOUD, "Initiate skip DTIM fail");
    }
}

/*************************************************************************
* FUNCTION:
*   Cloud_MqttSkipDtimSet
*
* DESCRIPTION:
*   set enable/disable skip dtim of mqtt module
*
* PARAMETERS
*   u8Enable :      [IN] true -> skip DTIM
*                        false -> no skip DTIM
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_MqttSkipDtimSet(uint8_t u8Enable, uint32_t u32Delay)
{
    T_OplErr ret = OPL_ERR;

    // stop delay to enable skip dtim timer, due to each sets
    // of skip dtim need to avoid the unexpected enable skip
    // dtim from delay time timeout
    Cloud_TimerStop(CLOUD_TMR_DELAY_TO_EN_SKIP_DTIM);

    if(true == u8Enable && u32Delay != 0)
    {
        OPL_LOG_DEBG(CLOUD, "delay %d to enable skip dtim", u32Delay);

        Cloud_TimerStart(CLOUD_TMR_DELAY_TO_EN_SKIP_DTIM, u32Delay);

        return;
    }

    ret = Opl_Wifi_Skip_Dtim_Set(g_u16CloudDtimId, u8Enable);

    OPL_LOG_DEBG(CLOUD, "%s skip DTIM %s", (u8Enable) ? "Enable" : "Disable",
                                           (OPL_OK == ret) ? "success" : "fail");
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

/*************************************************************************
* FUNCTION:
*   Cloud_TimerStopAll
*
* DESCRIPTION:
*   stop all timer function
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_TimerStopAll(void)
{
    uint8_t u8Count = 0;

    for(; u8Count < CLOUD_TMR_MAX; u8Count ++)
    {
        osTimerStop(g_tCloudTimer[u8Count]);
    }
}

/*************************************************************************
* FUNCTION:
*   Cloud_KeepAliveIntervalSet
*
* DESCRIPTION:
*   set keep alive interval time (won't take effect if cloud in online)
*
* PARAMETERS
*   u32KeepAliveInterval :
*                   [IN] keep alive interval time
*
* RETURNS
*   none
*
*************************************************************************/
T_OplErr Cloud_KeepAliveIntervalSet(uint32_t u32KeepAliveInterval)
{
    // check keep alive range
    if(CLOUD_KEEP_ALIVE_TIME_MAX < u32KeepAliveInterval || u32KeepAliveInterval < CLOUD_KEEP_ALIVE_TIME_MIN)
    {
        return OPL_ERR;
    }

    if(false == Cloud_OnlineStatusGet())
    {
        g_u32KeepAliveInterval = u32KeepAliveInterval;
        return OPL_OK;
    }

    return OPL_ERR;
}

/*************************************************************************
* FUNCTION:
*   Cloud_AutoConnectFlagGet
*
* DESCRIPTION:
*   get cloud auto-connect flag
*
* PARAMETERS
*   none
*
* RETURNS
*   bool :          [OUT] return flag status of cloud auto-connect flag
*
*************************************************************************/
bool Cloud_AutoConnectFlagGet(void)
{
    return (bool)g_tCloudConnInfo.u8AutoConn;
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
static void Cloud_MqttEventCallback(MQTTContext_t *ptMqttContext,
                                    MQTTPacketInfo_t *ptPacketInfo,
                                    MQTTDeserializedInfo_t *ptDeserializeInfo)
{
    // handle an incoming publish. The lower 4 bits of the publish packet
    // type is used for the dup, QoS, and retain flags. Hence masking
    // out the lower bits to check if the packet is publish
    if((ptPacketInfo->type & 0xF0U) == MQTT_PACKET_TYPE_PUBLISH)
    {
        // handle incoming publish
        OPL_LOG_INFO(CLOUD, "Received PUBLISH on %.*s topic", ptDeserializeInfo->pPublishInfo->topicNameLength, ptDeserializeInfo->pPublishInfo->pTopicName);
        OPL_LOG_INFO(CLOUD, "Payload: %.*s (%d)", ptDeserializeInfo->pPublishInfo->payloadLength, ptDeserializeInfo->pPublishInfo->pPayload, ptDeserializeInfo->pPublishInfo->payloadLength);

        T_CloudPayloadFmt tCloudPayloadFmt = {0};
        memcpy(&tCloudPayloadFmt.u8TopicName, ptDeserializeInfo->pPublishInfo->pTopicName, ptDeserializeInfo->pPublishInfo->topicNameLength);
        memcpy(&tCloudPayloadFmt.u8aPayloadBuf, ptDeserializeInfo->pPublishInfo->pPayload, ptDeserializeInfo->pPublishInfo->payloadLength);
        tCloudPayloadFmt.u16TopicNameLen = ptDeserializeInfo->pPublishInfo->topicNameLength;
        tCloudPayloadFmt.u16PayloadLen = ptDeserializeInfo->pPublishInfo->payloadLength;

        // dispatch received data to application
        Cloud_StatusCallback(CLOUD_CB_STA_DATA_RECV, &tCloudPayloadFmt, sizeof(T_CloudPayloadFmt));
    }
    else
    {
        // handle other packets
        switch(ptPacketInfo->type)
        {
            case MQTT_PACKET_TYPE_SUBACK:
            {
                OPL_LOG_INFO(CLOUD, "Received SUBACK");

                // update the global ACK packet identifier
                if(g_SubscribePacketAckIdentifier == ptDeserializeInfo->packetIdentifier)
                {
                    g_SubscribePacketAckIdentifier = 0;

                    // plus registered topic number since we received the sub ack
                    g_u8RegisteredTopicNumber = g_u8RegisteredTopicNumber + 1;
                    
                    // stop wait subscribe ack timer
                    Cloud_TimerStop(CLOUD_TMR_SUBSCRIBE_TIMEOUT);

                    // enable skip dtim with delay time
                    Cloud_MqttSkipDtimSet(true, CLOUD_DELAY_TO_EN_SKIP_DTIM_TIME);

                    // notify application that the subscribe activity done
                    Cloud_StatusCallback(CLOUD_CB_STA_SUB_DONE, NULL, 0);
                }

                break;
            }

            case MQTT_PACKET_TYPE_UNSUBACK:
            {
                OPL_LOG_INFO(CLOUD, "Received UNSUBACK");

                // update the global ACK packet identifier
                if(g_UnsubscribePacketAckIdentifier == ptDeserializeInfo->packetIdentifier)
                {
                    g_UnsubscribePacketAckIdentifier = 0;

                    // minus registered topic number since we received the unsub ack
                    if(0 != g_u8RegisteredTopicNumber)
                    {
                        g_u8RegisteredTopicNumber = g_u8RegisteredTopicNumber - 1;
                    }

                    // stop wait unsubscribe ack timer
                    Cloud_TimerStop(CLOUD_TMR_UNSUBSCRIBE_TIMEOUT);

                    // enable skip dtim with delay time
                    Cloud_MqttSkipDtimSet(true, CLOUD_DELAY_TO_EN_SKIP_DTIM_TIME);

                    // notify application that the unsubscribe activity done
                    Cloud_StatusCallback(CLOUD_CB_STA_UNSUB_DONE, NULL, 0);
                }

                break;
            }

            case MQTT_PACKET_TYPE_PINGRESP:
            {
                OPL_LOG_INFO(CLOUD, "Received PINGRESP");

                // stop wait keep alive ack timer
                Cloud_TimerStop(CLOUD_TMR_KEEP_ALIVE_TIMEOUT);

                // enable skip dtim with delay time
                Cloud_MqttSkipDtimSet(true, CLOUD_DELAY_TO_EN_SKIP_DTIM_TIME);

                // start next keep alive period
                Cloud_TimerStart(CLOUD_TMR_KEEP_ALIVE, g_u32KeepAliveInterval);

                break;
            }

            case MQTT_PACKET_TYPE_PUBACK:
            {
                OPL_LOG_INFO(CLOUD, "Received PUBACK");

                // update the global ACK packet identifier
                if(g_PublishPacketAckIdentifier == ptDeserializeInfo->packetIdentifier)
                {
                    g_PublishPacketAckIdentifier = 0;

                    // stop wait publish ack timer
                    Cloud_TimerStop(CLOUD_TMR_PUBLISH_TIMEOUT);

                    // enable skip dtim with delay time
                    Cloud_MqttSkipDtimSet(true, CLOUD_DELAY_TO_EN_SKIP_DTIM_TIME);

                    // notify application that the publish activity done
                    Cloud_StatusCallback(CLOUD_CB_STA_PUB_DONE, NULL, 0);
                }

                break;
            }

            // any other packet type is invalid
            default:
                OPL_LOG_ERRO(CLOUD, "Unknown packet type received: (%02x)", ptPacketInfo->type);
        }
    }
}

/*************************************************************************
* FUNCTION:
*   Cloud_MqttUnsolicitedDisconnectHandle
*
* DESCRIPTION:
*   handle unsolicited disconnect event
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static void Cloud_MqttUnsolicitedDisconnectHandle(void)
{
    if(true == Cloud_OnlineStatusGet())
    {
        // set flag as offline
        Cloud_OnlineStatusSet(false);

        Cloud_MqttSkipDtimSet(false, 0);

        AWS_MqttHelperDisconnectSession(&g_tMqttContext, &g_tNetworkContext);

        Cloud_MqttSkipDtimSet(true, 0);

        OPL_LOG_INFO(CLOUD, "MQTT un-soli disconnect");

        // restore each packet ack identifier
        g_SubscribePacketAckIdentifier = 0U;
        g_UnsubscribePacketAckIdentifier = 0U;
        g_PublishPacketAckIdentifier = 0U;

        // restore registered topic number
        g_u8RegisteredTopicNumber = 0;

        // stop all timer
        Cloud_TimerStopAll();

        // notify to app of disconnect result
        Cloud_StatusCallback(CLOUD_CB_STA_DISCONN, NULL, 0);

        // start retry timer if auto-reconnect sets
        if(true == g_tCloudConnInfo.u8AutoConn && true == Cloud_NetworkStatusGet())
        {
            Cloud_TimerStart(CLOUD_TMR_CONN_RETRY, CLOUD_RECONN_TIME);
        }
    }
}

/*************************************************************************
* FUNCTION:
*   Cloud_MqttRetryEstablishHandle
*
* DESCRIPTION:
*   handle retry establish event
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static void Cloud_MqttRetryEstablishHandle(void)
{
    // check network is up or down
    if(false == Cloud_NetworkStatusGet())
    {
        OPL_LOG_WARN(CLOUD, "Network not up, cloud connect won't do");
        return;
    }

    if(true == Cloud_OnlineStatusGet())
    {
        OPL_LOG_WARN(CLOUD, "Broker connected already");
        return;
    }

    bool ret = false;

    osMemoryPoolPcbInfoDump();

    // disable skip dtim
    Cloud_MqttSkipDtimSet(false, 0);

    ret = AWS_MqttHelperEstablishSession( &g_tMqttContext,
                                          &g_tNetworkContext,
                                          g_tCloudConnInfo.u8aHostAddr,
                                          g_tCloudConnInfo.u16HostPort);

    OPL_LOG_INFO(CLOUD, "MQTT re-connect %s", (ret) ? "SUCCESS":"FAIL");

    // osMemoryPoolPcbInfoDump();

    // notify to app of establish result
    Cloud_StatusCallback((ret) ? CLOUD_CB_STA_CONN_DONE : CLOUD_CB_STA_CONN_FAIL, NULL, 0);

    if(ret == true && MQTTConnected == g_tMqttContext.connectStatus)
    {
        // enable skip dtim with delay time, since the establish session had done
        // for the waiting ack in blocking scenario, so directly calling enable
        // skip dtim with delay time here
        Cloud_MqttSkipDtimSet(true, CLOUD_DELAY_TO_EN_SKIP_DTIM_TIME);

        // start keep alive timer
        Cloud_TimerStart(CLOUD_TMR_KEEP_ALIVE, g_u32KeepAliveInterval);
        
        // set flag as online
        Cloud_OnlineStatusSet(true);
    }
    else
    {
        // set flag as offline
        Cloud_OnlineStatusSet(false);

        // enable skip dtim
        Cloud_MqttSkipDtimSet(true, 0);

        // start retry timer if auto-reconnect sets
        if(true == g_tCloudConnInfo.u8AutoConn)
        {
            Cloud_TimerStart(CLOUD_TMR_CONN_RETRY, CLOUD_RECONN_TIME);
        }
    }
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
    if(false == g_blCloudTimerAndDtimInitFlag)
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

        // register skip dtim identifier
        Cloud_MqttSkipDtimInit();

        g_blCloudTimerAndDtimInitFlag = true;
    }

    Cloud_MqttSkipDtimSet(true, 0);

    // initialize topics table
    // memset(&g_tTxTopicTab, 0, sizeof(g_tTxTopicTab));
    // memset(&g_tRxTopicTab, 0, sizeof(g_tRxTopicTab));

    // initialize connection info
    memset(&g_tCloudConnInfo, 0U, sizeof(T_CloudConnInfo));

    // initialize contexts
    memset(&g_tMqttContext, 0U, sizeof(MQTTContext_t));
    memset(&g_tNetworkContext, 0U, sizeof(NetworkContext_t));
    memset(&g_tMbedtlsOplContext, 0U, sizeof(MbedtlsOplContext_t));
    memset(&g_tMbedtlsOplCredentials, 0U, sizeof(MbedtlsOplCredentials_t));

    // setup network context
    g_tNetworkContext.tMbedtlsOplContext = &g_tMbedtlsOplContext;
    g_tNetworkContext.tMbedtlsOplCredentials = &g_tMbedtlsOplCredentials;

    OPL_LOG_INFO(CLOUD,"Client ID: %s", AWS_MqttHelperClientIdGet());
    bool ret = false;

    // initialize mqtt
    // TODO: the certificate can be filled by using Cloud_MqttFileSet function
    // or using macro directly in aws_mqtt_cert.h
    ret = AWS_MqttHelperInit( &g_tMqttContext,
                              &g_tNetworkContext,
                              (char *)g_pu8MqttRootCA,
                              (char *)g_pu8MqttCert,
                              (char *)g_pu8MqttPriKey,
                              Cloud_MqttEventCallback);

    OPL_LOG_INFO(CLOUD, "AWS MQTT init %s", (ret) ? "SUCCESS":"FAIL");
    
    // notify to app of init result
    Cloud_StatusCallback((ret) ? CLOUD_CB_STA_INIT_DONE : CLOUD_CB_STA_INIT_FAIL, NULL, 0);
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
    // check network is up or down
    if(false == Cloud_NetworkStatusGet())
    {
        AT_FAIL(ACK_TAG_CLOUD_MQTT_ESTAB_REQ, NULL, 0);
        OPL_LOG_WARN(CLOUD, "Network not up, cloud connect won't do");
        return;
    }

    if(true == Cloud_OnlineStatusGet())
    {
        AT_FAIL(ACK_TAG_CLOUD_MQTT_ESTAB_REQ, NULL, 0);
        OPL_LOG_WARN(CLOUD, "Broker connected already");
        return;
    }

    bool ret = false;

    // copy income into global connection info
    T_CloudConnInfo tCloudConnInfo = {0};
    memcpy(&tCloudConnInfo, pData, sizeof(T_CloudConnInfo));

    if(0 != tCloudConnInfo.u16HostPort && 0 != tCloudConnInfo.u8aHostAddr[0])
    {
        OPL_LOG_DEBG(CLOUD, "Copy to global conn info");
        memcpy(&g_tCloudConnInfo, &tCloudConnInfo, sizeof(T_CloudConnInfo));
    }

    //osMemoryPoolPcbInfoDump();

    // disable skip dtim
    Cloud_MqttSkipDtimSet(false, 0);

    OPL_LOG_INFO(CLOUD, "MQTT connect start");

    ret = AWS_MqttHelperEstablishSession( &g_tMqttContext,
                                          &g_tNetworkContext,
                                          g_tCloudConnInfo.u8aHostAddr,
                                          g_tCloudConnInfo.u16HostPort);

    OPL_LOG_INFO(CLOUD, "MQTT connect %s", (ret) ? "SUCCESS":"FAIL");

    // notify to app of establish result
    Cloud_StatusCallback((ret) ? CLOUD_CB_STA_CONN_DONE : CLOUD_CB_STA_CONN_FAIL, NULL, 0);

    if(ret == true && MQTTConnected == g_tMqttContext.connectStatus)
    {
        // enable skip dtim with delay time, since the establish session had done
        // for the waiting ack in blocking scenario, so directly calling enable
        // skip dtim with delay time here
        Cloud_MqttSkipDtimSet(true, CLOUD_DELAY_TO_EN_SKIP_DTIM_TIME);

        // start keep alive timer
        Cloud_TimerStart(CLOUD_TMR_KEEP_ALIVE, g_u32KeepAliveInterval);
        
        // set flag as online
        Cloud_OnlineStatusSet(true);
    }
    else
    {
        // set flag as offline
        Cloud_OnlineStatusSet(false);

        // enable skip dtim
        Cloud_MqttSkipDtimSet(true, 0);

        // start retry timer if auto-reconnect sets
        if(true == g_tCloudConnInfo.u8AutoConn)
        {
            Cloud_TimerStart(CLOUD_TMR_CONN_RETRY, CLOUD_RECONN_TIME);
        }
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
    if(true == Cloud_OnlineStatusGet())
    {
        // set flag as offline
        Cloud_OnlineStatusSet(false);

        Cloud_MqttSkipDtimSet(false, 0);

        osSemaphoreWait(g_tYieldSemaphoreId, osWaitForever);   // Prevent disconnect while yield
        AWS_MqttHelperDisconnectSession(&g_tMqttContext, &g_tNetworkContext);
        osSemaphoreRelease(g_tYieldSemaphoreId);

        Cloud_MqttSkipDtimSet(true, 0);

        OPL_LOG_INFO(CLOUD, "MQTT disconnect");

        // restore each packet ack identifier
        g_SubscribePacketAckIdentifier = 0U;
        g_UnsubscribePacketAckIdentifier = 0U;
        g_PublishPacketAckIdentifier = 0U;

        // restore auto-connect flag due to disconnect handler was triggered by user
        g_tCloudConnInfo.u8AutoConn = false;

        // restore registered topic number
        g_u8RegisteredTopicNumber = 0;

        // stop all timer
        Cloud_TimerStopAll();

        // notify to app of disconnect result
        Cloud_StatusCallback(CLOUD_CB_STA_DISCONN, NULL, 0);
    }
    else
    {
        AT_FAIL(ACK_TAG_CLOUD_MQTT_DISCON_REQ, NULL, 0);
        OPL_LOG_WARN(CLOUD, "Broker not yet connected");
    }
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
    T_CloudTimerIdx tCloudTimerIdx = *((T_CloudTimerIdx *)pData);

    switch(tCloudTimerIdx)
    {
        case CLOUD_TMR_CONN_RETRY:
        {
            // call cloud retry establish
            Cloud_MqttRetryEstablishHandle();

            break;
        }

        case CLOUD_TMR_KEEP_ALIVE:
        {
            Cloud_KeepAliveHandler(u32EventId, pData, u32DataLen);
            break;
        }

        case CLOUD_TMR_KEEP_ALIVE_TIMEOUT:
        {
            OPL_LOG_ERRO(CLOUD, "Waiting PINGRESP timeout");

            // trigger disconnect and process reconnect
            Cloud_MqttUnsolicitedDisconnectHandle();

            break;
        }

        case CLOUD_TMR_PUBLISH_TIMEOUT:
        {
            OPL_LOG_ERRO(CLOUD, "Waiting PUBACK timeout");

            // trigger disconnect and process reconnect
            // Cloud_MqttUnsolicitedDisconnectHandle();

            g_PublishPacketAckIdentifier = 0;

            // notify application that the publish fail
            Cloud_StatusCallback(CLOUD_CB_STA_PUB_FAIL, NULL, 0);

            break;
        }

        case CLOUD_TMR_SUBSCRIBE_TIMEOUT:
        {
            OPL_LOG_ERRO(CLOUD, "Waiting SUBACK timeout");
            
            // trigger disconnect and process reconnect
            Cloud_MqttUnsolicitedDisconnectHandle();

            // notify application that the subscrbie topic fail
            Cloud_StatusCallback(CLOUD_CB_STA_SUB_FAIL, NULL, 0);

            break;
        }
        case CLOUD_TMR_UNSUBSCRIBE_TIMEOUT:
        {
            OPL_LOG_ERRO(CLOUD, "Waiting UNSUBACK timeout");

            // trigger disconnect and process reconnect
            Cloud_MqttUnsolicitedDisconnectHandle();

            // notify application that the unsubscribe topic fail
            Cloud_StatusCallback(CLOUD_CB_STA_UNSUB_FAIL, NULL, 0);

            break;
        }

        case CLOUD_TMR_DELAY_TO_EN_SKIP_DTIM:
        {
            // enable skip dtim
            Cloud_MqttSkipDtimSet(true, 0);
        }

        case CLOUD_TMR_MAX:
        default:
            break;
    }
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
    // check network is up or down
    if(false == Cloud_NetworkStatusGet())
    {
        OPL_LOG_WARN(CLOUD, "Network not up, ignore keep alive");
        return;
    }

    // check connection
    if(false == Cloud_OnlineStatusGet())
    {
        OPL_LOG_WARN(CLOUD, "Cloud disconnected");
        return;
    }

    int32_t ret = 0;

    // disable skip dtim
    Cloud_MqttSkipDtimSet(false, 0);
    
    ret = AWS_MqttHelperPing(&g_tMqttContext);

    OPL_LOG_INFO(CLOUD, "send ping %s", (0 == ret) ? "SUCCESS":"FAIL");

    if(0 == ret)
    {
        // start wait keep alive ack timer
        Cloud_TimerStart(CLOUD_TMR_KEEP_ALIVE_TIMEOUT, CLOUD_WAIT_ACK_TIME);
    }
    else
    {
        // enable skip dtim
        Cloud_MqttSkipDtimSet(true, 0);
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
    // check connection
    if(false == Cloud_OnlineStatusGet())
    {
        AT_FAIL(ACK_TAG_CLOUD_MQTT_PUB_DATA_REQ, NULL, 0);
        OPL_LOG_WARN(CLOUD, "Cloud disconnected");
        return;
    }

    int32_t ret = 0;

    T_CloudPayloadFmt ptCloudPayloadFmt;
    memcpy(&ptCloudPayloadFmt, pData, sizeof(T_CloudPayloadFmt));

    MQTTPublishInfo_t ptMqttPublishInfo;
    memset(&ptMqttPublishInfo, 0, sizeof(MQTTPublishInfo_t));

    ptMqttPublishInfo.qos = (MQTTQoS_t)ptCloudPayloadFmt.u8QoS;
    ptMqttPublishInfo.pTopicName = ptCloudPayloadFmt.u8TopicName;
    ptMqttPublishInfo.topicNameLength = ptCloudPayloadFmt.u16TopicNameLen;
    ptMqttPublishInfo.pPayload = ptCloudPayloadFmt.u8aPayloadBuf;
    ptMqttPublishInfo.payloadLength = ptCloudPayloadFmt.u16PayloadLen;

    if(MQTTQoS0 == ptMqttPublishInfo.qos)
    {
        // disable skip dtim
        Cloud_MqttSkipDtimSet(false, 0);

        ret = AWS_MqttHelperPublishToTopic(&g_tMqttContext, &ptMqttPublishInfo, NULL);
    }
    else
    {
        if(0 != g_PublishPacketAckIdentifier)
        {
            OPL_LOG_WARN(CLOUD, "Last publish un-finished");

            // notify application that the publish activity fail
            Cloud_StatusCallback(CLOUD_CB_STA_PUB_FAIL, &ptCloudPayloadFmt, sizeof(T_CloudPayloadFmt));

            return;
        }

        // disable skip dtim
        Cloud_MqttSkipDtimSet(false, 0);

        ret = AWS_MqttHelperPublishToTopic(&g_tMqttContext, &ptMqttPublishInfo, &g_PublishPacketAckIdentifier);
    }

    OPL_LOG_INFO(CLOUD, "send publish %s", (0 == ret) ? "SUCCESS":"FAIL");

    if(0 == ret)
    {
        if(MQTTQoS0 != ptMqttPublishInfo.qos)
        {
            // start wait publish ack timer
            Cloud_TimerStart(CLOUD_TMR_PUBLISH_TIMEOUT, CLOUD_WAIT_ACK_TIME);
        }
        else
        {
            // enable skip dtim with delay time, since the QoS0 type no need to wait
            // PUBACK so directly calling enable skip DTIM with delay time here.
            Cloud_MqttSkipDtimSet(true, CLOUD_DELAY_TO_EN_SKIP_DTIM_TIME);
        }
    }
    else
    {
        if(MQTTQoS0 != ptMqttPublishInfo.qos)
        {
            // reset the flag while sending publish fail
            g_PublishPacketAckIdentifier = 0;
        }

        // enable skip dtim
        Cloud_MqttSkipDtimSet(true, 0);

        // notify application that the subscribe activity fail
        Cloud_StatusCallback(CLOUD_CB_STA_PUB_FAIL, &ptCloudPayloadFmt, sizeof(T_CloudPayloadFmt));
    }
}

/*************************************************************************
* FUNCTION:
*   Cloud_RegisterTopicHandler
*
* DESCRIPTION:
*   register topic to cloud (CLOUD_EVT_TYPE_REGIST_TOPIC event)
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
void Cloud_RegisterTopicHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    int32_t ret = 0;

    if(true != Cloud_OnlineStatusGet())
    {
        AT_FAIL(ACK_TAG_CLOUD_MQTT_SUB_TOPIC_REQ, NULL, 0);
        OPL_LOG_WARN(CLOUD, "Broker not yet connected");
        return;
    }

    T_CloudTopicRegInfo ptCloudTopicRegInfo;
    memcpy(&ptCloudTopicRegInfo, pData, sizeof(T_CloudTopicRegInfo));

    MQTTSubscribeInfo_t ptMqttSubscribeInfo;
    memset(&ptMqttSubscribeInfo, 0, sizeof(MQTTSubscribeInfo_t));

    // check registered topic number
    if(CLOUD_TOPIC_NUMBER < g_u8RegisteredTopicNumber + 1)
    {
        // notify application that the subscribe activity fail
        Cloud_StatusCallback(CLOUD_CB_STA_SUB_FAIL, &ptCloudTopicRegInfo, sizeof(T_CloudTopicRegInfo));
        return;
    }

    if(0 != g_SubscribePacketAckIdentifier)
    {
        OPL_LOG_WARN(CLOUD, "Last subscribe un-finished");

        // notify application that the subscribe activity fail
        Cloud_StatusCallback(CLOUD_CB_STA_SUB_FAIL, &ptCloudTopicRegInfo, sizeof(T_CloudTopicRegInfo));

        return;
    }

    ptMqttSubscribeInfo.qos = (MQTTQoS_t)ptCloudTopicRegInfo.u8QoS;
    ptMqttSubscribeInfo.pTopicFilter = ptCloudTopicRegInfo.u8TopicName;
    ptMqttSubscribeInfo.topicFilterLength = ptCloudTopicRegInfo.u16TopicNameLen;

    // disable skip dtim
    Cloud_MqttSkipDtimSet(false, 0);

    ret = AWS_MqttHelperSubscribeToTopic(&g_tMqttContext, &ptMqttSubscribeInfo, &g_SubscribePacketAckIdentifier);

    OPL_LOG_INFO(CLOUD, "subscribe topic %s", (0 == ret) ? "SUCCESS":"FAIL");

    if(0 == ret)
    {
        // start wait subscribe ack timer
        Cloud_TimerStart(CLOUD_TMR_SUBSCRIBE_TIMEOUT, CLOUD_WAIT_ACK_TIME);
    }
    else
    {
        // reset the flag while subscribe topic fail
        g_SubscribePacketAckIdentifier = 0;

        // enable skip dtim
        Cloud_MqttSkipDtimSet(true, 0);

        // notify application that the subscribe activity fail
        Cloud_StatusCallback(CLOUD_CB_STA_SUB_FAIL, &ptCloudTopicRegInfo, sizeof(T_CloudTopicRegInfo));
    }
}

/*************************************************************************
* FUNCTION:
*   Cloud_UnRegisterTopicHandler
*
* DESCRIPTION:
*   un-register topuc from cloud (CLOUD_EVT_TYPE_UNREGIST_TOPIC event)
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
void Cloud_UnRegisterTopicHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    int32_t ret = 0;

    if(true != Cloud_OnlineStatusGet())
    {
        AT_FAIL(ACK_TAG_CLOUD_MQTT_UNSUB_TOPIC_REQ, NULL, 0);
        OPL_LOG_WARN(CLOUD, "Broker not yet connected");
        return;
    }

    T_CloudTopicRegInfo ptCloudTopicRegInfo;
    memcpy(&ptCloudTopicRegInfo, pData, sizeof(T_CloudTopicRegInfo));

    MQTTSubscribeInfo_t ptMqttUnsubscribeInfo;
    memset(&ptMqttUnsubscribeInfo, 0, sizeof(MQTTSubscribeInfo_t));

    // check registered topic number
    if(0 == g_u8RegisteredTopicNumber)
    {
        // notify application that the subscribe activity fail
        Cloud_StatusCallback(CLOUD_CB_STA_UNSUB_FAIL, &ptCloudTopicRegInfo, sizeof(T_CloudTopicRegInfo));
        return;
    }

    if(0 != g_UnsubscribePacketAckIdentifier)
    {
        OPL_LOG_WARN(CLOUD, "Last unsubscribe un-finished");

        // notify application that the unsubscribe activity fail
        Cloud_StatusCallback(CLOUD_CB_STA_UNSUB_FAIL, &ptCloudTopicRegInfo, sizeof(T_CloudTopicRegInfo));

        return;
    }

    ptMqttUnsubscribeInfo.qos = (MQTTQoS_t)ptCloudTopicRegInfo.u8QoS;
    ptMqttUnsubscribeInfo.pTopicFilter = ptCloudTopicRegInfo.u8TopicName;
    ptMqttUnsubscribeInfo.topicFilterLength = ptCloudTopicRegInfo.u16TopicNameLen;

    // disable skip dtim
    Cloud_MqttSkipDtimSet(false, 0);

    ret = AWS_MqttHelperUnsubscribeFromTopic(&g_tMqttContext, &ptMqttUnsubscribeInfo, &g_UnsubscribePacketAckIdentifier);

    OPL_LOG_INFO(CLOUD, "un-subscribe topic %s", (0 == ret) ? "SUCCESS":"FAIL");

    if(0 == ret)
    {
        // start wait unsubscribe ack timer
        Cloud_TimerStart(CLOUD_TMR_UNSUBSCRIBE_TIMEOUT, CLOUD_WAIT_ACK_TIME);
    }
    else
    {
        // reset flag while unsubscribe topic fail
        g_UnsubscribePacketAckIdentifier = 0;

        // enable skip dtim
        Cloud_MqttSkipDtimSet(true, 0);

        // notify application that the unsubscribe activity fail
        Cloud_StatusCallback(CLOUD_CB_STA_UNSUB_FAIL, &ptMqttUnsubscribeInfo, sizeof(MQTTSubscribeInfo_t));
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
    if(true == Cloud_OnlineStatusGet())
    {
        MQTTStatus_t mqttStatus;

        // OPL_LOG_INFO(CLOUD, "-->Yield"); 

        osSemaphoreWait(g_tYieldSemaphoreId, osWaitForever);
        mqttStatus = AWS_MqttHelperProcessLoop(&g_tMqttContext);
        osSemaphoreRelease(g_tYieldSemaphoreId);
        
        // OPL_LOG_INFO(CLOUD, "<--Yield %s", MQTT_Status_strerror(mqttStatus)); 

        if(MQTTSuccess != mqttStatus)
        {
            OPL_LOG_INFO(CLOUD, "<--Yield %s", MQTT_Status_strerror(mqttStatus));

            // check status again, since might happen the offline during process loop
            if(true == Cloud_OnlineStatusGet())
            {
                // recv error, trigger unsolicited disconnect
                Cloud_MqttUnsolicitedDisconnectHandle();
            }

            osDelay(2000);
        }
        osDelay(10);
    }

    // if(OPL_OK == EG_StatusWait(g_tCloudMqttCtrl, CLOUD_MQTT_EG_BIT_RX_PROC, 0xFFFFFFFF))

    // WARNING: IF DO NOTHING IN RECEIVE HANDLER, THE DELAY MUST EXIST
    // osDelay(1000);
}
