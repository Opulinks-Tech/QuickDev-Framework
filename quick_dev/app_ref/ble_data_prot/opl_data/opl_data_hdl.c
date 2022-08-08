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
*  opl_data_hdl.c
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
#include "netif.h"
#include "net_mngr_api.h"
#include "opl_data_hdl.h"
#include "opl_data_prot.h"
#include "qd_config.h"
#include "qd_module.h"
#include "wifi_mngr_api.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (OPL_DATA_ENABLED == 1)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

T_OplDataConnCfg g_tOplDataConnCfg = {0};

// Sec 5: declaration of global function prototype

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

#if (1 == FLITER_STRONG_AP_EN)
static void _OPL_DataHandler_FilterStrongRssiAP(T_WmScanInfo *pstScanInfo ,uint16_t u16apCount);
#endif

static void _OPL_DataHandler_SendSignalScanReport(uint16_t apCount, T_WmScanInfo *ap_list);
static int _OPL_DataHandler_SendScanReport(void);
static void _OPL_DataHandler_SendStatusInfo(uint16_t u16Type);

/***********
C Functions
***********/
// Sec 8: C Functions

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
#if (1 == FLITER_STRONG_AP_EN)
static void _OPL_DataHandler_FilterStrongRssiAP(T_WmScanInfo *pstScanInfo ,uint16_t u16apCount)
{
    uint8_t i = 0 , j = 0;

    for(i = 0 ; i < u16apCount ; i++)
    {
        for(j = 0 ; j < i ; j++)
        {
             //check whether ignore alreay
            if(false == pstScanInfo[j].u8IgnoreReport)
            {
                //compare the same ssid
                if((pstScanInfo[i].ssid_length != 0) && memcmp(pstScanInfo[j].ssid , pstScanInfo[i].ssid , WIFI_MAX_LENGTH_OF_SSID) == 0)
                {
                    //let
                    if(pstScanInfo[j].rssi < pstScanInfo[i].rssi)
                    {
                        pstScanInfo[j].u8IgnoreReport = true;
                    }
                    else
                    {
                        pstScanInfo[i].u8IgnoreReport = true;
                        break; //this AP sets ignore , don't need compare others.
                    }
                }
            }
        }
    }
}
#endif

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static void _OPL_DataHandler_SendSignalScanReport(uint16_t apCount, T_WmScanInfo *ap_list)
{
    uint8_t *data;
    int data_len;
    uint8_t *pos;
    int malloc_size = sizeof(T_WmScanInfo) * apCount;

    pos = data = malloc(malloc_size);
    if (data == NULL)
    {
        OPL_LOG_ERRO(OPL, "malloc fail");
        return;
    }

    for (int i = 0; i < apCount; ++i)
    {
        uint8_t len = ap_list[i].ssid_length;

        data_len = (pos - data);

        *pos++ = len;
        memcpy(pos, ap_list[i].ssid, len);
        pos += len;
        memcpy(pos, ap_list[i].bssid,6);
        pos += 6;
        *pos++ = ap_list[i].auth_mode;
        *pos++ = ap_list[i].rssi;
#ifdef CK_DATA_USE_CONNECTED
        *pos++ = ap_list[i].connected;
#else
        *pos++ = 0;
#endif
    }

    data_len = (pos - data);

    // BLEWIFI_DUMP("scan report data", data, data_len);

    /* create scan report data packet */
    OPL_DataSendEncap(OPL_DATA_RSP_SCAN_REPORT, data, data_len);
    // BleWifi_Ble_DataSendEncap(BLEWIFI_RSP_SCAN_REPORT, data, data_len);

    free(data);
}

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static int _OPL_DataHandler_SendScanReport(void)
{
    wifi_scan_info_t *pstAPList = NULL;
    wifi_auto_connect_info_t *info = NULL;
    T_WmScanInfo *pstWifiAPList = NULL;

    uint8_t u8APPAutoConnectGetApNum = 0;
    // uint8_t u8IsUpdate = false;
    uint16_t u16apCount = 0;

    int8_t ubAppErr = 0;
    int32_t i = 0, j = 0;

    // blewifi_wifi_get_ap_record_t stAPRecord;
    // blewifi_wifi_get_auto_conn_ap_info_t stAutoConnApInfo;

    // memset(&stAPRecord , 0 ,sizeof(blewifi_wifi_get_ap_record_t));

    // TODO: get ap number
    Opl_Wifi_ApNum_Get(&u16apCount);
    // BleWifi_Wifi_Query_Status(BLEWIFI_WIFI_GET_AP_NUM , (void *)&u16apCount);
    //wifi_scan_get_ap_num(&u16apCount);

    if (u16apCount == 0)
    {
        OPL_LOG_ERRO(OPL, "No AP found");
        goto err;
    }

    OPL_LOG_INFO(OPL, "AP num = %d", u16apCount);

    pstAPList = (wifi_scan_info_t *)malloc(sizeof(wifi_scan_info_t) * u16apCount);

    if (!pstAPList)
    {
        OPL_LOG_ERRO(OPL, "malloc fail");
        ubAppErr = -1;
        goto err;
    }

    // stAPRecord.pu16apCount = &u16apCount;
    // stAPRecord.pstScanInfo = pstAPList;

    // TODO: get ap record
    Opl_Wifi_ApRecord_Get(&u16apCount, pstAPList);
    // BleWifi_Wifi_Query_Status(BLEWIFI_WIFI_GET_AP_RECORD , (void *)&stAPRecord);
    //wifi_scan_get_ap_records(&u16apCount, pstAPList);

    // _BleWifi_Wifi_UpdateAutoConnList(u16apCount, pstAPList, &u8IsUpdate);

    pstWifiAPList = (T_WmScanInfo *)malloc(sizeof(T_WmScanInfo) * u16apCount);
    if (!pstWifiAPList) 
    {
        OPL_LOG_ERRO(OPL, "malloc fail");
        ubAppErr = -1;
        goto err;
    }

    memset(pstWifiAPList , 0 , sizeof(T_WmScanInfo) * u16apCount);

    // TODO: get auto connect ap number
    Opl_Wifi_AutoConnectApNum_Get(&u8APPAutoConnectGetApNum);
    // BleWifi_Wifi_Query_Status(BLEWIFI_WIFI_GET_AUTO_CONN_AP_NUM , &u8APPAutoConnectGetApNum);
    //wifi_auto_connect_get_ap_num(&ubAPPAutoConnectGetApNum);

    if (u8APPAutoConnectGetApNum)
    {
        info = (wifi_auto_connect_info_t *)malloc(sizeof(wifi_auto_connect_info_t) * u8APPAutoConnectGetApNum);
        if (!info)
        {
            OPL_LOG_ERRO(OPL, "malloc fail");
            ubAppErr = -1;
            goto err;
        }

        memset(info, 0, sizeof(wifi_auto_connect_info_t) * u8APPAutoConnectGetApNum);

        for (i = 0; i < u8APPAutoConnectGetApNum; i++)
        {
            // stAutoConnApInfo.u8Index = i;
            // stAutoConnApInfo.pstAutoConnInfo = info+i;

            // TODO: get auto connected ap info
            Opl_Wifi_AutoConnectApInfo_Get(i, info + i);
            // BleWifi_Wifi_Query_Status(BLEWIFI_WIFI_GET_AUTO_CONN_AP_INFO , &stAutoConnApInfo);
            //wifi_auto_connect_get_ap_info(i, (info+i));
        }
    }

    /* build blewifi ap list */
    for (i = 0; i < u16apCount; ++i)
    {
        memcpy(pstWifiAPList[i].ssid, pstAPList[i].ssid, sizeof(pstAPList[i].ssid));
        memcpy(pstWifiAPList[i].bssid, pstAPList[i].bssid, WIFI_MAC_ADDRESS_LENGTH);
        pstWifiAPList[i].rssi = pstAPList[i].rssi;
        pstWifiAPList[i].auth_mode = pstAPList[i].auth_mode;
        pstWifiAPList[i].ssid_length = strlen((const char *)pstAPList[i].ssid);
        pstWifiAPList[i].connected = 0;
#if (1 == FLITER_STRONG_AP_EN)
        pstWifiAPList[i].u8IgnoreReport = false;
#endif
        for (j = 0; j < u8APPAutoConnectGetApNum; j++)
        {
            if ((info+j)->ap_channel)
            {
                if(!memcmp(pstWifiAPList[i].ssid, (info+j)->ssid, sizeof((info+j)->ssid)) && !memcmp(pstWifiAPList[i].bssid, (info+j)->bssid, sizeof((info+j)->bssid)))
                {
                    pstWifiAPList[i].connected = 1;
                    break;
                }
            }
        }
    }

#if (1 == FLITER_STRONG_AP_EN)
    _OPL_DataHandler_FilterStrongRssiAP(pstWifiAPList , u16apCount);
#endif

    /* Send Data to BLE */
    /* Send AP inforamtion individually */
    for (i = 0; i < u16apCount; ++i)
    {
#if (1 == FLITER_STRONG_AP_EN)
        if(true == pstWifiAPList[i].u8IgnoreReport)
        {
            continue;
        }
#endif
        if(pstWifiAPList[i].ssid_length != 0)
        {
            _OPL_DataHandler_SendSignalScanReport(1, &pstWifiAPList[i]);
            osDelay(100);
        }
    }

err:
    if (pstAPList)
        free(pstAPList);

    if (pstWifiAPList)
        free(pstWifiAPList);

    if (info)
        free(info);

    return ubAppErr;
}

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static void _OPL_DataHandler_SendStatusInfo(uint16_t u16Type)
{
    uint8_t *pu8Data, *pu8Pos;
    uint8_t u8Status = 0, u8StrLen = 0;
    uint16_t u16DataLen;
    uint8_t u8aIp[4], u8aNetMask[4], u8aGateway[4];
    wifi_scan_info_t stInfo;
    struct netif *iface = netif_find("st1");

    u8aIp[0] = (iface->ip_addr.u_addr.ip4.addr >> 0) & 0xFF;
    u8aIp[1] = (iface->ip_addr.u_addr.ip4.addr >> 8) & 0xFF;
    u8aIp[2] = (iface->ip_addr.u_addr.ip4.addr >> 16) & 0xFF;
    u8aIp[3] = (iface->ip_addr.u_addr.ip4.addr >> 24) & 0xFF;

    u8aNetMask[0] = (iface->netmask.u_addr.ip4.addr >> 0) & 0xFF;
    u8aNetMask[1] = (iface->netmask.u_addr.ip4.addr >> 8) & 0xFF;
    u8aNetMask[2] = (iface->netmask.u_addr.ip4.addr >> 16) & 0xFF;
    u8aNetMask[3] = (iface->netmask.u_addr.ip4.addr >> 24) & 0xFF;

    u8aGateway[0] = (iface->gw.u_addr.ip4.addr >> 0) & 0xFF;
    u8aGateway[1] = (iface->gw.u_addr.ip4.addr >> 8) & 0xFF;
    u8aGateway[2] = (iface->gw.u_addr.ip4.addr >> 16) & 0xFF;
    u8aGateway[3] = (iface->gw.u_addr.ip4.addr >> 24) & 0xFF;

    Opl_Wifi_ApInfo_Get(&stInfo);
    // BleWifi_Wifi_Query_Status(BLEWIFI_WIFI_QUERY_AP_INFO , (void *)&stInfo);

    pu8Pos = pu8Data = malloc(sizeof(T_WmWifiStatusInfo));
    if (pu8Data == NULL)
    {
        OPL_LOG_ERRO(OPL, "malloc fail");
        return;
    }

    u8StrLen = strlen((char *)&stInfo.ssid);

    if (u8StrLen == 0)
    {
        u8Status = 1; // Return Failure
        if (u16Type == OPL_DATA_IND_IP_STATUS_NOTIFY)     // if failure, don't notify the status
            goto release;
    }
    else
    {
        u8Status = 0; // Return success
    }

    /* Status */
    *pu8Pos++ = u8Status;

    /* ssid length */
    *pu8Pos++ = u8StrLen;

   /* SSID */
    if (u8StrLen != 0)
    {
        memcpy(pu8Pos, stInfo.ssid, u8StrLen);
        pu8Pos += u8StrLen;
    }

   /* BSSID */
    memcpy(pu8Pos, stInfo.bssid, 6);
    pu8Pos += 6;

    /* IP */
    memcpy(pu8Pos, (char *)u8aIp, 4);
    pu8Pos += 4;

    /* MASK */
    memcpy(pu8Pos,  (char *)u8aNetMask, 4);
    pu8Pos += 4;

    /* GATEWAY */
    memcpy(pu8Pos,  (char *)u8aGateway, 4);
    pu8Pos += 4;

    u16DataLen = (pu8Pos - pu8Data);

    // BLEWIFI_DUMP("Wi-Fi status info data", pu8Data, u16DataLen);

    /* create Wi-Fi status info data packet */
    OPL_DataSendEncap(u16Type, pu8Data, u16DataLen);
    //BleWifi_Ble_DataSendEncap(BLEWIFI_RSP_WIFI_STATUS, pu8Data, u16DataLen);

release:
    free(pu8Data);
}

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void OPL_DataHandler_WifiScanDoneIndCb(T_OplErr tEvtRst)
{
    OPL_LOG_DEBG(OPL, "Wifi scan done ind %d", tEvtRst);

//     stScanConfig.ssid = MP_WIFI_DEFAULT_SSID;
//     stScanConfig.show_hidden = 1;
//     stScanConfig.scan_type = WIFI_SCAN_TYPE_MIX;

    // if(g_u8MpScanTestFlag == true)
    // {
    //     /* Get APs list */
    //     wifi_scan_get_ap_list(&stScanList);

    //     /* Search if AP matched */
    //     for (i=0; i< stScanList.num; i++)
    //     {
    //         if (memcmp(stScanList.ap_record[i].ssid, stScanConfig.ssid, strlen((char *)stScanConfig.ssid)) == 0)
    //         {
    //             u8IsMatched = true;
    //             break;
    //         }
    //     }
    //     if (true == u8IsMatched)  //find the specified SSID
    //     {
    //         g_u8MpIsFindTargetAP = true;
    //         g_u8MpScanTestFlag = false;
    //         g_u8MpScanProcessFinished = true;

    //         printf("MP mode start\r\n");

    //         if(stScanList.ap_record[i].rssi <= -60)
    //         {
    //             printf("NO_ROUTER\r\n");
    //             g_u8MpBlinkingMode = MP_BLINKING_NO_ROUTER;
    //         }
    //         else
    //         {
    //             printf("NO_SERVER\r\n");
    //             g_u8MpBlinkingMode = MP_BLINKING_NO_SERVER;
    //         }
    //         App_Ctrl_LedStatusChange();
    //     }
    //     else   //can not find the specified SSID, trigger wifi scan again
    //     {
    //         g_u8MpIsFindTargetAP = false;

    //         if(g_u8MpScanCnt < MP_WIFI_SCAN_RETRY_TIMES)
    //         {
    //             g_u8MpScanCnt ++;
    //             BleWifi_Wifi_Scan_Req(&stScanConfig);
    //         }
    //         else
    //         {
    //             g_u8MpScanCnt = 0;
    //             g_u8MpScanTestFlag = false;
    //             g_u8MpScanProcessFinished = true;
    //             App_Ctrl_MsgSend(APP_CTRL_MSG_WIFI_INIT_COMPLETE, NULL, 0);
    //         }
    //     }
    // }

    if(OPL_DATA_CONN_TYPE_BSSID == g_tOplDataConnCfg.u8ConnectType)
    {
        // CK_DATA_REQ_SCAN cmd, just do report scan list
        _OPL_DataHandler_SendScanReport();
        OPL_DataSendResponse(OPL_DATA_RSP_SCAN_END, 0);
    }
}

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void OPL_DataHandler_WifiConnectionIndCb(T_OplErr tEvtRst)
{
    OPL_LOG_DEBG(OPL, "Wifi connection ind %d", tEvtRst);

    if(OPL_OK == tEvtRst)
    {
        OPL_LOG_INFO(OPL, "Reply wifi connected");

        // send connect success
        OPL_DataSendResponse(OPL_DATA_RSP_CONNECT, OPL_DATA_WIFI_CONNECTED_DONE);
        
        // send ip message
        _OPL_DataHandler_SendStatusInfo(OPL_DATA_IND_IP_STATUS_NOTIFY);
    }
    else
    {
        OPL_LOG_INFO(OPL, "Reply wifi connect fail");

        // send connect fail
        OPL_DataSendResponse(OPL_DATA_RSP_CONNECT, OPL_DATA_WIFI_CONNECTED_FAIL);
    }

    // UpdateBatteryContent();

//     BleWifi_Wifi_UpdateBeaconInfo();
//     stSetDtim.u32DtimValue = (uint32_t)BleWifi_Wifi_GetDtimSetting();
//     stSetDtim.u32DtimEventBit = BW_WIFI_DTIM_EVENT_BIT_DHCP_USE;
//     BleWifi_Wifi_Set_Config(BLEWIFI_WIFI_SET_DTIM , (void *)&stSetDtim);

//     BleWifi_COM_EventStatusSet(g_tAppCtrlEventGroup , APP_CTRL_EVENT_BIT_WIFI_AUTOCONN_LED, false);
//     App_Ctrl_LedStatusChange();

//     if (false == BleWifi_COM_EventStatusGet(g_tAppCtrlEventGroup, APP_CTRL_EVENT_BIT_WAIT_UPDATE_HOST))
//     {
//         if(strncmp(g_tHostInfo.ubaHostInfoURL, HOSTINFO_URL, sizeof(HOSTINFO_URL))!=0)
//         {
//             Iot_Data_TxTask_MsgSend(IOT_DATA_TX_MSG_CLOUD_CONNECTION, NULL, 0);
//         }
//         else
//         {
//             printf("URL is default value, skip it.\n");
//         }
//     }

//     // if support cloud, send message cloud connect to app_ctrl or iot_data
// #if(STRESS_TEST_AUTO_CONNECT == 1)
//     g_u32StressTestCount++;
//     osDelay(APP_CTRL_RESET_DELAY);
//     printf("Auto test count = %u\r\n",g_u32StressTestCount);
//     BleWifi_Wifi_Stop_Conn();
//     BleWifi_Wifi_Start_Conn(NULL);
// #endif

}

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void OPL_DataHandler_WifiDisconnectionIndCb(T_OplErr tEvtRst)
{
    OPL_DataSendResponse(OPL_DATA_RSP_DISCONNECT, tEvtRst);
}
// static void App_Ctrl_TaskEvtHandler_WifiDisconnection(uint32_t u32EvtType, void *pData, uint32_t u32Len)
// {
//     blewifi_wifi_set_dtim_t stSetDtim = {0};
//     uint8_t *pu8Reason = (uint8_t*)(pData);

//     printf("BLEWIFI: MSG APP_CTRL_MSG_WIFI_DISCONNECTION\r\n");
//     printf("reason %d\r\n", *pu8Reason);
//     g_eLedIOPort = LED_IO_PORT_WIFI_DISCONNECTED;

// #ifdef __BLEWIFI_TRANSPARENT__
//     msg_print_uart1("WIFI DISCONNECTION\n");
// #endif

//     BleWifi_COM_EventStatusSet(g_tAppCtrlEventGroup, APP_CTRL_EVENT_BIT_NOT_CNT_SRV, false);
//     BleWifi_COM_EventStatusSet(g_tAppCtrlEventGroup , APP_CTRL_EVENT_BIT_WIFI_CONNECTING , false);
//     BleWifi_COM_EventStatusSet(g_tAppCtrlEventGroup , APP_CTRL_EVENT_BIT_WIFI_CONNECTED , false);
//     BleWifi_COM_EventStatusSet(g_tAppCtrlEventGroup , APP_CTRL_EVENT_BIT_WIFI_GOT_IP , false);

//     if(true == BleWifi_COM_EventStatusGet(g_tAppCtrlEventGroup, APP_CTRL_EVENT_BIT_WAIT_UPDATE_HOST) &&
//        true == BleWifi_COM_EventStatusGet(g_tAppCtrlEventGroup, APP_CTRL_EVENT_BIT_WIFI_USER_CONNECTING_EXEC))
//     {
//         BleWifi_COM_EventStatusSet(g_tAppCtrlEventGroup, APP_CTRL_EVENT_BIT_WAIT_UPDATE_HOST, false);
//         BleWifi_COM_EventStatusSet(g_tAppCtrlEventGroup , APP_CTRL_EVENT_BIT_WIFI_USER_CONNECTING_EXEC , false);

//         if(*pu8Reason == WIFI_REASON_CODE_PREV_AUTH_INVALID ||
//            *pu8Reason == WIFI_REASON_CODE_4_WAY_HANDSHAKE_TIMEOUT ||
//            *pu8Reason == WIFI_REASON_CODE_GROUP_KEY_UPDATE_TIMEOUT)
//         {

//             BleWifi_Ble_SendResponse(BLEWIFI_RSP_CONNECT, BLEWIFI_WIFI_PASSWORD_FAIL);
//         }
//         else if(*pu8Reason == WIFI_REASON_CODE_CONNECT_NOT_FOUND)
//         {
//             BleWifi_Ble_SendResponse(BLEWIFI_RSP_CONNECT, BLEWIFI_WIFI_AP_NOT_FOUND);
//         }
//         else
//         {
//             BleWifi_Ble_SendResponse(BLEWIFI_RSP_CONNECT, BLEWIFI_WIFI_CONNECTED_FAIL);
//         }
//     }
//     else
//     {
//         BleWifi_COM_EventStatusSet(g_tAppCtrlEventGroup , APP_CTRL_EVENT_BIT_WIFI_USER_CONNECTING_EXEC , false);
//     }

// #ifdef __SONOFF__
//     if (( true == BleWifi_COM_EventStatusGet(g_tAppCtrlEventGroup, APP_CTRL_EVENT_BIT_TEST_MODE))
//         && (true == BleWifi_COM_EventStatusGet(g_tAppCtrlEventGroup, APP_CTRL_EVENT_BIT_AT_WIFI_MODE)))
//     {
//         msg_print_uart1("AT+SIGNAL=0\r\n");
//         BleWifi_COM_EventStatusSet(g_tAppCtrlEventGroup , APP_CTRL_EVENT_BIT_AT_WIFI_MODE , false);
// #ifdef TEST_MODE_DEBUG_ENABLE
//         msg_print_uart1("WifiDisconnection\r\n");
// #endif
//         FinalizedATWIFIcmd();
//         return ;
//     }
// #endif

//     stSetDtim.u32DtimValue = 0;
//     stSetDtim.u32DtimEventBit = BW_WIFI_DTIM_EVENT_BIT_DHCP_USE;
//     BleWifi_Wifi_Set_Config(BLEWIFI_WIFI_SET_DTIM , (void *)&stSetDtim);
//     BleWifi_COM_EventStatusSet(g_tAppCtrlEventGroup , APP_CTRL_EVENT_BIT_WIFI_AUTOCONN_LED , true);
//     App_Ctrl_LedStatusChange();
// }

#endif /* OPL_DATA_ENABLED */
