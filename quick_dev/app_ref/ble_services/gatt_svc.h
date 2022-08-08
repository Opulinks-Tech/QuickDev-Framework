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
*  gatt_svc.h
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

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __GATT_SVC_H__
#define __GATT_SVC_H__

#ifdef __cplusplus
extern "C" {
#endif

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef enum E_GattSvcIdx
{
    GATT_SVC_IDX_SVC,
        
    GATT_SVC_IDX_SERVICE_CHANGE_CHAR,
    GATT_SVC_IDX_SERVICE_CHANGE_VAL,
    GATT_SVC_IDX_SERVICE_CHANGE_CFG,

    GATT_SVC_IDX_TOTAL,

} T_GattSvcIdx;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/*************************************************************************
* FUNCTION:
*   GATT_Svc_IndicateServiceChange
*
* DESCRIPTION:
*   indicate serivce change
*
* PARAMETERS
*   u16ConnHdl :    [IN] connection handle
*
* RETURNS
*   none
*
*************************************************************************/
void GATT_Svc_IndicateServiceChange(uint16_t u16ConnHdl);

/*************************************************************************
* FUNCTION:
*   GATT_Svc_Init
*
* DESCRIPTION:
*   initiate gatt service
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void GATT_Svc_Init(void);

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

#endif /* __GATT_SVC_H__ */
