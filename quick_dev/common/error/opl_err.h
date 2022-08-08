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
*  opl_err.h
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __OPL_ERR_H__
#define __OPL_ERR_H__

#ifdef __cplusplus
extern "C" {
#endif

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef enum E_OplErr
{
    OPL_OK                                          = 0,
    OPL_OK_WITH_NO_ACTION,
    OPL_ERR,
    OPL_ERR_ALLOC_MEMORY_FAIL,
    OPL_ERR_PARAM_INVALID,
    OPL_ERR_CASE_INVALID,
    OPL_ERR_RTOS_TASK_NOT_INIT,
    OPL_ERR_RTOS_TASK_REINIT,
    OPL_ERR_RTOS_QMSG_NOT_INIT,
    OPL_ERR_RTOS_QMSG_REINIT,
    OPL_ERR_RTOS_SEND_QMSG_FAIL,
    OPL_ERR_RTOS_PROC_QMSG_FAIL,
    OPL_ERR_RTOS_SEM_NOT_INIT,
    OPL_ERR_RTOS_SEM_REINIT,
    OPL_ERR_RTOS_EVT_NOT_FOUND,
    OPL_ERR_FSM_NOT_INIT,
    OPL_ERR_FSM_REINIT,
    OPL_ERR_FSM_ST_INVALID,
    OPL_ERR_FSM_EVT_INVALID,
    OPL_ERR_IND_CB_REG_INVALID,
    OPL_ERR_IND_CB_NOT_RELATED,
    OPL_ERR_USLCTED_CB_REG_INVALID,
    OPL_ERR_WIFI_EVTLOOP_REINIT,
    OPL_ERR_WIFI_SCAN_CMD_FAIL,
    OPL_ERR_WIFI_SCAN_CMD_TIMEOUT,
    OPL_ERR_WIFI_SCAN_FAIL,
    OPL_ERR_WIFI_SCAN_NO_AP,
    OPL_ERR_WIFI_AC_CMD_FAIL,
    OPL_ERR_WIFI_AC_LIST_EMPTY,
    OPL_ERR_WIFI_CONNECT_CMD_FAIL,
    OPL_ERR_WIFI_CONNECT_CMD_TIMEOUT,
    OPL_ERR_WIFI_CONNECT_FAIL,
    OPL_ERR_WIFI_DISCONNECT_CMD_FAIL,
    OPL_ERR_WIFI_DISCONNECT_CMD_TIMEOUT,
    OPL_ERR_WIFI_DISCONNECT_FAIL,
    OPL_ERR_BLE_TASK_REINIT,
    OPL_ERR_BLE_SVC_ASSIGN_INVALID,
    OPL_ERR_BLE_ENT_ADV_CMD_FAIL,
    OPL_ERR_BLE_ENT_ADV_FAIL,
    OPL_ERR_BLE_EXI_ADV_CMD_FAIL,
    OPL_ERR_BLE_EXI_ADV_FAIL,
    OPL_ERR_BLE_CONNECT_FAIL,
    OPL_ERR_BLE_DISCONNECT_CMD_FAIL,
    OPL_ERR_BLE_DISCONNECT_FAIL,
    OPL_OTA_SEQ_ERROR,
    OPL_OTA_FAIL,
    OPL_OTA_TIMEOUT,

    OPL_ERR_CODE_TOTAL,
} T_OplErr;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

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

#endif /* __OPL_ERR_H__ */
