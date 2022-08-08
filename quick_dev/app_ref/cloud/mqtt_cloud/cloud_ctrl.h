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

#include "cloud_config.h"
#include "aws_iot_mqtt_client.h"
#include "log.h"
#include "opl_err.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __CLOUD_CTRL_H__
#define __CLOUD_CTRL_H__

#ifdef __cplusplus
extern "C" {
#endif

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef enum E_CloudTimerIdx
{
    // user implement
    // *
    CLOUD_TMR_CONN_RETRY = 0,
    CLOUD_TMR_KEEP_ALIVE,
    // *

    CLOUD_TMR_MAX,
} T_CloudTimerIdx;

//---- for event use ----//
// cloud connction information
typedef struct S_CloudConnInfo
{
    uint8_t u8AutoConn;                                 // cloud auto-connect after unsolicited disconnect event
                                                        // <0> NOT do auto-connect after cloud disconnect (DEFAULT)
                                                        // <1> do auto-connect after cloud disconnect

    uint8_t u8Security;                                 // TLS or SSL connection security type (only activate if connection is a security type)
                                                        // <0> MBEDTLS_SSL_VERIFY_NONE (DEFAULT)
                                                        // <1> MBEDTLS_SSL_VERIFY_OPTIONAL
                                                        // <2> MBEDTLS_SSL_VERIFY_REQUIRED
                                                        // <3> MBEDTLS_SSL_VERIFY_UNSET

    uint8_t u8aHostAddr[CLOUD_HOST_URL_LEN];            // host ip address or url address

    uint16_t u16HostPort;                               // host port
} T_CloudConnInfo;

// cloud topic register information
typedef struct S_CloudTopicRegInfo
{
    uint8_t u8TopicIndex;                               // topic index (range define to CLOUD_TOPIC_NUMBER in cloud_config.h)
    uint8_t u8IsTopicRegisted;                          // register status
    uint8_t u8aTopicName[CLOUD_TOPIC_NAME_LEN];         // topic name in string type
} T_CloudTopicRegInfo;

// cloud topic register information pointer
typedef T_CloudTopicRegInfo *T_CloudTopicRegInfoPtr;

// cloud payload format
typedef struct S_CloudPayloadFmt
{
    uint8_t u8TopicIndex;                               // tx topic index (range define to CLOUD_TOPIC_NUMBER in cloud_config.h)
    uint32_t u32PayloadLen;                             // payload lens
    uint8_t u8aPayloadBuf[CLOUD_PAYLOAD_LEN];           // payload
} T_CloudPayloadFmt;


// typedef struct S_CloudMqttPublishData
// {
//     QoS tQos;
//     char *pcPubTopic;
//     uint16_t u16PubTopicLen;
// 	uint16_t payloadLen;
//     uint8_t payload[150];
// } T_CloudMqttPublishData;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

T_OplErr Cloud_TxTopicRegister(T_CloudTopicRegInfo *tCloudTopicRegInfo, void *vCallback);
T_OplErr Cloud_RxTopicRegister(T_CloudTopicRegInfo *tCloudTopicRegInfo, void *vCallback);
T_CloudTopicRegInfoPtr Cloud_TxTopicGet(void);
T_CloudTopicRegInfoPtr Cloud_RxTopicGet(void);

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
void Cloud_MqttSubscribeTopic(char *cTopic, uint16_t u16TopicLen, QoS tQoS, pApplicationHandler_t fpFunc);

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
void Cloud_MqttUnsubscribeTopic(char *cTopic, uint16_t u16TopicLen);

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
void Cloud_RxProcessGoingStateSet(bool blGoing);

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
bool Cloud_RxProcessGoingStateGet(void);

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
void Cloud_MqttSkipDtimSet(uint8_t u8Enable);

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
