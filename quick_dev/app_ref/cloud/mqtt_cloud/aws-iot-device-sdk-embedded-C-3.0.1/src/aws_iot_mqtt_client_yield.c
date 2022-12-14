/*
* Copyright 2015-2016 Amazon.com, Inc. or its affiliates. All Rights Reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License").
* You may not use this file except in compliance with the License.
* A copy of the License is located at
*
* http://aws.amazon.com/apache2.0
*
* or in the "license" file accompanying this file. This file is distributed
* on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
* express or implied. See the License for the specific language governing
* permissions and limitations under the License.
*/

// Based on Eclipse Paho.
/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander/Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

/**
 * @file aws_iot_mqtt_client_yield.c
 * @brief MQTT client yield API definitions
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "aws_iot_mqtt_client_common_internal.h"

#include "cloud_config.h"
#include "cloud_ctrl.h"

#ifdef BLEWIFI_ENHANCE_AWS
// #include "blewifi_ctrl.h"
#endif

#ifdef BLEWIFI_MQTT_PING_COUNT
uint32_t g_u32MqttPingCount = 0;
#endif

/**
  * This is for the case when the aws_iot_mqtt_internal_send_packet Fails.
  */
static void _aws_iot_mqtt_force_client_disconnect(AWS_IoT_Client *pClient) {
    // disable skip dtim
	Cloud_MqttSkipDtimSet(CLOUD_SKIP_DTIM_DISCONN, false);

    pClient->clientStatus.clientState = CLIENT_STATE_DISCONNECTED_ERROR;
    pClient->networkStack.disconnect(&(pClient->networkStack));
    pClient->networkStack.destroy(&(pClient->networkStack));

    // disable skip dtim
    osDelay(100);
	Cloud_MqttSkipDtimSet(CLOUD_SKIP_DTIM_DISCONN, true);
}

static IoT_Error_t _aws_iot_mqtt_handle_disconnect(AWS_IoT_Client *pClient) {
    IoT_Error_t rc;

    FUNC_ENTRY;

    rc = aws_iot_mqtt_disconnect(pClient);
    if(rc != SUCCESS) {
        // If the aws_iot_mqtt_internal_send_packet prevents us from sending a disconnect packet then we have to clean the stack
        _aws_iot_mqtt_force_client_disconnect(pClient);
    }

#if 1
    // restore the flags
    pClient->clientStatus.isRecvInProgress = 0;
    pClient->clientStatus.isWaitPubAck = 0;
    pClient->clientStatus.isWaitPingResp = 0;
#endif

    if(NULL != pClient->clientData.disconnectHandler) {
        pClient->clientData.disconnectHandler(pClient, pClient->clientData.disconnectHandlerData);
    }

    /* Reset to 0 since this was not a manual disconnect */
    pClient->clientStatus.clientState = CLIENT_STATE_DISCONNECTED_ERROR;
    FUNC_EXIT_RC(NETWORK_DISCONNECTED_ERROR);
}


static IoT_Error_t _aws_iot_mqtt_handle_reconnect(AWS_IoT_Client *pClient) {
    IoT_Error_t rc;

    FUNC_ENTRY;
    //IOT_INFO(" %s --> t expired  %d  %d", __FUNCTION__, pClient->reconnectDelayTimer.end_time.tv_sec, pClient->reconnectDelayTimer.end_time.tv_usec);
    if(!has_timer_expired(&(pClient->reconnectDelayTimer))) {
        /* Timer has not expired. Not time to attempt reconnect yet.
         * Return attempting reconnect */
        FUNC_EXIT_RC(NETWORK_ATTEMPTING_RECONNECT);
    }
   //IOT_INFO(" %s <-- t expired  %d  %d", __FUNCTION__, pClient->reconnectDelayTimer.end_time.tv_sec, pClient->reconnectDelayTimer.end_time.tv_usec);
    rc = NETWORK_PHYSICAL_LAYER_DISCONNECTED;
    if(NULL != pClient->networkStack.isConnected) {
        rc = pClient->networkStack.isConnected(&(pClient->networkStack));
    }

    if(NETWORK_PHYSICAL_LAYER_CONNECTED == rc) {
        rc = aws_iot_mqtt_attempt_reconnect(pClient);
        if(NETWORK_RECONNECTED == rc) {
            rc = aws_iot_mqtt_set_client_state(pClient, CLIENT_STATE_CONNECTED_IDLE,
                                               CLIENT_STATE_CONNECTED_YIELD_IN_PROGRESS);
            if(SUCCESS != rc) {
                FUNC_EXIT_RC(rc);
            }
            FUNC_EXIT_RC(NETWORK_RECONNECTED);
        }
    }

    pClient->clientData.currentReconnectWaitInterval *= 2;

    #ifdef BLEWIFI_ENHANCE_AWS
    if(AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL < pClient->clientData.currentReconnectWaitInterval) {
        pClient->clientData.currentReconnectWaitInterval = AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL;
    }
    #else
    if(AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL < pClient->clientData.currentReconnectWaitInterval) {
        FUNC_EXIT_RC(NETWORK_RECONNECT_TIMED_OUT_ERROR);
    }
    #endif
    
    countdown_ms(&(pClient->reconnectDelayTimer), pClient->clientData.currentReconnectWaitInterval);
    FUNC_EXIT_RC(rc);
}

#if 1
/**
 * @brief Send keep alive packet
 *
 * Called to send keep alive packet and return till received PINGRESP or error
 * (Maintain by opulinks)
 *
 * @param pClient Reference to the IoT Client
 *
 * @return An IoT Error Type defining successful/failed client processing.
 *         If this call results in an error it is likely the MQTT connection has dropped.
 *         iot_is_mqtt_connected can be called to confirm.
 */
IoT_Error_t aws_iot_mqtt_keep_alive(AWS_IoT_Client *pClient) {
    IoT_Error_t rc = SUCCESS;

#ifdef _ENABLE_THREAD_SUPPORT_
	IoT_Error_t threadRc;
#endif

    Timer timer;
    ClientState clientState = CLIENT_STATE_INVALID;
    size_t serialized_len;
    int nPublish = 40;

    FUNC_ENTRY;

    if(NULL == pClient) {
        FUNC_EXIT_RC(NULL_VALUE_ERROR);
    }

    if(0 == pClient->clientData.keepAliveInterval) {
        FUNC_EXIT_RC(SUCCESS);
    }

    // check client connection
    if(!aws_iot_mqtt_is_client_connected(pClient)) {
        FUNC_EXIT_RC(NETWORK_DISCONNECTED_ERROR);
    }

    //IOT_INFO(" %s --> t expired  %d  %d", __FUNCTION__, pClient->pingTimer.end_time.tv_sec,pClient->pingTimer.end_time.tv_usec);
    // if(!has_timer_expired(&pClient->pingTimer)) {
    //     FUNC_EXIT_RC(SUCCESS);
    // }
    //IOT_INFO(" %s <-- t expired  %d  %d", __FUNCTION__, pClient->pingTimer.end_time.tv_sec, pClient->pingTimer.end_time.tv_usec);

#if 1
    if(true == pClient->clientStatus.isWaitPingResp)
    {
        IOT_WARN("Still wait PINGRESP\r\n");
        rc = _aws_iot_mqtt_handle_disconnect(pClient);
        if(SUCCESS != rc)
        {
            FUNC_EXIT_RC(rc);
        }

        FUNC_EXIT_RC(MQTT_WAITING_PINGRESP);
    }
#else
    if(pClient->clientStatus.isPingOutstanding) {
        IOT_WARN("Last keep alive not finish");
        rc = _aws_iot_mqtt_handle_disconnect(pClient);
        FUNC_EXIT_RC(rc);
    }
#endif

    // wait till client state in connected idle or wait for cb return type
    while((CLIENT_STATE_CONNECTED_IDLE != clientState) && (CLIENT_STATE_CONNECTED_WAIT_FOR_CB_RETURN != clientState) )
    {
        clientState = aws_iot_mqtt_get_client_state(pClient);
        nPublish--;
        if(nPublish <= 0)
        {
			IOT_INFO("[%s]client state not in idle[%d]\r\n", __func__, clientState);
            FUNC_EXIT_RC(MQTT_CLIENT_NOT_IDLE_ERROR);
        }
        else
        {
            osDelay(100);
        }
    }

    /* Check if client is idle, if not another operation is in progress and we should return */
#if 0
    if(CLIENT_STATE_CONNECTED_IDLE != clientState && CLIENT_STATE_CONNECTED_WAIT_FOR_CB_RETURN != clientState) {
        IOT_INFO("Error %d : clientState --> %d \n",__LINE__, clientState);
        FUNC_EXIT_RC(MQTT_CLIENT_NOT_IDLE_ERROR);
    }

    rc = aws_iot_mqtt_set_client_state(pClient, clientState,
                                        CLIENT_STATE_CONNECTED_KEEPALIVE_IN_PROGRESS);
    if(SUCCESS != rc) {
        IOT_INFO("Set client state fail %d\n", __LINE__);
        FUNC_EXIT_RC(rc);
    }
#endif

    /* there is no ping outstanding - send one */
    init_timer(&timer);

    // countdown_ms(&timer, pClient->clientData.commandTimeoutMs);
    countdown_ms(&timer, pClient->clientData.keepAliveInterval);

    serialized_len = 0;
    rc = aws_iot_mqtt_internal_serialize_zero(pClient->clientData.writeBuf, pClient->clientData.writeBufSize,
                                              PINGREQ, &serialized_len);
    if(SUCCESS != rc) {
        goto done;
    }

    // disable skip dtim
    Cloud_TimerStop(CLOUD_TMR_KEEP_ALIVE_TIMEOUT);
    Cloud_TimerStop(CLOUD_TMR_KEEP_ALIVE_SKIP_DTIM_EN);
    Cloud_MqttSkipDtimSet(CLOUD_SKIP_DTIM_KEEPALIVE, false);

    /* send the ping packet */
    rc = aws_iot_mqtt_internal_send_packet(pClient, serialized_len, &timer);

    if(SUCCESS != rc) {

        // enable skip dtim
        Cloud_MqttSkipDtimSet(CLOUD_SKIP_DTIM_KEEPALIVE, true);

        //If sending a PING fails we can no longer determine if we are connected.  In this case we decide we are disconnected and begin reconnection attempts
        rc = _aws_iot_mqtt_handle_disconnect(pClient);

        goto done;
    }
    else
    {
        IOT_INFO("PING sending for keepalive\n");
        Cloud_TimerStart(CLOUD_TMR_KEEP_ALIVE_TIMEOUT, MQTT_COMMAND_TIMEOUT);
    }

#if 0
    // #ifdef BLEWIFI_MQTT_PING_COUNT
    // g_u32MqttPingCount += 1;
    // #else
    pClient->clientStatus.isPingOutstanding = true;
    // #endif
    
    /* start a timer to wait for PINGRESP from server */
    // countdown_sec(&pClient->pingTimer, pClient->clientData.keepAliveInterval);

    // rc = aws_iot_mqtt_internal_wait_for_read(pClient, PINGRESP, &pClient->pingTimer);
    rc = aws_iot_mqtt_internal_wait_for_read(pClient, PINGRESP, &timer);

    if(SUCCESS != rc) {
        goto done;
    }
#else

#ifdef _ENABLE_THREAD_SUPPORT_
    threadRc = aws_iot_mqtt_client_lock_mutex(pClient, &(pClient->clientData.wait_ping_resp_change_mutex));
    if(SUCCESS != threadRc) {
        FUNC_EXIT_RC(threadRc);
    }
#endif

    pClient->clientStatus.isWaitPingResp = true;

#ifdef _ENABLE_THREAD_SUPPORT_
    threadRc = aws_iot_mqtt_client_unlock_mutex(pClient, &(pClient->clientData.wait_ping_resp_change_mutex));
    if(SUCCESS != threadRc) {
        return threadRc;
    }
#endif

#endif

done:
#if 0
    // enable skip dtim
    Cloud_MqttSkipDtimSet(true);

    rc = aws_iot_mqtt_set_client_state(pClient, CLIENT_STATE_CONNECTED_KEEPALIVE_IN_PROGRESS, clientState);
	if(SUCCESS != rc) {
        IOT_INFO("Set client state fail %d\n", __LINE__);
	}
#endif

    FUNC_EXIT_RC(rc);
}
#else
#ifdef BLEWIFI_ENABLE_KEEPALIVE
static IoT_Error_t _aws_iot_mqtt_keep_alive(AWS_IoT_Client *pClient) {
    IoT_Error_t rc = SUCCESS;
    Timer timer;
    size_t serialized_len;

    FUNC_ENTRY;

    if(NULL == pClient) {
        FUNC_EXIT_RC(NULL_VALUE_ERROR);
    }

    if(0 == pClient->clientData.keepAliveInterval) {
        FUNC_EXIT_RC(SUCCESS);
    }
    //IOT_INFO(" %s --> t expired  %d  %d", __FUNCTION__, pClient->pingTimer.end_time.tv_sec,pClient->pingTimer.end_time.tv_usec);
    if(!has_timer_expired(&pClient->pingTimer)) {
        FUNC_EXIT_RC(SUCCESS);
    }
    //IOT_INFO(" %s <-- t expired  %d  %d", __FUNCTION__, pClient->pingTimer.end_time.tv_sec, pClient->pingTimer.end_time.tv_usec);

    #ifdef BLEWIFI_MQTT_PING_COUNT
    if(g_u32MqttPingCount >= BLEWIFI_MQTT_PING_COUNT_MAX) {
        rc = _aws_iot_mqtt_handle_disconnect(pClient);
        FUNC_EXIT_RC(rc);
    }
    #else
    if(pClient->clientStatus.isPingOutstanding) {
        rc = _aws_iot_mqtt_handle_disconnect(pClient);
        FUNC_EXIT_RC(rc);
    }
    #endif

    /* there is no ping outstanding - send one */
    init_timer(&timer);

    countdown_ms(&timer, pClient->clientData.commandTimeoutMs);
    serialized_len = 0;
    rc = aws_iot_mqtt_internal_serialize_zero(pClient->clientData.writeBuf, pClient->clientData.writeBufSize,
                                              PINGREQ, &serialized_len);
    if(SUCCESS != rc) {
        FUNC_EXIT_RC(rc);
    }

    /* send the ping packet */
    rc = aws_iot_mqtt_internal_send_packet(pClient, serialized_len, &timer);
    if(SUCCESS != rc) {
        //If sending a PING fails we can no longer determine if we are connected.  In this case we decide we are disconnected and begin reconnection attempts
        rc = _aws_iot_mqtt_handle_disconnect(pClient);
        FUNC_EXIT_RC(rc);
    }
    else
    {
        IOT_INFO("PING sending for keepalive\n");
    }

    #ifdef BLEWIFI_MQTT_PING_COUNT
    g_u32MqttPingCount += 1;
    #else
    pClient->clientStatus.isPingOutstanding = true;
    #endif
    
    /* start a timer to wait for PINGRESP from server */
    countdown_sec(&pClient->pingTimer, pClient->clientData.keepAliveInterval);

    FUNC_EXIT_RC(SUCCESS);
}
#endif
#endif

#if 1
IoT_Error_t aws_iot_mqtt_restore_keep_alive_skip_dtim(AWS_IoT_Client *pClient) {
    // enable skip dtim
    Cloud_MqttSkipDtimSet(CLOUD_SKIP_DTIM_KEEPALIVE, true);

    FUNC_EXIT_RC(SUCCESS);
}
#endif

/**
 * @brief Yield to the MQTT client
 *
 * Called to yield the current thread to the underlying MQTT client.  This time is used by
 * the MQTT client to manage PING requests to monitor the health of the TCP connection as
 * well as periodically check the socket receive buffer for subscribe messages.  Yield()
 * must be called at a rate faster than the keepalive interval.  It must also be called
 * at a rate faster than the incoming message rate as this is the only way the client receives
 * processing time to manage incoming messages.
 * This is the internal function which is called by the yield API to perform the operation.
 * Not meant to be called directly as it doesn't do validations or client state changes
 *
 * @param pClient Reference to the IoT Client
 * @param timeout_ms Maximum number of milliseconds to pass thread execution to the client.
 *
 * @return An IoT Error Type defining successful/failed client processing.
 *         If this call results in an error it is likely the MQTT connection has dropped.
 *         iot_is_mqtt_connected can be called to confirm.
 */
#if 1
static IoT_Error_t _aws_iot_mqtt_internal_yield(AWS_IoT_Client *pClient) {
#else
static IoT_Error_t _aws_iot_mqtt_internal_yield(AWS_IoT_Client *pClient, uint32_t timeout_ms) {
#endif
    IoT_Error_t yieldRc = SUCCESS;

// #ifdef _ENABLE_THREAD_SUPPORT_
// 	IoT_Error_t threadRc;
// #endif

    uint8_t packet_type;
    ClientState clientState;
    Timer timer;
    init_timer(&timer);
    countdown_ms(&timer, YIELD_TIMEOUT);

    FUNC_ENTRY;

    //IOT_INFO(" %s --> t expired  %d  %d", __FUNCTION__, timer.end_time.tv_sec, timer.end_time.tv_usec);
    // evaluate timeout at the end of the loop to make sure the actual yield runs at least once
#if 0
    do {
#endif
        clientState = aws_iot_mqtt_get_client_state(pClient);
        if(CLIENT_STATE_PENDING_RECONNECT == clientState) {
            if(AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL < pClient->clientData.currentReconnectWaitInterval) {
                yieldRc = NETWORK_RECONNECT_TIMED_OUT_ERROR;
#if 0
                break;
#else
                FUNC_EXIT_RC(yieldRc);
#endif
            }
            yieldRc = _aws_iot_mqtt_handle_reconnect(pClient);
            /* Network reconnect attempted, check if yield timer expired before
             * doing anything else */
#if 0
            continue;
#endif
        }

#if 1
        pClient->clientStatus.isRecvInProgress = true;
#endif

        yieldRc = aws_iot_mqtt_internal_cycle_read(pClient, &timer, &packet_type);

#if 1
        pClient->clientStatus.isRecvInProgress = false;
#endif

        if(SUCCESS == yieldRc) {
        #ifdef BLEWIFI_ENABLE_KEEPALIVE
            yieldRc = _aws_iot_mqtt_keep_alive(pClient);
        #else
            #ifdef BLEWIFI_POST_FAIL_COUNT
            extern uint32_t g_u32PostFailCnt;

            if(g_u32PostFailCnt >= BLEWIFI_POST_FAIL_COUNT_MAX)
            {
                printf("post_fail_cnt[%u]: _aws_iot_mqtt_handle_disconnect\n", g_u32PostFailCnt);
                yieldRc = _aws_iot_mqtt_handle_disconnect(pClient);
            }
            #endif
        #endif
        } else {
            // SSL read and write errors are terminal, connection must be closed and retried
            if(NETWORK_SSL_READ_ERROR == yieldRc || NETWORK_SSL_WRITE_ERROR == yieldRc || NETWORK_SSL_WRITE_TIMEOUT_ERROR == yieldRc) {
                yieldRc = _aws_iot_mqtt_handle_disconnect(pClient);
            }
        }

        if(NETWORK_DISCONNECTED_ERROR == yieldRc) {
            pClient->clientData.counterNetworkDisconnected++;
            if(1 == pClient->clientStatus.isAutoReconnectEnabled) {
                yieldRc = aws_iot_mqtt_set_client_state(pClient, CLIENT_STATE_DISCONNECTED_ERROR,
                                                        CLIENT_STATE_PENDING_RECONNECT);
                if(SUCCESS != yieldRc) {
                    FUNC_EXIT_RC(yieldRc);
                }

                pClient->clientData.currentReconnectWaitInterval = AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL;
                countdown_ms(&(pClient->reconnectDelayTimer), pClient->clientData.currentReconnectWaitInterval);
                /* Depending on timer values, it is possible that yield timer has expired
                 * Set to rc to attempting reconnect to inform client that autoreconnect
                 * attempt has started */
                yieldRc = NETWORK_ATTEMPTING_RECONNECT;
            } else {
#if 0
                break;
#endif
            }
        } else if(SUCCESS != yieldRc) {
#if 0
            break;
#endif
        }

#if 0
        osDelay(timeout_ms);
    } while(!has_timer_expired(&timer));
#endif
    //IOT_INFO(" %s <-- t expired  %d  %d", __FUNCTION__, timer.end_time.tv_sec, timer.end_time.tv_usec);
    FUNC_EXIT_RC(yieldRc);
}

/**
 * @brief Yield to the MQTT client
 *
 * Called to yield the current thread to the underlying MQTT client.  This time is used by
 * the MQTT client to manage PING requests to monitor the health of the TCP connection as
 * well as periodically check the socket receive buffer for subscribe messages.  Yield()
 * must be called at a rate faster than the keepalive interval.  It must also be called
 * at a rate faster than the incoming message rate as this is the only way the client receives
 * processing time to manage incoming messages.
 * This is the outer function which does the validations and calls the internal yield above
 * to perform the actual operation. It is also responsible for client state changes
 *
 * @param pClient Reference to the IoT Client
 * @param timeout_ms Maximum number of milliseconds to pass thread execution to the client.
 *
 * @return An IoT Error Type defining successful/failed client processing.
 *         If this call results in an error it is likely the MQTT connection has dropped.
 *         iot_is_mqtt_connected can be called to confirm.
 */
#if 1
IoT_Error_t aws_iot_mqtt_yield(AWS_IoT_Client *pClient) {
#else
IoT_Error_t aws_iot_mqtt_yield(AWS_IoT_Client *pClient, uint32_t timeout_ms) {
#endif

#if 0
    IoT_Error_t rc, yieldRc;
#else
    IoT_Error_t yieldRc;
#endif
    ClientState clientState;
    //int nPublish = 30000;

#if 1
    if(NULL == pClient)
    {
        IOT_INFO("Error %d  \n", __LINE__);
        FUNC_EXIT_RC(NULL_VALUE_ERROR);
    }
#else
    if(NULL == pClient || 0 == timeout_ms) {
        IOT_INFO("Error %d  \n",__LINE__);
        FUNC_EXIT_RC(NULL_VALUE_ERROR);
    }
#endif

    clientState = aws_iot_mqtt_get_client_state(pClient);
    /* Check if network was manually disconnected */
    if(CLIENT_STATE_DISCONNECTED_MANUALLY == clientState) {
        IOT_INFO("Error %d : clientState --> %d \n",__LINE__, clientState);
        FUNC_EXIT_RC(NETWORK_MANUALLY_DISCONNECTED);
    }

    /* If we are in the pending reconnect state, skip other checks.
     * Pending reconnect state is only set when auto-reconnect is enabled */
    if(CLIENT_STATE_PENDING_RECONNECT != clientState) {
        /* Check if network is disconnected and auto-reconnect is not enabled */
        if(!aws_iot_mqtt_is_client_connected(pClient)) {
            IOT_INFO("Error %d : clientState --> %d \n",__LINE__, clientState);
            FUNC_EXIT_RC(NETWORK_DISCONNECTED_ERROR);
        }

        #if 0
        while(clientState == CLIENT_STATE_CONNECTED_PUBLISH_IN_PROGRESS)
        {
            clientState = aws_iot_mqtt_get_client_state(pClient);
            nPublish = nPublish - 100;
            if(nPublish <= 0)
            {
                IOT_INFO(" %s clientState in CLIENT_STATE_CONNECTED_PUBLISH_IN_PROGRESS timeout \n",__LINE__);
                break;
            }
            else
            {
                osDelay(100);
            }
        }
        #endif


        /* Check if client is idle, if not another operation is in progress and we should return */
        if(CLIENT_STATE_CONNECTED_IDLE != clientState) {
            IOT_INFO("Error %d : clientState --> %d \n",__LINE__, clientState);
            FUNC_EXIT_RC(MQTT_CLIENT_NOT_IDLE_ERROR);
        }

#if 0
        rc = aws_iot_mqtt_set_client_state(pClient, CLIENT_STATE_CONNECTED_IDLE,
                                           CLIENT_STATE_CONNECTED_YIELD_IN_PROGRESS);
        if(SUCCESS != rc) {
            IOT_INFO("Error %d  \n",__LINE__);
            FUNC_EXIT_RC(rc);
        }
#endif
    }

#if 1
    yieldRc = _aws_iot_mqtt_internal_yield(pClient);
#else
    yieldRc = _aws_iot_mqtt_internal_yield(pClient, timeout_ms);
#endif

#if 0
    if(NETWORK_DISCONNECTED_ERROR != yieldRc && NETWORK_ATTEMPTING_RECONNECT != yieldRc) {
        rc = aws_iot_mqtt_set_client_state(pClient, CLIENT_STATE_CONNECTED_YIELD_IN_PROGRESS,
                                           CLIENT_STATE_CONNECTED_IDLE);
        if(SUCCESS == yieldRc && SUCCESS != rc) {
            yieldRc = rc;
        }
    }
#endif

    FUNC_EXIT_RC(yieldRc);
}

#ifdef __cplusplus
}
#endif

