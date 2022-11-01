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
#include "ble_mngr_api.h"
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

#if (CLOUD_TX_DATA_BACKUP_ENABLED == 1)
#include "ring_buffer.h"
#endif /* CLOUD_TX_DATA_BACKUP_ENABLED */

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

// global cloud status callback fp
T_CloudStatusCbFp g_tCloudStatusCbFp = NULL;

// global cloud configure information
T_CloudConnInfo g_tCloudConnInfo = {0};
T_CloudTopicRegInfo g_tTxTopicTab[CLOUD_TOPIC_NUMBER];
T_CloudTopicRegInfo g_tRxTopicTab[CLOUD_TOPIC_NUMBER];

// dtim id
uint16_t g_u16CloudDtimId[CLOUD_SKIP_DTIM_MAX] = {0};

static uint8_t g_u8BeenConnected = false;

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

    uint8_t u8TopicIndex = tCloudTopicRegInfo->u8TopicIndex;

    if(CLOUD_TOPIC_NUMBER <= u8TopicIndex)
    {
        return OPL_ERR_PARAM_INVALID;
    }

    if(true == g_tTxTopicTab[u8TopicIndex].u8IsTopicRegisted)
    {
        return OPL_ERR;
    }

    memcpy(&g_tTxTopicTab[u8TopicIndex], tCloudTopicRegInfo, sizeof(T_CloudTopicRegInfo));

    // register topic to cloud
    // *
    // mqtt no need the tx topic registeration
    // copy the tx topic register information to global registeration table
    tEvtRst = OPL_OK;
    // *

    if(OPL_OK == tEvtRst)
    {
        g_tTxTopicTab[u8TopicIndex].u8IsTopicRegisted = true;
    }
    else
    {
        memset(&g_tTxTopicTab[u8TopicIndex], 0, sizeof(T_CloudTopicRegInfo));
    }

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
    uint8_t u8TopicIndex = tCloudTopicRegInfo->u8TopicIndex;

    if(CLOUD_TOPIC_NUMBER <= u8TopicIndex)
    {
        return;
    }

    if(true == g_tTxTopicTab[u8TopicIndex].u8IsTopicRegisted)
    {
        return;
    }

    memcpy(&g_tTxTopicTab[u8TopicIndex], tCloudTopicRegInfo, sizeof(T_CloudTopicRegInfo));
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

    if(CLOUD_TOPIC_NUMBER <= u8TopicIndex)
    {
        return OPL_ERR_PARAM_INVALID;
    }

    if(false == g_tTxTopicTab[u8TopicIndex].u8IsTopicRegisted || 0 == g_tTxTopicTab[u8TopicIndex].u8TopicIndex)
    {
        return OPL_ERR;
    }

    // un-register topic from cloud
    // *
    // mqtt no need the tx topic registeration
    // remove the tx topic register information from global registeration table
    tEvtRst = OPL_OK;
    // *

    if(OPL_OK == tEvtRst)
    {
        memset(&g_tTxTopicTab[u8TopicIndex], 0, sizeof(T_CloudTopicRegInfo));
    }

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

    uint8_t u8TopicIndex = tCloudTopicRegInfo->u8TopicIndex;

    if(CLOUD_TOPIC_NUMBER <= u8TopicIndex)
    {
        return OPL_ERR_PARAM_INVALID;
    }

    if(true == g_tRxTopicTab[u8TopicIndex].u8IsTopicRegisted)
    {
        return OPL_ERR;
    }

    memcpy(&g_tRxTopicTab[u8TopicIndex], tCloudTopicRegInfo, sizeof(T_CloudTopicRegInfo));

    // register topic to cloud
    // *
    // send message to cloud kernel to process subscribe topic
    tEvtRst = Cloud_MsgSend(CLOUD_EVT_TYPE_REGIS_TOPIC, &u8TopicIndex, sizeof(uint8_t));
    // *

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
    uint8_t u8TopicIndex = tCloudTopicRegInfo->u8TopicIndex;

    if(CLOUD_TOPIC_NUMBER <= u8TopicIndex)
    {
        return;
    }

    if(true == g_tRxTopicTab[u8TopicIndex].u8IsTopicRegisted)
    {
        return;
    }

    memcpy(&g_tRxTopicTab[u8TopicIndex], tCloudTopicRegInfo, sizeof(T_CloudTopicRegInfo));
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

    if(CLOUD_TOPIC_NUMBER <= u8TopicIndex)
    {
        return OPL_ERR_PARAM_INVALID;
    }

    if(false == g_tRxTopicTab[u8TopicIndex].u8IsTopicRegisted || 0 == g_tRxTopicTab[u8TopicIndex].u8TopicIndex)
    {
        return OPL_ERR;
    }

    // un-register topic from cloud
    // *
    // send message to cloud kernel to process un-register topic
    tEvtRst = Cloud_MsgSend(CLOUD_EVT_TYPE_UNREGIS_TOPIC, &u8TopicIndex, sizeof(uint8_t));
    // *

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
T_OplErr Cloud_MqttSubscribeTopic(char *cTopic, uint16_t u16TopicLen, QoS tQoS, pApplicationHandler_t fpFunc)
{
    IoT_Error_t rc = FAILURE;
    T_OplErr tRst = OPL_ERR;

    IOT_INFO("sub topic %s (%d) [QoS%d]", cTopic, u16TopicLen, tQoS);

    if(MAX_SIZE_OF_TOPIC_LENGTH < u16TopicLen)
    {
        IOT_ERROR("topic lens invalid");
        return OPL_ERR_PARAM_INVALID;
    }

    uint8_t u8Retry = 0;

    while(u8Retry < MQTT_RETRY_COUNTS)
    {
        // subscribe topic
        rc = aws_iot_mqtt_subscribe(&client, cTopic, u16TopicLen, tQoS, fpFunc, NULL);

        if(SUCCESS != rc)
        {
            u8Retry ++;
            IOT_ERROR("--- subscribe retry %d", u8Retry);
            osDelay(500);
        }
        else
        {
            IOT_INFO("---> subscribe success");
            tRst = OPL_OK;
            break;
        }
    }

    // calling cloud status callback to notify application
    if(SUCCESS == rc)
    {
        Cloud_StatusCallback(CLOUD_CB_STA_SUB_DONE, NULL, 0);
    }
    else
    {
        Cloud_StatusCallback(CLOUD_CB_STA_SUB_FAIL, NULL, 0);
    }

    return tRst;
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
T_OplErr Cloud_MqttUnsubscribeTopic(char *cTopic, uint16_t u16TopicLen)
{
    IoT_Error_t rc = FAILURE;
    T_OplErr tRst = OPL_ERR;

    IOT_INFO("unsub topic %s (%d)", cTopic, u16TopicLen);

    if(MAX_SIZE_OF_TOPIC_LENGTH < u16TopicLen)
    {
        IOT_ERROR("topic lens invalid");
        return OPL_ERR_PARAM_INVALID;
    }

    uint8_t u8Retry = 0;

    // peding receive process
    // Cloud_RxProcessGoingStateSet(false);

    while(u8Retry < MQTT_RETRY_COUNTS)
    {
        // un-subscribe topic
        rc = aws_iot_mqtt_unsubscribe(&client, cTopic, u16TopicLen);

        if(SUCCESS != rc)
        {
            u8Retry ++;
            IOT_ERROR("--- un-subscribe retry %d", u8Retry);
            osDelay(500);
        }
        else
        {
            IOT_INFO("---> subscribe success");
            tRst = OPL_OK;
            break;
        }
    }

    // activate receive process
    // Cloud_RxProcessGoingStateSet(true);

    // calling cloud status callback to notify application
    if(SUCCESS == rc)
    {
        Cloud_StatusCallback(CLOUD_CB_STA_UNSUB_DONE, NULL, 0);
    }
    else
    {
        Cloud_StatusCallback(CLOUD_CB_STA_UNSUB_FAIL, NULL, 0);
    }

    return tRst;
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
    for(uint8_t u8Count = 0; u8Count < CLOUD_SKIP_DTIM_MAX; u8Count ++)
    {
        if(OPL_OK != Opl_Wifi_Skip_Dtim_Module_Reg(&g_u16CloudDtimId[u8Count]))
        {
            OPL_LOG_ERRO(CLOUD, "initiate skip DTIM id %d fail", u8Count);
        }
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
void Cloud_MqttSkipDtimSet(T_CloudSkipDtimIdx tCloudSkipDtimIdx, uint8_t u8Enable)
{
    uint8_t u8Idx = tCloudSkipDtimIdx;

    if(OPL_OK != Opl_Wifi_Skip_Dtim_Set(g_u16CloudDtimId[u8Idx], u8Enable))
    {
        OPL_LOG_ERRO(CLOUD, "skip DTIM fail (idx %d)", u8Idx);
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

            // check network is up or down
            if(true == Cloud_NetworkStatusGet())
            {
                IOT_INFO("Start re-connect timer");
                Cloud_TimerStart(CLOUD_TMR_CONN_RETRY, MQTT_AUTO_RECONN_TIME);
            }

            // calling cloud status callback to notify application
            Cloud_StatusCallback(CLOUD_CB_STA_DISCONN, NULL, 0);
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

    // reset topics table
    memset(&g_tTxTopicTab, 0, sizeof(g_tTxTopicTab));
    memset(&g_tRxTopicTab, 0, sizeof(g_tRxTopicTab));

    // event group regi
    Cloud_RxProcessGoingStateInit();

    // initiate MQTT DTIM id
    Cloud_MqttSkipDtimInit();

    IoT_Error_t rc = FAILURE;

    // copy the connection information to global
    memcpy(&g_tCloudConnInfo, (T_CloudConnInfo *)pData, sizeof(T_CloudConnInfo));

    // initialize mqtt protocol
    IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;

    IOT_INFO("\nMQTT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

    mqttInitParams.pHostURL = (char *)g_tCloudConnInfo.u8aHostAddr;
    mqttInitParams.port = g_tCloudConnInfo.u16HostPort;
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
        IOT_INFO("Network not up, cloud connect won't do");
        return;
    }

    if(true == Cloud_OnlineStatusGet())
    {
        IOT_INFO("Broker connected already");
        return;
    }

    IoT_Error_t rc = FAILURE;

    if(false == g_u8BeenConnected)
    {
        // connect to mqtt broker
        IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

        // TODO: refine the client id set, input by pData
        uint8_t u8aBleMac[6] = {0};
        Opl_Ble_MacAddr_Read(u8aBleMac);

        sprintf((char *)g_cCloudMqttClientId, "%s_%.2X:%.2X:%.2X:%.2X", MQTT_CLIENT_ID, u8aBleMac[2], u8aBleMac[3], u8aBleMac[4], u8aBleMac[5]);

        connectParams.MQTTVersion = MQTT_3_1_1;
        connectParams.isCleanSession = true;
        connectParams.isWillMsgPresent = false;
        connectParams.keepAliveIntervalInSec = MQTT_COMMAND_TIMEOUT; // in sec
        connectParams.pClientID = g_cCloudMqttClientId;
        connectParams.clientIDLen = (uint16_t) strlen(g_cCloudMqttClientId);

        connectParams.pUsername = MQTT_USERNAME;
        connectParams.usernameLen = (uint16_t)strlen(MQTT_USERNAME);
        connectParams.pPassword = MQTT_PASSWORD;
        connectParams.passwordLen = (uint16_t)strlen(MQTT_PASSWORD);

        IOT_INFO("%s connect to %s:%d", connectParams.pClientID, g_tCloudConnInfo.u8aHostAddr, g_tCloudConnInfo.u16HostPort);

        rc = aws_iot_mqtt_connect(&client, &connectParams);
        
        if(SUCCESS != rc)
        {
            IOT_ERROR("Cloud connect fail [rc %d]", rc);

            // calling cloud status callback to notify application
            Cloud_StatusCallback(CLOUD_CB_STA_CONN_FAIL, NULL, 0);
        }
        else
        {
            IOT_INFO("Cloud connected success");

            // register subscribe topics
            uint8_t u8Count = 0;
            for(u8Count = 0; u8Count < CLOUD_TOPIC_NUMBER; u8Count ++)
            {
                if(g_tRxTopicTab[u8Count].u8TopicIndex != 0)
                {
                    if(OPL_OK == Cloud_MqttSubscribeTopic((char *)g_tRxTopicTab[u8Count].u8aTopicName, strlen((char *)g_tRxTopicTab[u8Count].u8aTopicName), QOS1, (pApplicationHandler_t)g_tRxTopicTab[u8Count].fpFunc))
                    {
                        g_tRxTopicTab[u8Count].u8IsTopicRegisted = true;
                    }
                    else
                    {
                        // clear the information while register subscribe topic fail
                        memset(&g_tRxTopicTab[u8Count], 0, sizeof(T_CloudTopicRegInfo));
                    }
                }
            }

            // register publish topic (only set u8IsTopicRegisted as true)
            for(u8Count = 0; u8Count < CLOUD_TOPIC_NUMBER; u8Count ++)
            {
                if(g_tTxTopicTab[u8Count].u8TopicIndex != 0)
                {
                    g_tTxTopicTab[u8Count].u8IsTopicRegisted = true;
                }
            }

            Cloud_OnlineStatusSet(true);

            // set the g_u8BeenConnected as true
            g_u8BeenConnected = true;

            // calling cloud status callback to notify application
            Cloud_StatusCallback(CLOUD_CB_STA_CONN_DONE, NULL, 0);

            // activate receive process
            Cloud_RxProcessGoingStateSet(true);

            if(0 != CLOUD_KEEP_ALIVE_TIME)
            {
                Cloud_TimerStart(CLOUD_TMR_KEEP_ALIVE, CLOUD_KEEP_ALIVE_TIME);
            }
        }
    }
    else
    {
        // re-connect
        IoT_Error_t rc = FAILURE;
        rc = aws_iot_mqtt_attempt_reconnect(&client);

        if(NETWORK_RECONNECTED == rc || SUCCESS == rc)
        {
            IOT_INFO("Cloud connected success");

            Cloud_OnlineStatusSet(true);

            // calling cloud status callback to notify application
            Cloud_StatusCallback(CLOUD_CB_STA_RECONN_DONE, NULL, 0);

            // activate receive process
            Cloud_RxProcessGoingStateSet(true);

            if(0 != CLOUD_KEEP_ALIVE_TIME)
            {
                Cloud_TimerStart(CLOUD_TMR_KEEP_ALIVE, CLOUD_KEEP_ALIVE_TIME);
            }
        }
        else
        {
            IOT_WARN("Re-connect Failed [rc %d], retry after %d ms", rc, MQTT_AUTO_RECONN_TIME);

            Cloud_TimerStart(CLOUD_TMR_CONN_RETRY, MQTT_AUTO_RECONN_TIME);
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
    if(false == Cloud_OnlineStatusGet())
    {
        IOT_INFO("Broker already disconnected");
        return;
    }

    IoT_Error_t rc = FAILURE;

    rc = aws_iot_mqtt_disconnect(&client);

    if(SUCCESS != rc)
    {
        IOT_ERROR("Cloud disconnect fail [rc %d]", rc);
    }
    else
    {
        IOT_INFO("Cloud disconnect success");

        // peding receive process
        Cloud_RxProcessGoingStateSet(false);

        // stop keep alive timer
        Cloud_TimerStop(CLOUD_TMR_KEEP_ALIVE);

        Cloud_OnlineStatusSet(false);

        // calling cloud status callback to notify application
        Cloud_StatusCallback(CLOUD_CB_STA_DISCONN, NULL, 0);
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
            // check network is up or down
            if(false == Cloud_NetworkStatusGet())
            {
                IOT_INFO("Network not up, ignore cloud re-connect");
                return;
            }

            if(true == Cloud_OnlineStatusGet())
            {
                IOT_INFO("Broker connected already");
                return;
            }


            // re-connect
            IoT_Error_t rc = FAILURE;
            rc = aws_iot_mqtt_attempt_reconnect(&client);
        
            if(NETWORK_RECONNECTED == rc || SUCCESS == rc)
            {
                IOT_INFO("Re-connect Successful");

                Cloud_OnlineStatusSet(true);

                // calling cloud status callback to notify application
                Cloud_StatusCallback(CLOUD_CB_STA_RECONN_DONE, NULL, 0);

                // activate receive process
                Cloud_RxProcessGoingStateSet(true);

                if(0 != CLOUD_KEEP_ALIVE_TIME)
                {
                    Cloud_TimerStart(CLOUD_TMR_KEEP_ALIVE, CLOUD_KEEP_ALIVE_TIME);
                }
            }
            else
            {
                IOT_WARN("Re-connect Failed [rc %d], retry after %d ms", rc, MQTT_AUTO_RECONN_TIME);

                Cloud_TimerStart(CLOUD_TMR_CONN_RETRY, MQTT_AUTO_RECONN_TIME);
            }

            break;
        }

        case CLOUD_TMR_KEEP_ALIVE:
        {
            // Cloud_MsgSend(CLOUD_EVT_TYPE_KEEP_ALIVE, NULL, 0);
            Cloud_KeepAliveHandler(u32EventId, pData, u32DataLen);
            break;
        }

        case CLOUD_TMR_DATA_POST_TIMEOUT:
        {
            IOT_WARN("Waiting PUBACK timeout, restore flag");
            aws_iot_mqtt_restore_wait_pub_ack(&client);
            break;
        }

        case CLOUD_TMR_KEEP_ALIVE_TIMEOUT:
        {
            IOT_WARN("Waiting PINGRESP timeout, restore skip dtim");
            aws_iot_mqtt_restore_keep_alive_skip_dtim(&client);
            break;
        }

        case CLOUD_TMR_PUB_SKIP_DTIM_EN:
        {
            Cloud_MqttSkipDtimSet(CLOUD_SKIP_DTIM_PUBLIC, true);
            break;
        }

        case CLOUD_TMR_KEEP_ALIVE_SKIP_DTIM_EN:
        {
            Cloud_MqttSkipDtimSet(CLOUD_SKIP_DTIM_KEEPALIVE, true);
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
    // check network is up or down
    if(false == Cloud_NetworkStatusGet())
    {
        OPL_LOG_INFO(CLOUD, "Network not up, ignore keep alive");
        return;
    }

    // check connection
    if(false == Cloud_OnlineStatusGet())
    {   
        OPL_LOG_INFO(CLOUD, "Cloud disconnected");
        return;
    }
    
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

    if(true == Cloud_OnlineStatusGet())
    {
        Cloud_TimerStart(CLOUD_TMR_KEEP_ALIVE, CLOUD_KEEP_ALIVE_TIME);
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

    rc = aws_iot_mqtt_publish(&client,
                              (char *)g_tTxTopicTab[tCloudPayloadFmt.u8TopicIndex].u8aTopicName,
                              strlen((char *)g_tTxTopicTab[tCloudPayloadFmt.u8TopicIndex].u8aTopicName),
                              &tPubMsgParam);

    if(SUCCESS != rc)
    {
        // calling cloud status callback to notify application
        Cloud_StatusCallback(CLOUD_CB_STA_PUB_FAIL, NULL, 0);

        IOT_WARN("---Publish fail [rc %d]", rc);
    }
    else
    {
        // calling cloud status callback to notify application
        Cloud_StatusCallback(CLOUD_CB_STA_PUB_DONE, NULL, 0);

        IOT_INFO("Publish start");
    }

    IOT_INFO("<---Publish [rc %d]", rc);

    // activate receive process
    Cloud_RxProcessGoingStateSet(true);
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
    T_OplErr tEvtRst = OPL_ERR;

    uint8_t u8TopicIndex = *((uint8_t *)pData);

    // peding receive process
    Cloud_RxProcessGoingStateSet(false);

    // register topic to cloud
    tEvtRst = Cloud_MqttSubscribeTopic((char *)g_tRxTopicTab[u8TopicIndex].u8aTopicName, strlen((char *)g_tRxTopicTab[u8TopicIndex].u8aTopicName), QOS1, (pApplicationHandler_t)g_tRxTopicTab[u8TopicIndex].fpFunc);

    // peding receive process
    Cloud_RxProcessGoingStateSet(true);

    if(OPL_OK == tEvtRst)
    {
        g_tRxTopicTab[u8TopicIndex].u8IsTopicRegisted = true;
    }
    else
    {
        memset(&g_tRxTopicTab[u8TopicIndex], 0, sizeof(T_CloudTopicRegInfo));
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
    T_OplErr tEvtRst = OPL_ERR;

    uint8_t u8TopicIndex = *((uint8_t *)pData);

    // peding receive process
    Cloud_RxProcessGoingStateSet(false);

    // remove topic from cloud
    tEvtRst = Cloud_MqttUnsubscribeTopic((char *)g_tRxTopicTab[u8TopicIndex].u8aTopicName, strlen((char *)g_tRxTopicTab[u8TopicIndex].u8aTopicName));

    // peding receive process
    Cloud_RxProcessGoingStateSet(true);

    if(OPL_OK == tEvtRst)
    {
        memset(&g_tRxTopicTab[u8TopicIndex], 0, sizeof(T_CloudTopicRegInfo));
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
    IoT_Error_t rc = FAILURE;

    if(OPL_OK == EG_StatusWait(g_tCloudMqttCtrl, CLOUD_MQTT_EG_BIT_RX_PROC, 0xFFFFFFFF))
    {
        IOT_INFO("-->Yield");

#if 1
        rc = aws_iot_mqtt_yield(&client);
#else
        rc = aws_iot_mqtt_yield(&client, MQTT_YIELD_TIMEOUT);
#endif

        IOT_INFO("<--Yield [rc %d]", rc);
    }

    // osDelay(MQTT_YIELD_ROUND_DELAY);

    // WARNING: IF DO NOTHING IN RECEIVE HANDLER, THE DELAY MUST EXIST
    // osDelay(1000);
}
