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
*  wifi_autocon.h
*
*  Project:
*  --------
*
*  Description:
*  ------------
*  This include file is the auto-connect definition file
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
#include "log.h"
#include "opl_err.h"
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __WIFI_AUTOCON_H__
#define __WIFI_AUTOCON_H__

#ifdef __cplusplus
extern "C" {
#endif

// force to enable auto-connect while network manager was enabled
#if (NM_ENABLED == 1)
#undef  WM_AC_ENABLED
#define WM_AC_ENABLED                       (1)
#endif

#if (WM_AC_ENABLED == 1)

// module log
#if (WM_AC_LOG_ENABLED == 1)
#define AC_LOG_DEBG(...)                                OPL_LOG_DEBG(AC, __VA_ARGS__)
#define AC_LOG_INFO(...)                                OPL_LOG_INFO(AC, __VA_ARGS__)
#define AC_LOG_WARN(...)                                OPL_LOG_WARN(AC, __VA_ARGS__)
#define AC_LOG_ERRO(...)                                OPL_LOG_ERRO(AC, __VA_ARGS__)
#else
#define AC_LOG_DEBG(...)
#define AC_LOG_INFO(...)
#define AC_LOG_WARN(...)
#define AC_LOG_ERRO(...)
#endif

#ifndef WM_AC_RETRY_INTVL_TBL

static uint32_t g_u32WmAcRetryIntvlTbl[5] = 
{
    30000,
    30000,
    60000,
    60000,
    900000,
};

#define WM_AC_RETRY_INTVL_TBL               (g_u32WmAcRetryIntvlTbl)
#endif

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list
// State List
typedef enum E_AC_StateList
{
    AC_ST_NULL                      = 0x0000,
    AC_ST_IDLE                      = 0x0001,
    AC_ST_FAST_CONNECTING           = 0x0002,
    AC_ST_HIDDEN_SCANNING           = 0x0003,
    AC_ST_DEDICATE_CONNECTING       = 0x0004,
    AC_ST_HIDDEN_CONNECTING         = 0x0005,
    AC_ST_DISA_WAIT_IND             = 0x0006,
    AC_ST_DISA_WAIT_IND_NO_DISC     = 0x0007,
    AC_ST_DISA_WAIT_DISC            = 0x0008,

    AC_ST_TOTAL
} T_AC_StateList;

// Event List
typedef enum E_AC_EventList
{
    AC_EVT_ENABLE_AC_REQ            = 0x0000,
    AC_EVT_DISABLE_AC_REQ           = 0x0001,
    AC_EVT_DISABLE_AC_NO_DISC_REQ   = 0x0002,
    AC_EVT_CONNECT_SUCCESS_IND      = 0x0003,
    AC_EVT_CONNECT_FAIL_IND         = 0x0004,
    AC_EVT_SCAN_DONE_IND            = 0x0005,
    AC_EVT_DISC_SUCCESS_IND         = 0x0006,
    AC_EVT_DISCONNECT_IND           = 0x0007,
    AC_EVT_AC_TIMEOUT_IND           = 0x0008,
    AC_EVT_WIFI_RESET_IND           = 0x0009,

    AC_EVT_TOTAL
} T_AC_EventList;


/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable


// Sec 5: declaration of global function prototype
int32_t WM_AcInit(void);
T_FsmDef* WM_AcFsmDefGet(void);


/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable


// Sec 7: declaration of static function prototype


/***********
C Functions
***********/
// Sec 8: C Functions

#endif /* WM_AC_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* __WIFI_AUTOCON_H__ */
