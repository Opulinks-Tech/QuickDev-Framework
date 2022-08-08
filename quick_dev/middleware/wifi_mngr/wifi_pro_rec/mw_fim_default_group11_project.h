/******************************************************************************
*  Copyright 2017 - 2018, Opulinks Technology Ltd.
*  ----------------------------------------------------------------------------
*  Statement:
*  ----------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Opulinks Technology Ltd. (C) 2018
******************************************************************************/

/***********************
Head Block of The File
***********************/
#ifndef _MW_FIM_DEFAULT_GROUP11_PROJECT_H_
#define _MW_FIM_DEFAULT_GROUP11_PROJECT_H_

#ifdef __cplusplus
extern "C" {
#endif

// Sec 0: Comment block of the file


// Sec 1: Include File
#include "mw_fim.h"
#include "wifi_pro_rec.h"
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (WM_ENABLED == 1)

// the file ID
// xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx
// ^^^^ ^^^^ Zone (0~3)
//           ^^^^ ^^^^ Group (0~8), 0 is reserved for swap
//                     ^^^^ ^^^^ ^^^^ ^^^^ File ID, start from 0
typedef enum
{
    MW_FIM_IDX_GP11_PROJECT_START = 0x01010000,             // the start IDX of group 11

    MW_FIM_IDX_GP11_PROJECT_AP_PROFILE,

    MW_FIM_IDX_GP11_PROJECT_MAX
} E_MwFimIdxGroup11_Project;

/******************************
Declaration of data structure
******************************/
// Sec 3: structure, uniou, enum, linked list
// // Wifi SSID roaming
// typedef struct
// {
//     uint8_t u8Pwd[WIFI_LENGTH_PASSPHRASE];      //WIFI_LENGTH_PASSPHRASE=64         /**< The password of the target AP. >**/
//     uint8_t u8Ssid[WIFI_MAX_LENGTH_OF_SSID];    //WIFI_MAX_LENGTH_OF_SSID=32+11ac   /**< The SSID of the target AP. >**/
//     uint8_t u8Used;                                                          
//     uint8_t u8SeqNo;                                                                /**< The flag of seqno overflow >**/
//     uint8_t u8Padding;
// } T_MwFimGP11ApProfile;

#define WM_FIM_GP11_AP_PROFILE_DEF_SSID         "00000000"
#define WM_FIM_GP11_AP_PROFILE_DEF_PWD          "00000000"

#define MW_FIM_GP11_AP_PROFILE_SIZE             sizeof(T_PrApProfile)

// refer to wifi_pro_rec.h
#ifndef PR_AP_RPOFILE_MAX_NUM
#define MW_FIM_GP11_AP_PROFILE_NUM              (PR_AP_PROFILE_MAX_NUM)
#else
#define MW_FIM_GP11_AP_PROFILE_NUM              (4)
#endif


/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

extern const T_MwFimFileInfo g_taMwFimGroupTable11_project[];


// Sec 5: declaration of global function prototype


/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable


// Sec 7: declaration of static function prototype

#endif /* WM_ENABLED */

#ifdef __cplusplus
}
#endif

#endif // _MW_FIM_DEFAULT_GROUP11_PROJECT_H_
