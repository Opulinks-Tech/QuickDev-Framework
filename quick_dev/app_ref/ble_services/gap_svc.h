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
*  gap_pf.h
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

#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __GAP_PF_H__
#define __GAP_PF_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BLE_GAP_PF_DEVICE_NAME
#define BLE_GAP_PF_DEVICE_NAME                  "BLEWIFI APP"
#endif

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef enum E_GapSvcIdx
{
    GAP_SVC_IDX_SVC,

    GAP_SVC_IDX_DEVICE_NAME_CHAR,
    GAP_SVC_IDX_DEVICE_NAME_VAL,

    GAP_SVC_IDX_APPEARANCE_CHAR,
    GAP_SVC_IDX_APPEARANCE_VAL,

    GAP_SVC_IDX_CONN_PARAM_CHAR,
    GAP_SVC_IDX_CONN_PARAM_VAL,

    GAP_SVC_IDX_TOTAL,
} T_GapSvcIdx;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/*************************************************************************
* FUNCTION:
*   GAP_Svc_Init
*
* DESCRIPTION:
*   initiate gap service
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void GAP_Svc_Init(void);

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

#endif /* __GAP_PF_H__ */
