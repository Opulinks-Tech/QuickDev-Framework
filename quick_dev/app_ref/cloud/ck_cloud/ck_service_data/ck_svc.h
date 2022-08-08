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
*  ck_svc.h
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

#include "opl_err.h"
#include "ble_mngr.h"
#include "ck_data_hdl.h"
#include "ck_data_ptcl.h"
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __CK_SVC_H__
#define __CK_SVC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CK_SVC_SERVICE_UUID            0xAAAA

#define CK_SVC_DATA_IN_UUID            0xBBB0
#define CK_SVC_DATA_OUT_UUID           0xBBB1

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef enum E_CkSvcIdx
{
    CK_SVC_IDX_SVC,
	CK_SVC_IDX_DATA_IN_CHAR,
	CK_SVC_IDX_DATA_IN_VAL,
	CK_SVC_IDX_DATA_OUT_CHAR,
	CK_SVC_IDX_DATA_OUT_VAL,
	CK_SVC_IDX_DATA_OUT_CFG,

    CK_SVC_IDX_TOTAL,
} T_CkSvcIdx;

typedef enum E_CkSvcEvent
{
    // must more then user event base level
    CK_SVC_EVT_BASE = BM_USER_EVT_BASE + 0x200,
    CK_SVC_EVT_SEND_DATA,
    CK_SVC_EVT_SEND_TO_PEER,
    CK_SVC_EVT_SEND_TO_PEER_CFM,
    CK_SVC_EVT_TOTAL,

} T_CkSvcEvent;

// prototype of coolkit data protocol notify callback
typedef void (* T_CkDataProtocolNotifyCbFp)(T_CkDataEvent tCkDataEvent, T_OplErr tEvtRst);

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

extern T_CkDataProtocolNotifyCbFp g_tCkDataProtocolNotifyCb;

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
void CK_Svc_Init(T_CkDataProtocolNotifyCbFp tCkDataProtocolNotifyCbFp);

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

#endif /* __CK_SVC_H__ */
