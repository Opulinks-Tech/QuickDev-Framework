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
*  opl_data_prot.h
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

#ifndef __CK_DATA_PROT_H__
#define __CK_DATA_PROT_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (OPL_DATA_ENABLED == 1)

#define OPL_DATA_MAX_REC_PASSWORD_SIZE   (108)         // WIFI_LENGTH_PASSPHRASE = 64  CBC(64)= 80  BASE64(80) = 108

#define OPL_DATA_WIFI_CONNECTED_DONE     0
#define OPL_DATA_WIFI_CONNECTED_FAIL     1
#define OPL_DATA_WIFI_PASSWORD_FAIL      2
#define OPL_DATA_WIFI_AP_NOT_FOUND       3
#define OPL_DATA_WIFI_CONNECT_TIMEOUT    4

// #define NETWORK_STOP_DELAY              (5000)    // ms

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef enum E_OplDataEvent
{
    OPL_DATA_REQ_SCAN                            = 0x0,          // Wifi scan
    OPL_DATA_REQ_CONNECT                         = 0x1,          // Wifi connect
    OPL_DATA_REQ_DISCONNECT                      = 0x2,          // Wifi disconnect
    OPL_DATA_REQ_RECONNECT                       = 0x3,          // Wifi reconnect
    OPL_DATA_REQ_READ_DEVICE_INFO                = 0x4,          // Wifi read device information
    OPL_DATA_REQ_WRITE_DEVICE_INFO               = 0x5,          // Wifi write device information
    OPL_DATA_REQ_WIFI_STATUS                     = 0x6,          // Wifi read AP status
    OPL_DATA_REQ_RESET                           = 0x7,          // Wifi reset AP
	OPL_DATA_REQ_MANUAL_CONNECT_AP               = 0x8,          // Wifi connect AP by manual

    OPL_DATA_REQ_OTA_VERSION                     = 0x100,        // Ble OTA
    OPL_DATA_REQ_OTA_UPGRADE                     = 0x101,        // Ble OTA
    OPL_DATA_REQ_OTA_RAW                         = 0x102,        // Ble OTA
    OPL_DATA_REQ_OTA_END                         = 0x103,        // Ble OTA

    OPL_DATA_REQ_MP_START                        = 0x400,
    OPL_DATA_REQ_MP_CAL_VBAT                     = 0x401,
    OPL_DATA_REQ_MP_CAL_IO_VOLTAGE               = 0x402,
    OPL_DATA_REQ_MP_CAL_TMPR                     = 0x403,
    OPL_DATA_REQ_MP_SYS_MODE_WRITE               = 0x404,
    OPL_DATA_REQ_MP_SYS_MODE_READ                = 0x405,

    OPL_DATA_REQ_ENG_START                       = 0x600,
    OPL_DATA_REQ_ENG_SYS_RESET                   = 0x601,
    OPL_DATA_REQ_ENG_WIFI_MAC_WRITE              = 0x602,
    OPL_DATA_REQ_ENG_WIFI_MAC_READ               = 0x603,
    OPL_DATA_REQ_ENG_BLE_MAC_WRITE               = 0x604,
    OPL_DATA_REQ_ENG_BLE_MAC_READ                = 0x605,
    OPL_DATA_REQ_ENG_BLE_CMD                     = 0x606,
    OPL_DATA_REQ_ENG_MAC_SRC_WRITE               = 0x607,
    OPL_DATA_REQ_ENG_MAC_SRC_READ                = 0x608,
    OPL_DATA_REQ_ENG_TMPR_CAL_DATA_WRITE         = 0x609,
    OPL_DATA_REQ_ENG_TMPR_CAL_DATA_READ          = 0x60A,
    OPL_DATA_REQ_ENG_VDD_VOUT_VOLTAGE_READ       = 0x60B,
    //4 cmd ID unused
    OPL_DATA_REQ_ENG_BLE_CLOUD_INFO_WRITE        = 0x610,
    OPL_DATA_REQ_ENG_BLE_CLOUD_INFO_READ         = 0x611,

    OPL_DATA_REQ_APP_START                       = 0x800,
    OPL_DATA_REQ_APP_USER_DEF_TMPR_OFFSET_WRITE  = 0x801,
    OPL_DATA_REQ_APP_USER_DEF_TMPR_OFFSET_READ   = 0x802,

    OPL_DATA_RSP_SCAN_REPORT                     = 0x1000,
    OPL_DATA_RSP_SCAN_END                        = 0x1001,
    OPL_DATA_RSP_CONNECT                         = 0x1002,
    OPL_DATA_RSP_DISCONNECT                      = 0x1003,
    OPL_DATA_RSP_RECONNECT                       = 0x1004,
    OPL_DATA_RSP_READ_DEVICE_INFO                = 0x1005,
    OPL_DATA_RSP_WRITE_DEVICE_INFO               = 0x1006,
    OPL_DATA_RSP_WIFI_STATUS                     = 0x1007,
    OPL_DATA_RSP_RESET                           = 0x1008,

    OPL_DATA_RSP_OTA_VERSION                     = 0x1100,
    OPL_DATA_RSP_OTA_UPGRADE                     = 0x1101,
    OPL_DATA_RSP_OTA_RAW                         = 0x1102,
    OPL_DATA_RSP_OTA_END                         = 0x1103,

    OPL_DATA_RSP_MP_START                        = 0x1400,
    OPL_DATA_RSP_MP_CAL_VBAT                     = 0x1401,
    OPL_DATA_RSP_MP_CAL_IO_VOLTAGE               = 0x1402,
    OPL_DATA_RSP_MP_CAL_TMPR                     = 0x1403,
    OPL_DATA_RSP_MP_SYS_MODE_WRITE               = 0x1404,
    OPL_DATA_RSP_MP_SYS_MODE_READ                = 0x1405,

    OPL_DATA_RSP_ENG_START                       = 0x1600,
    OPL_DATA_RSP_ENG_SYS_RESET                   = 0x1601,
    OPL_DATA_RSP_ENG_WIFI_MAC_WRITE              = 0x1602,
    OPL_DATA_RSP_ENG_WIFI_MAC_READ               = 0x1603,
    OPL_DATA_RSP_ENG_BLE_MAC_WRITE               = 0x1604,
    OPL_DATA_RSP_ENG_BLE_MAC_READ                = 0x1605,
    OPL_DATA_RSP_ENG_BLE_CMD                     = 0x1606,
    OPL_DATA_RSP_ENG_MAC_SRC_WRITE               = 0x1607,
    OPL_DATA_RSP_ENG_MAC_SRC_READ                = 0x1608,
    OPL_DATA_RSP_ENG_TMPR_CAL_DATA_WRITE         = 0x1609,
    OPL_DATA_RSP_ENG_TMPR_CAL_DATA_READ          = 0x160A,
    OPL_DATA_RSP_ENG_VDD_VOUT_VOLTAGE_READ       = 0x160B,
    //4 cmd ID unused
    OPL_DATA_RSP_ENG_BLE_CLOUD_INFO_WRITE        = 0x1610,
    OPL_DATA_RSP_ENG_BLE_CLOUD_INFO_READ         = 0x1611,

    OPL_DATA_RSP_APP_START                       = 0x1800,
    OPL_DATA_RSP_APP_USER_DEF_TMPR_OFFSET_WRITE  = 0x1801,
    OPL_DATA_RSP_APP_USER_DEF_TMPR_OFFSET_READ   = 0x1802,

    OPL_DATA_IND_IP_STATUS_NOTIFY                = 0x2000,       // Wifi notify AP status

    OPL_DATA_TYPE_END                            = 0xFFFF

} T_OplDataEvent;

typedef enum E_OplOtaRspStatus
{
	OPL_DATA_OTA_SUCCESS,
	OPL_DATA_OTA_ERR_NOT_ACTIVE,
	OPL_DATA_OTA_ERR_HW_FAILURE,
	OPL_DATA_OTA_ERR_IN_PROGRESS,
	OPL_DATA_OTA_ERR_INVALID_LEN,
	OPL_DATA_OTA_ERR_CHECKSUM,
	OPL_DATA_OTA_ERR_MEM_CAPACITY_EXCEED,
} T_OplOtaRspStatus;

typedef void (* Opl_DataProtocolFp)(uint16_t type, uint8_t *data, int len);

typedef struct S_OplDataEventTable
{
    uint32_t                    u32EventId;
    Opl_DataProtocolFp          fpFunc;
} T_OplDataEventTable;

typedef struct S_OplDataHdrTag
{
    uint16_t u16EventId;
    uint16_t u16DataLen;
    uint8_t  au8Data[];
} T_OplDataHdrTag;

typedef struct S_OplOtaData
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
} T_OplOtaData;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

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
void OPL_DataRecvHandler(uint8_t *pu8Data, uint16_t u16DataLen);

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
void OPL_DataSendEncap(uint16_t u16Type, uint8_t *pu8Data, uint32_t u32TotalDataLen);

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
void OPL_DataSendResponse(uint16_t type_id, uint8_t status);

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

static void OPL_DataProtocol_Scan(uint16_t type, uint8_t *data, int len);
static void OPL_DataProtocol_Connect(uint16_t type, uint8_t *data, int len);
static void OPL_DataProtocol_Disconnect(uint16_t type, uint8_t *data, int len);
static void OPL_DataProtocol_Reconnect(uint16_t type, uint8_t *data, int len);
static void OPL_DataProtocol_ReadDeviceInfo(uint16_t type, uint8_t *data, int len);
static void OPL_DataProtocol_WriteDeviceInfo(uint16_t type, uint8_t *data, int len);
static void OPL_DataProtocol_WifiStatus(uint16_t type, uint8_t *data, int len);
static void OPL_DataProtocol_Reset(uint16_t type, uint8_t *data, int len);

#if (OTA_ENABLED == 1)
static void OPL_DataProtocol_OtaVersion(uint16_t type, uint8_t *data, int len);
static void OPL_DataProtocol_OtaUpgrade(uint16_t type, uint8_t *data, int len);
static void OPL_DataProtocol_OtaRaw(uint16_t type, uint8_t *data, int len);
static void OPL_DataProtocol_OtaEnd(uint16_t type, uint8_t *data, int len);
#endif

// static void OPL_DataProtocol_MpCalVbat(uint16_t type, uint8_t *data, int len);
// static void OPL_DataProtocol_MpCalIoVoltage(uint16_t type, uint8_t *data, int len);
// static void OPL_DataProtocol_MpCalTmpr(uint16_t type, uint8_t *data, int len);
static void OPL_DataProtocol_MpSysModeWrite(uint16_t type, uint8_t *data, int len);
static void OPL_DataProtocol_MpSysModeRead(uint16_t type, uint8_t *data, int len);
static void OPL_DataProtocol_EngSysReset(uint16_t type, uint8_t *data, int len);
static void OPL_DataProtocol_EngWifiMacWrite(uint16_t type, uint8_t *data, int len);
static void OPL_DataProtocol_EngWifiMacRead(uint16_t type, uint8_t *data, int len);
static void OPL_DataProtocol_EngBleMacWrite(uint16_t type, uint8_t *data, int len);
static void OPL_DataProtocol_EngBleMacRead(uint16_t type, uint8_t *data, int len);
static void OPL_DataProtocol_EngBleCmd(uint16_t type, uint8_t *data, int len);
static void OPL_DataProtocol_EngMacSrcWrite(uint16_t type, uint8_t *data, int len);
static void OPL_DataProtocol_EngMacSrcRead(uint16_t type, uint8_t *data, int len);

/***********
C Functions
***********/
// Sec 8: C Functions

#endif /* OPL_DATA_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* __CK_DATA_PROT_H__ */
