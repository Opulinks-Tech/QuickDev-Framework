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
*  ble_mngr.h
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

#include "ble_mngr_api.h"
#include "ble_hci_if.h"
#include "ble_cm_if.h"
#include "ble_smp_if.h"
#include "ble_gap_if.h"
#include "ble_gatt_if.h"
#include "ble_util.h"
#include "log.h"
#include "msg.h"
#include "opl_err.h"
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __BLE_MNGR_H__
#define __BLE_MNGR_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (BM_ENABLED == 1)

// module log
#if (BM_LOG_ENABLED == 1)
#define BM_LOG_DEBG(...)                                OPL_LOG_DEBG(BM, __VA_ARGS__)
#define BM_LOG_INFO(...)                                OPL_LOG_INFO(BM, __VA_ARGS__)
#define BM_LOG_WARN(...)                                OPL_LOG_WARN(BM, __VA_ARGS__)
#define BM_LOG_ERRO(...)                                OPL_LOG_ERRO(BM, __VA_ARGS__)
#else
#define BM_LOG_DEBG(...)
#define BM_LOG_INFO(...)
#define BM_LOG_WARN(...)
#define BM_LOG_ERRO(...)
#endif

#ifndef BM_SVC_NUM_MAX
#define BM_SVC_NUM_MAX                                  (5)
#endif

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

// BLE manager FSM event list
typedef enum E_BmEventList
{
    BM_EVT_BASE                 = 0x1600,  // base number according to LE msg base definition refered from ble_msg.h
    BM_EVT_INIT_REQ,
    BM_EVT_START_REQ,
    BM_EVT_STOP_REQ,
    BM_EVT_TOTAL,
} T_BmEventLlist;

// BLE manager FSM state list
typedef enum E_BmStateList
{
    BM_ST_NULL                  = 0x0000,
    BM_ST_INITING               = 0x0001,
    BM_ST_IDLE                  = 0x0002,
    BM_ST_WAIT_ADV              = 0x0003,
    BM_ST_ADVING                = 0x0004,
    BM_ST_STOP_WAIT_ENT_ADV     = 0x0005,
    BM_ST_STOP_WAIT_EXI_ADV     = 0x0006,
    BM_ST_CONNECTED             = 0x0007,
    BM_ST_STOP_WAIT_DISC        = 0x0008,
} T_BmStateList;

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
T_OplErr BM_AdvDataSet(uint8_t *pau8AdvData, uint8_t u8AdvDataLen);

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
T_OplErr BM_AdvParamSet(uint8_t u8Type, uint8_t u8AddrType, LE_BT_ADDR_T *ptPeerAddr, uint8_t u8Filter, uint16_t u16AdvItvlMin, uint16_t u16AdvItvlMax);

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
T_OplErr BM_AdvIntervalSet(uint16_t u16AdvItvlMin, uint16_t u16AdvItvlMax);

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
T_OplErr BM_ScanRspDataSet(uint8_t *pau8ScanRspData, uint8_t u8ScanRspDataLen);

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
T_OplErr BM_ServiceAssign(T_BmSvcHandle *tBmSvcHandle);

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
T_OplErr BM_UslctedCbReg(T_BmUslctedCbFp fpUslctedCb);

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
T_OplErr BM_SendMessage(uint16_t u16Evt, void *vData, uint32_t u32DataLen, uint32_t u32DelayMs);

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
BLE_APP_DATA_T* BM_EntityGet(void);

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
void BM_TaskInit(void);

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

#endif /* BM_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* __BLE_MNGR_H__ */
