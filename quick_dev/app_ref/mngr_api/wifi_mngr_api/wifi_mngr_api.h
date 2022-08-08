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
*  wifi_mngr_api.h
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

#include "sys_common_types.h"
#include "opl_err.h"
#include "qd_config.h"
#include "qd_module.h"
#include "wifi_api.h"
#include "wifi_pro_rec.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __WIFI_MNGR_API_H__
#define __WIFI_MNGR_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (WM_ENABLED == 1)

// unsolicited callback maximum register number
#ifndef WM_USLCTED_CB_REG_NUM
#define WM_USLCTED_CB_REG_NUM               (5)
#endif

// manufacturer name
#define WM_MANUFACTURER_NAME_LEN            32

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

// This structure defines the connect configuration
typedef struct S_WmConnConfig
{
    uint8_t	ssid[WIFI_MAX_LENGTH_OF_SSID];	                    // The SSID of the target AP.
    uint8_t	ssid_length;	                                    // The length of the SSID.
    uint8_t	bssid[WIFI_MAC_ADDRESS_LENGTH];	                    // The MAC address of the target AP.
    uint8_t	password[WIFI_LENGTH_PASSPHRASE];	                // The password of the target AP.
    uint8_t	password_length;	                                // The length of the password. If the length is 64, the password is regarded as PMK.
} __attribute__((packed)) T_WmConnConfig;

// This structure defines the information of scanned APs
typedef struct S_WmScanInfo
{
    uint8_t  ssid_length;                                       // Length of the SSID
    uint8_t  ssid[WIFI_MAX_LENGTH_OF_SSID];                     // Stores the predefined SSID
    uint8_t  bssid[WIFI_MAC_ADDRESS_LENGTH];                    // AP's MAC address
    uint8_t  auth_mode;                                         // Please refer to the definition of #wifi_auth_mode_t
    int8_t   rssi;                                              // Records the RSSI value when probe response is received
    uint8_t  connected;                                         // AP was connected before
#if (1 == FLITER_STRONG_AP_EN)
    uint8_t  u8IgnoreReport;                                    // if scan list has the same SSID , the weak RSSI AP don't report
#endif
} __attribute__((packed)) T_WmScanInfo;

// This structure defines the information of device
typedef struct S_WmDeviceInfo
{
	uint8_t  device_id[WIFI_MAC_ADDRESS_LENGTH];                // Stores the predefined device ID (MAC)
    uint8_t  name_len;                                          // Length of manufacturer
    uint8_t  manufacturer_name[WM_MANUFACTURER_NAME_LEN];       // Manufacturer name
} __attribute__((packed)) T_WmDeviceInfo;

// This structure defines the information of Wi-Fi status
typedef struct S_WmWifiStatusInfo
{
    uint8_t  status;                                            // Length of the SSID
    uint8_t  ssid_length;                                       // Length of the SSID
    uint8_t  ssid[WIFI_MAX_LENGTH_OF_SSID];                     // Stores the predefined SSID
    uint8_t  bssid[WIFI_MAC_ADDRESS_LENGTH];                    // AP's MAC address
    unsigned long   IP;                                         // Device's IP
    unsigned long   mask;                                       // Device's Mask
    unsigned long   Gateway;                                    // Device's Gateway
    unsigned long   DNS;                                        // Device's DNS
} __attribute__((packed)) T_WmWifiStatusInfo;

// This structure use for mac_addr_get_config_source
typedef struct S_WmWifiGetConfigSource
{
    mac_iface_t iface;
    mac_source_type_t *type;
} __attribute__((packed)) T_WmWifiGetConfigSource;

// This structure use for mac_addr_set_config_source
typedef struct S_WmWifiSetConfigSource
{
    mac_iface_t iface;
    mac_source_type_t type;
}__attribute__((packed)) T_WmWifiSetConfigSource;

// unsolicited event type
typedef enum E_WmUslctedEvtType
{
    USLCTED_CB_EVT_WIFI_UP      = 0,
    USLCTED_CB_EVT_WIFI_DOWN,
    USLCTED_CB_EVT_WIFI_RESET,
    USLCTED_CB_EVT_GOT_IP,
} T_WmUslctedEvtType;

typedef enum E_WmWifiConnectionState
{
    WM_WIFI_ST_CONNECTED        = 0,
    WM_WIFI_ST_NOT_CONNECT,
} T_WmWifiConnectionState;

// WI-FI autoconnect indicate callback typedef
typedef void (* T_AcEnableIndCbFp)(T_OplErr tEvtRst, uint8_t *pu8Data, uint32_t u32DataLen);
typedef void (* T_AcDisableIndCbFp)(T_OplErr tEvtRst);

// WI-FI agent indicate callback typedef
typedef void (* T_WaInitDoneIndCbFp)(T_OplErr tEvtRst);
typedef void (* T_WaScanDoneIndCbFp)(T_OplErr tEvtRst);
typedef void (* T_WaConnectIndCbFp)(T_OplErr tEvtRst);
typedef void (* T_WaDisconnectIndCbFp)(T_OplErr tEvtRst);

// WI-FI agent unsolicited callback typedef
typedef void (* T_WmUslctedCbFp)(T_WmUslctedEvtType tEvtType, T_OplErr tEvtRst, uint8_t *pu8Data, uint32_t u32DataLen);

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Init_Req
*
* DESCRIPTION:
*   WI-FI manager initiate function, and carried the init result in indicate callback
*
* PARAMETERS
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_Init_Req(T_WaInitDoneIndCbFp fpIndCb);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Scan_Req
*
* DESCRIPTION:
*   trigger WI-FI scan request, and carried the scan result in indicate callback
*
* PARAMETERS
*   ptWifiScanConfig :
*                   [IN] scan config structure
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_Scan_Req(wifi_scan_config_t *ptWifiScanConfig, T_WaScanDoneIndCbFp fpIndCb);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Conn_Req
*
* DESCRIPTION:
*   trigger WI-FI connect request, and carried the connect result in indicate callback
*
* PARAMETERS
*   ptWifiConnConfig :
*                   [IN] connect config structure
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_Conn_Req(T_WmConnConfig *ptWifiConnConfig, T_WaConnectIndCbFp fpIndCb);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Disc_Req
*
* DESCRIPTION:
*   trigger WI-FI disconnect request, and carried the disconnect result in indicate callback
*
* PARAMETERS
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_Disc_Req(T_WaDisconnectIndCbFp fpIndCb);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Uslctd_CB_Reg
*
* DESCRIPTION:
*   register unsolicited callback for WI-FI manager
*
* PARAMETERS
*   fpUslctdCb :    [IN] unsolicited callback function pointer
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_Uslctd_CB_Reg(T_WmUslctedCbFp fpUslctdCb);

#if (WM_AC_ENABLED == 1)
/*************************************************************************
* FUNCTION:
*   Opl_Wifi_AC_Enable_Req
*
* DESCRIPTION:
*   enable auto-connect request, and carried the enable result in indicate callback
*
* PARAMETERS
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_AC_Enable_Req(T_AcEnableIndCbFp fpIndCb);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_AC_Disable_Req
*
* DESCRIPTION:
*   disable auto-connect request, and carried the disable result in indicate callback
*
* PARAMETERS
*   blActDisconnect :
*                   [IN] true -> will do WI-FI disconnect while disable auto-connect
*                        false -> won't do WI-FI disconnect while disable auto-connect
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_AC_Disable_Req(bool blActDisconnect, T_AcDisableIndCbFp fpIndCb);

#endif /* WM_AC_ENABLED */

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Network_Status_Get
*
* DESCRIPTION:
*   get WI-FI manager status
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Opl_Wifi_Network_Status_Get(void);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Profile_Ins
*
* DESCRIPTION:
*   insert WI-FI AP information into AP profile list
*
* PARAMETERS
*   tNewProfile :   [IN] AP information structure
*
* RETURNS
*   none
*
*************************************************************************/
void Opl_Wifi_Profile_Ins(T_PrApProfile tNewProfile);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Profile_Get
*
* DESCRIPTION:
*   get WI-FI AP profile list
*
* PARAMETERS
*   none
*
* RETURNS
*   T_PrApProfilePtr :
*                   [OUT] AP profile list
*
*************************************************************************/
T_PrApProfilePtr Opl_Wifi_Profile_Get(void);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Profile_Clear
*
* DESCRIPTION:
*   clear WI-FI AP profile list
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Opl_Wifi_Profile_Clear(void);

//// Get Type WI-FI Apis
/*************************************************************************
* FUNCTION:
*   Opl_Wifi_ApInfo_Get
*
* DESCRIPTION:
*   get AP information
*
* PARAMETERS
*   ptApRecord :    [OUT] AP information
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_ApInfo_Get(wifi_ap_record_t *ptApRecord);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_MacAddr_Get
*
* DESCRIPTION:
*   get WI-FI mac address
*
* PARAMETERS
*   pu8Address :    [OUT] WI-FI mac address
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_MacAddr_Get(uint8_t *pu8Address);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_ManufName_Get
*
* DESCRIPTION:
*   get manufacturer name
*
* PARAMETERS
*   pu8Name :       [OUT] manufacturer name
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_ManufName_Get(uint8_t *pu8Name);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_ConfigSource_Get
*
* DESCRIPTION:
*   get configure source
*
* PARAMETERS
*   tWmWifiGetConfigSource :
                    [OUT] configure source structure
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_ConfigSource_Get(T_WmWifiGetConfigSource *tWmWifiGetConfigSource);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Rssi_Get
*
* DESCRIPTION:
*   get current rssi value
*
* PARAMETERS
*   pi8Rssi :       [OUT] rssi value
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_Rssi_Get(int8_t *pi8Rssi);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_ApNum_Get
*
* DESCRIPTION:
*   get AP number
*
* PARAMETERS
*   pu16Num :       [OUT] AP number
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_ApNum_Get(uint16_t *pu16Num);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_ApRecord_Get
*
* DESCRIPTION:
*   get AP record
*
* PARAMETERS
*   pu16ApCount :   [OUT] count number of AP
*   ptScanInfo :    [OUT] scan information sturcture
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_ApRecord_Get(uint16_t *pu16ApCount, wifi_scan_info_t *ptScanInfo);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_AutoConnectApNum_Get
*
* DESCRIPTION:
*   get auto-connect AP number
*
* PARAMETERS
*   pu8Num :        [OUT] auto-connect AP number
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_AutoConnectApNum_Get(uint8_t *pu8Num);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_AutoConnectApInfo_Get
*
* DESCRIPTION:
*   get specific auto-connect AP information
*
* PARAMETERS
*   u8Index :       [IN] index of auto-connect list
*   ptAutoConnInfo :
                    [OUT] auto-connect AP information structure
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_AutoConnectApInfo_Get(uint8_t u8Index, wifi_auto_connect_info_t *ptAutoConnInfo);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_AutoConnectListNum_Get
*
* DESCRIPTION:
*   get auto-connect list number
*
* PARAMETERS
*   pu8Num :        [OUT] auto-connect list number
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_AutoConnectListNum_Get(uint8_t *pu8Num);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_ScanList_Get
*
* DESCRIPTION:
*   get scan list result
*
* PARAMETERS
*   ptScanList :    [OUT] scan list structure
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_ScanList_Get(wifi_scan_list_t *ptScanList);

//// Set Type WI-FI Apis
/*************************************************************************
* FUNCTION:
*   Opl_Wifi_MacAddr_Set
*
* DESCRIPTION:
*   set WI-FI mac address
*
* PARAMETERS
*   pu8Address :    [IN] WI-FI mac address
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_MacAddr_Set(uint8_t *pu8Address);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_ManufName_Set
*
* DESCRIPTION:
*   set manufacturer name
*
* PARAMETERS
*   pu8Name :       [IN] manufacturer name
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_ManufName_Set(uint8_t *pu8Name);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_ConfigSource_Set
*
* DESCRIPTION:
*   set configure source
*
* PARAMETERS
*   tWmWifiSetConfigSource :
                    [IN] configure source structure
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_ConfigSource_Set(T_WmWifiSetConfigSource *tWmWifiSetConfigSource);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Skip_Dtim_Module_Reg
*
* DESCRIPTION:
*   register skip DTIM module
*
* PARAMETERS
*   u16ModuleId :   [OUT] skip DTIM module ID
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_Skip_Dtim_Module_Reg(uint16_t *u16ModuleId);

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Skip_Dtim_Set
*
* DESCRIPTION:
*   set skip DTIM 
*
* PARAMETERS
*   u16ModuleId :   [IN] registed skip DTIM module ID
*                        (must reigster by calling Opl_Wifi_Skip_Dtim_Module_Reg first)
*   u8Enable :      [IN] true -> skip DTIM
*                        false -> no skip DTIM
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_Skip_Dtim_Set(uint16_t u16ModuleId, uint8_t u8Enable);

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

#endif /* WM_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* __WIFI_MNGR_API_H__ */
