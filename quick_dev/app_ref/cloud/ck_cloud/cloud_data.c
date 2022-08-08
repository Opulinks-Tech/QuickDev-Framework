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
*  cloud_data.c
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
#include "bat_aux.h"
#include "cloud_data.h"
#include "cloud_ctrl.h"
#include "cloud_kernel.h"
#include "cloud_config.h"
#include "cloud_ota_http.h"
#if defined(MAGIC_LED)
#include "ck_svc.h"
#include "cloud_cmd_data.h"
#include "cbc_encrypt\cbc_encrypt.h"
#endif
#include "coolkit_websocket.h"
#if defined(OPL1000_A2) || defined(OPL1000_A3)
#include "driver_netlink.h"
#endif
#include "datetime\date_time.h"
#include "etharp.h"
#include "evt_group.h"
#include "httpclient.h"
#include "infra_cjson.h"
#include "mw_fim_default_group12_project.h"
#include "ota_mngr.h"
#include "qd_config.h"
#include "sha_256\sha_256.h"
#include "time_stmp.h"
#include "wifi_mngr_api.h"


// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#define PROPERTY_PAYLOAD_SIZE_300                       (300)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

extern T_MwFim_GP12_HttpPostContent g_tHttpPostContent;
extern T_MwFim_GP12_HttpHostInfo g_tHostInfo;

// Sec 5: declaration of global function prototype

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

char OTA_FULL_URL[256] = {0};
float g_fBatteryVoltage = 0;

#if CLOUD_HTTP_POST_ENHANCEMENT
    httpclient_t g_client = {0};
#else
    //httpclient_t client = {0};
#endif

// #if defined(MAGIC_LED)
uint64_t g_u64WaitSeqId;
// #endif
uint64_t g_u64TmpSeqId = 0;
uint8_t g_u8SeqIdOffset = 0;


// Sec 7: declaration of static function prototype

void PostToCloudDirectly(int8_t *u8pProperty_Payload, uint32_t u32Offset);

/***********
C Functions
***********/
// Sec 8: C Functions

/*************************************************************************
* FUNCTION:
*   UpdateBatteryContent
*
* DESCRIPTION:
*   update battery content to global variable for data post
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void UpdateBatteryContent(void)
{
    int i = 0;
    float fVBatPercentage = 0;

    for (i = 0 ;i < BAT_IO_MOVING_AVERAGE_COUNT ;i++)
    {
        fVBatPercentage = Bat_AuxAdc_Get();
    }
    g_fBatteryVoltage = fVBatPercentage;
}

/*************************************************************************
* FUNCTION:
*   Cloud_KeepAliveHandle
*
* DESCRIPTION:
*   process keep alive
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_KeepAliveHandle(void)
{
#if defined(MAGIC_LED)
    char property_payload[200] = {0};
    uint32_t u32Offset = 0;
    
    // int32_t s32TimeZoneSec = 8*3600;
    uint64_t u64MsgId = TSTMP_TimestampGet();
    u64MsgId += 8*3600;

    if(u64MsgId == g_u64TmpSeqId)
    {
        g_u8SeqIdOffset++;
    }
    else
    {
        g_u8SeqIdOffset=0;
    }
    
    g_u64TmpSeqId = u64MsgId;

    u64MsgId = (u64MsgId * 1000) + g_u8SeqIdOffset;
    
    g_u64WaitSeqId = u64MsgId;
    
    int len=snprintf(property_payload, 200, "{\"action\":\"date\",\"apikey\":\"%s\",\"deviceid\":\"%s\",\"d_seq\":%llu,\"userAgent\":\"device\"}",
                    g_Apikey, g_tHttpPostContent.ubaDeviceId, u64MsgId);
    
    tracer_drct_printf("----------\nPayload for WS:(%d)\n----------\n%s\n", u32Offset, property_payload);

    PostToCloudDirectly((int8_t *)property_payload, len);
    
    Cloud_TimerStart(CLOUD_TMR_REQ_DATE, CLOUD_KEEP_ALIVE_TIME);
    // osTimerStop(g_tmr_req_date);
    // osTimerStart(g_tmr_req_date, IOT_KEEP_ALIVE_TIMEOUT_TIME);//AUTO REPEAT after connect Server
#else
    ws_KeepAlive();
#endif
}

/*************************************************************************
* FUNCTION:
*   PostToCloudDirectly
*
* DESCRIPTION:
*   directly post data to cloud
*
* PARAMETERS
*   u8pProperty_Payload :
*                   [IN] data to post to cloud
*   u32Offset :     [IN] offset value
*
* RETURNS
*   none
*
*************************************************************************/
#if defined(MAGIC_LED)
extern uint64_t g_u64MsgIDFromAPP;

SHM_DATA void PostToCloudDirectly(int8_t *u8pProperty_Payload, uint32_t u32Offset)
{
    uint32_t ulTotalBodyLen=0;    
    char szBodyFmt[BUFFER_SIZE]={0};
    uint8_t u8IsDateReq=0;

//    printf("Post to Cloud:%s\n", u8pProperty_Payload);
    
    if(false == Cloud_OnlineStatusGet())
    // if( false == BleWifi_Ctrl_EventStatusGet(BLEWIFI_CTRL_EVENT_BIT_WSS_CONN))
    {
//        printf("Cloud is disconnect, no Post\n");
        return;
    }
    
    if(strstr((char *)u8pProperty_Payload, "date")!=0)
    {
        u8IsDateReq = 1;
    }
       
    memset(szBodyFmt, 0, sizeof(szBodyFmt));
                        
    ws_encode(szBodyFmt, &ulTotalBodyLen, (char *)u8pProperty_Payload, u32Offset);  // build the websocket data packet with header and encrypt key
    
    osSemaphoreWait(g_tAppSemaphoreId, osWaitForever);
    if(g_tx_ID!=g_tcp_hdl_ID)
    {
        g_tx_ID = g_tcp_hdl_ID;
    }
    osSemaphoreRelease(g_tAppSemaphoreId);
    
    // BleWifi_Wifi_SetDTIM(0);
    
    int ret = ws_write(szBodyFmt, ulTotalBodyLen);

    if(ret > 0){        
       if(0 == u8IsDateReq)
       {
           g_u8WaitingRspType = IOT_DATA_WAITING_TYPE_DATA_POST;
           printf("[ATS]WIFI Send Reply success(%llu, %d)\r\n", g_u64MsgIDFromAPP, strstr((char *)u8pProperty_Payload, "date"));
       }
       else
       {
           g_u8WaitingRspType = IOT_DATA_WAITING_TYPE_KEEPALIVE;
       }
       
       Cloud_TimerStart(CLOUD_TMR_WAIT_RX_RSP, CLOUD_TX_WAIT_TIMEOUT);
        // osTimerStop(g_iot_tx_wait_timeout_timer);
        // osTimerStart(g_iot_tx_wait_timeout_timer , IOT_TX_WAIT_TIMEOUT_TIME);
        // BleWifi_Ctrl_EventStatusSet(BLEWIFI_CTRL_EVENT_BIT_WAITING_RX_RSP, true);
    }
    else
    {        
        osSemaphoreWait(g_tAppSemaphoreId, osWaitForever);    
        Cloud_TimerStop(CLOUD_TMR_REQ_DATE);                
        // osTimerStop(g_tAppHeartbeatTimerId);
        // osTimerStop(g_tmr_req_date);

        if(((g_tcp_hdl_ID!=-1)
            &&(g_tx_ID == g_tcp_hdl_ID))
            &&(true == Cloud_OnlineStatusGet()))
            // &&(true == EG_StatusGet(g_tIotDataEventGroup , IOT_DATA_EVENT_BIT_CLOUD_CONNECTED)))
        {
            ret = ws_close();

            OPL_LOG_INFO(CLOUD, "Cloud disconnect");
            OPL_LOG_INFO(CLOUD, "wt: ws_close(ret=%d)", ret);

            g_tx_ID = -1;
            g_tcp_hdl_ID = -1;

            Cloud_OnlineStatusSet(false);
            // EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_CLOUD_CONNECTED, false);

            Cloud_ConnectRetry();

            osSemaphoreRelease(g_tAppSemaphoreId);
        }
        else
        {
           osSemaphoreRelease(g_tAppSemaphoreId);
        }

        // BleWifi_Wifi_SetDTIM(BleWifi_Ctrl_DtimTimeGet());                                                       
    }
}
#else
void PostToCloudDirectly(int8_t *u8pProperty_Payload, uint32_t u32Offset)
{
    // T_CloudRingBufData IoT_Properity;
    uint32_t ulTotalBodyLen=0;
    char szBodyFmt[BUFFER_SIZE]={0};
    uint8_t u8IsDateReq=0;

//    printf("Post to Cloud:%s\n", u8pProperty_Payload);

    if(false == Cloud_OnlineStatusGet())
    // if(false == EG_StatusGet(g_tIotDataEventGroup , IOT_DATA_EVENT_BIT_CLOUD_CONNECTED))
    {
//        printf("Cloud is disconnect, no Post\n");
        return;
    }

//    if(strstr((char *)u8pProperty_Payload, "date")!=0)
//    {
//        u8IsDateReq = 1;
//    }

    memset(szBodyFmt, 0, sizeof(szBodyFmt));

    ws_encode(szBodyFmt, &ulTotalBodyLen, (char *)u8pProperty_Payload, u32Offset);  // build the websocket data packet with header and encrypt key

    osSemaphoreWait(g_tAppSemaphoreId, osWaitForever);
    if(g_tcp_hdl_ID == -1)
    {
        osSemaphoreRelease(g_tAppSemaphoreId);
        return;
    }

    if(g_tx_ID!=g_tcp_hdl_ID)
    {
        g_tx_ID = g_tcp_hdl_ID;
    }
    osSemaphoreRelease(g_tAppSemaphoreId);

    int ret = ws_write(szBodyFmt, ulTotalBodyLen);
    if(ret>0)
    {
       if(0 == u8IsDateReq)
       {
        //    OPL_LOG_INFO(CLOUD, "WIFI Send Reply success(%llu, %d)", g_msgid, strstr((char *)u8pProperty_Payload, "date"));
       }
    }
    else
    {
        if(0 == u8IsDateReq)
        {
            // OPL_LOG_WARN(CLOUD, "WIFI Send Reply fail(%llu)", g_msgid);
        }
        else
        {
            // OPL_LOG_WARN(CLOUD, "Req Date Failed(%llu)", g_msgid);
        }


        // if(IoT_Properity.pu8Data!=NULL)
        // {
        //     free(IoT_Properity.pu8Data);
        // }

        osSemaphoreWait(g_tAppSemaphoreId, osWaitForever);

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

        if(((g_tcp_hdl_ID!=-1)
            &&(g_tx_ID == g_tcp_hdl_ID))
            &&(true == Cloud_OnlineStatusGet()))
            // &&(true == EG_StatusGet(g_tIotDataEventGroup , IOT_DATA_EVENT_BIT_CLOUD_CONNECTED)))
        {
            ret = ws_close();

            // OPL_LOG_INFO(CLOUD, "Cloud disconnect");
            // OPL_LOG_INFO(CLOUD, "wt: ws_close(ret=%d)", ret);

            g_tx_ID = -1;
            g_tcp_hdl_ID = -1;

            Cloud_OnlineStatusSet(false);
            // EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_CLOUD_CONNECTED, false);

            Cloud_ConnectRetry();

            osSemaphoreRelease(g_tAppSemaphoreId);
        }
        else
        {
           osSemaphoreRelease(g_tAppSemaphoreId);
        }
    }

    // Update Battery voltage for post data
    UpdateBatteryContent();
}
#endif

/*************************************************************************
* FUNCTION:
*   Cloud_SendCloudRspHandle
*
* DESCRIPTION:
*   send response data to cloud handler
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
#if defined(MAGIC_LED)
void Cloud_SendCloudRspHandle(uint8_t u8CtrlSource)
{
    T_CloudRingBufData tProperity = {0};
    char property_payload[300] = {0};
    unsigned char iv[IV_SIZE + 1] = {0};
    unsigned char ucCbcEncData[256];
    size_t uCbcEncLen = 0;
    unsigned char *ucBaseData;
    size_t uBaseLen = 0;
    uint32_t u32Offset = 0;

    if (OPL_OK == Cloud_RingBufPop(&g_stCloudRspQ, &tProperity))
    {                
        u32Offset = sprintf( (char*)property_payload, "{\"error\":%d,\"deviceid\":\"%s\",\"apikey\":\"%s\",\"userAgent\":\"device\",\"sequence\":\"%s\"}", 
        0,
        g_tHttpPostContent.ubaDeviceId,
        g_Apikey,
        tProperity.pu8Data
        );
        
        if(BLE_CTRL == u8CtrlSource)
        {
            if(true == APP_BleStatusGet())
            {
#if defined(MAXLIAO)
                // CK_DataSendProtocolCmd((uint8_t *)&property_payload, u32Offset);
#endif
                // printf("[ATS]BLE Send Reply success\r\n");
                memset(iv, '0' , IV_SIZE); //iv = "0000000000000000"
                _CK_DataWifiCbcEncrypt((void *)property_payload , u32Offset , iv , g_ucSecretKey , (void *)ucCbcEncData);
                // BleWifi_CBC_encrypt((void *)property_payload , u32Offset , iv , g_ucSecretKey , (void *)ucCbcEncData);
                uCbcEncLen = u32Offset;
                uCbcEncLen = (((uCbcEncLen  >> 4) + 1) << 4);
                int sSize = ((4 * uCbcEncLen / 3) + 3) & ~3;
                ucBaseData = malloc(sSize+1);
                if(ucBaseData == NULL)
                {
                    return;
                }
                mbedtls_base64_encode((unsigned char *)ucBaseData , sSize+1  ,&uBaseLen ,(unsigned char *)ucCbcEncData , uCbcEncLen);
                
                CK_DataSendEncap(CK_DATA_RES_PROTOCOL_CMD, ucBaseData, uBaseLen);
                // BleWifi_Ble_DataSendEncap(BLEWIFI_RES_CK_PROTOCOL_CMD, ucBaseData, uBaseLen);
                if(ucBaseData!=NULL)
                {
                    free(ucBaseData);
                    ucBaseData = NULL;
                }

            }
        }
        else if(CLOUD_CTRL == u8CtrlSource)
        {
            PostToCloudDirectly((int8_t *)property_payload, u32Offset);
        }
        
        //remove from Q
        Cloud_RingBufReadIdxUpdate(&g_stCloudRspQ);

        if(tProperity.pu8Data != NULL)
        {
            free(tProperity.pu8Data);
            tProperity.pu8Data = NULL;
            tProperity.u32DataLen = 0;
        }
    }
}
#else
void Cloud_SendCloudRspHandle(void)
{
    T_CloudRingBufData tProperity = {0};

    Opl_Wifi_Skip_Dtim_Set(g_u16IotDtimTxCloudAckPost, false);

    lwip_one_shot_arp_enable();

    if (OPL_OK == Cloud_RingBufPop(&g_stCloudRspQ, &tProperity))
    {
        PostToCloudDirectly((int8_t *)tProperity.pu8Data, tProperity.u32DataLen);

        Cloud_TimerStart(CLOUD_TMR_ACK_POST, CLOUD_ACK_POST_TIMEOUT);

        //remove from Q
        Cloud_RingBufReadIdxUpdate(&g_stCloudRspQ);

        if(tProperity.pu8Data != NULL)
        {
            free(tProperity.pu8Data);
        }
    }
}
#endif

/*************************************************************************
* FUNCTION:
*   Cloud_ConstructPostDataAndSend
*
* DESCRIPTION:
*   constructing the post data
*
* PARAMETERS
*   ptProperity :   [IN] data pop from ring buffer
*
* RETURNS
*   int8_t :        [OUT] cloud send result
*
*************************************************************************/
int8_t Cloud_ConstructPostDataAndSend(T_CloudRingBufData *ptProperity)
{
    uint8_t u8aPayload[BUFFER_SIZE] = {0};
    uint32_t u32PayloadLen = 0;
    int8_t s8Ret = 0;

    if(true == Cloud_OnlineStatusGet())
    // if(true == EG_StatusGet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_CLOUD_CONNECTED))
    {
        // OPL_LOG_INFO(CLOUD, "----------\nPayload for WS Send:(%d)\n----------\n%s", , IoT_Properity.ulLen, IoT_Properity.ubData);

        Cloud_DataConstruct(ptProperity->pu8Data, ptProperity->u32DataLen, &u8aPayload[0], &u32PayloadLen);
        // Cloud_PostDataConstruct(ptProperity, &u8aPayload[0] , &u32PayloadLen);

        if(g_u8PostRetry_IotRbData_Cnt == 0)
        {
            OPL_LOG_INFO(CLOUD, "WIFI Send data start(%llu)", g_msgid);
        }

        s8Ret = Coolkit_Cloud_Send(u8aPayload, u32PayloadLen);

        // Update Battery voltage for post data
        UpdateBatteryContent();
    }

    return s8Ret;
}

/*************************************************************************
* FUNCTION:
*   Cloud_DataConstruct
*
* DESCRIPTION:
*   construct post data (weak type)
*
* PARAMETERS
*   pInData :       [IN] input data
*   u32InDataLen :  [IN] input data len
*   pOutData :      [OUT] constructed data
*   pu32OutDataLen :[OUT] constructed data lens
*
* RETURNS
*   none
*
*************************************************************************/
#if defined(DOOR_SENSOR)
void Cloud_DataConstruct(uint8_t *pInData, uint32_t u32InDataLen, uint8_t *pOutData, uint32_t *pu32OutDataLen)
{

    // user implement

    uint32_t u32Offset = 0;
    uint8_t already_wrt_flag = 0;
    char s8aPayload[512] = {0};
    char *ps8Buf = s8aPayload;

    uint16_t uwProjectId=0;
    uint16_t uwChipId=0;
    uint16_t uwFirmwareId=0;
    float fVBatPercentage = 0;
    char FwVersion[24]={0};
    uint8_t ubaMacAddr[6]={0};
    int8_t rssi = 0;

    Sensor_Data_t *pstSensorData = NULL;

    // pstSensorData = (Sensor_Data_t *)ptProperity->pu8Data;
    pstSensorData = (Sensor_Data_t *)pInData;

    if(pstSensorData == NULL)
    {
        // OPL_LOG_ERRO(CLOUD, "pstSensorData is NULL");
        return;
    }

    /* WiFi MAC */
    wifi_config_get_mac_address(WIFI_MODE_STA, ubaMacAddr);

    if(0 == pstSensorData->fVBatPercentage) //has the battery been updated?
    {
        fVBatPercentage = g_fBatteryVoltage;

        if(fVBatPercentage > (float)BATTERY_HIGH_VOL)
        {
            fVBatPercentage = 100;
        }
        else if(fVBatPercentage < (float)BATTERY_LOW_VOL)
        {
            fVBatPercentage = 0;
        }
        else
        {
            fVBatPercentage = ((fVBatPercentage - (float)BATTERY_LOW_VOL) *100 / ((float)BATTERY_HIGH_VOL - (float)BATTERY_LOW_VOL));
        }
        pstSensorData->fVBatPercentage = fVBatPercentage;  // update battery
    }
    else
    {
        fVBatPercentage = pstSensorData->fVBatPercentage;
    }

    /* WiFi RSSI */
    if(0 == pstSensorData->rssi) //Has the rssi been updated?
    {
#if defined(OPL1000_A2) || defined(OPL1000_A3)
        // OPL_LOG_INFO(CLOUD, "Original RSSI is %d", wpa_driver_netlink_get_rssi());
        // rssi = wpa_driver_netlink_get_rssi() + BLEWIFI_COM_RF_RSSI_ADJUST;

        int8_t i8RssiOffset = 0;
        RF_PwrRssiOffsetGet(&i8RssiOffset);

        rssi = wpa_driver_netlink_get_rssi() + i8RssiOffset;

        pstSensorData->rssi = rssi; // update rssi
#elif defined(OPL2500_A0)
        Opl_Wifi_Rssi_Get(&rssi);

        // OPL_LOG_INFO(CLOUD, "Original RSSI is %d", rssi);

        int8_t i8RssiOffset = 0;
        RF_PwrRssiOffsetGet(&i8RssiOffset);

        rssi = rssi + i8RssiOffset;

        pstSensorData->rssi = rssi; // update rssi
#endif
    }
    else
    {
        rssi = pstSensorData->rssi;
    }

    u32Offset = sprintf( ps8Buf, "{");

    if(g_IsInitPost)
    {
        u32Offset += sprintf( ps8Buf +u32Offset, "\"init\":1");
        already_wrt_flag = 1;

        MwOta_VersionGet(&uwProjectId, &uwChipId, &uwFirmwareId);

        memset(FwVersion, 0x00, sizeof(FwVersion));

        sprintf(FwVersion, CKS_FW_VERSION_FORMAT, uwProjectId, uwChipId, uwFirmwareId);

        if (already_wrt_flag)
        {
            u32Offset += sprintf( ps8Buf +u32Offset, ",");
        }

        u32Offset += sprintf( ps8Buf +u32Offset, "\"fwVersion\":\"%s\"", FwVersion);
        already_wrt_flag = 1;

        if (already_wrt_flag)
        {
            u32Offset += sprintf( ps8Buf +u32Offset, ",");
        }

        u32Offset += sprintf( ps8Buf +u32Offset, POST_DATA_MACADDRESS_FORMAT,
        ubaMacAddr[0],
        ubaMacAddr[1],
        ubaMacAddr[2],
        ubaMacAddr[3],
        ubaMacAddr[4],
        ubaMacAddr[5]);

        already_wrt_flag = 1;

        if (already_wrt_flag)
        {
            u32Offset += sprintf( ps8Buf +u32Offset, ",");
        }

        u32Offset += sprintf( ps8Buf +u32Offset, "\"chipID\":\"%d\"", uwChipId);
        already_wrt_flag = 1;


        g_IsInitPost = 0;
    }

//    if( pstSensorData->ubaType==2 || pstSensorData->ubaType==3 )
    {
        if (already_wrt_flag)
        {
            u32Offset += sprintf( ps8Buf +u32Offset, ",");
        }

        u32Offset += sprintf( ps8Buf +u32Offset, "\"type\":\"%d\"", pstSensorData->ubaType);
        already_wrt_flag = 1;

        if (already_wrt_flag)
        {
            u32Offset += sprintf( ps8Buf +u32Offset, ",");
        }

        u32Offset += sprintf( ps8Buf +u32Offset, "\"switch\":\"%s\"", pstSensorData->ubaDoorStatus?"off":"on");
        already_wrt_flag = 1;
    }

    if (already_wrt_flag)
    {
        u32Offset += sprintf( ps8Buf +u32Offset, ",");
    }

    u32Offset += sprintf( ps8Buf +u32Offset, "\"rssi\":\"%d\"", rssi);
    already_wrt_flag = 1;

    if (already_wrt_flag)
    {
        u32Offset += sprintf( ps8Buf +u32Offset, ",");
    }

    u32Offset += sprintf( ps8Buf +u32Offset, "\"battery\":\"%d\"", (int)fVBatPercentage);
    already_wrt_flag = 1;

    u32Offset += sprintf( ps8Buf + u32Offset, "}");

    if(strlen(ps8Buf) < 5) return;

    // OPL_LOG_INFO(CLOUD, "Device Act Update: %s", ps8Buf);

    // g_msgid = Coolkit_Cloud_GetNewSeqID();
    g_msgid = pstSensorData->u64TimeStamp;

    // *pu32Len = sprintf( (char *)pu8Data, COOLLINK_WS_UPDATE_BODY_WITH_PARAMS,
    *pu32OutDataLen = sprintf( (char *)pOutData, COOLLINK_WS_UPDATE_BODY_WITH_PARAMS,
    g_tHttpPostContent.ubaDeviceId,
    g_Apikey,
    g_msgid,
    ps8Buf);
}
#endif

/*************************************************************************
* FUNCTION:
*   Cloud_DataParser
*
* DESCRIPTION:
*   parsing received data and activate to application
*
* PARAMETERS
*   pInData :       [IN] received data
*   u32InDataLen :  [IN] received data lens
*
* RETURNS
*   none
*
*************************************************************************/
#if defined(DOOR_SENSOR)
int8_t Cloud_DataParser(uint8_t *pInData, uint16_t u32InDataLen)
{
    // 1. handle data parser
    char szDecodeBuf[BUFFER_SIZE] = {0};
    int Decode_len = 0;

    ws_decode((unsigned char*)pInData, u32InDataLen, (unsigned char*)szDecodeBuf, BUFFER_SIZE , &Decode_len);

    if(0!=Decode_len)
    {
        // OPL_LOG_INFO(CLOUD, "WIFI Rcv data success(len=%d)\npayload:%s", Decode_len, szDecodeBuf);
    }

    if(strstr(szDecodeBuf, "action")!=0)
    {
        lite_cjson_t tRoot = {0};
        lite_cjson_t tUpdate = {0};
        char sBuf[32] = {0};
        const coollink_ws_command_t *cmd;

        /* Parse Request */
        if (!lite_cjson_parse(szDecodeBuf, u32InDataLen, &tRoot))
        {
            if (!lite_cjson_object_item(&tRoot, "action", 6, &tUpdate))
            {
                if(tUpdate.value_length < 32)
                {
                    memcpy(sBuf, tUpdate.value, tUpdate.value_length);
                    sBuf[tUpdate.value_length] = 0;
                }
            }

            for (int i = 0; coollink_ws_action_commands[i].pattern!= NULL; i++)
            {
                cmd = &coollink_ws_action_commands[i];
                if (strcmp(cmd->pattern, sBuf)==0) 
                {
                    coollink_ws_result_t Reslt = cmd->callback((uint8_t*)szDecodeBuf, Decode_len, false);
                }
            }
        }
        else
        {
            // OPL_LOG_INFO(CLOUD, "JSON Parse Error");
        }
    }
    else if(strstr(szDecodeBuf, "error")!=0)
    {
        coollink_ws_result_t Reslt = Coollink_ws_process_error((uint8_t*)szDecodeBuf, Decode_len, false);
    }

    return 0;
}
#endif

#if defined(DOOR_SENSOR)
coollink_ws_result_t Coollink_ws_process_update(uint8_t *szOutBuf, uint16_t out_length, uint8_t u8IsNeedToPost)
{
    lite_cjson_t tRoot = {0};
    lite_cjson_t tSequence = {0};
    lite_cjson_t tParams= {0};
//    lite_cjson_t tLocal= {0};
    lite_cjson_t tDefense = {0};
    lite_cjson_t tEnable = {0};
    lite_cjson_t tSwitch = {0};
    lite_cjson_t tTime = {0};
    lite_cjson_t tholdReminder = {0};

    uint8_t sSeqID[32] = {0};
    uint32_t u32Offset = 0;
    int8_t s8Enable =-1;
    int8_t s8Switchon = -1;
    int16_t s16Time = -1;
    T_CloudRingBufData stCloudRsp = {0};

    /* Parse Request */
    if (lite_cjson_parse((char*)szOutBuf, out_length, &tRoot))
    {
        // printf("JSON Parse Error\n");
        return COOLLINK_WS_RES_ERR;
    }

    if (!lite_cjson_object_item(&tRoot, "sequence", 8, &tSequence))
    {

        if(tSequence.value_length < 32)
        {
            memcpy(sSeqID, tSequence.value, tSequence.value_length);
            sSeqID[tSequence.value_length] = 0;
        }
        uint64_t u64TimestampMs = 0;
        u64TimestampMs = strtoull((char*)sSeqID, NULL, 10);

        g_msgid = u64TimestampMs;
        //printf("ORG SeqID: %llu\r\n", g_msgid);
    }

    if (!lite_cjson_object_item(&tRoot, "params", 6, &tParams))
    {

       if (!lite_cjson_object_item(&tParams, "defense", 7, &tDefense))
       {

            uint8_t u8Defense = tDefense.value_int;

           printf("defense: %d\r\n", u8Defense);

            APP_DoorSensorDefenseSet(u8Defense);
        }

        if (!lite_cjson_object_item(&tParams, "holdReminder", 12, &tholdReminder))
        {


            if (!lite_cjson_object_item(&tholdReminder, "enabled", 7, &tEnable))
            {
                s8Enable = tEnable.value_int;

               printf("enable: %d\r\n", s8Enable);
            }

            if (!lite_cjson_object_item(&tholdReminder, "switch", 6, &tSwitch))
            {

                if((tSwitch.value_length==3)&&strncmp(tSwitch.value,"off",3)==0)
                {
                    s8Switchon = DOOR_WARING_FLAG_CLOSE;
                }

                if((tSwitch.value_length==2)&&strncmp(tSwitch.value,"on",2)==0)
                {
                    s8Switchon = DOOR_WARING_FLAG_OPEN;
                }
                   printf("Switchon: %d\r\n", s8Switchon);
            }

            if (!lite_cjson_object_item(&tholdReminder, "time", 4, &tTime))
            {

                s16Time = tTime.value_int;
               printf("Time: %d\r\n", s16Time);
            }

            if((-1!=s8Enable) && (-1!=s8Switchon) && (-1!=s16Time))
            {
                T_MwFim_GP17_Hold_Reminder pstHoldReminder;
                pstHoldReminder.u8Enable = s8Enable;
                pstHoldReminder.u8Switch = s8Switchon;
                pstHoldReminder.u16Time = s16Time;

                // OPL_LOG_INFO(CLOUD, "Enable=%d, Switch=%d, Time=%d\n", s8Enable, s8Switchon, s16Time);

                APP_SendMessage(APP_EVT_DOOR_HOLD_REMIDER_SET, (uint8_t *)&pstHoldReminder, sizeof(T_MwFim_GP17_Hold_Reminder));
            }
        }
    }

    stCloudRsp.pu8Data = malloc(PROPERTY_PAYLOAD_SIZE_300);
    if(stCloudRsp.pu8Data == NULL)
    {
        // OPL_LOG_ERRO(CLOUD, "ubData malloc fail");
        return COOLLINK_WS_RES_ERR;
    }

    u32Offset = sprintf((char *)stCloudRsp.pu8Data , COOLLINK_WS_REPLY_BODY,
    0,
    g_tHttpPostContent.ubaDeviceId,
    g_Apikey,
    sSeqID
    );

    stCloudRsp.u32DataLen = u32Offset;

    if(OPL_OK == Cloud_RingBufPush(&g_stCloudRspQ, &stCloudRsp))
    {
        Cloud_MsgSend(CLOUD_EVT_TYPE_POST, NULL, 0);
    }
    else
    {
        // OPL_LOG_WARN(CLOUD, "rb push fail");
        free(stCloudRsp.pu8Data);
    }

    return COOLLINK_WS_RES_OK;
}

coollink_ws_result_t Coollink_ws_process_upgrade(uint8_t *szOutBuf, uint16_t out_length, uint8_t u8IsNeedToPost)
{
    OPL_LOG_INFO(CLOUD, "upgrade");

    int length;
    char URL[128] = {0};
    const char *PosStart = (const char *)szOutBuf;
    const char *PosEnd;
    const char *NeedleStart = "downloadUrl";
    const char *NeedleEnd = "\",\"";

    if ((PosStart=strstr(PosStart,NeedleStart)) != NULL)
    {
        // Assign String to another
        PosEnd = PosStart;

        // Search the match string
        if ((PosEnd=strstr(PosEnd, NeedleEnd)) != NULL)
        {
            // Calcuate the length
            length = PosEnd - PosStart;

            memset(URL, '\0', sizeof(URL));
            // Copy string to destination string
            strncpy(URL, (PosStart + (strlen(NeedleStart) + strlen(NeedleEnd))), (length - (strlen(NeedleStart) + strlen(NeedleEnd))));

            uint8_t ubaSHA256CalcStrBuf[SCRT_SHA_256_OUTPUT_LEN];

            uint32_t u32TimeStamp = 0; // current seconds of today (based on time zone of current DevSched)

            u32TimeStamp = TSTMP_TimestampGet();

            if (1 == SHA256_ValueForOta(u32TimeStamp, ubaSHA256CalcStrBuf))
            {
                // printf("\n SENSOR_DATA_FAIL \n");
                return COOLLINK_WS_RES_ERR;
            }

            int iOffset = 0;
            char baSha256Buf[68] = {0};

            for(int i = 0; i < SCRT_SHA_256_OUTPUT_LEN; i++)
            {
                iOffset += snprintf(baSha256Buf + iOffset, sizeof(baSha256Buf), "%02x", ubaSHA256CalcStrBuf[i]);
            }

            memset(OTA_FULL_URL,0x00, sizeof(OTA_FULL_URL));
            sprintf(OTA_FULL_URL, OTA_DATA_URL, URL, g_tHttpPostContent.ubaDeviceId, u32TimeStamp, baSha256Buf);

//            EG_StatusSet(BLEWIFI_CTRL_EVENT_BIT_OTA_MODE, true);

            // OPL_LOG_INFO(CLOUD, "OTA_DATA_URL: "OTA_DATA_URL"\n", URL, g_tHttpPostContent.ubaDeviceId, u32TimeStamp, baSha256Buf);

//            if(true == EG_StatusGet(APP_CTRL_EVENT_BIT_OTA_MODE))
            {
                // OPL_LOG_INFO(CLOUD, "Will Do OTA .....");
                // OPL_LOG_INFO(CLOUD, "\n OTA_FULL_URL = %s", OTA_FULL_URL);

#if (OTA_ENABLED == 1)
                Cloud_OtaTriggerReq(CLOUD_OTA_EVT_TYPE_DOWNLOAD, (uint8_t *)OTA_FULL_URL, strlen(OTA_FULL_URL));
#endif
            }
        }
    }

    return COOLLINK_WS_RES_OK;
}


coollink_ws_result_t Coollink_ws_process_error(uint8_t *szOutBuf, uint16_t out_length, uint8_t u8IsNeedToPost)
{
    lite_cjson_t tRoot = {0};
    lite_cjson_t tError = {0};
    lite_cjson_t tDate = {0};
    uint64_t result = 0;
    uint8_t IsPrntPostRlt = 1;
    T_CloudRingBufData ptProperity  = {0};

    /* Parse Request */
    if (lite_cjson_parse((char*)szOutBuf, out_length, &tRoot))
    {
        // OPL_LOG_WARN(CLOUD, "JSON Parse Error");
        return COOLLINK_WS_RES_ERR;
    }

    // printf("Cloud Resp Data:%s\n", szOutBuf);
    if (!lite_cjson_object_item(&tRoot, "error", 5, &tError))
    {
        // OPL_LOG_INFO(CLOUD, "Cloud Resp errorcode: %d", tError.value_int);
    }

    if (!lite_cjson_object_item(&tRoot, "date", 4, &tDate))
    {
        // printf("Cloud Resp Date:%s\n", tDate.value);
        int mon=0,d=0,y=0,h,min,s;
 
        sscanf(tDate.value,"%d-%d-%dT%d:%d:%d*",&y,&mon,&d,&h,&min,&s);
 
        // OPL_LOG_INFO(CLOUD, "Cloud Rsp Date: %d %d %d %d %d %d",y,mon,d,h,min,s);

        IsPrntPostRlt = 0;

        DateTime date;

        date.year=y;
        date.month=mon;
        date.day=d;
        date.hours=h;
        date.minutes=min;
        date.seconds=s;

        uint32_t u32Timestamp = 0;
        u32Timestamp = ConvertDateToUnixTime(&date);

        TSTMP_TimestampSync(u32Timestamp);

//        BleWifi_Ctrl_MsgSend(BLEWIFI_CTRL_MSG_DEV_SCHED_START, NULL, 0);
    }

    lite_cjson_t tSequence = {0};

    char sBuf[32] = {0};

    if (!lite_cjson_object_item(&tRoot, "d_seq", 5, &tSequence))
    {
        if(tSequence.value_length < 32)
        {
            memcpy(sBuf, tSequence.value, tSequence.value_length);
            sBuf[tSequence.value_length] = 0;
        }
        result = strtoull((char*)sBuf, NULL, 10);

        if( tError.value_int == 0 && g_msgid == result)
        {
            g_msgid = 0; //reset

            Cloud_TimerStop(CLOUD_TMR_WAIT_RX_RSP);

            EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_WAITING_RX_RSP, false);

            if(IOT_DATA_WAITING_TYPE_KEEPALIVE == g_u8WaitingRspType)
            {
                if (false == Cloud_RingBufCheckEmpty(&g_stKeepAliveQ))
                {
                    Cloud_RingBufPop(&g_stKeepAliveQ , &ptProperity);
                    Cloud_RingBufReadIdxUpdate(&g_stKeepAliveQ);

                    if(ptProperity.pu8Data!=NULL)
                    {
                        free(ptProperity.pu8Data);
                    }
                }

                osSemaphoreWait(g_tAppSemaphoreId, osWaitForever);
                g_u8PostRetry_KeepAlive_Cnt = 0;
                g_u8PostRetry_KeepAlive_Fail_Round = 0;
                osSemaphoreRelease(g_tAppSemaphoreId);
            }
            else if(IOT_DATA_WAITING_TYPE_DATA_POST == g_u8WaitingRspType)
            {
                // if clear the flag for last post retry after keep alive
                EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_LAST_POST_RETRY, false);

                if (false == Cloud_RingBufCheckEmpty(&g_stIotRbData))
                {
                    Cloud_RingBufPop(&g_stIotRbData , &ptProperity);
                    Cloud_RingBufReadIdxUpdate(&g_stIotRbData);

                    if(ptProperity.pu8Data!=NULL)
                    {
                        free(ptProperity.pu8Data);
                    }
                }

                // OPL_LOG_INFO(CLOUD, "post cnt = %u success" , (g_u8PostRetry_IotRbData_Cnt + 1));

                osSemaphoreWait(g_tAppSemaphoreId, osWaitForever);
                g_u8PostRetry_IotRbData_Cnt = 0;
                osSemaphoreRelease(g_tAppSemaphoreId);
            }

            osSemaphoreWait(g_tAppSemaphoreId, osWaitForever);
            g_u8WaitingRspType = IOT_DATA_WAITING_TYPE_NONE;
            osSemaphoreRelease(g_tAppSemaphoreId);

            if(1 == IsPrntPostRlt)
            {
                OPL_LOG_INFO(CLOUD, "WIFI Send Data success(%llu)", result);
            }

#if 0
            Cloud_MsgSend(CLOUD_EVT_TYPE_POST, NULL, 0);
#else
            Opl_Wifi_Skip_Dtim_Set(g_u16IotDtimWaitRspAckPost, false);

            Cloud_TimerStart(CLOUD_TMR_RSP_TCP_ACK, CLOUD_RSP_TCP_ACK_TIMEOUT);

            EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_WAITING_TCP_ACK, true);

            Cloud_MsgSend(CLOUD_EVT_TYPE_POST, NULL, 0);
#endif

            Opl_Wifi_Skip_Dtim_Set(g_u16IotDtimTxUse, true);
        }
        else if(g_msgid != result)
        {
            // OPL_LOG_WARN(CLOUD, "WIFI post fail(%llu)", result);
        }
    }

    if(0 != tError.value_int)
    {
        return COOLLINK_WS_RES_ERR;
    }

    return COOLLINK_WS_RES_OK;
}
#endif
