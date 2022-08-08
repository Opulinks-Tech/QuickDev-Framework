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
*  net_mngn.h
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
#include "fsm_kernel.h"
#include "net_mngr_api.h"
#include "log.h"
#include "opl_err.h"
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __NET_MNGR_H__
#define __NET_MNGR_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (NM_ENABLED == 1)

// module log
#if (NM_LOG_ENABLED == 1)
#define NM_LOG_DEBG(...)                                OPL_LOG_DEBG(NM, __VA_ARGS__)
#define NM_LOG_INFO(...)                                OPL_LOG_INFO(NM, __VA_ARGS__)
#define NM_LOG_WARN(...)                                OPL_LOG_WARN(NM, __VA_ARGS__)
#define NM_LOG_ERRO(...)                                OPL_LOG_ERRO(NM, __VA_ARGS__)
#else
#define NM_LOG_DEBG(...)
#define NM_LOG_INFO(...)
#define NM_LOG_WARN(...)
#define NM_LOG_ERRO(...)
#endif

/// Default definition setting
// WiFi connect timeout default value
#ifndef NM_WIFI_CNCT_DEF_TIMEOUT
#define NM_WIFI_CNCT_DEF_TIMEOUT                        (60000)
#endif

// WiFi connect max retry times
#ifndef NM_WIFI_CNCT_RETRY_MAX
#define NM_WIFI_CNCT_RETRY_MAX                          (5)
#endif

// DHCP timeout default value
#ifndef NM_WIFI_DHCP_DEF_TIMEOUT
#define NM_WIFI_DHCP_DEF_TIMEOUT                        (15000)
#endif

// AC enabled timeout default value
#ifndef NM_WIFI_AC_EN_DEF_TIMEOUT
#define NM_WIFI_AC_EN_DEF_TIMEOUT                       (3000)
#endif

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

// Network manager FSM event list
typedef enum E_NmEventList
{
    NM_EVT_BEGIN = 0x2000,
    //NM_EVT_INIT_REQ,
    NM_EVT_WIFI_SCAN_REQ,
    NM_EVT_WIFI_CNCT_REQ,
   
    NM_EVT_WIFI_INIT_IND,
    NM_EVT_WIFI_SCAN_IND,
    NM_EVT_WIFI_CNCT_PASS_IND,
    NM_EVT_WIFI_CNCT_FAIL_IND,
    NM_EVT_WIFI_DISC_IND,

    NM_EVT_WIFI_AC_EN_IND,
    NM_EVT_WIFI_AC_DISA_IND,

    NM_EVT_WIFI_UP_IND,
    NM_EVT_WIFI_DOWN_IND,
    NM_EVT_GOT_IP,
    NM_EVT_WIFI_RESET,

    NM_EVT_DHCP_TIMEOUT,
    NM_EVT_AC_EN_TIMEOUT,

    NM_EVT_TOTAL,
} T_NmEventList;

// WI-FI agent FSM state list
typedef enum E_NmStateList
{
    NM_ST_NULL              = 0,
    NM_ST_IDLE              = 1,

    NM_ST_WAIT_SCAN         = 2,
    NM_ST_WAIT_CNCT         = 3,

    NM_ST_WIFI_UP           = 4,
    NM_ST_GOT_IP            = 5,

    NM_ST_DHCP_WAIT_DISC    = 6,
    NM_ST_CNCT_WAIT_DISC    = 7,

    NM_ST_WIFI_UP_WAIT_SCAN = 8,
    NM_ST_WIFI_UP_ACPT_SCAN = 9,

    NM_ST_GOT_IP_WAIT_SCAN  = 10,

    NM_ST_IDLE_WAIT_AC_EN   = 11,

    NM_ST_AC_EN             = 12,
    NM_ST_AC_WIFI_UP        = 13,

    NM_ST_AC_DHCP_WAIT_DISC = 14,
    
    NM_ST_AC_SCAN_WAIT_DISC = 15,
    NM_ST_AC_CNCT_WAIT_DISC = 16,

    NM_ST_AC_GOT_IP         = 17,

    NM_ST_SCAN_WAIT_AC_EN   = 18,
    NM_ST_CNCT_WAIT_AC_EN   = 19,

    NM_ST_SCAN_WAIT_AC_DISA = 20,
    NM_ST_CNCT_WAIT_AC_DISA = 21,

    NM_ST_GOT_IP_WAIT_AC_EN = 22,

    NM_ST_QSET_WAIT_AC_EN   = 23,

} T_NmStateList;


// WI-FI connect configuration internal used
typedef struct S_NmWifiCnctConfInt
{
    uint8_t u8aBssid[WIFI_MAC_ADDRESS_LENGTH];  // The MAC address of the target AP.
    uint8_t u8aSsid[WIFI_MAX_LENGTH_OF_SSID];   // The SSID of the target AP.
    uint8_t u8SsidLen;             
    uint8_t u8aPwd[WIFI_LENGTH_PASSPHRASE];     // The password of the target AP.
    uint8_t u8PwdLen;                           // The length of the password. If the length is 64, the password is regarded as PMK.
    uint8_t u8Timeout;                          // Connect timeout
    uint8_t u8QuickSet;                         // Quick setting the AP profile and auto connect
    uint8_t u8Padding;                          // Padding
} T_NmWifiCnctConfInt;


/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

T_FsmDef * APP_NmFsmDefGet(void);

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

void APP_NmUslctdCbRun(T_NmEventList tEvtType, uint8_t *pu8Data, uint32_t u32DataLen);

static T_OplErr APP_NmInitIndHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);

static T_OplErr APP_NmScanReqHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);

static T_OplErr APP_NmCnctReqHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);

static T_OplErr APP_NmScanDoneHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);

static T_OplErr APP_NmCnctIndHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);

static T_OplErr APP_NmAcEnIndHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);

static T_OplErr APP_NmWiFiUpHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);

static T_OplErr APP_NmWiFiDownHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);

static T_OplErr APP_NmGotIpHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);

static T_OplErr APP_NmAcDisaIndHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);

static T_OplErr APP_NmDhcpTimeoutHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);

static T_OplErr APP_NmAcEnTimeoutHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);

static T_OplErr APP_NmWiFiDiscIndHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);

/***********
C Functions
***********/
// Sec 8: C Functions

#endif /* NM_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* __NET_MNGR_H__ */
