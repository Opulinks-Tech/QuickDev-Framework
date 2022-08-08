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
*  wifi_agent.h
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
#include "opl_err.h"
#include "wifi_mngr_api.h"
#include "wifi_agent_api.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __WIFI_AGENT_H__
#define __WIFI_AGENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (WM_ENABLED == 1)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

// WI-FI agent FSM event list
typedef enum E_WaEventList
{
    WA_EVT_INIT_REQ         = 0x0000,
    WA_EVT_SCAN_REQ         = 0x0001,
    WA_EVT_CONNECT_REQ      = 0x0002,
    WA_EVT_DISCONNECT_REQ   = 0x0003,

    WA_EVT_INIT_IND         = 0x0004,
    WA_EVT_SCAN_IND         = 0x0005,
    WA_EVT_CONNECT_PASS_IND = 0x0006,
    WA_EVT_CONNECT_FAIL_IND = 0x0007,
    WA_EVT_DISCONNECT_IND   = 0x0008,
    
    WA_EVT_TOTAL,
} T_WaEventList;

// WI-FI agent FSM state list
typedef enum E_WaStateList
{
    WA_ST_NULL              = 0x0000,
    WA_ST_INITING           = 0x0001,
    WA_ST_IDLE              = 0x0002,
    WA_ST_SCANING           = 0x0003,
    WA_ST_CONNECTING        = 0x0004,
    WA_ST_CONNECTED         = 0x0005,
    WA_ST_CONNECTED_SCAN    = 0x0006,
    WA_ST_DISCONNECTING     = 0x0007,
} T_WaStateList;

typedef enum E_WaConnectType
{
    WA_CONN_TYPE_NONE,
    WA_CONN_TYPE_FAST,
    WA_CONN_TYPE_SSID,
    WA_CONN_TYPE_BSSID,
} T_WaConnectType;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

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
T_FsmDef * WM_WaFsmDefGet(void);

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
void WM_WaInit(void);

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

static T_OplErr WM_WaInitRequestHandler       (T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
static T_OplErr WM_WaInitIndicateHandler      (T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
static T_OplErr WM_WaScanRequestHandler       (T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
static T_OplErr WM_WaScanIndicateHandler      (T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
static T_OplErr WM_WaConnectRequestHandler    (T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
static T_OplErr WM_WaConnectIndicateHandler   (T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
static T_OplErr WM_WaDisconnectRequestHandler (T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
static T_OplErr WM_WaDisconnectIndicateHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);

/***********
C Functions
***********/
// Sec 8: C Functions

#endif /* WM_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* __WIFI_AGENT_H__ */
