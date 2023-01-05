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
*  aws_mqtt_helper.h
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

#include "log.h"
#include "opl_err.h"

#include "core_mqtt.h"
#include "mbedtls_opl.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __AWS_MQTT_HELPER_H__
#define __AWS_MQTT_HELPER_H__

#ifdef __cplusplus
extern "C" {
#endif

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

bool AWS_MqttHelperInit( MQTTContext_t *ptMqttContext, 
                         NetworkContext_t *ptNetworkContext,
                         char *pRootCaLabel, 
                         char *pClientCertLabel, 
                         char *pPrivateKeyLabel,
                         MQTTEventCallback_t ptEventCbFunc);

bool AWS_MqttHelperEstablishSession( MQTTContext_t *ptMqttContext,
                                     NetworkContext_t *ptNetworkContext,
                                     char *pHostName,
                                     uint16_t u16Port);

bool AWS_MqttHelperDisconnectSession( MQTTContext_t *ptMqttContext,
                                      NetworkContext_t *ptNetworkContext);

int32_t AWS_MqttHelperPing( MQTTContext_t *ptMqttContext);

void AWS_MqttHelperClientIdSet(uint8_t *u8ClientId, uint16_t u16ClientIdLen);

uint8_t *AWS_MqttHelperClientIdGet(void);

int32_t AWS_MqttHelperSubscribeToTopic( MQTTContext_t *ptMqttContext,
                                        MQTTSubscribeInfo_t *ptMqttSubscribeInfo,
                                        uint16_t *pu16NextPacketIdentifier);

int32_t AWS_MqttHelperUnsubscribeFromTopic( MQTTContext_t *ptMqttContext,
                                            MQTTSubscribeInfo_t *ptMqttSubscribeInfo,
                                            uint16_t *pu16UnsubscribePacketAckIdentifier);

int32_t AWS_MqttHelperPublishToTopic( MQTTContext_t *ptMqttContext,
                                      MQTTPublishInfo_t *ptMqttPublishPacket,
                                      uint16_t *pu16PublishPacketAckIdentifier);

MQTTStatus_t AWS_MqttHelperProcessLoop( MQTTContext_t *ptMqttContext);

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

#endif /* __CLOUD_DATA_H__ */
