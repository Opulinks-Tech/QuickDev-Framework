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
*  opl_data_prot.c
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
#include "at_cmd_common.h"
#include "ble_mngr_api.h"
#include "cmsis_os.h"
#include "hal_auxadc_patch.h"
#include "hal_system.h"
#include "lwip/netif.h"
#include "mw_fim_default_group03.h"
#if defined(OPL1000_A2) || defined(OPL1000_A3)
#include "mw_fim_default_group03_patch.h"
#endif
#include "net_mngr_api.h"
#include "opl_svc.h"
#include "opl_data_prot.h"
#include "opl_data_hdl.h"
#include "ota_mngr.h"
#include "qd_config.h"
#include "qd_module.h"
#include "wifi_mngr_api.h"

#if (OPL_DATA_CURRENT_MEASURE_ENABLED == 1)
#include "wifi_agent.h"
#endif 

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (OPL_DATA_ENABLED == 1)

#define HI_UINT16(a)    (((a) >> 8) & 0xFF)
#define LO_UINT16(a)    ((a) & 0xFF)

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

#if (OPL_DATA_CURRENT_MEASURE_ENABLED == 1)
#define DTIM_PERIOD_MIN           100     //minimum Wi-Fi skip dtim (ms)
#define DTIM_PERIOD_MAX           3000    //maximum Wi-Fi skip dtim (ms)
#define BLE_ADV_INTEVAL_MIN     20     //minimum BLE advertise interval (ms)
#define BLE_ADV_INTEVAL_MAX     1000    //maximum BLE advertise interval (ms)
#define BLE_SLEEP_TIMER_MIN     15      //minimum timer sleep sleep time (ms)
#define BLE_SLEEP_TIMER_MAX     150000  //maximum timer sleep sleep time (ms)
#define BLE_WAKEUP_TIMER_MIN    0      //minimum timer sleep wakeup time (ms)
#define BLE_WAKEUP_TIMER_MAX    1000  //maximum timer sleep wakeup time (ms)
#endif

// #define AES_BLOCK_SIZE  (16)                 //for CBC

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef struct S_OplDataPacket{
    uint16_t total_len;
    uint16_t remain;
    uint16_t offset;
    uint8_t *aggr_buf;
} T_OplDataPacket;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

static T_OplDataEventTable g_tOplDataEventHandlerTbl[] =
{
    {OPL_DATA_REQ_SCAN,                      OPL_DataProtocol_Scan},
    {OPL_DATA_REQ_CONNECT,                   OPL_DataProtocol_Connect},
    {OPL_DATA_REQ_DISCONNECT,                OPL_DataProtocol_Disconnect},
    {OPL_DATA_REQ_RECONNECT,                 OPL_DataProtocol_Reconnect},
    {OPL_DATA_REQ_READ_DEVICE_INFO,          OPL_DataProtocol_ReadDeviceInfo},
    {OPL_DATA_REQ_WRITE_DEVICE_INFO,         OPL_DataProtocol_WriteDeviceInfo},
    {OPL_DATA_REQ_WIFI_STATUS,               OPL_DataProtocol_WifiStatus},
    {OPL_DATA_REQ_RESET,                     OPL_DataProtocol_Reset},

#if (OTA_ENABLED == 1)
    {OPL_DATA_REQ_OTA_VERSION,               OPL_DataProtocol_OtaVersion},
    {OPL_DATA_REQ_OTA_UPGRADE,               OPL_DataProtocol_OtaUpgrade},
    {OPL_DATA_REQ_OTA_RAW,                   OPL_DataProtocol_OtaRaw},
    {OPL_DATA_REQ_OTA_END,                   OPL_DataProtocol_OtaEnd},
#endif

    // {OPL_DATA_REQ_MP_CAL_VBAT,               OPL_DataProtocol_MpCalVbat},
    // {OPL_DATA_REQ_MP_CAL_IO_VOLTAGE,         OPL_DataProtocol_MpCalIoVoltage},
    // {OPL_DATA_REQ_MP_CAL_TMPR,               OPL_DataProtocol_MpCalTmpr},
    {OPL_DATA_REQ_MP_SYS_MODE_WRITE,         OPL_DataProtocol_MpSysModeWrite},
    {OPL_DATA_REQ_MP_SYS_MODE_READ,          OPL_DataProtocol_MpSysModeRead},

    {OPL_DATA_REQ_ENG_SYS_RESET,             OPL_DataProtocol_EngSysReset},
    {OPL_DATA_REQ_ENG_WIFI_MAC_WRITE,        OPL_DataProtocol_EngWifiMacWrite},
    {OPL_DATA_REQ_ENG_WIFI_MAC_READ,         OPL_DataProtocol_EngWifiMacRead},
    {OPL_DATA_REQ_ENG_BLE_MAC_WRITE,         OPL_DataProtocol_EngBleMacWrite},
    {OPL_DATA_REQ_ENG_BLE_MAC_READ,          OPL_DataProtocol_EngBleMacRead},
    {OPL_DATA_REQ_ENG_BLE_CMD,               OPL_DataProtocol_EngBleCmd},
    {OPL_DATA_REQ_ENG_MAC_SRC_WRITE,         OPL_DataProtocol_EngMacSrcWrite},
    {OPL_DATA_REQ_ENG_MAC_SRC_READ,          OPL_DataProtocol_EngMacSrcRead},

    {OPL_DATA_REQ_ENG_POWER_CONSUMPTION,     OPL_DataProtocol_EngPowerConsumtion}, 
    {0xFFFFFFFF,                            NULL},
};

T_OplDataPacket g_OplDataPacket = {0};

#if (OTA_ENABLED == 1)
T_OplOtaData *gTheOta = 0;
static uint16_t g_u16OtaSeqId = 0;
#endif

// Sec 7: declaration of static function prototype

static void _OPL_DataWifiSendDeviceInfo(T_WmDeviceInfo *dev_info);
static void _OPL_DataWifiSendStatusInfo(uint16_t u16Type);
static void OPL_DataProtocolProc(uint16_t type, uint8_t *data, int len);

/***********
C Functions
***********/
// Sec 8: C Functions

#if (OPL_DATA_CURRENT_MEASURE_ENABLED == 1)
static T_OplErr _OPL_QueryDeviceInfo(void) 
{
    T_OplErr tRet = OPL_ERR;
    uint8_t u8aIp[4];
    wifi_scan_info_t stInfo; 
    struct netif *iface = netif_find("st1");
    const uint8_t u8aNoIp[4] = {0};
    const uint8_t u8aNoBssid[6] = {0}; 

    u8aIp[0] = (iface->ip_addr.u_addr.ip4.addr >> 0) & 0xFF;
    u8aIp[1] = (iface->ip_addr.u_addr.ip4.addr >> 8) & 0xFF;
    u8aIp[2] = (iface->ip_addr.u_addr.ip4.addr >> 16) & 0xFF;
    u8aIp[3] = (iface->ip_addr.u_addr.ip4.addr >> 24) & 0xFF;

    // get ap info
    Opl_Wifi_ApInfo_Get((wifi_ap_record_t *)&stInfo);

    //Wi-Fi connected & Got Ip 
    if(memcmp(stInfo.bssid, u8aNoBssid, 6) != 0 && memcmp(u8aIp, u8aNoIp, 4) != 0)
    {
        tRet = OPL_OK;
    }
    else
    {
        tRet = OPL_ERR;
    }
    return tRet;
}
#endif /*OPL_DATA_CURRENT_MEASURE_ENABLED*/

static void _OPL_DataWifiSendDeviceInfo(T_WmDeviceInfo *dev_info)
{
    uint8_t *pu8Data;
    int sDataLen;
    uint8_t *pu8Pos;

    pu8Pos = pu8Data = malloc(sizeof(T_WmScanInfo));
    if (pu8Data == NULL) 
    {
        OPL_LOG_ERRO(OPL, "malloc fail");
        return;
    }

    memcpy(pu8Data, dev_info->device_id, WIFI_MAC_ADDRESS_LENGTH);
    pu8Pos += 6;

    if (dev_info->name_len > WM_MANUFACTURER_NAME_LEN)
    {
        dev_info->name_len = WM_MANUFACTURER_NAME_LEN;
    }

    *pu8Pos++ = dev_info->name_len;
    memcpy(pu8Pos, dev_info->manufacturer_name, dev_info->name_len);
    pu8Pos += dev_info->name_len;
    sDataLen = (pu8Pos - pu8Data);

    // BLEWIFI_DUMP("device info data", pu8Data, sDataLen);

    /* create device info data packet */
    OPL_DataSendEncap(OPL_DATA_RSP_READ_DEVICE_INFO, pu8Data, sDataLen);

    free(pu8Data);
}

static void _OPL_DataWifiSendStatusInfo(uint16_t u16Type)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_WIFI_STATUS");

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

    // get ap info
    Opl_Wifi_ApInfo_Get((wifi_ap_record_t *)&stInfo);

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
    //OPL_DataSendEncap(BLEWIFI_RSP_WIFI_STATUS, pu8Data, u16DataLen);

release:
    free(pu8Data);
}

static void OPL_DataProtocol_Scan(uint16_t type, uint8_t *data, int len)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_SCAN");

    // reset connection config table
    memset(&g_tOplDataConnCfg, 0, sizeof(T_OplDataConnCfg));

    g_tOplDataConnCfg.u8ConnectType = OPL_DATA_CONN_TYPE_BSSID;

    // trigger scan request
    APP_NmWifiScanReq(OPL_DataHandler_WifiScanDoneIndCb);
}

static void OPL_DataProtocol_Connect(uint16_t type, uint8_t *data, int len)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_CONNECT");

    // reset connection config table
    memset(&g_tOplDataConnCfg, 0, sizeof(T_OplDataConnCfg));

    g_tOplDataConnCfg.u8ConnectType = OPL_DATA_CONN_TYPE_BSSID;

    // copy bssid
    memcpy(g_tOplDataConnCfg.u8aBssid, &data[0], WIFI_MAC_ADDRESS_LENGTH);

    // set connected and timeout variable
#if CK_DATA_USE_CONNECTED
    g_tOplDataConnCfg.u8Connected = 0; // ignore data[6]
#endif

    // check and copy password
    if(data[7] == 0) //password len = 0
    {
        OPL_LOG_INFO(OPL, "pwd lens = 0");
        g_tOplDataConnCfg.u8PwdLen = 0;
        memset((char *)g_tOplDataConnCfg.u8aPwd, 0, WIFI_LENGTH_PASSPHRASE);
    }
    else
    {
        if(data[7] > OPL_DATA_MAX_REC_PASSWORD_SIZE)
        {
            OPL_LOG_INFO(OPL, "Not do manually connect %d", __LINE__);
            OPL_DataSendResponse(OPL_DATA_RSP_CONNECT, OPL_DATA_WIFI_PASSWORD_FAIL);
            return;
        }

        g_tOplDataConnCfg.u8PwdLen = data[7];
        memcpy(g_tOplDataConnCfg.u8aPwd, &data[8], data[7]);

        //printf("password = %s\r\n" , wifi_config_req_connect.sta_config.password);
        //printf("password_length = %u\r\n" , wifi_config_req_connect.sta_config.password_length);
    }

    ////// for debug  ////////////////
    OPL_LOG_DEBG(OPL, "conn_config.password=%s", g_tOplDataConnCfg.u8aPwd);
#if CK_DATA_USE_CONNECTED
    OPL_LOG_DEBG(OPL, "conn_config.connected=%d", g_tOplDataConnCfg.u8Connected);
#endif
    //////////////////////////////////

    T_NmWifiCnctConfig stConnConfig = {0};

    memcpy(stConnConfig.u8aBssid, g_tOplDataConnCfg.u8aBssid, WIFI_MAC_ADDRESS_LENGTH);
    memcpy(stConnConfig.u8aPwd, g_tOplDataConnCfg.u8aPwd, WIFI_LENGTH_PASSPHRASE);
    stConnConfig.u8PwdLen = g_tOplDataConnCfg.u8PwdLen;
    stConnConfig.u8Timeout = g_tOplDataConnCfg.u8Timeout;

    // trigger wifi connect
    APP_NmWifiCnctReq(&stConnConfig, OPL_DataHandler_WifiConnectionIndCb);
}

static void OPL_DataProtocol_Disconnect(uint16_t type, uint8_t *data, int len)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_DISCONNECT");

    Opl_Wifi_Disc_Req(OPL_DataHandler_WifiDisconnectionIndCb);
}

static void OPL_DataProtocol_Reconnect(uint16_t type, uint8_t *data, int len)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_RECONNECT");
}

static void OPL_DataProtocol_ReadDeviceInfo(uint16_t type, uint8_t *data, int len)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_READ_DEVICE_INFO");

    T_WmDeviceInfo stDevInfo = {0};
    char u8aManufacturerName[33] = {0};

    // get mac address
    Opl_Wifi_MacAddr_Get((uint8_t *)&stDevInfo.device_id[0]);

    // get manufacturer name
    Opl_Wifi_ManufName_Get((uint8_t *)&u8aManufacturerName);

    stDevInfo.name_len = strlen(u8aManufacturerName);
    memcpy(stDevInfo.manufacturer_name, u8aManufacturerName, stDevInfo.name_len);
    _OPL_DataWifiSendDeviceInfo(&stDevInfo);
}

static void OPL_DataProtocol_WriteDeviceInfo(uint16_t type, uint8_t *data, int len)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_WRITE_DEVICE_INFO");

    T_WmDeviceInfo stDevInfo = {0};

    memset(&stDevInfo, 0, sizeof(T_WmDeviceInfo));
    memcpy(stDevInfo.device_id, &data[0], WIFI_MAC_ADDRESS_LENGTH);
    stDevInfo.name_len = data[6];
    memcpy(stDevInfo.manufacturer_name, &data[7], stDevInfo.name_len);

    // set wifi mac address
    Opl_Wifi_MacAddr_Set((uint8_t *)&stDevInfo.device_id[0]);

    // set wifi manufacturer name
    Opl_Wifi_ManufName_Set((uint8_t *)&stDevInfo.manufacturer_name[0]);

    OPL_DataSendResponse(OPL_DATA_RSP_WRITE_DEVICE_INFO, 0);

    OPL_LOG_INFO(OPL, "Device ID: \""MACSTR"\"", MAC2STR(stDevInfo.device_id));
    OPL_LOG_INFO(OPL, "Device Manufacturer: %s", stDevInfo.manufacturer_name);
}

static void OPL_DataProtocol_WifiStatus(uint16_t type, uint8_t *data, int len)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_WIFI_STATUS");

    _OPL_DataWifiSendStatusInfo(OPL_DATA_RSP_WIFI_STATUS);
}

static void OPL_DataProtocol_Reset(uint16_t type, uint8_t *data, int len)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_RESET");

    // TODO: call clear profile
    Opl_Wifi_Profile_Clear();

    // TODO: call disconnect wifi
    Opl_Wifi_Disc_Req(NULL);

    OPL_DataSendResponse(OPL_DATA_RSP_RESET, 0);
}

#if (OTA_ENABLED == 1)
static void OPL_OtaSendVersionRsp(uint8_t status, uint16_t pid, uint16_t cid, uint16_t fid)
{
	uint8_t data[7];
	uint8_t *p = (uint8_t *)data;

	*p++ = status;
	*p++ = LO_UINT16(pid);
	*p++ = HI_UINT16(pid);
	*p++ = LO_UINT16(cid);
	*p++ = HI_UINT16(cid);
	*p++ = LO_UINT16(fid);
	*p++ = HI_UINT16(fid);

	OPL_DataSendEncap(OPL_DATA_RSP_OTA_VERSION, data, 7);
}

static void OPL_OtaSendUpgradeRsp(uint8_t status)
{
	OPL_DataSendEncap(OPL_DATA_RSP_OTA_UPGRADE, &status, 1);
}

static void OPL_SendEndRsp(uint8_t status, uint8_t stop)
{
	OPL_DataSendEncap(OPL_DATA_RSP_OTA_END, &status, 1);

    if (stop)
    {
        if (gTheOta)
        {
            if (status != OPL_DATA_OTA_SUCCESS)
            {
                OTA_UpgradeGiveUp(g_u16OtaSeqId);
            }

            free(gTheOta);
            gTheOta = NULL;

            if (status != OPL_DATA_OTA_SUCCESS)
            {
                // App_Ctrl_MsgSend(APP_CTRL_MSG_OTHER_OTA_OFF_FAIL, NULL, 0);
            }
            else
            {
                OTA_TriggerReboot(3000);
            }
        }
    }
}

static void OPL_OtaTimeoutIndCb(void)
{
    OPL_LOG_DEBG(OPL, "OTA timeout");

    OPL_OtaSendUpgradeRsp(OPL_DATA_OTA_ERR_NOT_ACTIVE);

    // OTA_TriggerReboot(3000);
}

static void OPL_DataProtocol_OtaVersion(uint16_t type, uint8_t *data, int len)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_OTA_VERSION");

	uint16_t pid;
	uint16_t cid;
	uint16_t fid;

    T_OplErr tEvtRst = OTA_CurrentVersionGet(&pid, &cid, &fid);

	if (OPL_OK != tEvtRst)
		OPL_OtaSendVersionRsp(OPL_DATA_OTA_ERR_HW_FAILURE, 0, 0, 0);
	else
		OPL_OtaSendVersionRsp(OPL_DATA_OTA_SUCCESS, pid, cid, fid);
}

static void OPL_DataProtocol_OtaUpgrade(uint16_t type, uint8_t *data, int len)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_OTA_UPGRADE");

    T_OplOtaData *ota = gTheOta;

    T_OplErr tEvtRst = OPL_OK;

	if (len != 26)
	{
		OPL_OtaSendUpgradeRsp(OPL_DATA_OTA_ERR_INVALID_LEN);
		return;
	}

	if (ota)
	{
		OPL_OtaSendUpgradeRsp(OPL_DATA_OTA_ERR_IN_PROGRESS);
		return;
	}

	ota = malloc(sizeof(T_OplOtaData));

	if (ota)
	{
		T_MwOtaFlashHeader *ota_hdr= (T_MwOtaFlashHeader*) &data[2];

		ota->pkt_idx = 0;
		ota->idx     = 0;
        ota->rx_pkt  = *(uint16_t *)&data[0];
        ota->proj_id = ota_hdr->uwProjectId;
        ota->chip_id = ota_hdr->uwChipId;
        ota->fw_id   = ota_hdr->uwFirmwareId;
        ota->total   = ota_hdr->ulImageSize;
        ota->chksum  = ota_hdr->ulImageSum;
		ota->curr 	 = 0;

        tEvtRst = OTA_UpgradeBegin(&g_u16OtaSeqId, ota_hdr, OPL_OtaTimeoutIndCb);

        if(OPL_OK == tEvtRst)
        {
            OPL_OtaSendUpgradeRsp(OPL_DATA_OTA_SUCCESS);
            gTheOta = ota;    
        }
        else
        {
            OPL_SendEndRsp(OPL_DATA_OTA_ERR_HW_FAILURE, TRUE);
        }
    }
    else
    {
        OPL_OtaSendUpgradeRsp(OPL_DATA_OTA_ERR_MEM_CAPACITY_EXCEED);
    }
}

static uint32_t OPL_OtaAdd(uint8_t *data, int len)
{
	uint16_t i;
	uint32_t sum = 0;

	for (i = 0; i < len; i++)
    {
		sum += data[i];
    }

    return sum;
}

static void OPL_DataProtocol_OtaRaw(uint16_t type, uint8_t *data, int len)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_OTA_RAW");

    T_OplOtaData *ota = gTheOta;

    T_OplErr tEvtRst = OPL_OK;

	if (!ota)
	{
		OPL_SendEndRsp(OPL_DATA_OTA_ERR_NOT_ACTIVE, FALSE);
        goto err;
	}

	if ((ota->curr + len) > ota->total)
	{
		OPL_SendEndRsp(OPL_DATA_OTA_ERR_INVALID_LEN, TRUE);
		goto err;
    }

	ota->pkt_idx++;
	ota->curr += len;
	ota->curr_chksum += OPL_OtaAdd(data, len);

	if ((ota->idx + len) >= 256)
	{
		UINT16 total = ota->idx + len;
		UINT8 *s = data;
		UINT8 *e = data + len;
		UINT16 cpyLen = 256 - ota->idx;

		if (ota->idx)
		{
			MemCopy(&ota->buf[ota->idx], s, cpyLen);
			s += cpyLen;
			total -= 256;
			ota->idx = 0;

            tEvtRst = OTA_WriteData(g_u16OtaSeqId, ota->buf, 256);
		}

        if(OPL_OK == tEvtRst)
        {
            while (total >= 256)
            {
                tEvtRst = OTA_WriteData(g_u16OtaSeqId, s, 256);
                s += 256;
                total -= 256;

                if(OPL_OK != tEvtRst) break;
            }

            if(OPL_OK == tEvtRst)
            {
                MemCopy(ota->buf, s, e - s);
                ota->idx = e - s;

                if((ota->curr == ota->total) && ota->idx)
                {
                    tEvtRst = OTA_WriteData(g_u16OtaSeqId, ota->buf, ota->idx);
                }
            }
        }
	}
	else
	{
		MemCopy(&ota->buf[ota->idx], data, len);
		ota->idx += len;

		if ((ota->curr == ota->total) && ota->idx)
		{
            tEvtRst = OTA_WriteData(g_u16OtaSeqId, ota->buf, ota->idx);
		}
	}

    if (OPL_OK == tEvtRst)
    {
        if (ota->rx_pkt && (ota->pkt_idx >= ota->rx_pkt))
        {
            OPL_DataSendEncap(OPL_DATA_RSP_OTA_RAW, 0, 0);
            ota->pkt_idx = 0;
        }
    }
    else
        OPL_SendEndRsp(OPL_DATA_OTA_ERR_HW_FAILURE, TRUE);

err:
	return;
}

static void OPL_DataProtocol_OtaEnd(uint16_t type, uint8_t *data, int len)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_OTA_END");
    
    T_OplOtaData *ota = gTheOta;

    uint8_t status = data[0];

    if (!ota)
    {
        OPL_SendEndRsp(OPL_DATA_OTA_ERR_NOT_ACTIVE, FALSE);
        goto err;
    }

    if (status == OPL_DATA_OTA_SUCCESS)
    {
        if (ota->curr == ota->total)
        {
            if(OPL_OK == OTA_UpgradeFinish(g_u16OtaSeqId))
            {
                OPL_SendEndRsp(OPL_DATA_OTA_SUCCESS, TRUE);
            }
            else
            {
                OPL_SendEndRsp(OPL_DATA_OTA_ERR_CHECKSUM, TRUE);
            }
        }
        else
        {
            OPL_SendEndRsp(OPL_DATA_OTA_ERR_INVALID_LEN, TRUE);
        }
    }
    else
    {
        if (ota) OTA_UpgradeGiveUp(g_u16OtaSeqId);

        // APP stop OTA
        OPL_SendEndRsp(OPL_DATA_OTA_SUCCESS, TRUE);
    }

    err:
    return;
}
#endif

// static void OPL_DataProtocol_MpCalVbat(uint16_t type, uint8_t *data, int len)
// {
//     OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_MP_CAL_VBAT");

//     float fTargetVbat;

//     memcpy(&fTargetVbat, &data[0], 4);
//     Hal_Aux_VbatCalibration(fTargetVbat);
//     OPL_DataSendResponse(OPL_DATA_RSP_MP_CAL_VBAT, 0);
// }

// static void OPL_DataProtocol_MpCalIoVoltage(uint16_t type, uint8_t *data, int len)
// {
//     OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_MP_CAL_IO_VOLTAGE");

//     float fTargetIoVoltage;
//     uint8_t ubGpioIdx;

//     memcpy(&ubGpioIdx, &data[0], 1);
//     memcpy(&fTargetIoVoltage, &data[1], 4);
//     Hal_Aux_IoVoltageCalibration(ubGpioIdx, fTargetIoVoltage);
//     OPL_DataSendResponse(OPL_DATA_RSP_MP_CAL_IO_VOLTAGE, 0);
// }

// static void OPL_DataProtocol_MpCalTmpr(uint16_t type, uint8_t *data, int len)
// {
//     OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_MP_CAL_TMPR");

//     OPL_DataSendResponse(OPL_DATA_RSP_MP_CAL_TMPR, 0);
// }

static void OPL_DataProtocol_MpSysModeWrite(uint16_t type, uint8_t *data, int len)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_MP_SYS_MODE_WRITE");

#if defined(OPL1000_A2) || defined(OPL1000_A3)
    T_MwFim_SysMode tSysMode;

    // set the settings of system mode
    tSysMode.ubSysMode = data[0];
    if (tSysMode.ubSysMode < MW_FIM_SYS_MODE_MAX)
    {
        if (MW_FIM_OK == MwFim_FileWrite(MW_FIM_IDX_GP03_PATCH_SYS_MODE, 0, MW_FIM_SYS_MODE_SIZE, (uint8_t*)&tSysMode))
        {
            // TODO: set system mode
            // App_Ctrl_SysModeSet(tSysMode.ubSysMode);

            OPL_DataSendResponse(OPL_DATA_RSP_MP_SYS_MODE_WRITE, 0);
            return;
        }
    }
#endif

    OPL_DataSendResponse(OPL_DATA_RSP_MP_SYS_MODE_WRITE, 1);
}

static void OPL_DataProtocol_MpSysModeRead(uint16_t type, uint8_t *data, int len)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_MP_SYS_MODE_READ");

    uint8_t ubSysMode;

    // TODO: get system mode
    // ubSysMode = App_Ctrl_SysModeGet();

    OPL_DataSendResponse(OPL_DATA_RSP_MP_SYS_MODE_READ, ubSysMode);
}

static void OPL_DataProtocol_EngSysReset(uint16_t type, uint8_t *data, int len)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_ENG_SYS_RESET");

    OPL_DataSendResponse(OPL_DATA_RSP_ENG_SYS_RESET, 0);

    // wait the BLE response, then reset the system
    osDelay(3000);
    Hal_Sys_SwResetAll();
}

static void OPL_DataProtocol_EngWifiMacWrite(uint16_t type, uint8_t *data, int len)
{
    uint8_t u8aMacAddr[6];
    T_WmWifiSetConfigSource stSetConfigSource;

    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_ENG_WIFI_MAC_WRITE");

    // save the mac address into flash
    memcpy(u8aMacAddr, &data[0], 6);

    // set wifi mac address
    Opl_Wifi_MacAddr_Set((uint8_t *)&u8aMacAddr[0]);

    // set config source
    // apply the mac address from flash
    stSetConfigSource.iface = MAC_IFACE_WIFI_STA;
    stSetConfigSource.type = MAC_SOURCE_FROM_FLASH;
    Opl_Wifi_ConfigSource_Set(&stSetConfigSource);

    OPL_DataSendResponse(OPL_DATA_RSP_ENG_WIFI_MAC_WRITE, 0);
}

static void OPL_DataProtocol_EngWifiMacRead(uint16_t type, uint8_t *data, int len)
{
    uint8_t u8aMacAddr[6];

    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_ENG_WIFI_MAC_READ");

    // get wifi mac address
    // get the mac address from flash
    Opl_Wifi_MacAddr_Get((uint8_t *)&u8aMacAddr[0]);

    OPL_DataSendEncap(OPL_DATA_RSP_ENG_WIFI_MAC_READ, u8aMacAddr, sizeof(u8aMacAddr));
}

static void OPL_DataProtocol_EngBleMacWrite(uint16_t type, uint8_t *data, int len)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_ENG_BLE_MAC_WRITE");

    // set ble mac address
    Opl_Ble_MacAddr_Write(data);

    OPL_DataSendResponse(OPL_DATA_RSP_ENG_BLE_MAC_WRITE, 0);
}

static void OPL_DataProtocol_EngBleMacRead(uint16_t type, uint8_t *data, int len)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_ENG_BLE_MAC_READ");

    // get ble mac address
    Opl_Ble_MacAddr_Read(data);

    OPL_DataSendEncap(OPL_DATA_RSP_ENG_BLE_MAC_READ, data, 6);
}

static void OPL_DataProtocol_EngBleCmd(uint16_t type, uint8_t *data, int len)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_ENG_BLE_CMD");

    msg_print_uart1("+BLE:%s\r\n", data);

    OPL_DataSendResponse(OPL_DATA_RSP_ENG_BLE_CMD, 0);
}

static void OPL_DataProtocol_EngMacSrcWrite(uint16_t type, uint8_t *data, int len)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_ENG_MAC_SRC_WRITE");

    T_OplErr tRet = OPL_ERR;
    T_WmWifiSetConfigSource stSetConfigSource;

    uint8_t sta_type, ble_type;

    sta_type = data[0];
    ble_type = data[1];

    OPL_LOG_DEBG(OPL, "Enter Mac Src Write: WiFi MAC Src=%d, BLE MAC Src=%d", sta_type, ble_type);

    stSetConfigSource.iface = MAC_IFACE_WIFI_STA;
    stSetConfigSource.type = (mac_source_type_t)sta_type;

    // set config source
    tRet = Opl_Wifi_ConfigSource_Set(&stSetConfigSource);

    if(OPL_OK != tRet)
    {
        goto done;
    }

    stSetConfigSource.iface = MAC_IFACE_BLE;
    stSetConfigSource.type = (mac_source_type_t)ble_type;

    // set config source
    tRet = Opl_Wifi_ConfigSource_Set(&stSetConfigSource);

    if(OPL_OK != tRet)
    {
        goto done;
    }

done:
    if (OPL_OK == tRet)
    {
        OPL_DataSendResponse(OPL_DATA_RSP_ENG_MAC_SRC_WRITE, 0);
    }
    else
    {
        OPL_DataSendResponse(OPL_DATA_RSP_ENG_MAC_SRC_WRITE, 1);
    }
}

static void OPL_DataProtocol_EngMacSrcRead(uint16_t type, uint8_t *data, int len)
{
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_ENG_MAC_SRC_READ");

    T_OplErr tRet = OPL_ERR;
    T_WmWifiGetConfigSource stGetConfigSource;

    uint8_t sta_type, ble_type;
    uint8_t MacSrc[2]={0};

    stGetConfigSource.iface = MAC_IFACE_WIFI_STA;
    stGetConfigSource.type = (mac_source_type_t *)&sta_type;

    // get config source
    tRet = Opl_Wifi_ConfigSource_Get(&stGetConfigSource);

    if(OPL_OK != tRet)
    {
        goto done;
    }

    stGetConfigSource.iface = MAC_IFACE_BLE;
    stGetConfigSource.type = (mac_source_type_t *)&ble_type;

    // get config source
    tRet = Opl_Wifi_ConfigSource_Get(&stGetConfigSource);

    if(OPL_OK != tRet)
    {
        goto done;
    }

    MacSrc[0] = sta_type;
    MacSrc[1] = ble_type;

    OPL_LOG_DEBG(OPL, "WiFi MAC Src=%d, BLE MAC Src=%d", MacSrc[0], MacSrc[1]);

done:
    if (OPL_OK == tRet)
    {
        OPL_DataSendEncap(OPL_DATA_RSP_ENG_MAC_SRC_READ, MacSrc, sizeof(MacSrc));
    }
    else
    {
        OPL_DataSendResponse(OPL_DATA_RSP_ENG_MAC_SRC_READ, 1);
    }
}


static void OPL_DataProtocol_EngPowerConsumtion(uint16_t type, uint8_t *data, int len)
{
#if (OPL_DATA_CURRENT_MEASURE_ENABLED == 1)
    OPL_LOG_DEBG(OPL, "OPL_DATA_REQ_ENG_POWER_CONSUMPTION");
    T_OplErr tRet = OPL_ERR; //For YC Test 20221124
    T_OplErr tQuery = OPL_ERR;
    bool bQueryFlag = false;
    T_BlePCStruct tPcStruct = {0};
    

    memcpy(&tPcStruct, data, len);

    //For YC Test 20221124
    OPL_LOG_INFO(APP, "u8SleepMode %d",     tPcStruct.u8SleepMode);
    OPL_LOG_INFO(APP, "u8BleAdvOnOff %d",   tPcStruct.u8BleAdvOnOff);
    OPL_LOG_INFO(APP, "u16BleAdvIntv %d",   tPcStruct.u16BleAdvIntv);
    OPL_LOG_INFO(APP, "u32DtimPeriod %d",   tPcStruct.u32DtimPeriod);
    OPL_LOG_INFO(APP, "u32SleepTime %d",    tPcStruct.u32SleepTime);
    OPL_LOG_INFO(APP, "u32WakeupTime %d",   tPcStruct.u32WakeupTime);

    switch(tPcStruct.u8SleepMode)
    {
        case SMART_SLEEP:
        {
            if(tPcStruct.u32DtimPeriod < DTIM_PERIOD_MIN || tPcStruct.u32DtimPeriod > DTIM_PERIOD_MAX)
            {
                goto done;
            }

            if(tPcStruct.u8BleAdvOnOff)
            {
                if(tPcStruct.u16BleAdvIntv < BLE_ADV_INTEVAL_MIN || tPcStruct.u16BleAdvIntv > BLE_ADV_INTEVAL_MAX)
                {
                    goto done;
                }
            }

            APP_SendMessage(APP_EVT_BLE_POWER_CONSUMPTION,(void*) &tPcStruct, sizeof(tPcStruct));

            break;
        }

        case TIMER_SLEEP:
        {
            //Check Sleep time range
            if(tPcStruct.u32SleepTime < BLE_SLEEP_TIMER_MIN || tPcStruct.u32SleepTime > BLE_SLEEP_TIMER_MAX)
            {
                goto done;
            }

            //Check Wakeup time range
            if(tPcStruct.u32WakeupTime > BLE_WAKEUP_TIMER_MAX)
            {
                goto done;
            }

            APP_SendMessage(APP_EVT_BLE_POWER_CONSUMPTION,(void*) &tPcStruct, sizeof(tPcStruct));

            break;
        }

        case DEEP_SLEEP:
        {
            APP_SendMessage(APP_EVT_BLE_POWER_CONSUMPTION,(void*) &tPcStruct, sizeof(tPcStruct));

            break;
        }

        case QUERY_STATUS: // Query device status Whether to connect to AP before implement Sleep
        {
            tQuery = _OPL_QueryDeviceInfo();
            bQueryFlag = true;
            break;
        }
    }
    tRet = OPL_OK;

done:
    if(false == bQueryFlag) // sleep request
    {
        if (OPL_OK == tRet)
        {
            OPL_DataSendResponse(OPL_DATA_RSP_ENG_POWER_CONSUMPTION, 0);
        }
        else
        {
            OPL_DataSendResponse(OPL_DATA_RSP_ENG_POWER_CONSUMPTION, 1);
        }
    }
    else // query request
    {
        if(OPL_OK == tQuery)
        {
            OPL_DataSendResponse(OPL_DATA_RSP_ENG_POWER_CONSUMPTION, 3);
        }
        else
        {
            OPL_DataSendResponse(OPL_DATA_RSP_ENG_POWER_CONSUMPTION, 4);
        }
    }
#else
    OPL_DataSendResponse(OPL_DATA_RSP_ENG_POWER_CONSUMPTION, 0xFF); // OPL_DATA_CURRENT_MEASURE_ENABLED == 0
#endif /*OPL_DATA_CURRENT_MEASURE_ENABLED*/
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
static void OPL_DataProtocolProc(uint16_t type, uint8_t *data, int len)
{
    uint32_t i = 0;

    while (g_tOplDataEventHandlerTbl[i].u32EventId != 0xFFFFFFFF)
    {
        // match
        if (g_tOplDataEventHandlerTbl[i].u32EventId == type)
        {
            g_tOplDataEventHandlerTbl[i].fpFunc(type, data, len);
            break;
        }

        i++;
    }

    // TODO: not match
    if (g_tOplDataEventHandlerTbl[i].u32EventId == 0xFFFFFFFF)
    {
        // OPL_LOG_WARN(OPL, "Event %x not found", type);
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
void OPL_DataRecvHandler(uint8_t *pu8Data, uint16_t u16DataLen)
{
    T_OplDataHdrTag *tHdrTag = NULL;
    uint16_t u16HdrLen = sizeof(T_OplDataHdrTag);

    /* 1.aggregate fragment data packet, only first frag packet has header */
    /* 2.handle blewifi data packet, if data frame is aggregated completely */


    if (g_OplDataPacket.offset == 0)
    {
        tHdrTag = (T_OplDataHdrTag*)pu8Data;

        OPL_LOG_DEBG(OPL, "Event: %X; DataLen: %d", tHdrTag->u16EventId, tHdrTag->u16DataLen);

        g_OplDataPacket.total_len = tHdrTag->u16DataLen + u16HdrLen;
        g_OplDataPacket.remain = g_OplDataPacket.total_len;
        g_OplDataPacket.aggr_buf = malloc(g_OplDataPacket.total_len);

        if (g_OplDataPacket.aggr_buf == NULL)
        {
            OPL_LOG_ERRO(OPL, "%s no mem, len %d", __func__, g_OplDataPacket.total_len);
           return;
        }
    }

    // error handle
    // if the size is overflow, don't copy the whole data
    if (u16DataLen > g_OplDataPacket.remain)
    {
        u16DataLen = g_OplDataPacket.remain;
    }


    memcpy(g_OplDataPacket.aggr_buf + g_OplDataPacket.offset, pu8Data, u16DataLen);
    g_OplDataPacket.offset += u16DataLen;
    g_OplDataPacket.remain -= u16DataLen;

    /* no frag or last frag packet */
    if (g_OplDataPacket.remain == 0)
    {
        tHdrTag = (T_OplDataHdrTag*)g_OplDataPacket.aggr_buf;
        OPL_DataProtocolProc(tHdrTag->u16EventId, g_OplDataPacket.aggr_buf + u16HdrLen,  (g_OplDataPacket.total_len - u16HdrLen));
        g_OplDataPacket.offset = 0;
        g_OplDataPacket.remain = 0;
        free(g_OplDataPacket.aggr_buf);
        g_OplDataPacket.aggr_buf = NULL;
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
void OPL_DataSendEncap(uint16_t u16Type, uint8_t *pu8Data, uint32_t u32TotalDataLen)
{
    T_OplDataHdrTag *tHdrTag = NULL;
    int remain_len = u32TotalDataLen;

    /* 1.fragment data packet to fit MTU size */

    /* 2.Pack blewifi header */
    tHdrTag = malloc(sizeof(T_OplDataHdrTag) + remain_len);
    if (tHdrTag == NULL)
    {
        OPL_LOG_ERRO(OPL, "malloc fail");
        return;
    }

    tHdrTag->u16EventId = u16Type;
    tHdrTag->u16DataLen = remain_len;
    if (tHdrTag->u16DataLen)
        memcpy(tHdrTag->au8Data, pu8Data, tHdrTag->u16DataLen);

    // BLEWIFI_DUMP("[BLEWIFI]:out packet", (uint8_t*)tHdrTag, (tHdrTag->data_len + sizeof(T_OplPairDataHdrTag)));

    /* 3.send app data to BLE stack */
    // BleWifi_Ble_SendAppMsgToBle(BW_APP_MSG_SEND_DATA, (tHdrTag->data_len + sizeof(T_OplPairDataHdrTag)), (uint8_t *)tHdrTag);
    Opl_Ble_Send_Message(OPL_SVC_EVT_SEND_DATA, (uint8_t *)tHdrTag, (tHdrTag->u16DataLen + sizeof(T_OplDataHdrTag)), 0);

    free(tHdrTag);
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
void OPL_DataSendResponse(uint16_t type_id, uint8_t status)
{
    OPL_DataSendEncap(type_id, &status, 1);
}

#endif /* OPL_DATA_ENABLED */
