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
*  aws_mqtt_helper.c
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

#include "aws_mqtt_helper.h"
#include "cloud_config.h"
#include "clock.h"
#include "wifi_mngr_api.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef MQTT_NETWORK_BUFFER_SIZE
    #error "Please define network buffer size in cloud_config.h"
#endif

#ifndef MQTT_PROCESS_TIMEOUT_MS
    #error "Please define timeout (msec) for process in cloud_config.h"
#endif

#ifndef MQTT_KEEP_ALIVE_INTERVAL_SEC
    #error "Please define keep liave (sec) for coreMQTT in cloud_config.h"
#endif

// length of client identifier
#define CLIENT_IDENTIFIER_LENGTH                        (64U)

// maximum number of outgoing publishes maintained in the application
// until an ack is received from teh broker
#define MAX_OUTGOING_PUBLISHES                          (5U)

// the length of the outgoing publish records array used by the coreMQTT
// library to track QoS > 0 packet ACKS for outgoing publishes.
#define OUTGOING_PUBLISH_RECORD_LEN                     (10U)

// the length of hte incoming publish records array used by the coreMQTT
// library to track QoS > 0 packet ACKS for incoming publishes.
#define INCOMING_PUBLISH_RECORD_LEN                     (10U)

// timeout for receiving CONNACK packet in milliseconds
#define CONNACK_RECV_TIMEOUT_MS                         (1000U)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

// Each compilation unit must define the NetworkContext struct
struct NetworkContext
{
    MbedtlsOplContext_t *tMbedtlsOplContext;
    MbedtlsOplCredentials_t *tMbedtlsOplCredentials;
};

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// the client identifier buffer
static uint8_t g_u8ClientIdBuffer[CLIENT_IDENTIFIER_LENGTH];

static bool g_blMqttSessionEstablished = false;

// the network buffer must remain valid for the lifetime of the MQTT context.
static uint8_t g_u8Buffer[MQTT_NETWORK_BUFFER_SIZE];

// with QoS > 0.
//
// this is passed into #MQTT_InitStatefulQoS to allow for QoS > 0.
static MQTTPubAckInfo_t g_OutgoingPublishRecords[OUTGOING_PUBLISH_RECORD_LEN];

// array to track the incoming publish records for incoming publishes
// with QoS > 0.
//
// this is passed into #MQTT_InitStatefulQoS to allow for QoS > 0.
static MQTTPubAckInfo_t g_IncomingPublishRecords[INCOMING_PUBLISH_RECORD_LEN];

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

bool AWS_MqttHelperInit( MQTTContext_t *ptMqttContext, 
                         NetworkContext_t *ptNetworkContext,
                         char *pRootCaLabel, 
                         char *pClientCertLabel, 
                         char *pPrivateKeyLabel,
                         MQTTEventCallback_t ptEventCbFunc)
{
    TransportInterface_t tTransportInterface;
    MQTTFixedBuffer_t tNetworkBuffer;
    MQTTStatus_t mqttStatus;
    bool ret = false;

    // initialize credentials for establishing TLS session
    ptNetworkContext->tMbedtlsOplCredentials->pRootCaLabel = pRootCaLabel;
    ptNetworkContext->tMbedtlsOplCredentials->pClientCertLabel = pClientCertLabel;
    ptNetworkContext->tMbedtlsOplCredentials->pPrivateKeyLabel = pPrivateKeyLabel;

    // AWS IoT requires devices to send the Server Name Indication (SNI)
    // extension to the Transport Layer Security (TLS) protocol and provide
    // the complete endpoint address in teh host_name field. Deatils about
    // SNI for AWS IoT can be found in the link below.
    // https://docs.aws.amazon.com/iot/latest/developerguide/transport-security.html
    ptNetworkContext->tMbedtlsOplCredentials->disableSni = false;

    // fill in TransportInterface send and receive function pointers
    tTransportInterface.pNetworkContext = ptNetworkContext;
    tTransportInterface.send = Mbedtls_Opl_Send;
    tTransportInterface.recv = Mbedtls_Opl_Recv;
    tTransportInterface.writev = NULL;

    // fill in values for network buffer
    tNetworkBuffer.pBuffer = g_u8Buffer;
    tNetworkBuffer.size = sizeof(g_u8Buffer);

    // initialize the MQTT library
    mqttStatus = MQTT_Init( ptMqttContext,
                            &tTransportInterface,
                            Clock_GetTimeMs,
                            ptEventCbFunc,
                            &tNetworkBuffer);

    if(MQTTSuccess != mqttStatus)
    {
        ret = false;
        OPL_LOG_DEBG(AWSH, "MQTT_Init failed %s", MQTT_Status_strerror(mqttStatus));
    }
    else
    {
        mqttStatus = MQTT_InitStatefulQoS( ptMqttContext,
                                           g_OutgoingPublishRecords,
                                           OUTGOING_PUBLISH_RECORD_LEN,
                                           g_IncomingPublishRecords,
                                           INCOMING_PUBLISH_RECORD_LEN);
        
        if(MQTTSuccess != mqttStatus)
        {
            ret = false;
            OPL_LOG_DEBG(AWSH, "MQTT_InitStatefulQoS failed %s", MQTT_Status_strerror(mqttStatus));
        }
        else
        {
            ret = true;
        }
    }

    return ret;
}

bool AWS_MqttHelperEstablishSession( MQTTContext_t *ptMqttContext,
                                     NetworkContext_t *ptNetworkContext,
                                     char *pHostName,
                                     uint16_t u16Port)
{
    MbedtlsOplStatus_t mbedtlsStatus = MBEDTLS_OPL_SUCCESS;
    MQTTStatus_t mqttStatus = MQTTSuccess;
    MQTTConnectInfo_t mqttConnectInfo;
    bool sessionPresent = false;
    bool ret = false;

    OPL_LOG_DEBG(AWSH, "%s connect to %s:%d", AWS_MqttHelperClientIdGet(), pHostName, u16Port);
    // OPL_LOG_DEBG(AWSH, "Client ID %s\n", AWS_MqttHelperClientIdGet());

    // establish a TLS session with the MQTT broker
    mbedtlsStatus = Mbedtls_Opl_Connect( ptNetworkContext, 
                                         pHostName,
                                         u16Port,
                                         ptNetworkContext->tMbedtlsOplCredentials,
                                         MQTT_PROCESS_TIMEOUT_MS);

    if(MBEDTLS_OPL_SUCCESS == mbedtlsStatus)
    {
        OPL_LOG_DEBG(AWSH, "Connection to broker success");

        // establish an MQTT session by sending a CONNECT packet

        // if #createCleanSession is true, start with a clean session
        // i.e. direct the MQTT broker to discard any previous session data.
        // if #createCleanSession is false, direct hte broker to attempt to
        // restablish a session which was already present.
        mqttConnectInfo.cleanSession = false;

        // the client identifier is used to uniquely identify this MQTT client to
        // the MQTT broker. In a production device the identifier can be something
        // unique, such as a device serial number.
        mqttConnectInfo.pClientIdentifier = (char *)g_u8ClientIdBuffer;
        mqttConnectInfo.clientIdentifierLength = strlen((char *)g_u8ClientIdBuffer);

        // the maximum time interval in seconds which is allowed to elapse between
        // two Control Packet. It is the responsibility of the client to ensure that
        // the interval between control packets being sent does not exceed the this
        // keep-alive value. In the absence of sending any other control packets,
        // the client MUST send a PINGREQ packet.
        //
        // In QuickDev demonstration, periodic keep alive process will be handled
        // by cloud template, so the following keepAliveSeconds variable sets for
        // connection data carried and let server's know the keep alive interval
        // of device.
        mqttConnectInfo.keepAliveSeconds = (uint16_t)MQTT_KEEP_ALIVE_INTERVAL_SEC;

        // username and password for authentication
        mqttConnectInfo.pUserName = NULL;
        mqttConnectInfo.userNameLength = 0U;
        mqttConnectInfo.pPassword = NULL;
        mqttConnectInfo.passwordLength = 0U;

        mqttStatus = MQTT_Connect( ptMqttContext,
                                   &mqttConnectInfo,
                                   NULL,
                                   CONNACK_RECV_TIMEOUT_MS,
                                   &sessionPresent);
        
        if(MQTTSuccess != mqttStatus)
        {
            OPL_LOG_DEBG(AWSH, "Connection with MQTT broker failed %s", MQTT_Status_strerror(mqttStatus));

            // end TLS session, the close TCP connection
            Mbedtls_Opl_Disconnect(ptNetworkContext);
        
            ret = false;
        }
        else
        {
            OPL_LOG_DEBG(AWSH, "MQTT connection successfully established with broker");
            ret = true;
        }
    }
    else
    {
        OPL_LOG_DEBG(AWSH, "Connection to broker failed");

        ret = false;
    }

    if(true == ret)
    {
        // keep a flag for indicating if MQTT session is established.
        // This flag will mark that an MQTT DISCONNECt has to be sent 
        // at the end of the demo even if there are intermediate failures.
        g_blMqttSessionEstablished = true;
    }

    return ret;
}

bool AWS_MqttHelperDisconnectSession( MQTTContext_t *ptMqttContext,
                                      NetworkContext_t *ptNetworkContext)
{
    MQTTStatus_t mqttStatus = MQTTSuccess;
    bool ret = false;

    if(true == g_blMqttSessionEstablished)
    {
        // send DISCONNECT
        mqttStatus = MQTT_Disconnect(ptMqttContext);

        if(MQTTSuccess != mqttStatus)
        {
            OPL_LOG_DEBG(AWSH, "Sending MQTT DISCONNECT failed %s", MQTT_Status_strerror(mqttStatus));
        }
        else
        {
            // MQTT DISCONNECT sent successfully
            ret = true;
        }
    }

    // end TLS session, the close TCP connection
    Mbedtls_Opl_Disconnect(ptNetworkContext);

    return ret;
}

int32_t AWS_MqttHelperPing( MQTTContext_t *ptMqttContext)
{
    int32_t ret = 0;
    MQTTStatus_t mqttStatus;

    mqttStatus = MQTT_Ping(ptMqttContext);

    if(MQTTSuccess != mqttStatus)
    {
        OPL_LOG_DEBG(AWSH, "Failed to send PING packet to broker %s", MQTT_Status_strerror(mqttStatus));
        ret = -1;
    }
    else
    {
        OPL_LOG_DEBG(AWSH, "PING send to broker");
    }

    return ret;
}

void AWS_MqttHelperClientIdSet(uint8_t *u8ClientId, uint16_t u16ClientIdLen)
{
    strncpy((char *)g_u8ClientIdBuffer, (char *)u8ClientId, u16ClientIdLen);
}

uint8_t *AWS_MqttHelperClientIdGet(void)
{
    return g_u8ClientIdBuffer;
}

int32_t AWS_MqttHelperSubscribeToTopic( MQTTContext_t *ptMqttContext,
                                        MQTTSubscribeInfo_t *ptMqttSubscribeInfo,
                                        uint16_t *pu16SubscribePacketAckIdentifier)
{
    int32_t ret = 0;
    MQTTStatus_t mqttStatus;

    if(NULL == pu16SubscribePacketAckIdentifier)
    {
        OPL_LOG_DEBG(AWSH, "Invalid parameter");
        return ret = -1;
    }
    
    // generate packet identifier for the SUBSCRIBE packet
    *pu16SubscribePacketAckIdentifier = MQTT_GetPacketId(ptMqttContext);

    mqttStatus = MQTT_Subscribe( ptMqttContext,
                                 ptMqttSubscribeInfo,
                                 1,
                                 *pu16SubscribePacketAckIdentifier);
    
    if(MQTTSuccess != mqttStatus)
    {
        OPL_LOG_DEBG(AWSH, "Failed to send SUBSCRIBE packet to broker %s", MQTT_Status_strerror(mqttStatus));
        ret = -1;
    }
    else
    {
        OPL_LOG_DEBG(AWSH, "SUBSCRIBE topic %.*s to broker (Next Pack ID %d)", ptMqttSubscribeInfo->topicFilterLength, ptMqttSubscribeInfo->pTopicFilter, *pu16SubscribePacketAckIdentifier);
    }

    return ret;
}


int32_t AWS_MqttHelperUnsubscribeFromTopic( MQTTContext_t *ptMqttContext,
                                            MQTTSubscribeInfo_t *ptMqttSubscribeInfo,
                                            uint16_t *pu16UnsubscribePacketAckIdentifier)
{
    int32_t ret = 0;
    MQTTStatus_t mqttStatus;

    if(NULL == pu16UnsubscribePacketAckIdentifier)
    {
        OPL_LOG_DEBG(AWSH, "Invalid parameter");
        return ret = -1;
    }
    
    // generate packet identifier for the UNSUBSCRIBE packet
    *pu16UnsubscribePacketAckIdentifier = MQTT_GetPacketId(ptMqttContext);

    mqttStatus = MQTT_Unsubscribe( ptMqttContext,
                                   ptMqttSubscribeInfo,
                                   1,
                                   *pu16UnsubscribePacketAckIdentifier);
    
    if(MQTTSuccess != mqttStatus)
    {
        OPL_LOG_DEBG(AWSH, "Failed to send UNSUBSCRIBE packet to broker %s", MQTT_Status_strerror(mqttStatus));
        ret = -1;
    }
    else
    {
        OPL_LOG_DEBG(AWSH, "UNSUBSCRIBE topic %.*s to broker (Next Pack ID %d)", ptMqttSubscribeInfo->topicFilterLength, ptMqttSubscribeInfo->pTopicFilter, *pu16UnsubscribePacketAckIdentifier);
    }

    return ret;    
}

int32_t AWS_MqttHelperPublishToTopic( MQTTContext_t *ptMqttContext,
                                      MQTTPublishInfo_t *ptMqttPublishPacket,
                                      uint16_t *pu16PublishPacketAckIdentifier)
{
    int32_t ret = 0;
    uint16_t u16PublishPacketAckIdentifier = 0;
    MQTTStatus_t mqttStatus;

    // generate packet identifer for the PUBLISH packet
    u16PublishPacketAckIdentifier = MQTT_GetPacketId(ptMqttContext);
    
    if(NULL != pu16PublishPacketAckIdentifier)
    {
        *pu16PublishPacketAckIdentifier = u16PublishPacketAckIdentifier;

        OPL_LOG_DEBG(AWSH, "packet ack id %d", *pu16PublishPacketAckIdentifier);
    }

    OPL_LOG_DEBG(AWSH, "Published payload: %.*s", ptMqttPublishPacket->payloadLength, ptMqttPublishPacket->pPayload);

    mqttStatus = MQTT_Publish( ptMqttContext,
                               ptMqttPublishPacket,
                               u16PublishPacketAckIdentifier);

    if(MQTTSuccess != mqttStatus)
    {
        OPL_LOG_DEBG(AWSH, "Failed to send PUBLISH packet to broker %s", MQTT_Status_strerror(mqttStatus));
        ret = -1;
    }
    else
    {
        OPL_LOG_DEBG(AWSH, "PUBLISH sent for topic %.*s to broker (Next Pack ID %d : %d)", ptMqttPublishPacket->topicNameLength, 
                                                                                           ptMqttPublishPacket->pTopicName, 
                                                                                           u16PublishPacketAckIdentifier,
                                                                                           *pu16PublishPacketAckIdentifier);

        // TODO: calling MQTT_ProcessLoop to process incoming publish echo
        // mqttStatus = processLoopWIthTimeout(ptMqttContext, MQTT_PROCESS_LOOP_TIMEOUT_MS);
    }

    return ret;
}

MQTTStatus_t AWS_MqttHelperProcessLoop( MQTTContext_t *ptMqttContext)
{
    MQTTStatus_t mqttStatus = MQTTSuccess;

    mqttStatus = MQTT_ReceiveLoop(ptMqttContext);

    return mqttStatus;
}
