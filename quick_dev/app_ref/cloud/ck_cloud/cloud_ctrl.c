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
#include "cloud_data.h"
#include "cloud_config.h"
#include "cloud_ctrl.h"
#include "cloud_kernel.h"
#include "cloud_http.h"
#if defined(MAGIC_LED)
#include "app_main.h"
#include "ck_svc.h"
#include "ck_data_ptcl.h"
#include "cloud_cmd_data.h"
#include "cbc_encrypt\cbc_encrypt.h"
#endif
#include "coolkit_websocket.h"
#include "etharp.h"
#include "evt_group.h"
#include "hal_system.h"
#include "wifi_mngr_api.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#define CLOUD_CONNECT_RETRY_INTERVAL_TBL_NUM            (6)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

extern httpclient_t g_client;

// Queue priority g_stCloudRspQ > g_stKeepAliveQ > g_stIotRbData
T_CloudRingBuf g_stCloudRspQ = {0};
T_CloudRingBuf g_stKeepAliveQ = {0};
T_CloudRingBuf g_stIotRbData = {0};

uint8_t g_u8WaitingRspType = IOT_DATA_WAITING_TYPE_NONE;
uint8_t g_u8PostRetry_KeepAlive_Cnt = 0;
uint8_t g_u8PostRetry_IotRbData_Cnt = 0;
uint8_t g_u8CloudRetryIntervalIdx = 0;
uint8_t g_u8PostRetry_KeepAlive_Fail_Round = 0;

uint16_t g_u16IotDtimTxUse;
uint16_t g_u16IotDtimTxCloudAckPost;
uint16_t g_u16IotDtimWaitRspAckPost;

EventGroupHandle_t g_tIotDataEventGroup;

#if defined(MAGIC_LED)
SHM_DATA char g_szBodyFmt[BUFFER_SIZE]={0};
#endif

// Sec 5: declaration of global function prototype

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

uint32_t g_u32aCloudRetryIntervalTbl[CLOUD_CONNECT_RETRY_INTERVAL_TBL_NUM] =
{
    0,
    30000,
    30000,
    60000,
    60000,
    600000,
};

// timer group
static osTimerId g_tCloudTimer[CLOUD_TMR_MAX] = {NULL};

// Sec 7: declaration of static function prototype

static void Cloud_DataPost(void);
static void Cloud_WaitRxRspTimeoutHandler(void);
static void Cloud_AckPostTimeoutHandler(void);
static void Cloud_RspTcpAckTimeoutHandler(void);
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
*   Cloud_ConnectRetry
*
* DESCRIPTION:
*   retry connection
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_ConnectRetry(void)
{
    if(0 == g_u32aCloudRetryIntervalTbl[g_u8CloudRetryIntervalIdx])
    {
        Cloud_MsgSend(CLOUD_EVT_TYPE_ESTABLISH, NULL, 0);
    }
    else
    {
        Cloud_TimerStart(CLOUD_TMR_CONN_RETRY, g_u32aCloudRetryIntervalTbl[g_u8CloudRetryIntervalIdx]);
    }

    OPL_LOG_INFO(CLOUD, "Cloud retry wait %u", g_u32aCloudRetryIntervalTbl[g_u8CloudRetryIntervalIdx]);

    if(g_u8CloudRetryIntervalIdx < CLOUD_CONNECT_RETRY_INTERVAL_TBL_NUM - 1)
    {
        g_u8CloudRetryIntervalIdx++;
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


/*************************************************************************
* FUNCTION:
*   Cloud_DataPost
*
* DESCRIPTION:
*   post data to cloud
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
#if defined(MAGIC_LED)
#ifndef MAXLIAO
static void Iot_Data_CloudDataPost(void)
{
        //data from ring buffer
    T_CloudRingBufData IoT_Properity;
    uint32_t ulTotalBodyLen=0;
    unsigned char iv[IV_SIZE + 1] = {0};
    unsigned char ucCbcEncData[BUFFER_SIZE];
    size_t uCbcEncLen = 0;
//    unsigned char ucBaseData[512];
    unsigned char *ucBaseData;
    size_t uBaseLen = 0;
    
//    if (IOT_RB_DATA_OK == IoT_Ring_Buffer_CheckEmpty(&g_stIotRbData))
//    {   
//        BleWifi_Wifi_SetDTIM(BleWifi_Ctrl_DtimTimeGet());
//        return;
//    }
    
    // get the IoT data here, or the data are from *data and len parameters.
    if (OPL_OK == Cloud_RingBufPop(&g_stIotRbData, &IoT_Properity))
    // if (IOT_RB_DATA_OK == IoT_Ring_Buffer_Pop(&g_stIotRbData, &IoT_Properity))
    {
        
        //printf("IoT_Properity.ubData[0]=%d\n", IoT_Properity.ubData[0]);    
        uint8_t u8aStatusBuf[BUFFER_SIZE]={0};
        uint32_t u32StatusBufLen = 0;
        assemble_post_property(&u8aStatusBuf[0], &u32StatusBufLen, IoT_Properity.pu8Data[0]);
        printf("----------\nPayload for Send:(%d)\n----------\n%s\n", u32StatusBufLen, u8aStatusBuf);  
        
        //IR command needs to post status via BLE and WiFi
        //BLE Command needs to post status via WiFi
        if(true == APP_BleStatusGet())
        // if( true == BleWifi_Ctrl_EventStatusGet(BLEWIFI_CTRL_EVENT_BIT_BLE_CONNECTED)) 
        {
            // if(CHK_BIT(IoT_Properity.ubData[0], LOCAL_CTRL) || CHK_BIT(IoT_Properity.ubData[0], CLOUD_CTRL))
            if(CHK_BIT(IoT_Properity.pu8Data[0], LOCAL_CTRL) || CHK_BIT(IoT_Properity.pu8Data[0], CLOUD_CTRL))
            {                 
    //            printf("[ATS]Resp LightStatus via BLE\n");
                memset(iv, '0' , IV_SIZE); //iv = "0000000000000000"
                uCbcEncLen = u32StatusBufLen;
                uCbcEncLen = (((uCbcEncLen  >> 4) + 1) << 4);
                _CK_DataWifiCbcEncrypt((void *)&u8aStatusBuf[0], u32StatusBufLen, iv, g_ucSecretKey, (void *)ucCbcEncData);
                // BleWifi_CBC_encrypt((void *)&u8aStatusBuf[0], u32StatusBufLen, iv, g_ucSecretKey, (void *)ucCbcEncData);

                int sSize = ((4 * uCbcEncLen / 3) + 3) & ~3;
                ucBaseData = malloc(sSize+1);

                if(ucBaseData == NULL)
                {
                    return;
                }
                mbedtls_base64_encode((unsigned char *)ucBaseData, sSize+1, &uBaseLen ,(unsigned char *)ucCbcEncData, uCbcEncLen);
                                             
                //            BleWifi_HexDump("Secret Key", (unsigned char*)g_ucSecretKey, strlen((char*)g_ucSecretKey));
                //            BleWifi_HexDump("Enc Data", (unsigned char*)ucBaseData, strlen((char*)ucBaseData));

                printf("[ATS]BLE Send data success(%llu)\r\n", g_u64WaitSeqId);            

                CK_DataSendEncap(CK_DATA_RES_PROTOCOL_CMD, ucBaseData, uBaseLen);
                // BleWifi_Ble_DataSendEncap(BLEWIFI_RES_CK_PROTOCOL_CMD, ucBaseData, uBaseLen);

                free(ucBaseData);
            }
            
        } //end of if( true == BleWifi_Ctrl_EventStatusGet(BLEWIFI_CTRL_EVENT_BIT_BLE_CONNECTED))
        
        
        
        // send the data to cloud
        if(true == Cloud_OnlineStatusGet())
        // if(true==BleWifi_Ctrl_EventStatusGet(BLEWIFI_CTRL_EVENT_BIT_WSS_CONN))
        {
            if(CHK_BIT(IoT_Properity.pu8Data[0], LOCAL_CTRL) || CHK_BIT(IoT_Properity.pu8Data[0], BLE_CTRL))
            {                
                memset(g_szBodyFmt, 0, sizeof(g_szBodyFmt));
                            
                //printf("----------\nPayload for WS Send:(%d)\n----------\n%s\n", u32StatusBufLen, u8aStatusBuf);
                printf("[ATS]WIFI Send data start(%llu)\r\n", g_u64WaitSeqId);
                
                ws_encode(g_szBodyFmt, &ulTotalBodyLen, (char *)&u8aStatusBuf[0], u32StatusBufLen);  // build the websocket data packet with header and encrypt key
                // WS_build_data_packet(g_szBodyFmt, &ulTotalBodyLen, (char *)&u8aStatusBuf[0], u32StatusBufLen);  // build the websocket data packet with header and encrypt key
                
                ws_sem_lock(osWaitForever);
                if(g_tx_ID!=g_tcp_hdl_ID)
                {
                    g_tx_ID = g_tcp_hdl_ID;
                }
                ws_sem_unlock();
                
                // BleWifi_Wifi_SetDTIM(0);
                Opl_Wifi_Skip_Dtim_Set(g_u16IotDtimTxUse, false);
                
                #ifdef AUTO_POST_TEST
                ATS_POST_SET_TIME(start_time)                
                #endif
                
                int ret = ws_write(g_szBodyFmt, ulTotalBodyLen);
                // int ret = http_write(g_szBodyFmt, ulTotalBodyLen);                               
                    
                if(ret>0){
    //                printf("[ATS]WIFI Send data success\r\n");
    #if 0                
                    if(IoT_Properity.ubData!=NULL)
                        free(IoT_Properity.ubData);
                    IoT_Ring_Buffer_ReadIdxUpdate();
                    Iot_Data_TxTask_MsgSend(IOT_DATA_TX_MSG_DATA_POST,NULL, 0);
    #endif                
                }
                else
                {
                    printf("[ATS]WIFI Send data fail\r\n");
                    #ifdef AUTO_POST_TEST
                    ATS_POST_INC(post_data.fail_count)
                    ATS_PRINT_ONE_LOG
                    ATS_CLEAN_FAIL_STATS
                    #endif
                    

                    if(IoT_Properity.pu8Data!=NULL)
                        free(IoT_Properity.pu8Data);

                    Cloud_RingBufReadIdxUpdate(&g_stIotRbData);
                    // IoT_Ring_Buffer_ReadIdxUpdate(&g_stIotRbData);
                    
                    ws_sem_lock(osWaitForever);                    

                    Cloud_TimerStop(CLOUD_TMR_REQ_DATE);

                    if((g_tcp_hdl_ID!=-1)
                        &&(g_tx_ID == g_tcp_hdl_ID)
                        && (true == Cloud_OnlineStatusGet()))
                        // && (true == EG_StatusGet(g_tIotDataEventGroup , IOT_DATA_EVENT_BIT_CLOUD_CONNECTED)))
                    {
                        ret = ws_close();
                        printf("[ATS]Cloud disconnect\r\n");
                        printf("wt: ws_close(ret=%d)\n", ret);
                        g_tx_ID = -1;
                        g_tcp_hdl_ID = -1;

                        Cloud_OnlineStatusSet(false);
                        // EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_CLOUD_CONNECTED, false);
                        Cloud_ConnectRetry();
                        ws_sem_unlock();
                    }
                    else
                    {
                        ws_sem_unlock();
                    }

                    // osTimerStop(g_tAppHeartbeatTimerId);
                    // osTimerStop(g_tmr_req_date);
    //                 if((g_tcp_hdl_ID!=-1)&&(g_tx_ID == g_tcp_hdl_ID))
    //                 {
    //                     ret=http_close();
    //                     printf("wt: http_close(ret=%d)\n", ret);
    //                     g_tx_ID = -1;
    //                     g_tcp_hdl_ID = -1;
    //                     ws_sem_unlock(); 
    //                     BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_WSS_CONN, false);
    // //                    Iot_Data_TxTask_MsgSend(IOT_DATA_TX_MSG_EST_COOLKIT_HTTP_CONNECTION,NULL, 0);
    //                     Iot_Data_TxTask_MsgSend(IOT_DATA_TX_MSG_EST_COOLKIT_WSS_CONNECTION,NULL, 0);
    //                     return;
    //                 }
    //                 else
    //                 {
    //                    ws_sem_unlock(); 
    //                 }                        

                    Opl_Wifi_Skip_Dtim_Set(g_u16IotDtimTxUse, true);
                    // BleWifi_Wifi_SetDTIM(BleWifi_Ctrl_DtimTimeGet());
                    
                }
            }                                   
        }//end of if(true==BleWifi_Ctrl_EventStatusGet(BLEWIFI_CTRL_EVENT_BIT_WSS_CONN))
            
        if(IoT_Properity.pu8Data!=NULL)
            free(IoT_Properity.pu8Data);

        Cloud_RingBufReadIdxUpdate(&g_stIotRbData);
        // IoT_Ring_Buffer_ReadIdxUpdate(&g_stIotRbData);

        Cloud_MsgSend(CLOUD_EVT_TYPE_POST, NULL, 0);
        // Iot_Data_TxTask_MsgSend(IOT_DATA_TX_MSG_DATA_POST,NULL, 0);
            
    }//end of if (IOT_RB_DATA_OK == IoT_Ring_Buffer_Pop(&IoT_Properity))   
}
#else
static void Iot_Data_CloudDataPost(void)
{
    //data from ring buffer
    T_CloudRingBufData IoT_Properity;
    
//    if (IOT_RB_DATA_OK == IoT_Ring_Buffer_CheckEmpty(&g_stIotRbData))
//    {   
//        BleWifi_Wifi_SetDTIM(BleWifi_Ctrl_DtimTimeGet());
//        return;
//    }
    
    // get the IoT data here, or the data are from *data and len parameters.
    if (OPL_OK == Cloud_RingBufPop(&g_stIotRbData, &IoT_Properity))
    {
        //printf("IoT_Properity.ubData[0]=%d\n", IoT_Properity.ubData[0]);    
        uint8_t u8aStatusBuf[BUFFER_SIZE]={0};
        uint32_t u32StatusBufLen = 0;
        assemble_post_property(&u8aStatusBuf[0], &u32StatusBufLen, IoT_Properity.pu8Data[0]);

        printf("----------\nPayload for Send:(%d)\n----------\n%s\n", u32StatusBufLen, u8aStatusBuf);  
        
        //IR command needs to post status via BLE and WiFi
        //BLE Command needs to post status via WiFi
        if(true == APP_BleStatusGet())
        {
            if(CHK_BIT(IoT_Properity.pu8Data[0], LOCAL_CTRL) || CHK_BIT(IoT_Properity.pu8Data[0], CLOUD_CTRL))
            {
                CK_DataSendProtocolCmd(u8aStatusBuf, u32StatusBufLen);
            }
        } //end of if( true == BleWifi_Ctrl_EventStatusGet(BLEWIFI_CTRL_EVENT_BIT_BLE_CONNECTED))

        // send the data to cloud
        if(true == Cloud_OnlineStatusGet())
        {
            if(CHK_BIT(IoT_Properity.pu8Data[0], LOCAL_CTRL) || CHK_BIT(IoT_Properity.pu8Data[0], BLE_CTRL))
            {
                Coolkit_Cloud_Send(u8aStatusBuf, u32StatusBufLen);
            }                                   
        }//end of if(true==BleWifi_Ctrl_EventStatusGet(BLEWIFI_CTRL_EVENT_BIT_WSS_CONN))
            
        // if(IoT_Properity.ubData!=NULL)
        //     free(IoT_Properity.ubData);
    
        // IoT_Ring_Buffer_ReadIdxUpdate(&g_stIotRbData);

        // Iot_Data_TxTask_MsgSend(IOT_DATA_TX_MSG_DATA_POST,NULL, 0);

        //5. free the tx data from ring buffer
        // if ((u32Ret == IOT_DATA_POST_RET_CONTINUE_DELETE) || (u32Ret == IOT_DATA_POST_RET_STOP_DELETE))
        {
            Cloud_RingBufReadIdxUpdate(&g_stIotRbData);

            if (IoT_Properity.pu8Data != NULL)
            {
                free(IoT_Properity.pu8Data);
            }
        }

        //6. trigger the next Tx data post
        // if ((u32Ret == IOT_DATA_POST_RET_CONTINUE_DELETE) || (u32Ret == IOT_DATA_POST_RET_CONTINUE_KEEP))
        {
            Cloud_MsgSend(CLOUD_EVT_TYPE_POST, NULL, 0);
        }
            
    }//end of if (IOT_RB_DATA_OK == IoT_Ring_Buffer_Pop(&IoT_Properity))   
}
#endif
#elif defined(DOOR_SENSOR)
static void Cloud_DataPost(void)
{
    T_CloudRingBufData tProperity = {0};
    uint32_t u32Ret;

    // send the data to cloud

    //1. add check if cloud connection or not
    if (true == Cloud_NetworkStatusGet() &&
        true == Cloud_OnlineStatusGet())
    {
        //2. check ringbuffer is empty or not, get data from ring buffer
        if (true == Cloud_RingBufCheckEmpty(&g_stIotRbData))
            return;

        if (OPL_OK != Cloud_RingBufPop(&g_stIotRbData, &tProperity))
            return;

        // check need waiting rx rsp
        if (true == EG_StatusGet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_WAITING_RX_RSP))
            return;

        Opl_Wifi_Skip_Dtim_Set(g_u16IotDtimTxUse, false);

        lwip_one_shot_arp_enable();

        //3. contruct post data
        //4. send to Cloud
        u32Ret = Cloud_ConstructPostDataAndSend(&tProperity);

        //5. free the tx data from ring buffer
        if ((u32Ret == IOT_DATA_POST_RET_CONTINUE_DELETE) || (u32Ret == IOT_DATA_POST_RET_STOP_DELETE))
        {
            Cloud_RingBufReadIdxUpdate(&g_stIotRbData);

            if (tProperity.pu8Data != NULL)
            {
                free(tProperity.pu8Data);
            }
        }

        //6. trigger the next Tx data post
        if ((u32Ret == IOT_DATA_POST_RET_CONTINUE_DELETE) || (u32Ret == IOT_DATA_POST_RET_CONTINUE_KEEP))
        {
            Cloud_MsgSend(CLOUD_EVT_TYPE_POST, NULL, 0);
        }
    }
}
#endif

/*************************************************************************
* FUNCTION:
*   Cloud_WaitRxRspTimeoutHandler
*
* DESCRIPTION:
*   wait rx response timeout handle
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
#if defined(MAGIC_LED)
static void Cloud_WaitRxRspTimeoutHandler(void)
{
    printf("Wait Rx Timeout\r\n");

    //timeout
    EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_WAITING_RX_RSP, false);

    if(IOT_DATA_WAITING_TYPE_KEEPALIVE == g_u8WaitingRspType)
    {
        T_CloudRingBufData ptProperity;

        if(false == Cloud_RingBufCheckEmpty(&g_stKeepAliveQ))
        // if (IOT_RB_DATA_OK != IoT_Ring_Buffer_CheckEmpty(&g_stKeepAliveQ))
        {
            Cloud_RingBufPop(&g_stKeepAliveQ, &ptProperity);
            Cloud_RingBufReadIdxUpdate(&g_stKeepAliveQ);
            // IoT_Ring_Buffer_Pop(&g_stKeepAliveQ , &ptProperity);
            // IoT_Ring_Buffer_ReadIdxUpdate(&g_stKeepAliveQ);

            if(ptProperity.pu8Data!=NULL)
                free(ptProperity.pu8Data);
        }
    }

    g_u8WaitingRspType = IOT_DATA_WAITING_TYPE_NONE;

    Cloud_MsgSend(CLOUD_EVT_TYPE_POST, NULL, 0);
    // Iot_Data_TxTask_MsgSend(IOT_DATA_TX_MSG_DATA_POST, NULL, 0);

    Opl_Wifi_Skip_Dtim_Set(g_u16IotDtimTxUse, true);
    // BleWifi_Wifi_SetDTIM(BleWifi_Ctrl_DtimTimeGet());
}
#elif defined(DOOR_SENSOR)
static void Cloud_WaitRxRspTimeoutHandler(void)
{
    T_CloudRingBufData ptProperity = {0};
    uint16_t u16QueueCount = 0;
    uint8_t u8Discard = false;

    // check need waiting rx rsp
    if (true == EG_StatusGet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_WAITING_RX_RSP))
    {
        // OPL_LOG_WARN(CLOUD, "Wait Rx Timeout");

        //Jeff App_Ctrl_MsgSend(APP_CTRL_MSG_DATA_POST_FAIL , NULL , 0);

        //timeout
        EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_WAITING_RX_RSP, false);

        if(IOT_DATA_WAITING_TYPE_KEEPALIVE == g_u8WaitingRspType)
        {
            ws_sem_lock(osWaitForever);
            g_u8PostRetry_KeepAlive_Cnt ++;
            ws_sem_unlock();

            if(g_u8PostRetry_KeepAlive_Cnt >= IOT_DATA_KEEP_ALIVE_RETRY_MAX)
            {
                ws_sem_lock(osWaitForever);
                g_u8PostRetry_KeepAlive_Fail_Round++;
                ws_sem_unlock();

                if (false == Cloud_RingBufCheckEmpty(&g_stKeepAliveQ))
                {
                    Cloud_RingBufPop(&g_stKeepAliveQ , &ptProperity);
                    Cloud_RingBufReadIdxUpdate(&g_stKeepAliveQ);

                    if(ptProperity.pu8Data!=NULL)
                    {
                        free(ptProperity.pu8Data);
                    }
                }
                
                ws_sem_lock(osWaitForever);
                g_u8PostRetry_KeepAlive_Cnt = 0;
                ws_sem_unlock();

                if(g_u8PostRetry_KeepAlive_Fail_Round >= IOT_DATA_KEEP_ALIVE_FAIL_ROUND_MAX)
                {
                    // OPL_LOG_INFO(CLOUD, "keep alive fail round >= %u , cloud disconnect", IOT_DATA_KEEP_ALIVE_FAIL_ROUND_MAX);

                    ws_sem_lock(osWaitForever);
                    g_u8PostRetry_KeepAlive_Fail_Round = 0; //reset

                    if(true == Cloud_OnlineStatusGet())
                    {
                        // OPL_LOG_INFO(CLOUD, "disconnect by self");
                        Cloud_MsgSend(CLOUD_EVT_TYPE_DISCONNECT, NULL, 0);
                        Cloud_MsgSend(CLOUD_EVT_TYPE_ESTABLISH, NULL, 0);
                    }
                    ws_sem_unlock();
                }
            }
        }
        else if(IOT_DATA_WAITING_TYPE_DATA_POST == g_u8WaitingRspType)
        {
            ws_sem_lock(osWaitForever);
            g_u8PostRetry_IotRbData_Cnt ++;
            ws_sem_unlock();

            Cloud_RingBufGetQueueCount(&g_stIotRbData , &u16QueueCount);

            // OPL_LOG_INFO(CLOUD, "post retry cnt = %u", g_u8PostRetry_IotRbData_Cnt);
            // OPL_LOG_INFO(CLOUD, "WIFI Send data fail(%llu)", g_msgid);

            if(IOT_DATA_QUEUE_LAST_DATA_CNT == u16QueueCount) // last data
            {
#if 0
                if(g_u8PostRetry_IotRbData_Cnt == IOT_DATA_POST_RETRY_MAX)
                {
                    ws_sem_lock(osWaitForever);
    //                EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_POST_FAIL_RECONNECT, true);
    //                if(true == Cloud_OnlineStatusGet())
    //                // if (true == EG_StatusGet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_CLOUD_CONNECTED))
    //                {
    //                    printf("disconnect by self\r\n");
    //                    Iot_Data_TxTask_MsgSend(IOT_DATA_TX_MSG_CLOUD_DISCONNECTION, NULL, 0);
    //                    Iot_Data_TxTask_MsgSend(IOT_DATA_TX_MSG_CLOUD_CONNECTION, NULL, 0);
    //                }
                    ws_sem_unlock();
                }
                else
#endif
                if(g_u8PostRetry_IotRbData_Cnt >= IOT_DATA_LAST_DATA_POST_RETRY_MAX)
                {
                    // if the last post is fail, set the flag to retry it after keep alive
                    EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_LAST_POST_RETRY, true);

                    u8Discard = true;
                    // OPL_LOG_INFO(CLOUD, "post cnt = %u discard", g_u8PostRetry_IotRbData_Cnt);

                    if (false == Cloud_RingBufCheckEmpty(&g_stIotRbData))
                    {
                        Cloud_RingBufPop(&g_stIotRbData , &ptProperity);
                        Cloud_RingBufReadIdxUpdate(&g_stIotRbData);

                        if(ptProperity.pu8Data!=NULL)
                        {
                            free(ptProperity.pu8Data);
                        }
                    }
                    ws_sem_lock(osWaitForever);
                    g_u8PostRetry_IotRbData_Cnt = 0;
                    ws_sem_unlock();
                }

                if(false == u8Discard)
                {
                    // OPL_LOG_INFO(CLOUD, "post cnt = %u continues", g_u8PostRetry_IotRbData_Cnt);
                }
            }
            else //not last data
            {
                if(g_u8PostRetry_IotRbData_Cnt >= IOT_DATA_POST_RETRY_MAX)
                {
                    u8Discard = true;
                    // OPL_LOG_INFO(CLOUD,"post cnt = %u discard", g_u8PostRetry_IotRbData_Cnt);

                    if (false == Cloud_RingBufCheckEmpty(&g_stIotRbData))
                    {
                        Cloud_RingBufPop(&g_stIotRbData , &ptProperity);
                        Cloud_RingBufReadIdxUpdate(&g_stIotRbData);

                        if(ptProperity.pu8Data!=NULL)
                        {
                            free(ptProperity.pu8Data);
                        }
                    }
                    ws_sem_lock(osWaitForever);
                    g_u8PostRetry_IotRbData_Cnt = 0;
    //                EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_POST_FAIL_RECONNECT, true);
    //                if(true == Cloud_OnlineStatusGet())
    //                // if (true == EG_StatusGet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_CLOUD_CONNECTED))
    //                {
    //                    printf("disconnect by self\r\n");
    //                    Iot_Data_TxTask_MsgSend(IOT_DATA_TX_MSG_CLOUD_DISCONNECTION, NULL, 0);
    //                    Iot_Data_TxTask_MsgSend(IOT_DATA_TX_MSG_CLOUD_CONNECTION, NULL, 0);
    //                }
                    ws_sem_unlock();
                }

                if(false == u8Discard)
                {
                    // OPL_LOG_INFO(CLOUD, "post cnt = %u continues", g_u8PostRetry_IotRbData_Cnt);
                }
            }
        }
    }

    ws_sem_lock(osWaitForever);
    g_u8WaitingRspType = IOT_DATA_WAITING_TYPE_NONE;
    ws_sem_unlock();

    // Iot_Data_TxTask_MsgSend(IOT_DATA_TX_MSG_CLOUD_POST, NULL, 0);
    Cloud_MsgSend(CLOUD_EVT_TYPE_POST, NULL, 0);

    Opl_Wifi_Skip_Dtim_Set(g_u16IotDtimTxUse, true);
}
#endif

/*************************************************************************
* FUNCTION:
*   Cloud_AckPostTimeoutHandler
*
* DESCRIPTION:
*   ack post timeout handle
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static void Cloud_AckPostTimeoutHandler(void)
{
    Opl_Wifi_Skip_Dtim_Set(g_u16IotDtimTxCloudAckPost, true);
}

/*************************************************************************
* FUNCTION:
*   Cloud_RspTcpAckTimeoutHandler
*
* DESCRIPTION:
*   response tcp ack timeout handle
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static void Cloud_RspTcpAckTimeoutHandler(void)
{
    Opl_Wifi_Skip_Dtim_Set(g_u16IotDtimWaitRspAckPost, true);

    EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_WAITING_TCP_ACK, false);

    // Iot_Data_TxTask_MsgSend(IOT_DTA_TX_MSG_CLOUD_POST, NULL, 0);
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
    // initiate websocket
    ws_init();

    // initiate skip dtim module
    Opl_Wifi_Skip_Dtim_Module_Reg(&g_u16IotDtimTxUse);
    Opl_Wifi_Skip_Dtim_Module_Reg(&g_u16IotDtimTxCloudAckPost);
    Opl_Wifi_Skip_Dtim_Module_Reg(&g_u16IotDtimWaitRspAckPost);

    osTimerDef_t timer_def;

    // create timers
    timer_def.ptimer = Cloud_TimeoutCallback;
    for(uint32_t i = 0; i < CLOUD_TMR_MAX; i ++)
    {
        g_tCloudTimer[i] = osTimerCreate(&timer_def, osTimerOnce, (void *)i);
        
        if(NULL == g_tCloudTimer[i])
        {
            // OPL_LOG_ERRO(CLOUD, "Create cloud timer [%d] fail", i);
        }
    }

    // craete event group
    if(OPL_OK != EG_Create(&g_tIotDataEventGroup))
    {
        // OPL_LOG_ERRO(CLOUD, "Create event group fail");
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
#if defined(MAGIC_LED)
SHM_DATA void Cloud_EstablishHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
#else
void Cloud_EstablishHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
#endif
{
    // OPL_LOG_INFO(CLOUD, "Cloud Connection");
    int ret = 0;
    uint8_t u8Discard = false;

    if(true == Cloud_NetworkStatusGet())
    {
        if(false == Cloud_OnlineStatusGet())
        {
            // OPL_LOG_INFO(CLOUD, "Cloud Init");
            OPL_LOG_INFO(CLOUD, "Cloud connecting");

            // 1. establish connection
            // 2. determine the connect status
            ret = Connect_coolkit_http();

            if( 0 == ret)
            {
                ret = Connect_coolkit_wss();

                if(ret == COOLKIT_REG_SUCCESS)
                {
                    OPL_LOG_INFO(CLOUD, "Cloud connected success");
                    //reg success
                }
                else if(ret == COOLKIT_NOT_REG)
                {
                    //device No Register, return to idle
                    // OPL_LOG_WARN(CLOUD, "Cloud Reg fail");
                    OPL_LOG_WARN(CLOUD, "Cloud connected fail");
                }
                else
                {
                    OPL_LOG_WARN(CLOUD, "Cloud connected fail");
                    goto fail;
                }
            }
            else
            {
                OPL_LOG_WARN(CLOUD, "Cloud connected fail");
                goto fail;
            }
        }
    }

    //return to idle
    return;

fail:
    if (true == EG_StatusGet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_POST_FAIL_RECONNECT))
    {
        uint16_t u16QueueCount = 0;
        T_CloudRingBufData ptProperity = {0};

        Cloud_RingBufGetQueueCount(&g_stIotRbData , &u16QueueCount);

        ws_sem_lock(osWaitForever);
        g_u8PostRetry_IotRbData_Cnt ++;
        ws_sem_unlock();

        if(IOT_DATA_QUEUE_LAST_DATA_CNT == u16QueueCount) // last data
        {
            if(g_u8PostRetry_IotRbData_Cnt >= IOT_DATA_LAST_DATA_POST_RETRY_MAX)
            {
                u8Discard = true;
                // OPL_LOG_INFO(CLOUD, "post cnt = %u discard", g_u8PostRetry_IotRbData_Cnt);

                if (false == Cloud_RingBufCheckEmpty(&g_stIotRbData))
                {
                    Cloud_RingBufPop(&g_stIotRbData , &ptProperity);
                    Cloud_RingBufReadIdxUpdate(&g_stIotRbData);

                    if(ptProperity.pu8Data!=NULL)
                    {
                        free(ptProperity.pu8Data);
                    }
                }
                ws_sem_lock(osWaitForever);
                g_u8PostRetry_IotRbData_Cnt = 0;
                ws_sem_unlock();

                EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_POST_FAIL_RECONNECT, false);

                Cloud_ConnectRetry(); // do normal connectin retry

            }
            else
            {
                osDelay(100);

                Cloud_MsgSend(CLOUD_EVT_TYPE_ESTABLISH, NULL, 0); // connect immdiately
            }

            if(false == u8Discard)
            {
                // OPL_LOG_INFO(CLOUD, "post cnt = %u continues", g_u8PostRetry_IotRbData_Cnt);
            }
        }
        else //not last data
        {
            if(g_u8PostRetry_IotRbData_Cnt >= IOT_DATA_POST_RETRY_MAX)
            {
                u8Discard = true;
                // OPL_LOG_INFO(CLOUD, "post cnt = %u discard", g_u8PostRetry_IotRbData_Cnt);

                if (false == Cloud_RingBufCheckEmpty(&g_stIotRbData))
                {
                    Cloud_RingBufPop(&g_stIotRbData , &ptProperity);
                    Cloud_RingBufReadIdxUpdate(&g_stIotRbData);

                    if(ptProperity.pu8Data!=NULL)
                    {
                        free(ptProperity.pu8Data);
                    }
                }
                ws_sem_lock(osWaitForever);
                g_u8PostRetry_IotRbData_Cnt = 0;
                ws_sem_unlock();
            }

            osDelay(100);

            Cloud_MsgSend(CLOUD_EVT_TYPE_ESTABLISH, NULL, 0); // connect immediately

            if(false == u8Discard)
            {
                // OPL_LOG_INFO(CLOUD, "post cnt = %u continues", g_u8PostRetry_IotRbData_Cnt);
            }
        }
    }
    else
    {
        Cloud_ConnectRetry();
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
#if defined(MAGIC_LED)
    ws_close();

    Cloud_OnlineStatusSet(false);
#else
    ws_sem_lock(osWaitForever);

    Cloud_TimerStop(CLOUD_TMR_REQ_DATE);

    if (true == EG_StatusGet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_WAITING_RX_RSP))
    {
        if (IOT_DATA_WAITING_TYPE_DATA_POST == g_u8WaitingRspType)
        {
            // OPL_LOG_WARN(CLOUD, "WIFI Send data fail(%llu)", g_msgid);
        }
    }

    Cloud_TimerStop(CLOUD_TMR_WAIT_RX_RSP);

    g_u8WaitingRspType = IOT_DATA_WAITING_TYPE_NONE;

    EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_WAITING_RX_RSP, false);

    if ((g_tcp_hdl_ID!=-1)
        && (true == Cloud_OnlineStatusGet()))
        // && (true == EG_StatusGet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_CLOUD_CONNECTED)))
    {
        // 1. close connection
        int Res = ws_close();
        OPL_LOG_INFO(CLOUD, "Cloud disconnect");
        OPL_LOG_INFO(CLOUD, "rd: ws_close(ret=%d)", Res);

        g_tx_ID = -1;
        g_rx_ID = -1;
        g_tcp_hdl_ID = -1;

        // 2. change the connection status
        Cloud_OnlineStatusSet(false);

        ws_sem_unlock();
    }
    else
    {
        ws_sem_unlock();
    }
#endif
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

    // user implement
    // *
    switch(tCloudTimerIdx)
    {
        case CLOUD_TMR_CONN_RETRY:
            Cloud_EstablishHandler(u32EventId, pData, u32DataLen); // connect immediately
            break;

        case CLOUD_TMR_WAIT_RX_RSP:
            Cloud_WaitRxRspTimeoutHandler();
            break;
        
        case CLOUD_TMR_ACK_POST:
            Cloud_AckPostTimeoutHandler();
            break;

        case CLOUD_TMR_RSP_TCP_ACK:
            Cloud_RspTcpAckTimeoutHandler();
            break;

        case CLOUD_TMR_REQ_DATE:
            Cloud_MsgSend(CLOUD_EVT_TYPE_KEEP_ALIVE, NULL, 0);
            // Cloud_KeepAliveHandler(u32EventId, pData, u32DataLen);
            break;

        case CLOUD_TMR_MAX:
        default:
            // OPL_LOG_ERRO(CLOUD, "invalid timer index %d", tCloudTimerIdx);
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
    // 1. post keep alive data
    T_CloudRingBufData stKeppAlive = {0}; //Keep alive has no data

    Cloud_RingBufPush(&g_stKeepAliveQ, &stKeppAlive);

    Cloud_MsgSend(CLOUD_EVT_TYPE_POST, NULL, 0);

#if defined(DOOR_SENSOR)
    Cloud_TimerStart(CLOUD_TMR_REQ_DATE, g_u16HbInterval*1000);//AUTO REPEAT after connect Server
#endif
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
#if defined(DOOR_SENSOR)
    uint8_t u8NeedColudConnectionCheck = 0;

    if(NULL != pData)
    {
        u8NeedColudConnectionCheck = *(uint8_t *)pData;
    }

    if(IOT_DATA_POST_CHECK_CLOUD_CONNECTION == u8NeedColudConnectionCheck)
    {
        // OPL_LOG_INFO(CLOUD, "cloud retry table idx reset");
        g_u8CloudRetryIntervalIdx = 0;

        if(false == Cloud_OnlineStatusGet())
        {
            Cloud_MsgSend(CLOUD_EVT_TYPE_ESTABLISH, NULL, 0);
        }
    }

    if(true == Cloud_NetworkStatusGet() &&
       true == Cloud_OnlineStatusGet())
#endif
    {
        //Pri g_stCloudRspQ > g_stKeepAliveQ > g_stIotRbData
        if(false == Cloud_RingBufCheckEmpty(&g_stCloudRspQ))
        {
#if defined(MAGIC_LED)
            uint8_t *u8CtrlSource = (uint8_t *)pData;

            Cloud_SendCloudRspHandle(*u8CtrlSource);
#else
            Cloud_SendCloudRspHandle();
#endif
        }
        else if (false == EG_StatusGet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_WAITING_RX_RSP))
        {
            if(false == Cloud_RingBufCheckEmpty(&g_stKeepAliveQ))
            {
                Cloud_KeepAliveHandle();
            }
            else if(false == Cloud_RingBufCheckEmpty(&g_stIotRbData))
            {
#if defined(MAGIC_LED)
                Iot_Data_CloudDataPost();
#else
                Cloud_DataPost();
#endif
            }
        }
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
    // initialize ring buffers
    Cloud_RingBufInit(&g_stCloudRspQ, IOT_RB_CLOUD_RSP_COUNT);
    Cloud_RingBufInit(&g_stKeepAliveQ, IOT_RB_KEEP_ALIVE_COUNT);
    Cloud_RingBufInit(&g_stIotRbData, IOT_RB_COUNT);
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

#if defined(MAGIC_LED)
void Cloud_SendFirstPostHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    uint8_t *u8Ctrl_Source = 0;
    
    g_IsInitPost = true;
    
    u8Ctrl_Source = (uint8_t*)pData;
    
    user_post_property(true, *u8Ctrl_Source);
}
#endif

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
#if defined(MAGIC_LED)
SHM_DATA void Cloud_ReceiveHandler(void)
{
    SHM_DATA static char szBuf[1664]={0};
    memset(szBuf,0,1664);
    SHM_DATA static char szOutBuf[1664] = {0};
    memset(szOutBuf,0,1664);
    int out_length=0;

    ws_sem_lock(osWaitForever);
    if(g_rx_ID!=g_tcp_hdl_ID)
    {
        g_rx_ID = g_tcp_hdl_ID;
    }
    ws_sem_unlock();
    
    int ret = ws_recv(szBuf, 1664);
//                printf("xxxhttp read ret = %d\r\n", ret);
    #ifdef AUTO_POST_TEST
    ATS_POST_SET_TIME(end_time) 
    #endif
                    
    if(ret>0)
    {
        ws_decode((unsigned char*)szBuf, ret, (unsigned char*)szOutBuf, 1664, &out_length);
        
        // printf("http read ret(%d) is %s (%d)\r\n", ret, szOutBuf, out_length);
        // printf("[ATS]WIFI Rcv data success(len=%d) ret = %d\r\n", out_length, ret);
        
        if(out_length==0)
        {
            Opl_Wifi_Skip_Dtim_Set(g_u16IotDtimTxUse, true);
            return;
        }

        uint8_t u8CtrlSource = CLOUD_CTRL;
        CK_CloudProcessParser(szOutBuf, out_length, u8CtrlSource);
        // App_Json_Parser_Handle((uint8_t*)szOutBuf, out_length, CLOUD_CTRL);

        // BleWifi_Wifi_SetDTIM(BleWifi_Ctrl_DtimTimeGet());

    }
    else if(ret<0)
    {
        // printf("[ATS]WIFI Rcv data fail\r\n");
        #ifdef AUTO_POST_TEST
        ATS_POST_INC(post_data.fail_count)
        ATS_PRINT_ONE_LOG
        ATS_CLEAN_FAIL_STATS
        #endif
        
        ws_sem_lock(osWaitForever);

        Cloud_TimerStop(CLOUD_TMR_REQ_DATE);
        if(((g_tcp_hdl_ID!=-1)
           &&(g_rx_ID == g_tcp_hdl_ID))
           &&(true == Cloud_OnlineStatusGet()))
        {
            int Res = ws_close();
            // printf("[ATS]Cloud disconnect\r\n");
            // printf("rd: ws_close(ret=%d)\n", Res);

            g_rx_ID = -1;
            g_tcp_hdl_ID = -1;
            Cloud_OnlineStatusSet(false);
            // EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_CLOUD_CONNECTED, false);
            Cloud_ConnectRetry();
            ws_sem_unlock();
        }
        else
        {
            ws_sem_unlock();
        }
//         osTimerStop(g_tAppHeartbeatTimerId);
//         osTimerStop(g_tmr_req_date);
//         if((g_tcp_hdl_ID!=-1)&&(g_rx_ID == g_tcp_hdl_ID))
//         {
//             ret=http_close();
//             printf("rd: http_close(ret=%d)\n", ret);
            
//             g_rx_ID = -1;
//             g_tcp_hdl_ID = -1;
//             ws_sem_unlock();
//             BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_WSS_CONN, false);
// //                        Iot_Data_TxTask_MsgSend(IOT_DATA_TX_MSG_EST_COOLKIT_HTTP_CONNECTION,NULL, 0);
//             Iot_Data_TxTask_MsgSend(IOT_DATA_TX_MSG_EST_COOLKIT_WSS_CONNECTION,NULL, 0);
            
//         }
//         else
//         {
//             ws_sem_unlock();
//         }                        
        // BleWifi_Wifi_SetDTIM(BleWifi_Ctrl_DtimTimeGet());
        
    }

    Opl_Wifi_Skip_Dtim_Set(g_u16IotDtimTxUse, true);

}
#else
void Cloud_ReceiveHandler(void)
{
    uint8_t u8RecvBuf[CLOUD_RCV_BUFFER_SIZE] = {0};
    uint32_t ulRecvlen = 0;
    int8_t s8Ret = -1;

    //1. Check Cloud conection or not
    if(true == Cloud_OnlineStatusGet())
    // if (OPL_OK == EG_StatusWait(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_CLOUD_CONNECTED, 0xFFFFFFFF))
    {
        //2. Recv data from cloud
        s8Ret = Coolkit_Cloud_Recv(u8RecvBuf, &ulRecvlen);
        if(s8Ret<0)
        {
            // OPL_LOG_WARN(CLOUD, "Recv data from Cloud fail (ret=%d)", s8Ret);
            goto fail;
        }

        //3. Data parser
        s8Ret = Cloud_DataParser(u8RecvBuf, ulRecvlen);
        if(s8Ret<0)
        {
            // OPL_LOG_WARN(CLOUD, "Iot_Data_Parser failed");
            goto fail;
        }
    }

    s8Ret = 0;

fail:
    if(s8Ret < 0)
    {
        // OPL_LOG_WARN(CLOUD, "Recv data from Cloud fail");
        osDelay(2000);
    }

    // WARNING: IF DO NOTHING IN RECEIVE HANDLER, THE DELAY MUST EXIST
    // osDelay(1000);
}
#endif
