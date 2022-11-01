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
*  ps_config_bl.h
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

#include <stdint.h>

// <<< Use Configuration Wizard in Context Menu >>>\n

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __PS_CONFIG_BL_H__
#define __PS_CONFIG_BL_H__

#ifdef __cplusplus
extern "C" {
#endif

//==========================================================
// <h> Power save
//==========================================================

// <o> PS_ENABLED - smart sleep enable
#ifndef PS_ENABLED
#define PS_ENABLED                              (1)
#endif

// <o> PS_MAC_LAY_ENABLED - mac layer sleep after got ip
#ifndef PS_MAC_LAY_ENABLED
#define PS_MAC_LAY_ENABLED                      (1)
#endif

// <o> WM_DTIM_PERIOD_TIME
#ifndef WM_DTIM_PERIOD_TIME
#define WM_DTIM_PERIOD_TIME                     (1000)
#endif

// <o> WM_AC_RETRY_INTVL_TBL - auto-connect in wifi manager retry interval table (ms)
// <i> modify below array to fit with your needs
#ifndef WM_AC_RETRY_INTVL_TBL

static uint32_t g_u32WmAcRetryIntvlTbl[5] =
{
    30000,
    30000,
    30000,
    60000,
    60000,
};

#define WM_AC_RETRY_INTVL_TBL                   (g_u32WmAcRetryIntvlTbl)
#endif

// </h>

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

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

// <<< end of configuration section >>>

#endif /* __PS_CONFIG_BL_H__ */
