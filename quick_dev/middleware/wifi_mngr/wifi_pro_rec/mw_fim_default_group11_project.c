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
// Sec 0: Comment block of the file


// Sec 1: Include File

#include "mw_fim_default_group11.h"
#include "mw_fim_default_group11_project.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (WM_ENABLED == 1)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list
// the default data of SSID roaming
const T_PrApProfile g_tMwFimDefaultGp11ApProfile = {0};
// the address buffer of SSID roaming
uint32_t g_u32aMwFimAddrBufferGP11ApProfile[MW_FIM_GP11_AP_PROFILE_NUM];

// the default data of fixed SSID roaming
const T_PrApProfile g_tMwFimDefaultGp11FixedApProfile =

{
    .u8Ssid     = WM_FIM_GP11_AP_PROFILE_DEF_SSID,
    .u8Pwd      = WM_FIM_GP11_AP_PROFILE_DEF_PWD,
    .u8Used     = 1,
    .u8SeqNo    = 0,
};


/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// the information table of group 11
const T_MwFimFileInfo g_taMwFimGroupTable11_project[] =
{
    {MW_FIM_IDX_GP11_PROJECT_AP_PROFILE, MW_FIM_GP11_AP_PROFILE_NUM,  MW_FIM_GP11_AP_PROFILE_SIZE, (uint8_t*)&g_tMwFimDefaultGp11ApProfile, g_u32aMwFimAddrBufferGP11ApProfile},
    // the end, don't modify and remove it
    {0xFFFFFFFF,            0x00,              0x00,               NULL,                            NULL}
};


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

#endif /* WM_ENABLED */
