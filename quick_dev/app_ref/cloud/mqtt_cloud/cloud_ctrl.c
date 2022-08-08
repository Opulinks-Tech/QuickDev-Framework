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
#include "etharp.h"
#include "evt_group.h"
#include "cmsis_os.h"
#include "cloud_ctrl.h"
#include "cloud_kernel.h"
#include "wifi_mngr_api.h"

#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "opl_aws_iot_config.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#define CLOUD_MQTT_EG_BIT_RX_PROC                       (0x00000001U)

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

static char g_cCloudMqttClientId[MQTT_CLIENT_ID_LEN] = {"\0"};

// mqtt client
AWS_IoT_Client client;

// event group of mqtt control
EventGroupHandle_t g_tCloudMqttCtrl;

// global cloud configure information
T_CloudConnInfo g_tCloudConnInfo = {0};
T_CloudTopicRegInfo g_tTxTopicTab[CLOUD_TOPIC_NUMBER];
T_CloudTopicRegInfo g_tRxTopicTab[CLOUD_TOPIC_NUMBER];

// dtim id
uint16_t g_u16CloudDtimId = 0;

// Sec 7: declaration of static function prototype

static void Cloud_TimeoutCallback(void const *argu);

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
    // mqtt no need the tx topic registeration
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
    // mqtt call subscribe topic to register rx topic

    // subscribe topic name should be a static buffer, otherwise the pointer will be kept update to new one
    Cloud_MqttSubscribeTopic((char *)g_tRxTopicTab[u8TopicIndex].u8aTopicName, strlen((char *)g_tRxTopicTab[u8TopicIndex].u8aTopicName), QOS1, (pApplicationHandler_t)vCallback);
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
*   Cloud_MqttSubscribeTopic
*
* DESCRIPTION:
*   subscribe topic
*
* PARAMETERS
*   cTopic :        [IN] topic name (must be a static in memory)
*   u16TopicLen :   [IN] topic name lens
*   tQoS :          [IN] QoS level
*   fpFunc :        [IN] callback function pointer of subscribed topic
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_MqttSubscribeTopic(char *cTopic, uint16_t u16TopicLen, QoS tQoS, pApplicationHandler_t fpFunc)
{
    IoT_Error_t rc = FAILURE;

    IOT_INFO("sub topic %s (%d) [QoS%d]", cTopic, u16TopicLen, tQoS);

    if(MAX_SIZE_OF_TOPIC_LENGTH < u16TopicLen)
    {
        IOT_ERROR("topic lens invalid");
        return;        
    }

    uint8_t retry = 0;

    // peding receive process
    Cloud_RxProcessGoingStateSet(false);

    while(retry < 10)
    {
        // disable skip dtim
        Cloud_MqttSkipDtimSet(false);

        // subscribe topic
        rc = aws_iot_mqtt_subscribe(&client, cTopic, u16TopicLen, tQoS, fpFunc, NULL);

        // enable skip dtim
        Cloud_MqttSkipDtimSet(true);

        if(SUCCESS != rc)
        {
            retry ++;
            IOT_ERROR("--- subscribe retry %d", retry);
            osDelay(500);
        }
        else
        {
            IOT_INFO("---> subscribe success");
            break;
        }
    }

    // activate receive process
    Cloud_RxProcessGoingStateSet(true);
}

/*************************************************************************
* FUNCTION:
*   Cloud_MqttUnsubscribeTopic
*
* DESCRIPTION:
*   un-subscribe topic
*
* PARAMETERS
*   cTopic :        [IN] topic name (must be a static in memory)
*   u16TopicLen :   [IN] topic name lens
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_MqttUnsubscribeTopic(char *cTopic, uint16_t u16TopicLen)
{
    IoT_Error_t rc = FAILURE;
    IOT_INFO("unsub topic %s (%d)", cTopic, u16TopicLen);

    if(MAX_SIZE_OF_TOPIC_LENGTH < u16TopicLen)
    {
        IOT_ERROR("topic lens invalid");
        return;
    }

    uint8_t retry = 0;

    // peding receive process
    Cloud_RxProcessGoingStateSet(false);

    while(retry < 10)
    {
        // disable skip dtim
        Cloud_MqttSkipDtimSet(false);

        // un-subscribe topic
        rc = aws_iot_mqtt_unsubscribe(&client, cTopic, u16TopicLen);

        // enable skip dtim
        Cloud_MqttSkipDtimSet(true);

        if(SUCCESS != rc)
        {
            retry ++;
            IOT_ERROR("--- un-subscribe retry %d", retry);
            osDelay(500);
        }
        else
        {
            IOT_INFO("---> subscribe success");
            break;
        }
    }

    // activate receive process
    Cloud_RxProcessGoingStateSet(true);
}

/*************************************************************************
* FUNCTION:
*   Cloud_RxProcessGoingStateInit
*
* DESCRIPTION:
*   initiate receive process event group
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_RxProcessGoingStateInit(void)
{
    EG_Create(&g_tCloudMqttCtrl);
}

/*************************************************************************
* FUNCTION:
*   Cloud_RxProcessGoingStateSet
*
* DESCRIPTION:
*   set receive process going/pending
*   (setting will be activate in next loop of yield)
*
* PARAMETERS
*   blGoing :       [IN] true -> receive process activate
*                        false -> pending the receive process in next loop, and will keep active when set true
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_RxProcessGoingStateSet(bool blGoing)
{
    EG_StatusSet(g_tCloudMqttCtrl, CLOUD_MQTT_EG_BIT_RX_PROC, blGoing);
}

/*************************************************************************
* FUNCTION:
*   Cloud_RxProcessGoingStateGet
*
* DESCRIPTION:
*   get receive process going/pending state
*
* PARAMETERS
*   none
*
* RETURNS
*   blGoing :       [IN] true -> receive process activate
*                        false -> pending the receive process in next loop, and will keep active when set true
*
*************************************************************************/
bool Cloud_RxProcessGoingStateGet(void)
{
    return (bool)EG_StatusGet(g_tCloudMqttCtrl, CLOUD_MQTT_EG_BIT_RX_PROC);
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
        OPL_LOG_ERRO(CLOUD, "initiate skip DTIM id fail");
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
void Cloud_MqttSkipDtimSet(uint8_t u8Enable)
{
    if(OPL_OK != Opl_Wifi_Skip_Dtim_Set(g_u16CloudDtimId, u8Enable))
    {
        OPL_LOG_ERRO(CLOUD, "skip DTIM fail");
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

/*************************************************************************
* FUNCTION:
*   Cloud_MqttDisconnectIndCallback
*
* DESCRIPTION:
*   mqtt disconnect event indicate callback function
*
* PARAMETERS
*   pClient :       [IN] client data
*   data :          [IN] data assigned at init
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_MqttDisconnectIndCallback(AWS_IoT_Client *pClient, void *data)
{
    IOT_WARN("Broker indicate disconnect");

    if(NULL == pClient)
    {
        return;
    }

    IOT_UNUSED(data);

    if(true == g_tCloudConnInfo.u8AutoConn)
    {
        // start re-connect timer to retry connection
        if(aws_iot_is_autoreconnect_enabled(pClient))
        {
            // if auto reconnect enabled, then no need to care with
            IOT_INFO("Auto reconnect is enabled, Reconnecting attempt will start now");
        }
        else
        {
            // if auto reconnect not enabled, start re-connect timer to retry connection
            IOT_WARN("Auto reconnect not enabled. Starting manual reconnect...");

            // peding receive process
            Cloud_RxProcessGoingStateSet(false);

            // stop keep alive timer
            Cloud_TimerStop(CLOUD_TMR_KEEP_ALIVE);

            Cloud_OnlineStatusSet(false);

            Cloud_TimerStart(CLOUD_TMR_CONN_RETRY, MQTT_AUTO_RECONN_TIME);
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

    // event group regi
    Cloud_RxProcessGoingStateInit();

    // initiate MQTT DTIM id
    Cloud_MqttSkipDtimInit();
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
    if(true == aws_iot_mqtt_is_client_connected(&client))
    {
        IOT_INFO("Broker connected already");
        return;
    }

    IoT_Error_t rc = FAILURE;

    // copy the connection information to global
    g_tCloudConnInfo = *((T_CloudConnInfo *)pData);

    // initialize mqtt protocol
    IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;

    IOT_INFO("\nMQTT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

    mqttInitParams.pHostURL = (char *)&g_tCloudConnInfo.u8aHostAddr;
    mqttInitParams.port = g_tCloudConnInfo. u16HostPort;
    mqttInitParams.enableAutoReconnect = false;
    mqttInitParams.isSSLHostnameVerify = MQTT_SSL_HOST_NAME_VERF;
    mqttInitParams.mqttCommandTimeout_ms = MQTT_COMMAND_TIMEOUT;
    mqttInitParams.tlsHandshakeTimeout_ms = MQTT_TLS_HANDSHAKE_TIMEOUT;
    mqttInitParams.disconnectHandler = Cloud_MqttDisconnectIndCallback;
    mqttInitParams.disconnectHandlerData = NULL;

    rc = aws_iot_mqtt_init(&client, &mqttInitParams);

    if(SUCCESS != rc)
    {
        IOT_ERROR("aws_iot_mqtt_init returned error : %d ", rc);
    }

    // connect to mqtt broker
    IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

    // TODO: refine the client id set, input by pData
    strcpy(g_cCloudMqttClientId, MQTT_CLIENT_ID);

    connectParams.MQTTVersion = MQTT_3_1_1;
    connectParams.isCleanSession = true;
    connectParams.isWillMsgPresent = false;
    connectParams.keepAliveIntervalInSec = CLOUD_KEEP_ALIVE_TIME / 1000; // in sec
    connectParams.pClientID = g_cCloudMqttClientId;
    connectParams.clientIDLen = (uint16_t) strlen(g_cCloudMqttClientId);

    connectParams.pUsername = MQTT_USERNAME;
    connectParams.usernameLen = (uint16_t)strlen(MQTT_USERNAME);
    connectParams.pPassword = MQTT_PASSWORD;
    connectParams.passwordLen = (uint16_t)strlen(MQTT_PASSWORD);

    IOT_INFO("%s connect to %s:%d", connectParams.pClientID, mqttInitParams.pHostURL, mqttInitParams.port);
    
    // disable skip dtim
    Cloud_MqttSkipDtimSet(false);

    rc = aws_iot_mqtt_connect(&client, &connectParams);

    // enable skip dtim
    Cloud_MqttSkipDtimSet(true);
    
    if(SUCCESS != rc)
    {
        IOT_ERROR("Connect to %s:%d error [rc %d]", MQTT_HOST_URL, MQTT_HOST_PORT, rc);
    }
    else
    {
        IOT_INFO("Connect success");

        Cloud_OnlineStatusSet(true);

        // notify to application
        APP_SendMessage(APP_EVT_CLOUD_CONNECT_IND, NULL, 0);

        // Cloud_TimerStart(CLOUD_TMR_MQTT_DELAY_START, 500);

        // activate receive process
        Cloud_RxProcessGoingStateSet(true);

        if(0 != CLOUD_KEEP_ALIVE_TIME)
        {
            Cloud_TimerStart(CLOUD_TMR_KEEP_ALIVE, CLOUD_KEEP_ALIVE_TIME);
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
    if(false == aws_iot_mqtt_is_client_connected(&client))
    {
        IOT_INFO("Broker already disconnected");
        return;
    }

    IoT_Error_t rc = FAILURE;

    rc = aws_iot_mqtt_disconnect(&client);

    if(SUCCESS != rc)
    {
        IOT_ERROR("Disconnect fail [rc %d]", rc);
    }
    else
    {
        IOT_INFO("Disconnect success");

        // peding receive process
        Cloud_RxProcessGoingStateSet(false);

        // stop keep alive timer
        Cloud_TimerStop(CLOUD_TMR_KEEP_ALIVE);

        Cloud_OnlineStatusSet(false);

        // notify to application
        APP_SendMessage(APP_EVT_CLOUD_DISCONNECT_IND, NULL, 0);
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
    // user implement
    // 1. timer timeout handle
    T_CloudTimerIdx tCloudTimerIdx = *((T_CloudTimerIdx *)pData);

    // *
    switch(tCloudTimerIdx)
    {
        // case of timer id (refer to T_CloudTimerIdx)
        case CLOUD_TMR_CONN_RETRY:
        {
            if(true == aws_iot_mqtt_is_client_connected(&client))
            {
                IOT_INFO("Broker connected already");
                return;
            }

            // disable skip dtim
            Cloud_MqttSkipDtimSet(false);

            // re-connect
            IoT_Error_t rc = FAILURE;
            rc = aws_iot_mqtt_attempt_reconnect(&client);
        
            // enable skip dtim
            Cloud_MqttSkipDtimSet(true);
    
            if(NETWORK_RECONNECTED == rc || SUCCESS == rc)
            {
                IOT_INFO("Re-connect Successful");

                // activate receive process
                Cloud_RxProcessGoingStateSet(true);

                if(0 != CLOUD_KEEP_ALIVE_TIME)
                {
                    Cloud_TimerStart(CLOUD_TMR_KEEP_ALIVE, CLOUD_KEEP_ALIVE_TIME);
                }
        
                Cloud_OnlineStatusSet(true);
            }
            else
            {
                IOT_WARN("Re-connect Failed [rc %d], retry after %d ms", rc, MQTT_AUTO_RECONN_TIME);

                Cloud_TimerStart(CLOUD_TMR_CONN_RETRY, MQTT_AUTO_RECONN_TIME);
            }

            break;
        }

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
    IoT_Error_t rc = FAILURE;

    // peding receive process
    Cloud_RxProcessGoingStateSet(false);

    rc = aws_iot_mqtt_keep_alive(&client);

    // activate receive process
    Cloud_RxProcessGoingStateSet(true);

    if(SUCCESS != rc)
    {
        IOT_ERROR("Keep alive fail [rc %d]", rc);
    }

    Cloud_TimerStart(CLOUD_TMR_KEEP_ALIVE, CLOUD_KEEP_ALIVE_TIME);
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

        return;
    }

    T_CloudPayloadFmt tCloudPayloadFmt = *((T_CloudPayloadFmt *)pData);

    if(CLOUD_TOPIC_NUMBER <= tCloudPayloadFmt.u8TopicIndex)
    {
        return;
    }

    IoT_Error_t rc = FAILURE;
    IoT_Publish_Message_Params tPubMsgParam;

    tPubMsgParam.qos = QOS1;
    tPubMsgParam.isRetained = 0;
    tPubMsgParam.payload = (void *)tCloudPayloadFmt.u8aPayloadBuf;
    tPubMsgParam.payloadLen = tCloudPayloadFmt.u32PayloadLen;

    // peding receive process
    Cloud_RxProcessGoingStateSet(false);

    IOT_INFO("Pub \"%s\"(%d) to %s", tPubMsgParam.payload, tPubMsgParam.payloadLen, g_tTxTopicTab[tCloudPayloadFmt.u8TopicIndex].u8aTopicName);

    IOT_INFO("--->Publish");

    uint8_t u8Retry = 0;

    while(u8Retry < 3)
    {
        rc = aws_iot_mqtt_publish(&client,
                                  (char *)g_tTxTopicTab[tCloudPayloadFmt.u8TopicIndex].u8aTopicName,
                                  strlen((char *)g_tTxTopicTab[tCloudPayloadFmt.u8TopicIndex].u8aTopicName),
                                  &tPubMsgParam);

        if(SUCCESS != rc)
        {
            u8Retry ++;
            IOT_WARN("---Publish fail [rc %d], retry %d", rc, u8Retry);
        }
        else
        {
            break;
        }
    }

    IOT_INFO("<---Publish [rc %d]", rc);

    // activate receive process
    Cloud_RxProcessGoingStateSet(true);
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
    IoT_Error_t rc = FAILURE;

    if(OPL_OK == EG_StatusWait(g_tCloudMqttCtrl, CLOUD_MQTT_EG_BIT_RX_PROC, 0xFFFFFFFF))
    {
        IOT_INFO("-->Yield");

        rc = aws_iot_mqtt_yield(&client, MQTT_YIELD_TIMEOUT);

        IOT_INFO("<--Yield [rc %d]", rc);
    }

    osDelay(MQTT_YIELD_ROUND_DELAY);

    // WARNING: IF DO NOTHING IN RECEIVE HANDLER, THE DELAY MUST EXIST
    // osDelay(1000);
}
