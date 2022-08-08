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
*  wifi_mngr.h
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
#include "log.h"
#include "lwip_helper.h"
#include "wifi_agent.h"
#include "wifi_autocon.h"
#include "wifi_pro_rec.h"
#include "wifi_event.h"
#include "wifi_mngr_api.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __WIFI_MNGR_H__
#define __WIFI_MNGR_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (WM_ENABLED == 1)

// module log
#if (WM_LOG_ENABLED == 1)
#define WM_LOG_DEBG(...)                                OPL_LOG_DEBG(WM, __VA_ARGS__)
#define WM_LOG_INFO(...)                                OPL_LOG_INFO(WM, __VA_ARGS__)
#define WM_LOG_WARN(...)                                OPL_LOG_WARN(WM, __VA_ARGS__)
#define WM_LOG_ERRO(...)                                OPL_LOG_ERRO(WM, __VA_ARGS__)
#else
#define WM_LOG_DEBG(...)
#define WM_LOG_INFO(...)
#define WM_LOG_WARN(...)
#define WM_LOG_ERRO(...)
#endif

#define WM_TASK_PRIORITY                                (osPriorityNormal)
#define WM_QUEUE_SIZE                                   (20)
#define WM_TASK_STACK_SIZE                              (800)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef enum E_WmEvent
{
    WM_EVT_INIT_REQ                 = 0x0000,
    WM_EVT_INIT_IND                 = 0x0001,
    WM_EVT_SCAN_REQ                 = 0x0002,
    WM_EVT_SCAN_IND                 = 0x0003,
    WM_EVT_CONNECT_REQ              = 0x0004,
    WM_EVT_CONNECT_PASS_IND         = 0x0005,
    WM_EVT_CONNECT_FAIL_IND         = 0x0006,
    WM_EVT_DISCONNECT_REQ           = 0x0007,
    WM_EVT_DISCONNECT_IND           = 0x0008,
#if (WM_AC_ENABLED == 1)
    WM_EVT_AC_ENABLE_REQ            = 0x0009,
    WM_EVT_AC_DISABLE_REQ           = 0x000A,
    WM_EVT_AC_DISABLE_NO_DISC_REQ   = 0x000B,
    WM_EVT_AC_TIMEOUT_IND           = 0x000C,
#endif
    WM_EVT_TOTAL,
} T_WmEvent;

typedef struct S_WmMessage
{
    T_WmEvent tWmEventId;
    FsmIndicateCbFunc fpIndCb;
    uint32_t u32DataLen;
    uint8_t u8aData[];
} T_WmMessage;

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
int WM_WifiEventHandlerCb(wifi_event_id_t event_id, void *data, uint16_t length);

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
T_OplErr WM_SendMessage(T_WmEvent tWmEventId, uint8_t *u8Data, uint32_t u32DataLen, FsmIndicateCbFunc fpIndCb);

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
T_OplErr WM_UslctedCbReg(T_WmUslctedCbFp fpUslctedCb);

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
void WM_UslctedCbReset(void);

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
void WM_UslctedCbRun(T_WmUslctedEvtType tEvtType, T_OplErr tEvtRst, uint8_t *pu8Data, uint32_t u32DataLen);

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
void WM_TaskInit(void);

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

#endif /* __WIFI_MNGR_H__ */
