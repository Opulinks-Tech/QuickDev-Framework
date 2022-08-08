/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

/**
 * @file subscribe_publish_sample.c
 * @brief simple MQTT publish and subscribe on the same topic
 *
 * This example takes the parameters from the aws_iot_config.h file and establishes a connection to the AWS IoT MQTT Platform.
 * It subscribes and publishes to the same topic - "sdkTest/sub"
 *
 * If all the certs are correct, you should see the messages received by the application in a loop.
 *
 * The application takes in the certificate path, host name , port and the number of times the publish should happen.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
//#include <unistd.h>
#include <limits.h>
#include <string.h>

#include "opl_aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "blewifi_ctrl.h"
#include "blewifi_wifi_api.h"
#include "ps_public.h"
#include "wifi_api.h"
#include "mw_fim_default_group14_project.h"
#include "Hal_vic.h"
#include "blewifi_configuration.h"
#include "sensor_https.h"
#include "sensor_data.h"

#include "cmsis_os.h"
#include "iot_data.h"
#include "blewifi_common.h"
#include "lwip/etharp.h"
#include "sensor_auxadc.h"
#include "sys_common_ctrl.h"
#include "mw_ota.h"
#include "util_func.h"


#define HOST_ADDRESS_SIZE          255
#define SUBSCRIBE_DOOR_TOPIC       "OPL/AWS/%02x%02x%02x/DOOR"
#define SUBSCRIBE_LIGHT_TOPIC      "OPL/AWS/%02x%02x%02x/LIGHT"

#define MQTT_CLIENT_ID_LEN         64
#define MQTT_USER_NAME_LEN         64
#define MQTT_PASSWD_LEN            64

char g_sMqttClientId[MQTT_CLIENT_ID_LEN] = {0};
char g_sMqttUserName[MQTT_USER_NAME_LEN] = {0};
char g_sMqttPasswd[MQTT_PASSWD_LEN] = {0};
char g_s8aFwVersion[32] = {0};
uint8_t g_u8aWifiMacAddr[6] = {0};
uint8_t g_u8aBleBdAddr[6] = {0};

volatile uint8_t g_u8AwsInit = 0;
volatile uint8_t g_u8MqttInitConn = 0;

T_MwFim_GP14_DS_KEY_INFO g_tDSKeyInfo;
T_MwFim_GP14_DS_MQTT_INFO g_tDSMQTTInfo;
//T_MwFim_GP14_AWS_PRIVATE_KEY g_tAWSDevicePrivateKeys;
//T_MwFim_GP14_AWS_CERT_PEM    g_tAWSDeviceCertPEM;

//extern osTimerId    g_tAppCtrlHttpPostTimer;
uint8_t g_nLightStatus = 0;

/**
 * @brief Default MQTT HOST URL is pulled from the aws_iot_config.h
 */
char HostAddress[HOST_ADDRESS_SIZE] = AWS_IOT_MQTT_HOST;

/**
 * @brief Default MQTT port is pulled from the aws_iot_config.h
 */
uint32_t port = AWS_IOT_MQTT_PORT;

/**
 * @brief This parameter will avoid infinite loop of publish and exit the program after certain number of publishes
 */
uint32_t publishCount = 0;

AWS_IoT_Client client;

#ifdef BLEWIFI_POST_FAIL_COUNT
uint32_t g_u32PostFailCnt = 0;
uint32_t g_u32IotDataTxTimeout = 0;
#endif


extern float g_fTemperature;


void iot_subscribe_callback_handler_Door(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                                IoT_Publish_Message_Params *params, void *pData) {
    IOT_UNUSED(pData);
    IOT_UNUSED(pClient);
    IOT_INFO("Subscribe Door callback");
    tracer_drct_printf("%.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *) params->payload);
}


void iot_subscribe_callback_handler_Light(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                                IoT_Publish_Message_Params *params, void *pData) {
    IOT_UNUSED(pData);
    IOT_UNUSED(pClient);
    IOT_INFO("Subscribe Light callback");
    BleWifi_Wifi_SetDTIM(0);

    if(strncmp((const char *) params->payload, "on", strlen("on")) == 0)
    {
        IOT_INFO("++ %.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *) params->payload);
        Hal_Vic_GpioOutput(LED_IO_PORT, GPIO_LEVEL_HIGH);
        g_nLightStatus = 1;
    }
    else if(strncmp((const char *) params->payload, "off", strlen("off")) == 0)
    {
        IOT_INFO("-- %.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *) params->payload);
        Hal_Vic_GpioOutput(LED_IO_PORT, GPIO_LEVEL_LOW);
        g_nLightStatus = 0;
    }
    else
    {
        IOT_INFO("%.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *) params->payload);
    }
}


void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data) {
    IOT_WARN("MQTT Disconnect");
    IoT_Error_t rc = FAILURE;

    if(NULL == pClient) {
        return;
    }

    IOT_UNUSED(data);

    if(aws_iot_is_autoreconnect_enabled(pClient)) {
        IOT_INFO("Auto Reconnect is enabled, Reconnecting attempt will start now");
    } else {
        IOT_WARN("Auto Reconnect not enabled. Starting manual reconnect...");
        rc = aws_iot_mqtt_attempt_reconnect(pClient);
        if(NETWORK_RECONNECTED == rc) {
            IOT_WARN("Manual Reconnect Successful");
        } else {
            IOT_WARN("Manual Reconnect Failed - %d", rc);
        }
    }
}

#if (IOT_DEVICE_DATA_TX_EN == 1)
int mqtt_main(int argc, char **argv) {

    IoT_Error_t rc = FAILURE;

    IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
    IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

    IOT_INFO("\nAWS IoT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

    if (MW_FIM_OK != MwFim_FileRead(MW_FIM_IDX_GP14_PROJECT_DS_KEY_INFO, 0, MW_FIM_GP14_DS_KEY_INFO_SIZE, (uint8_t*)&g_tDSKeyInfo))
    {
        IOT_INFO("\nMwFim Read DS_DEVICE_INFO Fail\n");
        // if fail, get the default value
        memcpy(&g_tDSKeyInfo.client_id, &g_tMwFimDefaultGp14DSKeyInfo.client_id, CLIENT_ID_PREFIX_SIZE);
        memcpy(&g_tDSKeyInfo.username, &g_tMwFimDefaultGp14DSKeyInfo.username, USERNAME_SIZE);
        memcpy(&g_tDSKeyInfo.password, &g_tMwFimDefaultGp14DSKeyInfo.password, PASSWORD_SIZE);
    }
    else
    {
        IOT_INFO("\nMwFim Read DS_DEVICE_INFO OK\n");
    }


    if (MW_FIM_OK != MwFim_FileRead(MW_FIM_IDX_GP14_PROJECT_DS_MQTT_INFO, 0, MW_FIM_GP14_DS_MQTT_INFO_SIZE, (uint8_t*)&g_tDSMQTTInfo))
    {
        IOT_INFO("\nMwFim Read DS_DEVICE_INFO Fail\n");
        // if fail, get the default value
        memcpy(&g_tDSMQTTInfo.host , &g_tMwFimDefaultGp16DSMQTTInfo.host , HOST_SIZE);
        g_tDSMQTTInfo.port = g_tMwFimDefaultGp16DSMQTTInfo.port;
        memcpy(&g_tDSMQTTInfo.topic , &g_tMwFimDefaultGp16DSMQTTInfo.topic , TOPIC_SIZE);
    }
    else
    {
        IOT_INFO("\nMwFim Read DS_DEVICE_INFO OK\n");
    }

#if 0
    memset(&g_tAWSDevicePrivateKeys, 0x00, sizeof(g_tAWSDevicePrivateKeys));


    //if (MW_FIM_OK != MwFim_FileRead(MW_FIM_IDX_GP13_PROJECT_AWS_PRIVATE_KEYS, 0, PRIVATE_KEY_SIZE, (uint8_t*)&temp_Pri))
    if (MW_FIM_OK != MwFim_FileRead(MW_FIM_IDX_GP14_PROJECT_AWS_PRIVATE_KEYS, 0, MW_FIM_GP14_AWS_PRIVATE_KEY_SIZE, (uint8_t*)(&g_tAWSDevicePrivateKeys)))
    {
        IOT_INFO("\nMwFim Read AWS_PRIVATE_KEYS Fail\n");
        // if fail, get the default value
        memcpy(g_tAWSDevicePrivateKeys.PrivateKey, g_tMwFimDefaultGp14AWSPrivateKey.PrivateKey, MW_FIM_GP14_AWS_PRIVATE_KEY_SIZE);
    }
    else
    {
        IOT_INFO("\nMwFim Read AWS_PRIVATE_KEYS OK\n");

    }

    memset(&g_tAWSDeviceCertPEM, 0x00, sizeof(g_tAWSDeviceCertPEM));

    //if (MW_FIM_OK != MwFim_FileRead(MW_FIM_IDX_GP13_PROJECT_AWS_CERT_PEM, 0, CERT_PEM_SIZE, (uint8_t*)&temp_cert))
    if (MW_FIM_OK != MwFim_FileRead(MW_FIM_IDX_GP14_PROJECT_AWS_CERT_PEM, 0, MW_FIM_GP14_AWS_CERT_PEM_SIZE, (uint8_t*)(&g_tAWSDeviceCertPEM)))

    {
        IOT_INFO("\nMwFim Read AWS_CERT_PEM Fail\n");
        // if fail, get the default value
        memcpy(g_tAWSDeviceCertPEM.CertPEM, g_tMwFimDefaultGp14AWSCertPEM.CertPEM, MW_FIM_GP14_AWS_CERT_PEM_SIZE);
    }
    else
    {
        IOT_INFO("\nMwFim Read AWS_CERT_PEM OK\n");

    }
#endif

    mqttInitParams.enableAutoReconnect = false; // We enable this later below
    mqttInitParams.pHostURL = g_tDSMQTTInfo.host;
    mqttInitParams.port = port;
    mqttInitParams.mqttCommandTimeout_ms = 5000;
    mqttInitParams.tlsHandshakeTimeout_ms = 20000;
#ifdef AWS_SENSOR_CFG_DS
    mqttInitParams.isSSLHostnameVerify = false;
#else
    mqttInitParams.isSSLHostnameVerify = true;
#endif
    mqttInitParams.disconnectHandler = disconnectCallbackHandler;
    mqttInitParams.disconnectHandlerData = NULL;

    uint8_t ubaMacAddr[6];

    // get the mac address from flash
    wifi_config_get_mac_address(WIFI_MODE_STA, ubaMacAddr);

    memset(SubscribeTopic_Door,0x00,sizeof(SubscribeTopic_Door));
    sprintf(SubscribeTopic_Door,SUBSCRIBE_DOOR_TOPIC,ubaMacAddr[3], ubaMacAddr[4], ubaMacAddr[5]);


    memset(SubscribeTopic_Light,0x00,sizeof(SubscribeTopic_Light));
    sprintf(SubscribeTopic_Light,SUBSCRIBE_LIGHT_TOPIC,ubaMacAddr[3], ubaMacAddr[4], ubaMacAddr[5]);

    rc = aws_iot_mqtt_init(&client, &mqttInitParams);
    if(SUCCESS != rc) {
        IOT_ERROR("aws_iot_mqtt_init returned error : %d ", rc);
        return rc;
    }

    connectParams.isCleanSession = true;
    connectParams.MQTTVersion = MQTT_3_1_1;
    connectParams.keepAliveIntervalInSec = 120;
    connectParams.pClientID = g_tDSKeyInfo.client_id;
    connectParams.clientIDLen = (uint16_t) strlen(g_tDSKeyInfo.client_id);
    connectParams.isWillMsgPresent = false;

    BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_IOT_INIT, false);

    IOT_INFO("%s connect to %s:%d", connectParams.pClientID, mqttInitParams.pHostURL, mqttInitParams.port)

    rc = aws_iot_mqtt_connect(&client, &connectParams);

    if(SUCCESS != rc) {
        IOT_ERROR("Err(%d) connect to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
        return rc;
    }

    #if 0
    /*
     * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
     *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
     *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
     */
    rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
    if(SUCCESS != rc) {
        IOT_ERROR("Unable to set Auto Reconnect to true - %d", rc);
        return rc;
    }
    #endif

    IOT_INFO("Subscribing Light topic : %s", SubscribeTopic_Light);
    rc = aws_iot_mqtt_subscribe(&client, SubscribeTopic_Light, strlen(SubscribeTopic_Light), QOS1, iot_subscribe_callback_handler_Light, NULL);
    if(SUCCESS != rc) {
        IOT_ERROR("Error subscribing Light : %d ", rc);
        return rc;
    }

     // init behavior
    BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_IOT_INIT, true);

    // When got ip then start timer to post data
    //osTimerStop(g_tAppCtrlHttpPostTimer);
    //osTimerStart(g_tAppCtrlHttpPostTimer, POST_DATA_TIME);

    BleWifi_Ctrl_EventStatusSet(CAN_DO_IOT_RX,true);

    while(NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc)
    {
        IOT_INFO("-->RX Loop");
        if (false == BleWifi_Ctrl_EventStatusWait(BLEWIFI_CTRL_EVENT_BIT_WIFI_GOT_IP, 0xFFFFFFFF))
        {
            return rc;
        }

        //Max time the yield function will wait for read messages
        if (true == BleWifi_Ctrl_EventStatusWait(CAN_DO_IOT_RX , YIELD_TIMEOUT))
        {
            BleWifi_Ctrl_EventStatusSet(CAN_DO_IOT_TX,false);
            IOT_INFO("-->RX yield");
            rc = aws_iot_mqtt_yield(&client, YIELD_TIMEOUT);
            IOT_INFO("<--RX yield");
            #if 0
            if(NETWORK_ATTEMPTING_RECONNECT == rc) {
                IOT_INFO("-->NETWORK_ATTEMPTING_RECONNECT");
                // If the client is attempting to reconnect we will skip the rest of the loop.
                continue;
            }
            #endif
            BleWifi_Ctrl_EventStatusSet(CAN_DO_IOT_TX,true);
        }
        else
        {
            IOT_INFO("--> CANNOT DO RX NOW");
            osDelay(1000);
        }

        if(SENSOR_DATA_OK == Sensor_Data_CheckEmpty())
        {
            BleWifi_Wifi_SetDTIM(YIELD_TIMEOUT);
        }
        else
        {
            IOT_INFO("-->TX data queue not empty");
            osDelay(100);
        }

        IOT_INFO("<--RX Loop");
    }

    // Wait for all the messages to be received
    aws_iot_mqtt_yield(&client, 100);
    IOT_ERROR("An error occurred in the loop. Error Code is %d \n", rc);

    BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_IOT_INIT, false);

    return rc;
}
#else //#if (IOT_DEVICE_DATA_TX_EN == 1)
uint32_t g_u32IotNextPostTimeSec = 0;

void iot_yeild_handle(AWS_IoT_Client *ptClient, IoT_Client_Init_Params *ptInitParam, IoT_Client_Connect_Params *ptConnParam)
{
    uint32_t u32TimeoutMs = IOT_DATA_YIELD_REPEAT_TIMEOUT;
    uint32_t u32YieldTimeout = IOT_DATA_YIELD_IDLE_TIMEOUT;
    uint32_t u32OrigState = CLIENT_STATE_INVALID;
    uint32_t u32CurrState = CLIENT_STATE_INVALID;
    IoT_Error_t tRet = FAILURE;
    uint32_t u32StartSec = 0;
    uint32_t u32StartMs = 0;
    uint32_t u32ProcSec = 3;

    osTimerStop(g_tAppIotDataRxTimerId);

    util_get_current_time(&u32StartSec, &u32StartMs);

    printf("current time [%u.%03u] sec, next_post_sec[%u]\n", u32StartSec, u32StartMs, g_u32IotNextPostTimeSec);

    u32OrigState = aws_iot_mqtt_get_client_state(ptClient);

    if(g_u8MqttInitConn)
    {
        if(u32OrigState != CLIENT_STATE_CONNECTED_IDLE)
        {
            u32ProcSec = BLEWIFI_CONNECT_TIMEOUT + 1;
        }
    }
    else
    {
        u32ProcSec = BLEWIFI_CONNECT_TIMEOUT + 2;
    }

    if((g_u32IotNextPostTimeSec >= u32StartSec) && (u32StartSec + u32ProcSec) >= g_u32IotNextPostTimeSec)
    {
        u32TimeoutMs = ((g_u32IotNextPostTimeSec - u32StartSec) + 2) * 1000;

        printf("post would be affected by yield_sec[%u]: defer yeild[%u]\n", u32ProcSec, u32TimeoutMs);

        osTimerStart(g_tAppIotDataRxTimerId, u32TimeoutMs);

        goto done;
    }

    if (true != BleWifi_Ctrl_EventStatusGet(BLEWIFI_CTRL_EVENT_BIT_WIFI_GOT_IP))
    {
        printf("GOT_IP is false\n");

        if(u32OrigState != CLIENT_STATE_CONNECTED_IDLE)
        {
            // do not restart rx timer when both wifi and mqtt are disconnected
            printf("state[%d]: suspend rx_yield_timer\n", u32OrigState);
            goto done;
        }
    }

    if(g_u8MqttInitConn)
    {
        uint8_t u8RestoreDtim = 0;

        if(u32OrigState == CLIENT_STATE_CONNECTED_IDLE)
        {
            //BleWifi_Wifi_SetDTIM(BleWifi_Ctrl_DtimTimeGet());
        }
        else
        {
            BleWifi_Wifi_SetDTIM(0);
            u8RestoreDtim = 1;
            lwip_one_shot_arp_enable();

            u32YieldTimeout = IOT_DATA_YIELD_RECONNECT_TIMEOUT;
        }

        tRet = aws_iot_mqtt_yield(&client, u32YieldTimeout);

        if(u8RestoreDtim)
        {
            BleWifi_Wifi_SetDTIM(BleWifi_Ctrl_DtimTimeGet());
        }

        IOT_INFO("<--RX yield[%d] timeout[%u]", tRet, u32YieldTimeout);
    }
    else
    {
        IoT_Error_t tRet = FAILURE;

        IOT_INFO("%s connect to %s:%d", ptConnParam->pClientID, ptInitParam->pHostURL, ptInitParam->port)

        BleWifi_Wifi_SetDTIM(0);

        tRet = aws_iot_mqtt_connect(ptClient, ptConnParam);

        BleWifi_Wifi_SetDTIM(BleWifi_Ctrl_DtimTimeGet());

        if(tRet == SUCCESS)
        {
            if(Sensor_Auxadc_Tmpr_Get(&g_fTemperature) != SENSOR_AUX_OK)
            {
                printf("Sensor_Auxadc_Tmpr_Get fail\n");
            }

             // init behavior
            BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_IOT_INIT, true);

            g_u8MqttInitConn = 1;

            printf("MqttInitConn[%u]\n", g_u8MqttInitConn);
        }
        else
        {
            IOT_ERROR("Err(%d) connect to %s:%d", tRet, ptInitParam->pHostURL, ptInitParam->port);
            lwip_one_shot_arp_enable();

            u32TimeoutMs = IOT_DATA_CONNECT_REPEAT_TIMEOUT;
        }
    }

    u32CurrState = aws_iot_mqtt_get_client_state(ptClient);

    if(u32CurrState == CLIENT_STATE_CONNECTED_IDLE)
    {
        if((u32OrigState != CLIENT_STATE_CONNECTED_IDLE) && (g_u32IotNextPostTimeSec))
        {
            // disconnect to connected: based on post interval, restart post as soon as possible

            uint32_t u32CurrSec = 0;

            util_get_current_time(&u32CurrSec, NULL);

            if(g_u32IotNextPostTimeSec > (u32CurrSec + 1))
            {
                uint32_t u32TxTimeoutMs = ((g_u32IotNextPostTimeSec - u32CurrSec) * 1000) % IOT_DATA_POST_REPEAT_TIMEOUT;

                osTimerStop(g_tAppIotDataTxTimerId);

                printf("restart TX timer[%u]\n", u32TxTimeoutMs);

                osTimerStart(g_tAppIotDataTxTimerId, u32TxTimeoutMs);
            }
        }
    }
    else
    {
        if(u32OrigState == CLIENT_STATE_CONNECTED_IDLE)
        {
            // connected to disconnect

            printf("state[%u] to [%u]: prepare to reconnect\n", u32OrigState, u32CurrState);
            u32TimeoutMs = 1000;
        }

        printf("restart RX timer[%u]\n", u32TimeoutMs);

        osTimerStart(g_tAppIotDataRxTimerId, u32TimeoutMs);
    }

done:
    return;
}

#ifdef BLEWIFI_POST_FAIL_COUNT
void iot_post_handle(AWS_IoT_Client *ptClient)
{
    uint32_t u32OrigState = CLIENT_STATE_INVALID;
    uint8_t u8PostDone = 0;
    uint32_t u32StartSec = 0;
    uint32_t u32StartMs = 0;
    uint32_t u32EndSec = 0;
    uint32_t u32EndMs = 0;
    uint32_t u32DiffMs = IOT_DATA_POST_REPEAT_TIMEOUT;
    uint32_t u32DurMs = 0;
    uint32_t u32TimeoutMs = 0;
    uint32_t u32AdvMs = IOT_DATA_POST_ADV_TIME;

    osTimerStop(g_tAppIotDataTxTimerId);

    if (true != BleWifi_Ctrl_EventStatusGet(BLEWIFI_CTRL_EVENT_BIT_WIFI_GOT_IP))
    {
        printf("GOT_IP is false\n");
        goto done;
    }

    util_get_current_time(&u32StartSec, &u32StartMs);

    printf("current time [%u.%03u] sec\n", u32StartSec, u32StartMs);

    u32OrigState = aws_iot_mqtt_get_client_state(ptClient);

    if(u32OrigState == CLIENT_STATE_CONNECTED_IDLE)
    {
        if(g_u32PostFailCnt < BLEWIFI_POST_FAIL_COUNT_MAX)
        {
            BleWifi_Wifi_SetDTIM(BLEWIFI_WIFI_DTIM_INTERVAL_WORK_MODE);

            if(!Sensor_Post_Data()) // one_shot_arp might be set in Sensor_Post_Data().
            {
                u8PostDone = 1;
            }

            BleWifi_Wifi_SetDTIM(BleWifi_Ctrl_DtimTimeGet());

            if(u8PostDone)
            {
                BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_CLOUD_POST_DATA_SUCCESS, true);
                BleWifi_Ctrl_LedStatusChange(0);
                BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_CLOUD_POST_DATA_SUCCESS, false);

                g_u32PostFailCnt = 0;
                printf("[%s %d] set post_fail_cnt[%u]\n", __func__, __LINE__, g_u32PostFailCnt);
            }
            else
            {
                BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_CLOUD_POST_DATA_FAIL, true);
                BleWifi_Ctrl_LedStatusChange(0);
                BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_CLOUD_POST_DATA_FAIL, false);

                g_u32PostFailCnt += 1;
                printf("[%s %d] set post_fail_cnt[%u]\n", __func__, __LINE__, g_u32PostFailCnt);
            }

            if(g_u32PostFailCnt >= BLEWIFI_POST_FAIL_COUNT_MAX)
            {
                uint32_t u32RxTimeoutMs = 1000;

                osTimerStop(g_tAppIotDataRxTimerId);

                printf("restart RX timer[%u]\n", u32RxTimeoutMs);

                osTimerStart(g_tAppIotDataRxTimerId, u32RxTimeoutMs);
            }
        }
        else
        {
            BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_CLOUD_SERVER_OFFLINE, true);
            BleWifi_Ctrl_LedStatusChange(0);
            BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_CLOUD_SERVER_OFFLINE, false);
        }

        if(g_u32PostFailCnt >= BLEWIFI_POST_FAIL_COUNT_MAX)
        {
            printf("[%s %d] post_fail_cnt[%u]: ignore data post\n", __func__, __LINE__, g_u32PostFailCnt);
            u32DiffMs = IOT_DATA_POST_RETRY_TIMEOUT;
        }
    }
    else
    {
        printf("[%s %d] state[%u] is Not connect_idle: ignore data post\n", __func__, __LINE__, u32OrigState);

        BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_CLOUD_SERVER_OFFLINE, true);
        BleWifi_Ctrl_LedStatusChange(0);
        BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_CLOUD_SERVER_OFFLINE, false);

        u32DiffMs = IOT_DATA_POST_RETRY_TIMEOUT;
    }

    util_get_current_time(&u32EndSec, &u32EndMs);

    u32DurMs = (u32EndSec - u32StartSec) * 1000 + (u32EndMs - u32StartMs);

    if(u32DiffMs > IOT_DATA_POST_REPEAT_TIMEOUT)
    {
        u32AdvMs = (u32AdvMs * u32DiffMs) / IOT_DATA_POST_REPEAT_TIMEOUT;
    }

    if(u32DiffMs > (u32DurMs + u32AdvMs))
    {
        u32TimeoutMs = u32DiffMs - (u32DurMs + u32AdvMs);
    }
    else
    {
        u32TimeoutMs = 1;
    }

    g_u32IotNextPostTimeSec = u32StartSec + ((u32DiffMs + u32StartMs) / 1000);

    printf("restart TX timer[%u], next_post_sec[%u]\n", u32TimeoutMs, g_u32IotNextPostTimeSec);

    osTimerStart(g_tAppIotDataTxTimerId, u32TimeoutMs);

done:
    return;
}
#else //#ifdef BLEWIFI_POST_FAIL_COUNT
void iot_post_handle(AWS_IoT_Client *ptClient)
{
    uint32_t u32OrigState = CLIENT_STATE_INVALID;
    uint8_t u8PostDone = 0;
    uint32_t u32StartSec = 0;
    uint32_t u32StartMs = 0;
    uint32_t u32EndSec = 0;
    uint32_t u32EndMs = 0;
    uint32_t u32DiffMs = IOT_DATA_POST_REPEAT_TIMEOUT;
    uint32_t u32DurMs = 0;
    uint32_t u32TimeoutMs = 0;
    uint32_t u32AdvMs = 2;

    osTimerStop(g_tAppIotDataTxTimerId);

    if (true != BleWifi_Ctrl_EventStatusGet(BLEWIFI_CTRL_EVENT_BIT_WIFI_GOT_IP))
    {
        printf("GOT_IP is false\n");
        goto done;
    }

    util_get_current_time(&u32StartSec, &u32StartMs);

    u32OrigState = aws_iot_mqtt_get_client_state(ptClient);

    if(u32OrigState == CLIENT_STATE_CONNECTED_IDLE)
    {
        BleWifi_Wifi_SetDTIM(0);

        if(!Sensor_Post_Data()) // one_shot_arp would be set in Sensor_Post_Data().
        {
            u8PostDone = 1;
        }

        BleWifi_Wifi_SetDTIM(BleWifi_Ctrl_DtimTimeGet());

        if(u8PostDone)
        {
            BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_CLOUD_POST_DATA_SUCCESS, true);
            BleWifi_Ctrl_LedStatusChange(0);
            BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_CLOUD_POST_DATA_SUCCESS, false);
        }
        else
        {
            BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_CLOUD_POST_DATA_FAIL, true);
            BleWifi_Ctrl_LedStatusChange(0);
            BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_CLOUD_POST_DATA_FAIL, false);
        }
    }
    else
    {
        printf("[%s %d] state[%u] is Not connect_idle: ignore data post\n", __func__, __LINE__, u32OrigState);

        BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_CLOUD_SERVER_OFFLINE, true);
        BleWifi_Ctrl_LedStatusChange(0);
        BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_CLOUD_SERVER_OFFLINE, false);
    }

    util_get_current_time(&u32EndSec, &u32EndMs);

    u32DurMs = (u32EndSec - u32StartSec) * 1000 + (u32EndMs - u32StartMs);

    if(u32DiffMs > (u32DurMs + u32AdvMs))
    {
        u32TimeoutMs = u32DiffMs - (u32DurMs + u32AdvMs);
    }
    else
    {
        u32TimeoutMs = 1;
    }

    g_u32IotNextPostTimeSec = u32StartSec + ((u32DiffMs + u32StartMs) / 1000);

    printf("restart TX timer[%u]\n", u32TimeoutMs);

    osTimerStart(g_tAppIotDataTxTimerId, u32TimeoutMs);

done:
    return;
}
#endif //#ifdef BLEWIFI_POST_FAIL_COUNT

int mqtt_main(int argc, char **argv) {

    IoT_Error_t rc = FAILURE;

    IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
    IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;
    uint16_t u16ProjectId = 0;
    uint16_t u16ChipId = 0;
    uint16_t u16FirmwareId = 0;

    //IOT_INFO("\nAWS IoT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);
    IOT_INFO("\nAWS IoT SDK Version %d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

    if (MW_FIM_OK != MwFim_FileRead(MW_FIM_IDX_GP14_PROJECT_DS_KEY_INFO, 0, MW_FIM_GP14_DS_KEY_INFO_SIZE, (uint8_t*)&g_tDSKeyInfo))
    {
        IOT_INFO("\nMwFim Read DS_DEVICE_INFO Fail\n");
        // if fail, get the default value
        memcpy(&g_tDSKeyInfo.client_id_prefix, &g_tMwFimDefaultGp14DSKeyInfo.client_id_prefix, CLIENT_ID_PREFIX_SIZE);
        memcpy(&g_tDSKeyInfo.username, &g_tMwFimDefaultGp14DSKeyInfo.username, USERNAME_SIZE);
        memcpy(&g_tDSKeyInfo.password, &g_tMwFimDefaultGp14DSKeyInfo.password, PASSWORD_SIZE);
    }
    else
    {
        IOT_INFO("\nMwFim Read DS_DEVICE_INFO OK\n");
    }

    if (MW_FIM_OK != MwFim_FileRead(MW_FIM_IDX_GP14_PROJECT_DS_MQTT_INFO, 0, MW_FIM_GP14_DS_MQTT_INFO_SIZE, (uint8_t*)&g_tDSMQTTInfo))
    {
        IOT_INFO("\nMwFim Read DS_DEVICE_INFO Fail\n");
        // if fail, get the default value
        memcpy(&g_tDSMQTTInfo.host , &g_tMwFimDefaultGp14DSMQTTInfo.host , HOST_SIZE);
        g_tDSMQTTInfo.port = g_tMwFimDefaultGp14DSMQTTInfo.port;
        memcpy(&g_tDSMQTTInfo.topic , &g_tMwFimDefaultGp14DSMQTTInfo.topic , TOPIC_SIZE);
    }
    else
    {
        IOT_INFO("\nMwFim Read DS_DEVICE_INFO OK\n");
    }

#if 0
    memset(&g_tAWSDevicePrivateKeys, 0x00, sizeof(g_tAWSDevicePrivateKeys));


    //if (MW_FIM_OK != MwFim_FileRead(MW_FIM_IDX_GP13_PROJECT_AWS_PRIVATE_KEYS, 0, PRIVATE_KEY_SIZE, (uint8_t*)&temp_Pri))
    if (MW_FIM_OK != MwFim_FileRead(MW_FIM_IDX_GP14_PROJECT_AWS_PRIVATE_KEYS, 0, MW_FIM_GP14_AWS_PRIVATE_KEY_SIZE, (uint8_t*)(&g_tAWSDevicePrivateKeys)))
    {
        IOT_INFO("\nMwFim Read AWS_PRIVATE_KEYS Fail\n");
        // if fail, get the default value
        memcpy(g_tAWSDevicePrivateKeys.PrivateKey, g_tMwFimDefaultGp14AWSPrivateKey.PrivateKey, MW_FIM_GP14_AWS_PRIVATE_KEY_SIZE);
    }
    else
    {
        //IOT_INFO("\nMwFim Read AWS_PRIVATE_KEYS OK\n");
    }

    memset(&g_tAWSDeviceCertPEM, 0x00, sizeof(g_tAWSDeviceCertPEM));

    //if (MW_FIM_OK != MwFim_FileRead(MW_FIM_IDX_GP13_PROJECT_AWS_CERT_PEM, 0, CERT_PEM_SIZE, (uint8_t*)&temp_cert))
    if (MW_FIM_OK != MwFim_FileRead(MW_FIM_IDX_GP14_PROJECT_AWS_CERT_PEM, 0, MW_FIM_GP14_AWS_CERT_PEM_SIZE, (uint8_t*)(&g_tAWSDeviceCertPEM)))

    {
        IOT_INFO("\nMwFim Read AWS_CERT_PEM Fail\n");
        // if fail, get the default value
        memcpy(g_tAWSDeviceCertPEM.CertPEM, g_tMwFimDefaultGp14AWSCertPEM.CertPEM, MW_FIM_GP14_AWS_CERT_PEM_SIZE);
    }
    else
    {
        //IOT_INFO("\nMwFim Read AWS_CERT_PEM OK\n");

    }
#endif
    mqttInitParams.enableAutoReconnect = true;

    mqttInitParams.pHostURL = g_tDSMQTTInfo.host;
    mqttInitParams.port = g_tDSMQTTInfo.port;
    mqttInitParams.mqttCommandTimeout_ms = 5000;
    mqttInitParams.tlsHandshakeTimeout_ms = 20000;

#ifdef AWS_SENSOR_CFG_DS
    //unnecessary to verify server's certificate
    mqttInitParams.isSSLHostnameVerify = false;
#else
    mqttInitParams.isSSLHostnameVerify = true;
#endif

    mqttInitParams.disconnectHandler = disconnectCallbackHandler;
    mqttInitParams.disconnectHandlerData = NULL;

    rc = aws_iot_mqtt_init(&client, &mqttInitParams);

    if(SUCCESS != rc) {
        IOT_ERROR("aws_iot_mqtt_init returned error : %d ", rc);
        return rc;
    }

    MwOta_VersionGet(&u16ProjectId, &u16ChipId, &u16FirmwareId);
    snprintf(g_s8aFwVersion, sizeof(g_s8aFwVersion), "%u.%u.%u", u16ProjectId, u16ChipId, u16FirmwareId);

    wifi_config_get_mac_address(WIFI_MODE_STA, g_u8aWifiMacAddr);
    get_ble_bd_addr(g_u8aBleBdAddr);

    snprintf(g_sMqttClientId, sizeof(g_sMqttClientId), "%s%02X%02X%02X%02X%02X%02X",
             g_tDSKeyInfo.client_id_prefix,
             g_u8aWifiMacAddr[0], g_u8aWifiMacAddr[1], g_u8aWifiMacAddr[2],
             g_u8aWifiMacAddr[3], g_u8aWifiMacAddr[4], g_u8aWifiMacAddr[5]);

    snprintf(g_sMqttUserName, sizeof(g_sMqttUserName), g_tDSKeyInfo.username);
    snprintf(g_sMqttPasswd, sizeof(g_sMqttPasswd), g_tDSKeyInfo.password);

    #ifdef BLEWIFI_ENHANCE_AWS
    connectParams.keepAliveIntervalInSec = BLEWIFI_MQTT_PING_INTERVAL;
    #else
    connectParams.keepAliveIntervalInSec = 120;
    #endif

    connectParams.isCleanSession = true;
    connectParams.MQTTVersion = MQTT_3_1_1;

    connectParams.pClientID = g_sMqttClientId;
    connectParams.clientIDLen = (uint16_t)strlen(g_sMqttClientId);

    connectParams.pUsername = g_sMqttUserName;
    connectParams.usernameLen = (uint16_t)strlen(g_sMqttUserName);

    connectParams.pPassword = g_sMqttPasswd;
    connectParams.passwordLen = (uint16_t)strlen(g_sMqttPasswd);

    connectParams.isWillMsgPresent = false;

    BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_IOT_INIT, false);

    while(1)
    {
        osEvent rxEvent;
        xIotDataMessage_t *rxMsg = NULL;

        /* Wait event */
        rxEvent = osMessageGet(g_tAppIotDataRxQueueId, osWaitForever);

        if(rxEvent.status != osEventMessage)
        {
            continue;
        }

        IOT_INFO("-->RX Loop");

        rxMsg = (xIotDataMessage_t *)rxEvent.value.p;

        if (true == BleWifi_Ctrl_EventStatusGet(BLEWIFI_CTRL_EVENT_BIT_OTA))
        {
            BleWifi_Wifi_SetDTIM(0);

            printf("OTA is running: defer event[%u]\n", rxMsg->event);

            osDelay(5000);
            Iot_Data_Rx_MsgSend(rxMsg->event, NULL, 0);
            goto done;
        }

        if(rxMsg->event == IOT_DATA_RX_MSG_RESTART)
        {
            printf("RESTART\n");

            #ifdef BLEWIFI_POST_FAIL_COUNT
            g_u32PostFailCnt = 0;
            #endif

            iot_yeild_handle(&client, &mqttInitParams, &connectParams);

            iot_post_handle(&client);
        }
        else if(rxMsg->event == IOT_DATA_RX_MSG_MQTT_YIELD)
        {
            printf("MQTT_YIELD\n");

            iot_yeild_handle(&client, &mqttInitParams, &connectParams);
        }
        else if(rxMsg->event == IOT_DATA_RX_MSG_DATA_POST)
        {
            printf("DATA_POST\n");

            iot_post_handle(&client);
        }
        else if(rxMsg->event == IOT_DATA_RX_MSG_DATA_RECONNECT)
        {
            if (MW_FIM_OK != MwFim_FileRead(MW_FIM_IDX_GP14_PROJECT_DS_KEY_INFO, 0, MW_FIM_GP14_DS_KEY_INFO_SIZE, (uint8_t*)&g_tDSKeyInfo))
            {
                IOT_INFO("\nMwFim Read DS_DEVICE_INFO Fail\n");
                // if fail, get the default value
                memcpy(&g_tDSKeyInfo.client_id_prefix, &g_tMwFimDefaultGp14DSKeyInfo.client_id_prefix, CLIENT_ID_PREFIX_SIZE);
                memcpy(&g_tDSKeyInfo.username, &g_tMwFimDefaultGp14DSKeyInfo.username, USERNAME_SIZE);
                memcpy(&g_tDSKeyInfo.password, &g_tMwFimDefaultGp14DSKeyInfo.password, PASSWORD_SIZE);
            }
            else
            {
                IOT_INFO("\nMwFim Read DS_DEVICE_INFO OK\n");
            }

            if (MW_FIM_OK != MwFim_FileRead(MW_FIM_IDX_GP14_PROJECT_DS_MQTT_INFO, 0, MW_FIM_GP14_DS_MQTT_INFO_SIZE, (uint8_t*)&g_tDSMQTTInfo))
            {
                IOT_INFO("\nMwFim Read DS_DEVICE_INFO Fail\n");
                // if fail, get the default value
                memcpy(&g_tDSMQTTInfo.host , &g_tMwFimDefaultGp14DSMQTTInfo.host , HOST_SIZE);
                g_tDSMQTTInfo.port = g_tMwFimDefaultGp14DSMQTTInfo.port;
                memcpy(&g_tDSMQTTInfo.topic , &g_tMwFimDefaultGp14DSMQTTInfo.topic , TOPIC_SIZE);
            }
            else
            {
                IOT_INFO("\nMwFim Read DS_DEVICE_INFO OK\n");
            }

            snprintf(g_sMqttClientId, sizeof(g_sMqttClientId), "%s%02X%02X%02X%02X%02X%02X",
                     g_tDSKeyInfo.client_id_prefix,
                     g_u8aWifiMacAddr[0], g_u8aWifiMacAddr[1], g_u8aWifiMacAddr[2],
                     g_u8aWifiMacAddr[3], g_u8aWifiMacAddr[4], g_u8aWifiMacAddr[5]);

            snprintf(g_sMqttUserName, sizeof(g_sMqttUserName), g_tDSKeyInfo.username);
            snprintf(g_sMqttPasswd, sizeof(g_sMqttPasswd), g_tDSKeyInfo.password);

            connectParams.pClientID = g_sMqttClientId;
            connectParams.clientIDLen = (uint16_t)strlen(g_sMqttClientId);

            connectParams.pUsername = g_sMqttUserName;
            connectParams.usernameLen = (uint16_t)strlen(g_sMqttUserName);

            connectParams.pPassword = g_sMqttPasswd;
            connectParams.passwordLen = (uint16_t)strlen(g_sMqttPasswd);

            g_u32PostFailCnt = BLEWIFI_POST_FAIL_COUNT_MAX;
            aws_iot_mqtt_set_connect_params(&client , &connectParams);
            iot_yeild_handle(&client, &mqttInitParams, &connectParams);
        }

    done:
        /* Release buffer */
        if (rxMsg != NULL)
        {
            free(rxMsg);
        }

        IOT_INFO("<--RX Loop\n");
    }
}
#endif //#if (IOT_DEVICE_DATA_TX_EN == 1)
