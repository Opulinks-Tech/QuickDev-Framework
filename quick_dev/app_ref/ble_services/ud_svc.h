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
*  ud_pf.h
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

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __UD_PF_H__
#define __UD_PF_H__

#ifdef __cplusplus
extern "C" {
#endif

#define UD_SVC_SERVICE_UUID            0x1234

#define UD_SVC_TX_CHAR_UUID            0x5353
#define UD_SVC_RX_CHAR_UUID            0x6464

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef enum E_UdSvcIdx
{
    UD_SVC_IDX_SVC,
    UD_SVC_IDX_TX_CHAR,
    UD_SVC_IDX_TX_VAL,
    UD_SVC_IDX_RX_CHAR,
    UD_SVC_IDX_RX_VAL,
    UD_SVC_IDX_RX_CFG,

    UD_SVC_IDX_TOTAL,
} T_UdSvcIdx;

// typedef enum E_UdSvcEvent
// {
//     // must more then user event base level
//     UD_SVC_EVT_BASE = BM_USER_EVT_BASE + 0x100,
//     UD_SVC_EVT_XXXXX,
//     UD_SVC_EVT_OOOOO,
//     UD_SVC_EVT_TOTAL,

// } T_UdSvcEvent;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/*************************************************************************
* FUNCTION:
*   UD_Svc_RxDataOutNotify
*
* DESCRIPTION:
*   notify data to host
*
* PARAMETERS
*   pu8Data :       [IN] data to post
*   u32DataLen :    [IN] post data lens
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr UD_Svc_RxDataOutNotify(uint8_t *pu8Data, uint32_t u32DataLen);

/*************************************************************************
* FUNCTION:
*   UD_Svc_Init
*
* DESCRIPTION:
*   initiate user define service
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void UD_Svc_Init(void);

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

#endif /* __UD_PF_H__ */
