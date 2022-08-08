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
*  sys_configuration.h
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

// <<< Use Configuration Wizard in Context Menu >>>\n

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __QD_MODULE_H__
#define __QD_MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif

//==========================================================
// <h> Common module
//==========================================================

// <e> BAT_MEAS_ENABLED - battery measurement
//==========================================================
#ifndef BAT_MEAS_ENABLED
#define BAT_MEAS_ENABLED                        (0)
#endif

// <o> BAT_MEAS_LOG_ENABLED - battery measurment log enable
#ifndef BAT_MEAS_LOG_ENABLED
#define BAT_MEAS_LOG_ENABLED                    (0)
#endif

// <o> BAT_MEAS_IO_PORT - battery gpio index
// <i> refer to hal_vic.h
#ifndef BAT_MEAS_IO_PORT
#define BAT_MEAS_IO_PORT                        (GPIO_IDX_02)
#endif

// </e>

// <e> TIME_STMP_ENABLED - time stamp module
//==========================================================
#ifndef TIME_STMP_ENABLED
#define TIME_STMP_ENABLED                       (0)
#endif

// <o> TIME_STMP_LOG_ENABLED - time stamp log enable
#ifndef TIME_STMP_LOG_ENABLED
#define TIME_STMP_LOG_ENABLED                   (0)
#endif

// <o> TIME_STMP_INIT_SEC - time stamp initiate second
// <0=> 1970 second
// <1483228800=> 2017 second
// <1546300800=> 2019 second
#ifndef TIME_STMP_INIT_SEC
#define TIME_STMP_INIT_SEC                      (1617825338)
#endif

// </e>

// <e> FSM_ENABLED - Finite state machine
//==========================================================
#ifndef FSM_ENABLED
#define FSM_ENABLED                             (1)
#endif

// <o> FSM_LOG_ENABLED - Finite state machine log enable
#ifndef FSM_LOG_ENABLED
#define FSM_LOG_ENABLED                         (0)
#endif

// </e>

// </h>

//==========================================================
// <h> WI-FI module
//==========================================================

// <e> WM_ENABLED - WI-FI manager module
//==========================================================
#ifndef WM_ENABLED
#define WM_ENABLED                              (1)
#endif

// <o> WM_AT_ENABLED - WI-FI manager at cmd enable
#ifndef WM_AT_ENABLED
#define WM_AT_ENABLED                           (0)
#endif

// <o> WM_LOG_ENABLED - WI-FI manager log enable
#ifndef WM_LOG_ENABLED
#define WM_LOG_ENABLED                          (0)
#endif

// <o> WM_USLCTED_CB_REG_NUM - WI-FI manager unsolicited callback number
#ifndef WM_USLCTED_CB_REG_NUM
#define WM_USLCTED_CB_REG_NUM                   (5)
#endif

// <o> WM_PR_FIX_AP_ENABLED - WI-FI manager profile record enable/disable fix AP
#ifndef WM_PR_FIX_AP_ENABLED
#define WM_PR_FIX_AP_ENABLED                    (0)
#endif

// <o> WM_PR_FIX_AP_NUM - WI-FI manager profile record fix AP number
#ifndef WM_PR_FIX_AP_NUM
#define WM_PR_FIX_AP_NUM                        (1)
#endif

// <o> WM_PR_UNFIX_AP_NUM - WI-FI manager profile record un-fix AP number
// <i> If WM_PR_FIX_AP_ENABLED enabled, total AP number be stored in fim is WM_PR_FIX_AP_NUM + WM_PR_UNFIX_AP_NUM,
// otherwise, total AP number will be WM_PR_UNFIX_AP_NUM
#ifndef WM_PR_UNFIX_AP_NUM
#define WM_PR_UNFIX_AP_NUM                      (4)
#endif

// </e>

// <e> NM_ENABLED - network manager module
//==========================================================
#ifndef NM_ENABLED
#define NM_ENABLED                              (1)
#endif

// <o> NM_AT_ENABLED - network manager at cmd enable
#ifndef NM_AT_ENABLED
#define NM_AT_ENABLED                           (0)
#endif

// <o> NM_LOG_ENABLED - network manager log enable
#ifndef NM_LOG_ENABLED
#define NM_LOG_ENABLED                          (0)
#endif

// <o> NM_WIFI_CNCT_DEF_TIMEOUT - network manager WI-FI connect default timeout value (ms)
#ifndef NM_WIFI_CNCT_DEF_TIMEOUT
#define NM_WIFI_CNCT_DEF_TIMEOUT                (60000)
#endif

// <o> NM_WIFI_CNCT_RETRY_MAX - netework manager WI-FI connect maximum retry times
#ifndef NM_WIFI_CNCT_RETRY_MAX
#define NM_WIFI_CNCT_RETRY_MAX                  (5)
#endif

// <o> NM_WIFI_DHCP_DEF_TIMEOUT - network manager DHCP default timeout value (ms)
#ifndef NM_WIFI_DHCP_DEF_TIMEOUT
#define NM_WIFI_DHCP_DEF_TIMEOUT                (15000)
#endif

// <o> NM_WIFI_AC_EN_DEF_TIMEOUT - network manager AC enable default timeout value (ms)
#ifndef NM_WIFI_AC_EN_DEF_TIMEOUT
#define NM_WIFI_AC_EN_DEF_TIMEOUT               (3000)
#endif

// </e>

// <e> WM_AC_ENABLED - WI-FI auto-connect module
//==========================================================
#ifndef WM_AC_ENABLED
#define WM_AC_ENABLED                           (1)
#endif

// <o> WM_AC_AT_ENABLED - WI-FI auto-connect at cmd enable
#ifndef WM_AC_AT_ENABLED
#define WM_AC_AT_ENABLED                        (0)
#endif

// <o> WM_AC_LOG_ENABLED - WI-FI auto-connect log enable
#ifndef WM_AC_LOG_ENABLED
#define WM_AC_LOG_ENABLED                       (0)
#endif

// </e>

// </h>

//==========================================================
// <h> BLE manager
//==========================================================

// <e> BM_ENABLED - BLE manager module
//==========================================================
#ifndef BM_ENABLED
#define BM_ENABLED                              (0)
#endif

// <o> BM_AT_ENABLED - BLE manager at cmd enable
#ifndef BM_AT_ENABLED
#define BM_AT_ENABLED                           (0)
#endif

// <o> BM_LOG_ENABLED - BLE manager log enable
#ifndef BM_LOG_ENABLED
#define BM_LOG_ENABLED                          (0)
#endif

// <o> BM_SVC_NUM_MAX - maximum service number to be register
#ifndef BM_SVC_NUM_MAX
#define BM_SVC_NUM_MAX                          (5)
#endif

// <o> BM_DEF_DESIRED_MIN_CONN_INTERVAL - minimum connection interval (units of 1.25ms)
#ifndef BM_DEF_DESIRED_MIN_CONN_INTERVAL
#define BM_DEF_DESIRED_MIN_CONN_INTERVAL        (100)
#endif

// <o> BM_DEF_DESIRED_MAX_CONN_INTERVAL - maximum connection interval (uints of 1.25ms)
#ifndef BM_DEF_DESIRED_MAX_CONN_INTERVAL
#define BM_DEF_DESIRED_MAX_CONN_INTERVAL        (200)
#endif

// <o> BM_DEF_DESIRED_SLAVE_LATENCY - slave latency
#ifndef BM_DEF_DESIRED_SLAVE_LATENCY
#define BM_DEF_DESIRED_SLAVE_LATENCY            (0)
#endif

// <o> BM_DEF_DESIRED_SUPERVERSION_TIMEOUT - superversion timeout time
#ifndef BM_DEF_DESIRED_SUPERVERSION_TIMEOUT
#define BM_DEF_DESIRED_SUPERVERSION_TIMEOUT     (500)
#endif

// </e>

// </h>

//==========================================================
// <h> OPL Data protocol
//==========================================================

// <e> OPL_DATA_ENABLED - Opulinks BLE WI-FI data protocol
//==========================================================
#ifndef OPL_DATA_ENABLED
#define OPL_DATA_ENABLED                        (1)
#endif

// </e>

// </h>

//==========================================================
// <h> OTA - WI-FI and BLE OTA
//==========================================================

// <e> OTA_ENABLED - OTA functions
//==========================================================
#ifndef OTA_ENABLED
#define OTA_ENABLED                             (1)
#endif

// <o> OTA_LOG_ENABLED - OTA functions log enable
#ifndef OTA_LOG_ENABLED
#define OTA_LOG_ENABLED                         (1)
#endif

// <o> OTA_PROCESS_PERIOD_TIMEOUT - OTA process timeout period (ms)
#ifndef OTA_PROCESS_PERIOD_TIMEOUT
#define OTA_PROCESS_PERIOD_TIMEOUT              (300000)
#endif

// </e>

// </h>

//==========================================================
// <h> Host mode
//==========================================================

// <e> HOST_MODE_ENABLED - host mode
//==========================================================
#ifndef HOST_MODE_ENABLED
#define HOST_MODE_ENABLED                       (0)
#endif

// <o> HOST_MODE_LOG_ENABLED - host mode log enable
#ifndef HOST_MODE_LOG_ENABLED
#define HOST_MODE_LOG_ENABLED                   (0)
#endif

// <o> HOST_MODE_USE_AT - at cmd mode
#ifndef HOST_MODE_USE_AT
#define HOST_MODE_USE_AT                        (0)
#endif

// </e>

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

#endif /* __QD_MODULE_H__ */
