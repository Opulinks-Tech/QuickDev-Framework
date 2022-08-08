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
*  ota_mngr.h
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
#include "opl_err.h"
#include "mw_ota.h"
#include "mw_ota_def.h"
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __OTA_MNGR_H__
#define __OTA_MNGR_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (OTA_ENABLED == 1)

// module log
#if (OTA_LOG_ENABLED == 1)
#define OTA_LOG_DEBG(...)                               OPL_LOG_DEBG(OTA, __VA_ARGS__)
#define OTA_LOG_INFO(...)                               OPL_LOG_INFO(OTA, __VA_ARGS__)
#define OTA_LOG_WARN(...)                               OPL_LOG_WARN(OTA, __VA_ARGS__)
#define OTA_LOG_ERRO(...)                               OPL_LOG_ERRO(OTA, __VA_ARGS__)
#else
#define OTA_LOG_DEBG(...)
#define OTA_LOG_INFO(...)
#define OTA_LOG_WARN(...)
#define OTA_LOG_ERRO(...)
#endif

#ifndef OTA_PROCESS_PERIOD_TIMEOUT
#define OTA_PROCESS_PERIOD_TIMEOUT                      (300000)
#endif

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef enum E_OtaEventList
{
    OTA_EVT_PREPARE_REQ     = 0x0000,
    OTA_EVT_DATA_IN_REQ     = 0x0001,
    OTA_EVT_FINISH_REQ      = 0x0002,
    OTA_EVT_GIVEUP_REQ      = 0x0003,
    OTA_EVT_PROC_TIMEOUT    = 0x0004,
    OTA_EVT_REBOOT_REQ      = 0x0005,

    OTA_EVT_TOTAL,
} T_OtaEventList;

typedef enum E_OtaStateList
{
    OTA_ST_IDLE             = 0x0000,
    OTA_ST_PROC             = 0x0001,

    OTA_ST_TOTAL,
} T_OtaStateList;

typedef void (* T_OtaProcTimeoutIndCbFp)(void);

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
T_OplErr OTA_CurrentVersionGet(uint16_t *u16PrjId, uint16_t *u16ChipId, uint16_t *u16FwId);

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
T_OplErr OTA_UpgradeBegin(uint16_t *pu16SeqId, T_MwOtaFlashHeader *ptOtaHdr, T_OtaProcTimeoutIndCbFp tOtaProcTimeoutIndCb);

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
T_OplErr OTA_WriteData(uint16_t u16SeqId, uint8_t *pu8Data, uint32_t u32DataLen);

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
T_OplErr OTA_UpgradeFinish(uint16_t u16SeqId);

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
T_OplErr OTA_UpgradeGiveUp(uint16_t u16SeqId);

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
T_OplErr OTA_TriggerReboot(uint16_t u16Delay);

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
T_FsmDef *OTA_FsmDefGet(void);

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
void OTA_Init(void);

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

static T_OplErr OTA_PrepareRequestHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
static T_OplErr OTA_DataInRequestHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
static T_OplErr OTA_FinishRequestHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
static T_OplErr OTA_GiveUpRequestHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
static T_OplErr OTA_RebootRequestHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);

/***********
C Functions
***********/
// Sec 8: C Functions

#ifdef __cplusplus
}
#endif

#endif /* OTA_ENABLED */

#endif /* __OTA_MNGR_H__ */
