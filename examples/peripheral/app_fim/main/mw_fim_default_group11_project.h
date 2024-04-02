/******************************************************************************
*  Copyright 2017 - 2023, Opulinks Technology Ltd.
*  ----------------------------------------------------------------------------
*  Statement:
*  ----------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Opulinks Technology Ltd. (C) 2023
******************************************************************************/

/***********************
Head Block of The File
***********************/
#ifndef _MW_FIM_DEFAULT_GROUP21_PROJECT_H_
#define _MW_FIM_DEFAULT_GROUP21_PROJECT_H_

#ifdef __cplusplus
extern "C" {
#endif

// Sec 0: Comment block of the file


// Sec 1: Include File
#include "mw_fim.h"
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

// the file ID
// xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx
// ^^^^ ^^^^ Zone (0~3)
//           ^^^^ ^^^^ Group (0~8), 0 is reserved for swap
//                     ^^^^ ^^^^ ^^^^ ^^^^ File ID, start from 0
typedef enum
{
    MW_FIM_IDX_GP21_PROJECT_START = 0x02010000,             // the start IDX of group 21

    MW_FIM_IDX_GP21_PROJECT_WEIGHT_CONFIG,

    MW_FIM_IDX_GP21_PROJECT_MAX
} E_MwFimIdxGroup21_Project;

/******************************
Declaration of data structure
******************************/
// Sec 3: structure, uniou, enum, linked list
// Type define structure of weithg config
typedef struct
{
    uint8_t u8aUserID[16];
    uint8_t u8aUintOfWeight[8];
    uint16_t u16Height;
    uint8_t u8Age;
    uint8_t u8Padding;
} T_MwFimGP21WeightConfig;

//#define WM_FIM_GP11_AP_PROFILE_DEF_SSID         "00000000"
//#define WM_FIM_GP11_AP_PROFILE_DEF_PWD          "00000000"

#define MW_FIM_GP21_WEIGHT_CONFIG_SIZE             sizeof(T_MwFimGP21WeightConfig)

#define MW_FIM_GP21_WEIGHT_CONFIG_NUM              (4)


/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

extern const T_MwFimFileInfo g_taMwFimGroupTable21_project[];


// Sec 5: declaration of global function prototype


/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable


// Sec 7: declaration of static function prototype


#ifdef __cplusplus
}
#endif

#endif // _MW_FIM_DEFAULT_GROUP21_PROJECT_H_
