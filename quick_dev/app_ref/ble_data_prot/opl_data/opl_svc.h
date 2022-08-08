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
*  opl_svc.h
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
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __OPL_SVC_H__
#define __OPL_SVC_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (OPL_DATA_ENABLED == 1)

#define OPL_SVC_SERVICE_UUID            0xAAAA

#define OPL_SVC_DATA_IN_UUID            0xBBB0
#define OPL_SVC_DATA_OUT_UUID           0xBBB1

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef enum E_OplSvcIdx
{
    OPL_SVC_IDX_SVC,
	OPL_SVC_IDX_DATA_IN_CHAR,
	OPL_SVC_IDX_DATA_IN_VAL,
	OPL_SVC_IDX_DATA_OUT_CHAR,
	OPL_SVC_IDX_DATA_OUT_VAL,
	OPL_SVC_IDX_DATA_OUT_CFG,

    OPL_SVC_IDX_TOTAL,
} T_OplSvcIdx;

typedef enum E_OplSvcEvent
{
    // must more then user event base level
    OPL_SVC_EVT_BASE = BM_USER_EVT_BASE + 0x200, // TODO: modify offset implement way
    OPL_SVC_EVT_SEND_DATA,
    OPL_SVC_EVT_SEND_TO_PEER,
    OPL_SVC_EVT_SEND_TO_PEER_CFM,
    OPL_SVC_EVT_TOTAL,

} T_OplSvcEvent;

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
void OPL_Svc_Init(void);

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

#endif /* OPL_DATA_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* __OPL_SVC_H__ */
