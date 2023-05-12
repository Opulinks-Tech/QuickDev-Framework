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
*  qd_config.h
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

#include "rf_pwr.h"
#include "sys_cfg.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __QD_CONFIG_H__
#define __QD_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

//==========================================================
// <h> OPL Chip ID check
//==========================================================
#if defined(OPL1000_A2) || defined(OPL1000_A3) || defined(OPL2500_A0)

#else
#error "no define chip id in global definition in 'Preprocessor Symbols'"
#endif

// </h>

//==========================================================
// <h> RF configuration
//==========================================================

// <o> RF_CFG_DEF_PWR_SET - default tx power setup
// <i> refer to rf_pwr.h
#ifndef RF_CFG_DEF_PWR_SET
#define RF_CFG_DEF_PWR_SET                      (RF_PWR_LVL_D0)
#endif

#if 0 // Should be enable for OPL2500P
#define EXT_PA_ENABLED                          (1)
#else
#define EXT_PA_ENABLED                          (0)
#endif

// </h>

//==========================================================
// <h> System configuration
//==========================================================

// <o> SYS_CFG_32K_SWITCH_TO_RC - 32KHz use Xtal or RC
// <0=> Xtal
// <1=> RC
#ifndef SYS_CFG_32K_SWITCH_TO_RC
#define SYS_CFG_32K_SWITCH_TO_RC                (0)
#endif

// <o> SYS_CFG_CLK_RATE - system clock frequency
// <i> refer to sys_cfg.h - T_SysCfgClkIdx
#ifndef SYS_CFG_CLK_RATE
#define SYS_CFG_CLK_RATE                        (SYS_CFG_CLK_22_MHZ)
#endif


// <o> SYS_CFG_PS_MODE - power save mode
// <0=> Power mode - Low Power
// <1=> Power mode - Balance
// <2=> Power mode - Performance
// <else> User define
#ifndef SYS_CFG_PS_MODE
#define SYS_CFG_PS_MODE                         (3)
#endif

//----------------------------------------------------------
// Related to "SYS_CFG_PS_MODE" option (please read the below informations):
//
// DO NOT MODIFY THE LINKED FILE - may effect all projects setup
// Low Power - file located in '%sdk location%\quick_dev\common\config\pm_config_lp.h'
// Balance - file located in '%sdk location%\quick_dev\common\config\pm_config_bl.h'
// Performance - file located in '%sdk location%\quick_dev\common\config\pm_config_pf.h'
//
// MODIFY OPTIONS -  
// User define - modify each items selection base on project needs
//----------------------------------------------------------
#if (SYS_CFG_PS_MODE == 0)
#include "ps_config_lp.h"

#elif (SYS_CFG_PS_MODE == 1)
#include "ps_config_bl.h"

#elif (SYS_CFG_PS_MODE == 2)
#include "ps_config_pf.h"

#else
// <o> PS_ENABLED - smart sleep enable
#ifndef PS_ENABLED
#define PS_ENABLED                              (0)
#endif

// <o> PS_MAC_LAY_ENABLED - mac layer sleep after got ip
#ifndef PS_MAC_LAY_ENABLED
#define PS_MAC_LAY_ENABLED                      (1)
#endif

// <o> WM_DTIM_PERIOD_TIME
#ifndef WM_DTIM_PERIOD_TIME
#define WM_DTIM_PERIOD_TIME                     (3000)
#endif

// <o> WM_AC_RETRY_INTVL_TBL - auto-connect in wifi manager retry interval table (ms)
// <i> modify below array to fit with your needs
#ifndef WM_AC_RETRY_INTVL_TBL

static uint32_t g_u32WmAcRetryIntvlTbl[5] =
{
    30000,
    30000,
    60000,
    60000,
    900000,
};

#define WM_AC_RETRY_INTVL_TBL                   (g_u32WmAcRetryIntvlTbl)
#endif

#endif

// </h>

//==========================================================
// <h> Common configuration
//==========================================================

// <o> BLE_GAP_PF_DEVICE_NAME - device name shows in GAP service
#ifndef BLE_GAP_PF_DEVICE_NAME
#define BLE_GAP_PF_DEVICE_NAME                  "OPL_APP"
#endif

// </h>

//==========================================================
// <h> User configuration
//==========================================================

// <o> APP_HOST_MODE_PWR_SAVE_DEMO - host mode demo, power saving configure
// <i> this configuration must set same on master & slave
// <0=> using smart sleep
// <1=> using deep sleep
#ifndef APP_HOST_MODE_PWR_SAVE_DEMO
#define APP_HOST_MODE_PWR_SAVE_DEMO             (0)
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

#endif /* __QD_CONFIG_H__ */
