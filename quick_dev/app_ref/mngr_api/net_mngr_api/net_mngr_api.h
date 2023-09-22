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
*  net_mngr_api.h
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

/******************************************************************************
//! \section net_mngr Introduction
//!
//! Network manager handling the WI-FI control and auto-connect scenario accommodation,
//! it provide the easy interface for user just focus on wifi scaning and connection,
//! and will reply the result of WI-FI status to application (option) side to let user
//! can handover the WI-FI status and do next activity.
//!
//! Network manager usage:
//!     1. to using the network manager, the NM_ENABLED must be enabled before using,
//!        the definition "NM_ENABLED" contents in qd_module.h in sys_config folder
//!        in project.
//!
//!     2. calling APP_NmInit() while application initiating to init the network
//!        manager progresser.
//!
//!
//!
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

#ifndef __NET_MNGR_API_H__
#define __NET_MNGR_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (NM_ENABLED == 1)

// Network manager event offset, should be defined in APP?
//#define EVT_OFFSET_NM           (0x0100)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

// WI-FI connect configuration
typedef struct S_NmWifiCnctConfig
{
    uint8_t u8aBssid[WIFI_MAC_ADDRESS_LENGTH];  // The MAC address of the target AP.
    uint8_t u8aSsid[WIFI_MAX_LENGTH_OF_SSID];   // The SSID of the target AP.
    uint8_t u8SsidLen;             
    uint8_t u8aPwd[WIFI_LENGTH_PASSPHRASE];     // The password of the target AP.
    uint8_t u8PwdLen;                           // The length of the password. If the length is 64, the password is regarded as PMK.
    uint8_t u8Timeout;                          // Connect timeout
} T_NmWifiCnctConfig;

// WI-FI Quick connect set configuration
typedef struct S_NmWifiQCnctSet
{
    uint8_t u8aSsid[WIFI_MAX_LENGTH_OF_SSID];   // The SSID of the target AP.
    uint8_t u8SsidLen;             
    uint8_t u8aPwd[WIFI_LENGTH_PASSPHRASE];     // The password of the target AP.
    uint8_t u8PwdLen;                           // The length of the password. If the length is 64, the password is regarded as PMK.
} T_NmWifiQCnctSet;

// Netowrk manager unsolicited event type
typedef enum E_NmUslctdEvtType
{
    NM_USLCTD_EVT_NETWORK_INIT = 0,
    NM_USLCTD_EVT_NETWORK_UP,
    NM_USLCTD_EVT_NETWORK_DOWN,
    NM_USLCTD_EVT_NETWORK_RESET,
} T_NmUslctdEvtType;

// Netowrk manager indicate callback typedef
typedef void (* T_NmScanDoneIndCbFp)(T_OplErr tEvtRst);
typedef void (* T_NmCnctIndCbFp)(T_OplErr tEvtRst);
typedef void (* T_NmStopIndCbFp)(T_OplErr tEvtRst);
typedef void (* T_NmResumeIndCbFp)(T_OplErr tEvtRst);
typedef void (* T_NmQCnctSetIndCbFp)(T_OplErr tEvtRst);

// Network manager unsolicited callback typedef
typedef void (* T_NmUslctdCbFp)(T_NmUslctdEvtType tEvtType, uint8_t *pu8Data, uint32_t u32DataLen);

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/*************************************************************************
* FUNCTION:
*   APP_NmEventProc
*
* DESCRIPTION:
*   network manager event process
*   (context in app_main, should runs in app_main task)
*
* PARAMETERS
*   u32EventId :    [IN] network manager event id (see in net_mngr.h)
*   u8Data :        [IN] data send to network manager
*   u32DataLen :    [IN] data lens send to network manager
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr APP_NmEventProc(uint32_t u32EventId, uint8_t *u8Data, uint32_t u32DataLen);

/*************************************************************************
* FUNCTION:
*   APP_NmInit
*
* DESCRIPTION:
*   network manager initiate function (optional to assign unsolicited callback)
*
* PARAMETERS
*   u8AcEnable :    [IN] enable auto-connect
*   fpUslctdCb :    [IN] unsolicited callback function pointer
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr APP_NmInit(uint8_t u8AcEnable, T_NmUslctdCbFp fpUslctdCb);

/*************************************************************************
* FUNCTION:
*   APP_NmInitAndCnct
*
* DESCRIPTION:
*   network manager initiate function and trigger WI-FI connect direclty (optional to assign unsolicited callback)
*   (SSID & PWD connection only, and will clear all profile record only store the assigned one)
*
* PARAMETERS
*   fpUslctdCb :    [IN] unsolicited callback function pointer
*   ptNmWifiCnctConfig :
*                   [IN] WI-FI connect config
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr APP_NmInitAndCnct(T_NmUslctdCbFp fpUslctdCb, T_NmWifiCnctConfig *ptNmWifiCnctConfig);

/*************************************************************************
* FUNCTION:
*   APP_NmWifiCnctReq
*
* DESCRIPTION:
*   trigger WI-FI quick connect set request, thaat is insert AP profile and enable auto connect
*
* PARAMETERS
*   ptNmWifiQCnctSet :
*                   [IN] WI-FI quick connect set configuration
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr APP_NmQuickCnctSetReq(T_NmWifiQCnctSet *ptNmWifiQCnctSet, T_NmQCnctSetIndCbFp fpIndCb);

/*************************************************************************
* FUNCTION:
*   APP_NmWifiScanReq
*
* DESCRIPTION:
*   trigger WI-FI scan request, and carried the scan result in indicate callback
*
* PARAMETERS
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr APP_NmWifiScanReq(T_NmScanDoneIndCbFp fpIndCb);

/*************************************************************************
* FUNCTION:
*   APP_NmWifiCnctReq
*
* DESCRIPTION:
*   trigger WI-FI connect request, and carried the connect result in indicate callback
*
* PARAMETERS
*   ptWmWifiCnctConfig :
*                   [IN] WI-FI connect configuration
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr APP_NmWifiCnctReq(T_NmWifiCnctConfig *ptNmWifiCnctConfig, T_NmCnctIndCbFp fpIndCb);

/*************************************************************************
* FUNCTION:
*   APP_NmWifiStopReq
*
* DESCRIPTION:
*   Stop all WiFi activities (disconnect WiFi and disable Auto-connect)
*
* PARAMETERS
*   ptWmWifiCnctConfig :
*                   [IN] WI-FI connect configuration
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr APP_NmWifiStopReq(T_NmStopIndCbFp fpIndCb);


/*************************************************************************
* FUNCTION:
*   APP_NmWifiResumeReq
*
* DESCRIPTION:
*   Resume WiFi (Enable auto connect), should only be called after WiFi stop
*
* PARAMETERS
*   ptWmWifiCnctConfig :
*                   [IN] WI-FI connect configuration
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr APP_NmWifiResumeReq(T_NmResumeIndCbFp fpIndCb);

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

#endif /* NM_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* __NET_MNGR_API_H__ */
