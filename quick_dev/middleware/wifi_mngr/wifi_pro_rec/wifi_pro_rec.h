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
*  wifi_mngr_api.h
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

#include "cmsis_os.h"
#include "mw_fim_default_group11_project.h"
#include "qd_config.h"
#include "qd_module.h"
#include "wifi_types.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef WM_PROFILE_REC_H_
#define WM_PROFILE_REC_H_

#ifdef __cplusplus
extern "C" {
#endif

#if (WM_ENABLED == 1)

// <o> WM_PR_FIX_AP_ENABLED - WI-FI manager profile record enable/disable fix AP
#ifndef WM_PR_FIX_AP_ENABLED
#define WM_PR_FIX_AP_ENABLED                    (1)
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

// maximum number of AP profile
#if (WM_PR_FIX_AP_ENABLED == 1)
#define PR_AP_PROFILE_MAX_NUM                   (WM_PR_FIX_AP_NUM + WM_PR_UNFIX_AP_NUM)
#else
#define PR_AP_PROFILE_MAX_NUM                   (WM_PR_UNFIX_AP_NUM)
#endif

#define PR_AP_OFFSET                            WM_PR_FIX_AP_NUM
#define PR_SEQ_MAX_NUM                          (0xFF)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

// Wifi SSID roaming
typedef struct S_PrApProfile
{
    uint8_t u8Pwd[WIFI_LENGTH_PASSPHRASE];      //WIFI_LENGTH_PASSPHRASE=64         /**< The password of the target AP. >**/
    uint8_t u8Ssid[WIFI_MAX_LENGTH_OF_SSID];    //WIFI_MAX_LENGTH_OF_SSID=32+11ac   /**< The SSID of the target AP. >**/
    uint8_t u8Used;                                                          
    uint8_t u8SeqNo;                                                                /**< The flag of seqno overflow >**/
    uint8_t u8Padding;
} T_PrApProfile;

typedef T_PrApProfile   *T_PrApProfilePtr;

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
void WM_PrInit(void);

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
int WM_PrInsert(T_PrApProfile tNewProfile);

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
void WM_PrClear(void);

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
T_PrApProfilePtr WM_PrGet(void);

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
uint32_t WM_PrProfileCount(void);

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

#endif /* WM_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* WM_PROFILE_REC_H_ */
