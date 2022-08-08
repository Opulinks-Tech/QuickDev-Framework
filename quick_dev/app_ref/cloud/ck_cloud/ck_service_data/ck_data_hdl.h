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
*  ck_data_hdl.h
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
#include "wifi_api.h"
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __CK_DATA_HDL_H__
#define __CK_DATA_HDL_H__

#ifdef __cplusplus
extern "C" {
#endif

// #define CK_DATA_USE_CONNECTED

#define CK_DATA_CONN_TYPE_BSSID     0
#define CK_DATA_CONN_TYPE_SSID      1

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef struct S_CkDataConnCfg
{
    uint8_t u8ConnectType;                                      // CK_DATA_CONN_TYPE_BSSID or CK_DATA_CONN_TYPE_SSID.
    uint8_t	u8aSsid[WIFI_MAX_LENGTH_OF_SSID];	                // The SSID of the target AP.
    uint8_t	u8SsidLen;	                                        // The length of the SSID.
    uint8_t u8aBssid[WIFI_MAC_ADDRESS_LENGTH];                  // The MAC address of the target AP.
    uint8_t u8aPwd[WIFI_LENGTH_PASSPHRASE];                     // The password of the target AP.
    uint8_t u8PwdLen;                                           // The length of the password. If the length is 64, the password is regarded as PMK.
#if CK_DATA_USE_CONNECTED
    uint8_t u8Connected;
#endif
    uint8_t u8Timeout;                                          // Connect timeout (second).
} T_CkDataConnCfg;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

extern T_CkDataConnCfg g_tCkDataConnCfg;

extern uint8_t g_u8IsManuallyConnectScanRetry;

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
void CK_DataHandler_WifiScanDoneIndCb(T_OplErr tEvtRst);

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
void CK_DataHandler_WifiConnectionIndCb(T_OplErr tEvtRst);

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
void CK_DataHandler_WifiDisconnectionIndCb(T_OplErr tEvtRst);

// __weak void Ble_DataConstruct(uint8_t *pInData, uint32_t u32InDataLen, uint8_t *pOutData, uint32_t *u32OutDataLen);
// __weak int8_t Ble_DataParser(uint8_t *pInData, uint16_t u32InDataLen);

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

#ifdef __cplusplus
}
#endif

#endif /* __CK_DATA_HDL_H__ */
