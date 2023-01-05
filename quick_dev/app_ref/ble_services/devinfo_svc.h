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
*  devinfo_svc.h
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

#ifndef __DEVINFO_SVC_H__
#define __DEVINFO_SVC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define FW_REV_VAL_MAX_LEN                          (6)
#define MODEL_NB_VAL_MAX_LEN                        (15)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef enum E_DevInfoSvcIdx
{
    DEV_INFO_SVC_IDX_SVC,

    DEV_INFO_SVC_IDX_FW_REV_CHAR,
    DEV_INFO_SVC_IDX_FW_REV_VAL,

    DEV_INFO_SVC_IDX_MODEL_NB_CHAR,
    DEV_INFO_SVC_IDX_MODEL_NB_VAL,

    DEV_INFO_SVC_IDX_TOTAL,
} T_DevInfoSvcIdx;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/*************************************************************************
* FUNCTION:
*   DevInfo_Svc_FirmwareRevision_Set
*
* DESCRIPTION:
*   setting firmware revision
*
* PARAMETERS
*   pu8Data :       [IN] data
*   u32DataLen :    [IN] data lens
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr DevInfo_Svc_FirmwareRevision_Set(uint8_t *pu8Data, uint32_t u32DataLen);

/*************************************************************************
* FUNCTION:
*   DevInfo_Svc_ModelNumber_Set
*
* DESCRIPTION:
*   setting model number
*
* PARAMETERS
*   pu8Data :       [IN] data to post
*   u32DataLen :    [IN] post data lens
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr DevInfo_Svc_ModelNumber_Set(uint8_t *pu8Data, uint32_t u32DataLen);

/*************************************************************************
* FUNCTION:
*   DevInfo_Svc_Init
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
void DevInfo_Svc_Init(void);

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

#endif /* __DEVINFO_SVC_H__ */
