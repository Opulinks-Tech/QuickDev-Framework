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

//#include "mw_fim_default_group21.h"
#include "mw_fim_default_group21_project.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous


/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list
// the default data of weight config
const T_MwFimGP21WeightConfig g_tMwFimDefaultGp21WeightConfig =
{
    .u8aUserID          = "user",
    .u8aUintOfWeight    = "kg",
    .u16Height          = 183,
    .u8Age              = 35,
};
// the address buffer of weight config
uint32_t g_u32aMwFimAddrBufferGP21WeightConfig[MW_FIM_GP21_WEIGHT_CONFIG_NUM];


/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// the information table of group 21
const T_MwFimFileInfo g_taMwFimGroupTable21_project[] =
{
    {MW_FIM_IDX_GP21_PROJECT_WEIGHT_CONFIG, MW_FIM_GP21_WEIGHT_CONFIG_NUM,  MW_FIM_GP21_WEIGHT_CONFIG_SIZE, (uint8_t*)&g_tMwFimDefaultGp21WeightConfig, g_u32aMwFimAddrBufferGP21WeightConfig},
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

