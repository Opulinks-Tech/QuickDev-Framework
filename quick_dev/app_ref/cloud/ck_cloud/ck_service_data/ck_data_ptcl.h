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
*  ck_data_ptcl.h
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

#include "opl_err.h"
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __CK_DATA_PTCL_H__
#define __CK_DATA_PTCL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_AUTH_DATA_SIZE              (64)          // CBC(UUID(36)) = 48 , BASE64(CBC(UUID(36))) = 64
#define MAX_AUTH_TOKEN_DATA_SIZE        (108)         // CBC(UUID(36) + "_"(1) + UUID(36)) = 80 , BASE(80) = 108
#define UUID_SIZE                       (36)          // 8-4-4-4-12
#define UUID_ENC_SIZE                   (48)          // CBC(UUID(36)) = 48
#define SECRETKEY_LEN                   (16)
#define IV_SIZE                         (16)
#define ENC_UUID_TO_BASE64_SIZE         (64)          // base64 max size ((4 * n / 3) + 3) & ~3
#define MAX_RSP_BASE64_API_KEY_LEN      (108)         // BASE(80) = 108
#define MAX_RSP_BASE64_CHIP_ID_LEN      (44)          // BASE(32) = 44

#define CK_DATA_MAX_REC_PASSWORD_SIZE   (108)         // WIFI_LENGTH_PASSPHRASE = 64  CBC(64)= 80  BASE64(80) = 108

#define CK_DATA_WIFI_CONNECTED_DONE     0
#define CK_DATA_WIFI_CONNECTED_FAIL     1
#define CK_DATA_WIFI_PASSWORD_FAIL      2
#define CK_DATA_WIFI_AP_NOT_FOUND       3
#define CK_DATA_WIFI_CONNECT_TIMEOUT    4

#define NETWORK_STOP_DELAY              (5000)    // ms

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef enum E_CkDataEvent
{
    CK_DATA_REQ_AUTH                            = 0x0,          // auth
    CK_DATA_RSP_AUTH                            = 0x1,
    CK_DATA_REQ_AUTH_TOKEN                      = 0x2,          // Wifi scan
    CK_DATA_RSP_AUTH_TOKEN                      = 0x3,          // Wifi scan
    CK_DATA_REQ_SCAN                            = 0x4,          // Wifi scan
    CK_DATA_RSP_SCAN_REPORT                     = 0x5,
    CK_DATA_RSP_SCAN_END                        = 0x6,
    CK_DATA_REQ_CONNECT                         = 0x7,          // Wifi connect
    CK_DATA_RSP_CONNECT                         = 0x8,
    CK_DATA_REQ_APP_DEVICE_INFO                 = 0x9,          // for CKS
    CK_DATA_RSP_APP_DEVICE_INFO                 = 0xA,          // for CKS
    CK_DATA_REQ_APP_HOST_INFO                   = 0xB,          // for CKS
    CK_DATA_RSP_APP_HOST_INFO                   = 0xC,          // for CKS
    CK_DATA_REQ_MANUAL_CONNECT_AP               = 0xD,          // Wifi connect AP by manual
    CK_DATA_IND_IP_STATUS_NOTIFY                = 0xE,          // Wifi notify AP status
    CK_DATA_REQ_PROTOCOL_CMD                    = 0xF,          // Coolkit protocol request
    CK_DATA_RES_PROTOCOL_CMD                    = 0x10,         // Coolkit protocol response

    // CK_DATA_REQ_SCAN                            = 0x3000,       // Wifi scan
    // CK_DATA_REQ_CONNECT                         = 0x3001,       // Wifi connect
    CK_DATA_REQ_DISCONNECT                      = 0x3002,       // Wifi disconnect
    CK_DATA_REQ_RECONNECT                       = 0x3003,       // Wifi reconnect
    CK_DATA_REQ_READ_DEVICE_INFO                = 0x3004,       // Wifi read device information
    CK_DATA_REQ_WRITE_DEVICE_INFO               = 0x3005,       // Wifi write device information
    CK_DATA_REQ_WIFI_STATUS                     = 0x3006,       // Wifi read AP status
    CK_DATA_REQ_RESET                           = 0x3007,       // Wifi reset AP
	// CK_DATA_REQ_MANUAL_CONNECT_AP               = 0x8,          // Wifi connect AP by manual

    CK_DATA_REQ_OTA_VERSION                     = 0x100,        // Ble OTA
    CK_DATA_REQ_OTA_UPGRADE                     = 0x101,        // Ble OTA
    CK_DATA_REQ_OTA_RAW                         = 0x102,        // Ble OTA
    CK_DATA_REQ_OTA_END                         = 0x103,        // Ble OTA

    CK_DATA_REQ_MP_START                        = 0x400,
    CK_DATA_REQ_MP_CAL_VBAT                     = 0x401,
    CK_DATA_REQ_MP_CAL_IO_VOLTAGE               = 0x402,
    CK_DATA_REQ_MP_CAL_TMPR                     = 0x403,
    CK_DATA_REQ_MP_SYS_MODE_WRITE               = 0x404,
    CK_DATA_REQ_MP_SYS_MODE_READ                = 0x405,

    CK_DATA_REQ_ENG_START                       = 0x600,
    CK_DATA_REQ_ENG_SYS_RESET                   = 0x601,
    CK_DATA_REQ_ENG_WIFI_MAC_WRITE              = 0x602,
    CK_DATA_REQ_ENG_WIFI_MAC_READ               = 0x603,
    CK_DATA_REQ_ENG_BLE_MAC_WRITE               = 0x604,
    CK_DATA_REQ_ENG_BLE_MAC_READ                = 0x605,
    CK_DATA_REQ_ENG_BLE_CMD                     = 0x606,
    CK_DATA_REQ_ENG_MAC_SRC_WRITE               = 0x607,
    CK_DATA_REQ_ENG_MAC_SRC_READ                = 0x608,
    CK_DATA_REQ_ENG_TMPR_CAL_DATA_WRITE         = 0x609,
    CK_DATA_REQ_ENG_TMPR_CAL_DATA_READ          = 0x60A,
    CK_DATA_REQ_ENG_VDD_VOUT_VOLTAGE_READ       = 0x60B,

    //4 cmd ID unused
    CK_DATA_REQ_ENG_BLE_CLOUD_INFO_WRITE        = 0x610,
    CK_DATA_REQ_ENG_BLE_CLOUD_INFO_READ         = 0x611,

    CK_DATA_REQ_APP_START                       = 0x800,
    CK_DATA_REQ_APP_USER_DEF_TMPR_OFFSET_WRITE  = 0x801,
    CK_DATA_REQ_APP_USER_DEF_TMPR_OFFSET_READ   = 0x802,

    // CK_DATA_RSP_SCAN_REPORT                     = 0x1000,
    // CK_DATA_RSP_SCAN_END                        = 0x1001,
    // CK_DATA_RSP_CONNECT                         = 0x1002,
    CK_DATA_RSP_DISCONNECT                      = 0x1003,
    CK_DATA_RSP_RECONNECT                       = 0x1004,
    CK_DATA_RSP_READ_DEVICE_INFO                = 0x1005,
    CK_DATA_RSP_WRITE_DEVICE_INFO               = 0x1006,
    CK_DATA_RSP_WIFI_STATUS                     = 0x1007,
    CK_DATA_RSP_RESET                           = 0x1008,

    CK_DATA_RSP_OTA_VERSION                     = 0x1100,
    CK_DATA_RSP_OTA_UPGRADE                     = 0x1101,
    CK_DATA_RSP_OTA_RAW                         = 0x1102,
    CK_DATA_RSP_OTA_END                         = 0x1103,

    CK_DATA_RSP_MP_START                        = 0x1400,
    CK_DATA_RSP_MP_CAL_VBAT                     = 0x1401,
    CK_DATA_RSP_MP_CAL_IO_VOLTAGE               = 0x1402,
    CK_DATA_RSP_MP_CAL_TMPR                     = 0x1403,
    CK_DATA_RSP_MP_SYS_MODE_WRITE               = 0x1404,
    CK_DATA_RSP_MP_SYS_MODE_READ                = 0x1405,

    CK_DATA_RSP_ENG_START                       = 0x1600,
    CK_DATA_RSP_ENG_SYS_RESET                   = 0x1601,
    CK_DATA_RSP_ENG_WIFI_MAC_WRITE              = 0x1602,
    CK_DATA_RSP_ENG_WIFI_MAC_READ               = 0x1603,
    CK_DATA_RSP_ENG_BLE_MAC_WRITE               = 0x1604,
    CK_DATA_RSP_ENG_BLE_MAC_READ                = 0x1605,
    CK_DATA_RSP_ENG_BLE_CMD                     = 0x1606,
    CK_DATA_RSP_ENG_MAC_SRC_WRITE               = 0x1607,
    CK_DATA_RSP_ENG_MAC_SRC_READ                = 0x1608,
    CK_DATA_RSP_ENG_TMPR_CAL_DATA_WRITE         = 0x1609,
    CK_DATA_RSP_ENG_TMPR_CAL_DATA_READ          = 0x160A,
    CK_DATA_RSP_ENG_VDD_VOUT_VOLTAGE_READ       = 0x160B,
    
    //4 cmd ID unused
    CK_DATA_RSP_ENG_BLE_CLOUD_INFO_WRITE        = 0x1610,
    CK_DATA_RSP_ENG_BLE_CLOUD_INFO_READ         = 0x1611,

    CK_DATA_RSP_APP_START                       = 0x1800,
    CK_DATA_RSP_APP_USER_DEF_TMPR_OFFSET_WRITE  = 0x1801,
    CK_DATA_RSP_APP_USER_DEF_TMPR_OFFSET_READ   = 0x1802,

    // CK_DATA_IND_IP_STATUS_NOTIFY                = 0x2000,     // Wifi notify AP status

    CK_DATA_TYPE_END                            = 0xFFFF

} T_CkDataEvent;

typedef enum E_CkOtaRspStatus
{
	CK_DATA_OTA_SUCCESS,
	CK_DATA_OTA_ERR_NOT_ACTIVE,
	CK_DATA_OTA_ERR_HW_FAILURE,
	CK_DATA_OTA_ERR_IN_PROGRESS,
	CK_DATA_OTA_ERR_INVALID_LEN,
	CK_DATA_OTA_ERR_CHECKSUM,
	CK_DATA_OTA_ERR_MEM_CAPACITY_EXCEED,
} T_CkOtaRspStatus;

typedef void (* CK_DataProtocolFp)(uint16_t type, uint8_t *data, int len);

typedef struct S_CkDataEventTable
{
    uint32_t             u32EventId;
    CK_DataProtocolFp    fpFunc;
} T_CkDataEventTable;

typedef struct S_CkDataHdrTag
{
    uint16_t u16EventId;
    uint16_t u16DataLen;
    uint8_t  au8Data[];
} T_CkDataHdrTag;

typedef struct S_CkOtaData
{
	uint16_t proj_id;
	uint16_t chip_id;
	uint16_t fw_id;
	uint32_t chksum;
	uint32_t curr_chksum;

	uint32_t total;
	uint32_t curr;
	uint16_t pkt_idx;
	uint16_t rx_pkt;
	uint16_t flag;

	uint8_t  buf[300];
	uint16_t idx;
} T_CkOtaData;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

#if defined(MAGIC_LED)
extern unsigned char g_ucSecretKey[SECRETKEY_LEN + 1];
#endif

// Sec 5: declaration of global function prototype

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
void CK_DataRecvHandler(uint8_t *pu8Data, uint16_t u16DataLen);

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
void CK_DataSendEncap(uint16_t u16Type, uint8_t *pu8Data, uint32_t u32TotalDataLen);

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
void CK_DataSendResponse(uint16_t type_id, uint8_t status);

#if defined(MAGIC_LED)
void CK_DataSendProtocolCmd(uint8_t *pu8Data, uint32_t u32DataLen);
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
void CK_DataPairingModeSet(bool blInPair);

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
bool CK_DataPairingModeGet(void);

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
void CK_DataDelayToStopBleAdvTimerStart(uint32_t u32MilliSec);

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
void CK_DataDelayToStopBleAdvTimerStop(void);

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
void CK_DataDelayToStopBleAdvTimerInit(void);

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

static void CK_DataProtocol_Auth(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_AuthToken(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_Scan(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_Connect(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_Manually_Connect_AP(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_Disconnect(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_Reconnect(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_AppDeviceInfo(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_AppHostInfo(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_ReadDeviceInfo(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_WriteDeviceInfo(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_WifiStatus(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_Reset(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_ProtocolCmd(uint16_t type, uint8_t *data, int len);

#if (OTA_ENABLED == 1)
static void CK_DataProtocol_OtaVersion(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_OtaUpgrade(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_OtaRaw(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_OtaEnd(uint16_t type, uint8_t *data, int len);
#endif

#if 0
static void CK_DataProtocol_MpCalVbat(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_MpCalIoVoltage(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_MpCalTmpr(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_MpSysModeWrite(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_MpSysModeRead(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_EngSysReset(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_EngWifiMacWrite(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_EngWifiMacRead(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_EngBleMacWrite(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_EngBleMacRead(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_EngBleCmd(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_EngMacSrcWrite(uint16_t type, uint8_t *data, int len);
static void CK_DataProtocol_EngMacSrcRead(uint16_t type, uint8_t *data, int len);
#endif

/***********
C Functions
***********/
// Sec 8: C Functions

#ifdef __cplusplus
}
#endif

#endif /* __CK_DATA_PTCL_H__ */
