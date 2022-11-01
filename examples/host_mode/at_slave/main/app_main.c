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
*  app_main.c
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

#include <stdarg.h>
#include "app_main.h"
#include "app_at_cmd.h"
#include "ble_mngr.h"
#include "cmsis_os.h"
#include "cloud_ctrl.h"
#include "cloud_config.h"
#include "cloud_kernel.h"
#include "opl_data_prot.h"
#include "opl_data_hdl.h"
#include "opl_svc.h"
#include "ota_mngr.h"
#include "gap_svc.h"
#include "gatt_svc.h"
#include "log.h"
#include "net_mngr_api.h"
#include "pwr_save.h"
#include "rf_pwr.h"
#include "wifi_mngr_api.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#define HOST_MODE_WAKEUP_PIN                            (GPIO_IDX_04)
#define HOST_MODE_CMD_SEND                              (msg_print_uart1)

#define AT_CMD_ACK_DATA_LEN                             (128)

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

osThreadId g_tAppTaskId;
osMessageQId g_tAppQueueId;
osTimerId g_tAppOplPrvTimer;
osTimerId g_tSysTimer;

static T_AppEvtHandlerTbl g_tAppEvtHandlerTbl[] = 
{
    {APP_EVT_SYS_TIMER_TIMEOUT,             APP_EvtHandler_SysTimerTimeout},
    {APP_EVT_ENT_SLEEP_REQ,                 APP_EvtHandler_EntSleepReq},

    {APP_EVT_OPL_PRV_START_REQ,             APP_EvtHandler_OplPrvSwitchReq},
    {APP_EVT_OPL_PRV_STOP_REQ,              APP_EvtHandler_OplPrvSwitchReq},
    {APP_EVT_OPL_PRV_TIMEOUT_IND,           APP_EvtHandler_OplPrvSwitchReq},

    {APP_EVT_BLE_STATUS_REQ,                APP_EvtHandler_BleWifiStatusReq},
    {APP_EVT_BLE_START_ADV_IND,             APP_EvtHandler_BleEventIndicate},
    {APP_EVT_BLE_STOP_ADV_IND,              APP_EvtHandler_BleEventIndicate},
    {APP_EVT_BLE_CONNECTED_IND,             APP_EvtHandler_BleEventIndicate},
    {APP_EVT_BLE_DISCONNECTED_IND,          APP_EvtHandler_BleEventIndicate},
    {APP_EVT_BLE_DATA_IND,                  APP_EvtHandler_BleDataInd},

    {APP_EVT_WIFI_SCAN_REQ,                 APP_EvtHandler_WifiCommandReq},
    {APP_EVT_WIFI_CONN_REQ,                 APP_EvtHandler_WifiCommandReq},
    {APP_EVT_WIFI_QCONN_REQ,                APP_EvtHandler_WifiCommandReq},
    {APP_EVT_WIFI_STATUS_REQ,               APP_EvtHandler_BleWifiStatusReq},

    {APP_EVT_WIFI_SCAN_IND,                 APP_EvtHandler_WifiScanInd},
    {APP_EVT_NETWORK_UP_IND,                APP_EvtHandler_NetworkUp},
    {APP_EVT_NETWORK_DOWN_IND,              APP_EvtHandler_NetworkDown},
    {APP_EVT_NETWORK_RESET_IND,             APP_EvtHandler_NetworkReset},

    {APP_EVT_CLOUD_CONNECT_REQ,             APP_EvtHandler_CloudConnectReq},
    {APP_EVT_CLOUD_DISCONNECT_REQ,          APP_EvtHandler_CloudDisconnectReq},
    {APP_EVT_CLOUD_SEND_REQ,                APP_EvtHandler_CloudSendReq},

    {APP_EVT_CLOUD_CONNECT_IND,             APP_EvtHandler_CloudConnectInd},
    {APP_EVT_CLOUD_DISCONNECT_IND,          APP_EvtHandler_CloudDisconnectInd},
    {APP_EVT_CLOUD_RECV_IND,                APP_EvtHandler_CloudRecvInd},

    {APP_EVT_MASTER_WAKEUP,                 APP_EvtHandler_MasterWakeup},
    {APP_EVT_AT_TEST_REQ,                   APP_EvtHandler_AtTestReq},

    {0xFFFFFFFF,                            NULL},
};

// host mode ack command list in string (the indexing of string array must same as T_HostModeAckCmdList)
int8_t *i8HostModeAckCmdListStrTbl[] = 
{
    "+BOOT",
    "+READY",
    "+WAKEUP",
    "+PROVISION",
    "+BLESTS",
    "+WIFISCAN",
    "+WIFICONN",
    "+WIFIDISCONN",
    "+NETWORKUP",
    "+NETWORKDOWN",
    "+NETWORKRESET",
    "+WIFISTS",
    "+CLOUDUP",
    "+CLOUDDOWN",
    "+CLOUDKEEPALIVE",
    "+CLOUDTXTOPIC",
    "+CLOUDRXTOPIC",
    "+TXRSP",
    "+RXDATA",
    "+TEST",
    "+NOTSUPPORT",
    "OK",
    "ERROR",

    // add your command here
};

uint8_t g_u8WifiStatus = 0;
uint8_t g_u8BleStatus = 0;

// Sec 7: declaration of static function prototype

void APP_WakeupPinInit(void);
void APP_WakeupPinDeInit(void);
void APP_BleAdvDataInit(void);
void APP_BleScanRspDataInit(void);
void APP_SysInit(void);
void APP_DataInit(void);
void APP_TaskInit(void);
void APP_BleInit(void);
void APP_NetInit(void);
void APP_CldInit(void);
void APP_UserAtInit(void);

/***********
C Functions
***********/
// Sec 8: C Functions

// indicate callback for each type request

//////////////////////////////////// 
//// Callback group
//////////////////////////////////// 

// add your callback function here

void APP_NmUnsolicitedCallback(T_NmUslctdEvtType tEvtType, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // tEvtType refer to net_mngr_api.h
    switch(tEvtType)
    {
        case NM_USLCTD_EVT_NETWORK_UP:
        {
            APP_SendMessage(APP_EVT_NETWORK_UP_IND, pu8Data, u32DataLen);

            g_u8WifiStatus = 1;

#if defined(HOST_MODE_MQTT)
            // set network up (for cloud to checking wifi connection)
            Cloud_NetworkStatusSet(true);
#endif

            break;
        }
        case NM_USLCTD_EVT_NETWORK_DOWN:
        {
            APP_SendMessage(APP_EVT_NETWORK_DOWN_IND, NULL, 0);

            g_u8WifiStatus = 0;

#if defined(HOST_MODE_MQTT)
            // set network down (for cloud to checking wifi connection)
            Cloud_NetworkStatusSet(false);
#endif

            break;
        }
        case NM_USLCTD_EVT_NETWORK_RESET:
        {
            APP_SendMessage(APP_EVT_NETWORK_RESET_IND, NULL, 0);

            g_u8WifiStatus = 0;

#if defined(HOST_MODE_MQTT)
            // set network down (for cloud to checking wifi connection)
            Cloud_NetworkStatusSet(false);
#endif

            break;
        }
        default:
        {
            // should not be here

            break;
        }
    }
}

void APP_BleUnsolicitedCallback(uint16_t u16EvtType, T_OplErr tEvtRst, uint8_t *pu8Data, uint32_t u32DataLen)
{
    switch(u16EvtType)
    {
        case USLCTED_CB_EVT_BLE_INIT:
        {
            // initialize ble advertise data
            APP_BleAdvDataInit();

            // initialize ble scan response data
            APP_BleScanRspDataInit();

            g_u8BleStatus = 0;

            break;
        }
        case USLCTED_CB_EVT_BLE_ENT_ADVERTISE:
        {
            APP_SendMessage(APP_EVT_BLE_START_ADV_IND, NULL, 0);

            g_u8BleStatus = 1;

            break;
        }
        case USLCTED_CB_EVT_BLE_EXI_ADVERTISE:
        {
            APP_SendMessage(APP_EVT_BLE_STOP_ADV_IND, NULL, 0);

            g_u8BleStatus = 0;

            break;
        }
        case USLCTED_CB_EVT_BLE_CONNECTED:
        {
            APP_SendMessage(APP_EVT_BLE_CONNECTED_IND, pu8Data, u32DataLen);

            g_u8BleStatus = 2;

            break;
        }
        case USLCTED_CB_EVT_BLE_DISCONNECT:
        {
            APP_SendMessage(APP_EVT_BLE_DISCONNECTED_IND, NULL, 0);

            g_u8BleStatus = 0;

            break;
        }
        default:
        {
            // should not be here
         
            break;
        }
    }
}

void APP_CloudStatusCallback(T_CloudStatus tCloudStatus, void *pData, uint32_t u32DataLen)
{
    switch(tCloudStatus)
    {
        case CLOUD_CB_STA_INIT_DONE:
        case CLOUD_CB_STA_INIT_FAIL:
            break;
        case CLOUD_CB_STA_CONN_DONE:
        {
            APP_SendMessage(APP_EVT_CLOUD_CONNECT_IND, NULL, 0);
            break;
        }
        case CLOUD_CB_STA_CONN_FAIL:
        case CLOUD_CB_STA_RECONN_DONE:
            break;
        case CLOUD_CB_STA_DISCONN:
        {
            APP_SendMessage(APP_EVT_CLOUD_DISCONNECT_IND, NULL, 0);
            break;
        }
#if defined(HOST_MODE_TCP)
        case CLOUD_CB_STA_RECV_IND:
        {
            APP_SendMessage(APP_EVT_CLOUD_RECV_IND, pData, u32DataLen);
            break;
        }
#elif defined(HOST_MODE_MQTT)
        case CLOUD_CB_STA_SUB_DONE:
        case CLOUD_CB_STA_SUB_FAIL:
        case CLOUD_CB_STA_UNSUB_DONE:
        case CLOUD_CB_STA_UNSUB_FAIL:
        case CLOUD_CB_STA_PUB_DONE:
        case CLOUD_CB_STA_PUB_FAIL:
#endif
        default:
            break;
    }
}


void APP_NmScanDoneIndCallback(T_OplErr tEvtRst)
{
    APP_SendMessage(APP_EVT_WIFI_SCAN_IND, NULL, 0);
}

void APP_NmConnectRequestIndCallback(T_OplErr tEvtRst)
{
    if(OPL_OK != tEvtRst)
    {
    //     APP_SendMessage(APP_EVT_WIFI_CONNECT_FAIL, NULL, 0);
        APP_SendMessage(APP_EVT_NETWORK_UP_IND, NULL, 0);
    }

}

static void APP_OplPrvTimerTimeoutHandler(void const *argu)
{
    APP_SendMessage(APP_EVT_OPL_PRV_TIMEOUT_IND, NULL, 0);
}

static void APP_SysTimerTimeoutHandler(void const *argu)
{
    APP_SendMessage(APP_EVT_SYS_TIMER_TIMEOUT, NULL, 0);
}

static void APP_WakeupPinCallback(E_GpioIdx_t tGpioIdx)
{
    // ISR level dont do printf
    APP_WakeupPinDeInit();

    // send event message
    APP_SendMessage(APP_EVT_MASTER_WAKEUP, NULL, 0);
}

//////////////////////////////////// 
//// Event handler group
////////////////////////////////////

// add your event handler function here

static void APP_EvtHandler_SysTimerTimeout(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{		
    APP_HostModeResponseAck(AT_CMD_ACK_DEVICE_READY, NULL, 0);
}

static void APP_EvtHandler_EntSleepReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    uint8_t u8SleepMode = *((uint8_t *)pData);

    APP_WakeupPinInit();

    if(1 == u8SleepMode)
    {
        PS_EnterSmartSleep(0);
    }
    else if(2 == u8SleepMode)
    {
        // enter timer sleep
    }
    else if(3 == u8SleepMode)
    {
        // enter deep sleep
    }
}

static void APP_EvtHandler_OplPrvSwitchReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    uint8_t u8Payload = 0x30; // '0' : success

    switch(u32EventId)
    {
        case APP_EVT_OPL_PRV_START_REQ:
        {
            uint32_t u32OplPrvTimeout = *((uint32_t *)pData);

            if(OPL_OK != Opl_Ble_Start_Req(false))
            {
                u8Payload = 0x33; // '3' : req err
                APP_HostModeResponseAck(AT_CMD_ACK_PROVISION, &u8Payload, sizeof(u8Payload));
                break;
            }

            osTimerStart(g_tAppOplPrvTimer, u32OplPrvTimeout * 1000);

            break;
        }

        case APP_EVT_OPL_PRV_STOP_REQ:
        {
            if(OPL_OK != Opl_Ble_Stop_Req())
            {
                u8Payload = 0x33; // '3' : req err
                APP_HostModeResponseAck(AT_CMD_ACK_PROVISION, &u8Payload, sizeof(u8Payload));
                break;
            }

            osTimerStop(g_tAppOplPrvTimer);

            break;
        }

        case APP_EVT_OPL_PRV_TIMEOUT_IND:
        {
            if(OPL_OK != Opl_Ble_Stop_Req())
            {
                u8Payload = 0x33; // '3' : req err
                APP_HostModeResponseAck(AT_CMD_ACK_PROVISION, &u8Payload, sizeof(u8Payload));
            }
            else
            {
                u8Payload = 0x31; // '1' : timeout
                APP_HostModeResponseAck(AT_CMD_ACK_PROVISION, &u8Payload, sizeof(u8Payload));
            }

            osTimerStop(g_tAppOplPrvTimer);

            break;
        }

        default:
            break;
    }
}

// static void APP_EvtHandler_BleDataSendReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
// {

// }

static void APP_EvtHandler_BleEventIndicate(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    uint8_t u8Payload = 0x30; // '0' : success

    switch(u32EventId)
    {
        case APP_EVT_BLE_START_ADV_IND:
        {
            APP_HostModeResponseAck(AT_CMD_ACK_PROVISION, &u8Payload, sizeof(u8Payload));

            uint8_t u8aBleMac[6] = {0};
            Opl_Ble_MacAddr_Read(u8aBleMac);

            OPL_LOG_INFO(APP, "BLE advertising...Device mac [%0X:%0X:%0X:%0X:%0X:%0X]", u8aBleMac[0],
                                                                                        u8aBleMac[1],
                                                                                        u8aBleMac[2],
                                                                                        u8aBleMac[3],
                                                                                        u8aBleMac[4],
                                                                                        u8aBleMac[5]);
            break;
        }

        case APP_EVT_BLE_STOP_ADV_IND:
        {
            u8Payload = 0x32; // '2' : user req
            APP_HostModeResponseAck(AT_CMD_ACK_PROVISION, &u8Payload, sizeof(u8Payload));

            OPL_LOG_INFO(APP, "BLE stop advertise");

            break;
        }

        case APP_EVT_BLE_CONNECTED_IND:
        {
            uint8_t u8aBlePeerMac[6] = {0};
            memcpy(u8aBlePeerMac, pData, sizeof(u8aBlePeerMac));

            OPL_LOG_INFO(APP, "BLE connected...Peer mac [%0X:%0X:%0X:%0X:%0X:%0X]", u8aBlePeerMac[0],
                                                                                    u8aBlePeerMac[1],
                                                                                    u8aBlePeerMac[2],
                                                                                    u8aBlePeerMac[3],
                                                                                    u8aBlePeerMac[4],
                                                                                    u8aBlePeerMac[5]);
            break;
        }

        case APP_EVT_BLE_DISCONNECTED_IND:
        {
            OPL_LOG_INFO(APP, "BLE disconnected");
            break;
        }

        default:
            break;
    }
}

static void APP_EvtHandler_BleDataInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_DataRecvHandler(pData, (uint16_t)u32DataLen);
}

static void APP_EvtHandler_WifiCommandReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    uint8_t u8Payload = 0x30; // '0' : success
    switch(u32EventId)
    {
        case APP_EVT_WIFI_SCAN_REQ:
        {
            OPL_LOG_INFO(APP, "scan req");

            if(OPL_OK != APP_NmWifiScanReq(APP_NmScanDoneIndCallback))
            {
                u8Payload = 0x33; // '3' : req err
                APP_HostModeResponseAck(AT_CMD_ACK_WIFI_SCAN, &u8Payload, sizeof(u8Payload));
            }

            break;
        }

        case APP_EVT_WIFI_CONN_REQ:
        {
            T_NmWifiCnctConfig tWifiCnctConfig = *((T_NmWifiCnctConfig *)pData);

            if(OPL_OK != APP_NmWifiCnctReq(&tWifiCnctConfig, APP_NmConnectRequestIndCallback))
            {
                u8Payload = 0x33; // '3' : req err
                APP_HostModeResponseAck(AT_CMD_ACK_WIFI_CONN, &u8Payload, sizeof(u8Payload));
            }

            break;
        }

        case APP_EVT_WIFI_QCONN_REQ:
        {
            T_NmWifiQCnctSet tNmWifiQCnctSet = *((T_NmWifiQCnctSet *)pData);

            if(OPL_OK != APP_NmQuickCnctSetReq(&tNmWifiQCnctSet, APP_NmConnectRequestIndCallback))
            {
                u8Payload = 0x33; // '3' : req err
                APP_HostModeResponseAck(AT_CMD_ACK_WIFI_CONN, &u8Payload, sizeof(u8Payload));
            }

            break;
        }

        default:
            break;
    }
}

static void APP_EvtHandler_WifiScanInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    uint8_t u8Payload = 0x30; // '0' : success

    wifi_scan_list_t *pstScanList = NULL;

    pstScanList = (wifi_scan_list_t *)malloc(sizeof(wifi_scan_list_t));

    if(NULL == pstScanList)
    {
        OPL_LOG_ERRO(APP, "malloc fail");

        u8Payload = 0x33; // '3' : req err
        APP_HostModeResponseAck(AT_CMD_ACK_WIFI_SCAN, &u8Payload, sizeof(u8Payload));

        return;
    }

    memset(pstScanList, 0, sizeof(wifi_scan_list_t));

    // get the information of scan list
    Opl_Wifi_ScanList_Get(pstScanList);

    int iCount;
    for (iCount = 0; iCount < pstScanList->num; iCount++)
    {
        if (999 != pstScanList->ap_record[iCount].rssi)
        {
            // SSID LEN + ((MAC LEN * 2) + COLON) + (channel + rssi + comma)
            // (32+1)   + ((6*2)+5)               + 10
            char cApInfo[WIFI_MAX_LENGTH_OF_SSID + ((WIFI_MAC_ADDRESS_LENGTH * 2) + 5) + 10];

            sprintf(cApInfo, "%s,%x:%x:%x:%x:%x:%x,%d,%d", pstScanList->ap_record[iCount].ssid
                                                          , pstScanList->ap_record[iCount].bssid[0]
                                                          , pstScanList->ap_record[iCount].bssid[1]
                                                          , pstScanList->ap_record[iCount].bssid[2]
                                                          , pstScanList->ap_record[iCount].bssid[3]
                                                          , pstScanList->ap_record[iCount].bssid[4]
                                                          , pstScanList->ap_record[iCount].bssid[5]
                                                          , pstScanList->ap_record[iCount].channel
                                                          , pstScanList->ap_record[iCount].rssi);

            APP_HostModeResponseAck(AT_CMD_ACK_WIFI_SCAN, (uint8_t *)cApInfo, strlen(cApInfo));
        }
    }

    free(pstScanList);

    APP_HostModeResponseAck(AT_CMD_ACK_WIFI_SCAN, &u8Payload, sizeof(u8Payload));
}

static void APP_EvtHandler_NetworkUp(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "Network connected");

    uint8_t u8Payload = 0x30; // '0' : success

    // find ip number in network interface
    uint32_t u32Ip = *((uint32_t*)pData);
    uint8_t u8aIp[4] = {0};

    u8aIp[0] = (u32Ip >> 0) & 0xFF;
    u8aIp[1] = (u32Ip >> 8) & 0xFF;
    u8aIp[2] = (u32Ip >> 16) & 0xFF;
    u8aIp[3] = (u32Ip >> 24) & 0xFF;

    OPL_LOG_INFO(APP, "WI-FI IP [%d.%d.%d.%d]", u8aIp[0], u8aIp[1], u8aIp[2], u8aIp[3]);

    APP_HostModeResponseAck(AT_CMD_ACK_NETWORK_UP, &u8Payload, sizeof(u8Payload));

    // stop provision timer
    osTimerStop(g_tAppOplPrvTimer);
}

static void APP_EvtHandler_NetworkDown(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "Network disconnected");

    uint8_t u8Payload = 0x30; // '0' : success

    APP_HostModeResponseAck(AT_CMD_ACK_NETWORK_DOWN, &u8Payload, sizeof(u8Payload));

    Cloud_MsgSend(CLOUD_EVT_TYPE_DISCONNECT, NULL, 0);
}

static void APP_EvtHandler_NetworkReset(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "Network reset");

    uint8_t u8Payload = 0x30; // '0' : success

    APP_HostModeResponseAck(AT_CMD_ACK_NETWORK_RESET, &u8Payload, sizeof(u8Payload));
}

static void APP_EvtHandler_BleWifiStatusReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    uint8_t u8Payload = 0x30; // '0' : success

    if(APP_EVT_BLE_STATUS_REQ == u32EventId)
    {
        u8Payload += g_u8BleStatus;

        APP_HostModeResponseAck(AT_CMD_ACK_BLE_STATUS, &u8Payload, sizeof(u8Payload));
    }
    else if(APP_EVT_WIFI_STATUS_REQ == u32EventId)
    {
        u8Payload += g_u8WifiStatus;

        APP_HostModeResponseAck(AT_CMD_ACK_WIFI_STATUS, &u8Payload, sizeof(u8Payload));
    }
}

static void APP_EvtHandler_CloudConnectReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    uint8_t u8Payload = 0x30; // '0' : success

    T_CloudConnInfo tCloudConnInfo = *((T_CloudConnInfo *)pData);
    T_OplErr tEvtRst = Cloud_MsgSend(CLOUD_EVT_TYPE_ESTABLISH, (uint8_t *)&tCloudConnInfo, sizeof(tCloudConnInfo));

    if(OPL_OK != tEvtRst)
    {
        u8Payload = 0x33; // '3' : req err
        APP_HostModeResponseAck(AT_CMD_ACK_CLOUD_CONNECT, &u8Payload, sizeof(u8Payload));
    }
}

static void APP_EvtHandler_CloudDisconnectReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    uint8_t u8Payload = 0x30; // '0' : success

    T_OplErr tEvtRst = Cloud_MsgSend(CLOUD_EVT_TYPE_DISCONNECT, NULL, 0);

    if(OPL_OK != tEvtRst)
    {
        u8Payload = 0x33; // '3' : req err
        APP_HostModeResponseAck(AT_CMD_ACK_CLOUD_DISCONNECT, &u8Payload, sizeof(u8Payload));
    }
}

static void APP_EvtHandler_CloudSendReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    // nothing to do
}

static void APP_EvtHandler_CloudConnectInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    uint8_t u8Payload = 0x30; // '0' : success

    APP_HostModeResponseAck(AT_CMD_ACK_CLOUD_CONNECT, &u8Payload, sizeof(u8Payload));
}

static void APP_EvtHandler_CloudDisconnectInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    uint8_t u8Payload = 0x30; // '0' : success

    APP_HostModeResponseAck(AT_CMD_ACK_CLOUD_DISCONNECT, &u8Payload, sizeof(u8Payload));
}

static void APP_EvtHandler_CloudRecvInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
#if defined(HOST_MODE_TCP)
    OPL_LOG_INFO(AT, "rcv: %d:%s", (int)u32DataLen, (char *)pData);

    static char cRecvData[CLOUD_PAYLOAD_LEN];
    uint32_t u32RecvDataStrLen = 0;
    memset(cRecvData, 0, sizeof(cRecvData));

    u32RecvDataStrLen += sprintf(cRecvData, "%d:%s", (int)u32DataLen, (char *)pData);
    APP_HostModeResponseAck(AT_CMD_ACK_CLOUD_RX_RECV, (uint8_t *)cRecvData, u32RecvDataStrLen);
#endif
}

static void APP_EvtHandler_MasterWakeup(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "Received wakeup signal");

#if (APP_HOST_MODE_PWR_SAVE_MODE == 0)
    // exit from smart sleep
    PS_ExitSmartSleep();

    APP_HostModeResponseAck(AT_CMD_ACK_WAKEUP_FROM_SLEEP, NULL, 0);
#endif
}

static void APP_EvtHandler_AtTestReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    APP_HostModeResponseAck(AT_CMD_ACK_TEST, NULL, 0);
}

//////////////////////////////////// 
//// APP function group
////////////////////////////////////

// add your application function here
T_OplErr APP_HostModeResponseAck(T_HostModeAckCmdList tHostModeAckCmdIdx, uint8_t *payload, uint32_t payloadlen)
{
    int8_t i8AckMsg[AT_CMD_ACK_DATA_LEN] = {0};
    char *i8AckMsgPtr = (char *)&i8AckMsg;

    uint32_t u32AckMsgOffset= 0;

    if(AT_CMD_ACK_MAX <= tHostModeAckCmdIdx)
    {
        OPL_LOG_WARN(APP, "ACK cmd index %d", tHostModeAckCmdIdx);
        return OPL_ERR;
    }

    u32AckMsgOffset += sprintf(i8AckMsgPtr + u32AckMsgOffset, "%s", (char *)i8HostModeAckCmdListStrTbl[tHostModeAckCmdIdx]);

    if(payload != NULL)
    {
        u32AckMsgOffset += sprintf(i8AckMsgPtr + u32AckMsgOffset, ":");

        memcpy(i8AckMsgPtr + u32AckMsgOffset, payload, payloadlen);
        u32AckMsgOffset += payloadlen;

        // u32AckMsgOffset += sprintf(i8AckMsgPtr + u32AckMsgOffset, ":%s", payload);
    }

    u32AckMsgOffset += sprintf(i8AckMsgPtr + u32AckMsgOffset, "\r\n");

    // show sended msg
    OPL_LOG_INFO(APP, "ACK msg:(%d) %s", u32AckMsgOffset, i8AckMsgPtr);

    // transit data out thorugh AT port
    HOST_MODE_CMD_SEND(i8AckMsgPtr);

    return OPL_OK;
}

void APP_WakeupPinInit(void)
{
    uint32_t u32PinLevel = 0;

    // Get the status of GPIO (Low / High)
    u32PinLevel = Hal_Vic_GpioInput(HOST_MODE_WAKEUP_PIN);

    if(GPIO_LEVEL_LOW == u32PinLevel)
    {
        Hal_Vic_GpioCallBackFuncSet(HOST_MODE_WAKEUP_PIN, APP_WakeupPinCallback);
        Hal_Vic_GpioDirection(HOST_MODE_WAKEUP_PIN, GPIO_INPUT);
        Hal_Vic_GpioIntTypeSel(HOST_MODE_WAKEUP_PIN, INT_TYPE_RISING_EDGE);
        Hal_Vic_GpioIntInv(HOST_MODE_WAKEUP_PIN, 0);
        Hal_Vic_GpioIntMask(HOST_MODE_WAKEUP_PIN, 0);
        Hal_Vic_GpioIntEn(HOST_MODE_WAKEUP_PIN, 1);
    }
    else
    {
        Hal_Vic_GpioCallBackFuncSet(HOST_MODE_WAKEUP_PIN, APP_WakeupPinCallback);
        Hal_Vic_GpioDirection(HOST_MODE_WAKEUP_PIN, GPIO_INPUT);
        Hal_Vic_GpioIntTypeSel(HOST_MODE_WAKEUP_PIN, INT_TYPE_FALLING_EDGE);
        Hal_Vic_GpioIntInv(HOST_MODE_WAKEUP_PIN, 1);
        Hal_Vic_GpioIntMask(HOST_MODE_WAKEUP_PIN, 0);
        Hal_Vic_GpioIntEn(HOST_MODE_WAKEUP_PIN, 1);
    }
}

void APP_WakeupPinDeInit(void)
{
    Hal_Vic_GpioIntEn(HOST_MODE_WAKEUP_PIN, 0);
}

void APP_BleAdvDataInit(void)
{
    // ble advertise data inititate

    // user modify
    // *

    uint8_t u8Len;
    uint8_t au8BleAdvertData[] =
    {
        0x02,
        GAP_ADTYPE_FLAGS,
        GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
        // connection interval range
        0x05,
        GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE, 
        UINT16_LO(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
        UINT16_HI(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
        UINT16_LO(DEFAULT_DESIRED_MAX_CONN_INTERVAL),
        UINT16_HI(DEFAULT_DESIRED_MAX_CONN_INTERVAL),
        0x02,
        GAP_ADTYPE_POWER_LEVEL,
        0,
        0x11,
        GAP_ADTYPE_128BIT_COMPLETE,
        0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00
    };

    // error handle
    u8Len = sizeof(au8BleAdvertData);

    if (u8Len > BLE_ADV_SCAN_BUF_SIZE)
    {
        u8Len = BLE_ADV_SCAN_BUF_SIZE;
    }

    Opl_Ble_Advertise_Data_Set(au8BleAdvertData, u8Len);

    // *
}

void APP_BleScanRspDataInit(void)
{
    // ble scan response data inititate

    // user modify
    // *

    char u8aBleName[31];
    uint8_t u8BleNameLen = 0;
    uint8_t au8BleScanRspData[BLE_ADV_SCAN_BUF_SIZE];
    uint8_t u8aBleMac[6] = {0};

    Opl_Ble_MacAddr_Read(u8aBleMac);

    sprintf(u8aBleName, "%s_%0X:%0X:%0X:%0X", BLE_GAP_PF_DEVICE_NAME, 
                                              u8aBleMac[2],
                                              u8aBleMac[3],
                                              u8aBleMac[4],
                                              u8aBleMac[5]);

    u8BleNameLen = strlen(u8aBleName);

    au8BleScanRspData[0] = (u8BleNameLen + 1);
    au8BleScanRspData[1] = GAP_ADTYPE_LOCAL_NAME_COMPLETE;

    MemCopy((au8BleScanRspData + 2), u8aBleName, u8BleNameLen);

    if(OPL_OK != Opl_Ble_ScanRsp_Data_Set(au8BleScanRspData, (u8BleNameLen + 2)))
    {
        OPL_LOG_ERRO(APP, "Scan Rsp Data Set Fail");
    }

    // *
}

void APP_SysInit(void)
{
    // initialize rf power setting
    RF_PwrSet(RF_CFG_DEF_PWR_SET);

    // initialize ota manager
    OTA_Init();

    // user implement
}

void APP_DataInit(void)
{
    // user implement
}

void APP_TaskInit(void)
{
    // create timer
    osTimerDef_t tTimerDef;

    tTimerDef.ptimer = APP_SysTimerTimeoutHandler;
    g_tSysTimer = osTimerCreate(&tTimerDef, osTimerOnce, NULL);
    if(g_tSysTimer == NULL)
    {
        OPL_LOG_ERRO(APP, "Create sys timer fail");
    }

    tTimerDef.ptimer = APP_OplPrvTimerTimeoutHandler;
    g_tAppOplPrvTimer = osTimerCreate(&tTimerDef, osTimerOnce, NULL);
    if(g_tAppOplPrvTimer == NULL)
    {
        OPL_LOG_ERRO(APP, "Create opl prv timer fail");
    }

    // create message queue
    osMessageQDef_t tQueueDef;

    tQueueDef.item_sz = sizeof(T_AppMsgStruct);
    tQueueDef.queue_sz = APP_QUEUE_SIZE;
    g_tAppQueueId = osMessageCreate(&tQueueDef, NULL);

    if(g_tAppQueueId == NULL)
    {
        OPL_LOG_ERRO(APP, "Create queue fail");
    }

    // create task
    osThreadDef_t tTaskDef;
    
    tTaskDef.name = "App Main";
    tTaskDef.stacksize = APP_TASK_STACK_SIZE;
    tTaskDef.tpriority = APP_TASK_PRIORITY;
    tTaskDef.pthread = APP_Main;
    g_tAppTaskId = osThreadCreate(&tTaskDef, NULL);

    if(g_tAppTaskId == NULL)
    {
        OPL_LOG_ERRO(APP, "Create task fail");
    }

    OPL_LOG_INFO(APP, "Create task ok");

    // user implement
}

void APP_BleInit(void)
{
    // assign unsolicited callback function
    Opl_Ble_Uslctd_CB_Reg(&APP_BleUnsolicitedCallback);

    // register service
    GAP_Svc_Init();
    GATT_Svc_Init();

    // register opl service
    OPL_Svc_Init();

    // initialize the ble manager (auto-adv)
    Opl_Ble_Init_Req(false);

    // user implement
}

void APP_NetInit(void)
{
    // Network manager initialize (auto-connect enable)
    APP_NmInit(true, &APP_NmUnsolicitedCallback);

    // user implement
}

void APP_CldInit(void)
{
    // initialize cloud status callback
    Cloud_StatusCallbackRegister(APP_CloudStatusCallback);

#if defined(HOST_MODE_TCP)
    // initialize cloud
    Cloud_Init();

#elif defined(HOST_MODE_MQTT)
    // initialize cloud connect information
    T_CloudConnInfo tCloudConnInfo;

    tCloudConnInfo.u8AutoConn = 1;
    tCloudConnInfo.u8Security = 0;
    tCloudConnInfo.u16HostPort = MQTT_HOST_PORT;
    strcpy((char *)tCloudConnInfo.u8aHostAddr, MQTT_HOST_URL);

    Cloud_Init(&tCloudConnInfo);
#endif
}

void APP_UserAtInit(void)
{
    // add at cmd and enable CR/LF
    AT_CmdListAdd(1);  // #define CRLF_ENABLE (1)
}

//////////////////////////////////// 
//// APP task group
//////////////////////////////////// 
/*************************************************************************
* FUNCTION:
*   APP_SendMessage
*
* DESCRIPTION:
*   Send message queue to APP task
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
T_OplErr APP_SendMessage(uint32_t u32EventId, uint8_t *pu8Data, uint32_t u32DataLen)
{
    if(NULL == g_tAppQueueId)
    {
        OPL_LOG_WARN(APP, "Queue not init");

        return OPL_ERR_RTOS_QMSG_NOT_INIT;
    }

    T_AppMsgStruct *ptMsg = (T_AppMsgStruct *)malloc(sizeof(T_AppMsgStruct) + u32DataLen);

    if(NULL == ptMsg)
    {
        OPL_LOG_ERRO(APP, "Alloc WM message fail");
        return OPL_ERR_ALLOC_MEMORY_FAIL;
    }

    // prepare the data
    ptMsg->u32EventId = u32EventId;
    ptMsg->u32DataLen = u32DataLen;

    if(0 != ptMsg->u32DataLen)
    {
        memcpy(ptMsg->pau8Data, pu8Data, u32DataLen);
    }

    // send message
    if(osOK != osMessagePut(g_tAppQueueId, (uint32_t)ptMsg, 0))
    {
        OPL_LOG_ERRO(APP, "Send message fail");
        free(ptMsg);

        return OPL_ERR_RTOS_SEND_QMSG_FAIL;
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION:
*   APP_EventProcess
*
* DESCRIPTION:
*   Message processor
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
T_OplErr APP_EventProcess(T_AppMsgStruct *ptMsg)
{
    // event in user app define list
    if((APP_EVT_BEGIN <= ptMsg->u32EventId) && (ptMsg->u32EventId <= APP_EVT_TOTAL))
    {   
        uint32_t i = 0;
        while(g_tAppEvtHandlerTbl[i].u32EventId != 0xFFFFFFFF)
        {
            // matched
            if(g_tAppEvtHandlerTbl[i].u32EventId == ptMsg->u32EventId)
            {
                g_tAppEvtHandlerTbl[i].fpFunc(ptMsg->u32EventId, ptMsg->pau8Data, ptMsg->u32DataLen);
                break;
            }

            i ++;
        }

        // not match
        if(g_tAppEvtHandlerTbl[i].u32EventId == 0xFFFFFFFF)
        {
            OPL_LOG_WARN(APP, "can't find event in event table");

            return OPL_ERR_RTOS_EVT_NOT_FOUND;
        }

        return OPL_OK;
    }
    else
    {
        return OPL_ERR_RTOS_EVT_NOT_FOUND;
    }
}

/*************************************************************************
* FUNCTION:
*   AppInit
*
* DESCRIPTION:
*   the initial of application
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void APP_Main(void *args)
{
    osEvent tEvent;
    T_AppMsgStruct *ptMsg;

    for(;;)
    {
        // wait event
        tEvent = osMessageGet(g_tAppQueueId, osWaitForever);

        if (tEvent.status == osEventMessage)
        {
            ptMsg = (T_AppMsgStruct *)tEvent.value.p;

            if(OPL_ERR_RTOS_EVT_NOT_FOUND == APP_EventProcess(ptMsg))
            {
#if (NM_ENABLED == 1)
                // while event not found in user define, try network manager
                APP_NmEventProc(ptMsg->u32EventId, ptMsg->pau8Data, ptMsg->u32DataLen);
#endif
            }

            if(ptMsg != NULL)
            {
                free(ptMsg);
            }
        }
    }
}

/*************************************************************************
* FUNCTION:
*   AppInit
*
* DESCRIPTION:
*   the initial of application
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void APP_MainInit(void)
{
    // create main task
    OPL_LOG_INFO(APP, "App Main Init");

    APP_SysInit();

    APP_DataInit();

    APP_TaskInit();

    APP_BleInit();

    APP_NetInit();

    APP_CldInit();

    APP_UserAtInit();
    
    // enter smart sleep after 5s
#if (PS_ENABLED == 1)
    PS_EnterSmartSleep(5000);
#endif
    
    osTimerStart(g_tSysTimer, 5000);

    APP_HostModeResponseAck(AT_CMD_ACK_BOOT_UP, NULL, 0);
}
