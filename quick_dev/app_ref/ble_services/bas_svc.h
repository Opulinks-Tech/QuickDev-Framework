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
*  bas_svc.h
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

#include "ble_mngr.h"
#include "opl_err.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __BAS_SVC_H__
#define __BAS_SVC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define BAS_SVC_SERVICE_UUID            0x180F

#define BAS_SVC_BAT_LVL_CHAR_UUID       0x2A19

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef enum E_UdSvcIdx
{
    BAS_SVC_IDX_SVC,
    BAS_SVC_IDX_BAT_LVL_CHAR,
    BAS_SVC_IDX_BAT_LVL_VAL,
    BAS_SVC_IDX_BAT_LVL_CFG,

    BAS_SVC_IDX_TOTAL,
} T_UdSvcIdx;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/*************************************************************************
* FUNCTION:
*   BAS_Svc_BatteryLevelNotify
*
* DESCRIPTION:
*   notify battery level to host
*
* PARAMETERS
*   u8Percent :     [IN] percentage of battery level
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr BAS_Svc_BatteryLevelNotify(uint8_t u8Percent);

/*************************************************************************
* FUNCTION:
*   BAS_Svc_Init
*
* DESCRIPTION:
*   initiate battery service
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void BAS_Svc_Init(void);

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

#endif /* __UD_SVC_H__ */
