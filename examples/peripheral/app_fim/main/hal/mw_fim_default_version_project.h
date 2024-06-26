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
#ifndef _MW_FIM_DEFAULT_VERSION_PROJECT_H_
#define _MW_FIM_DEFAULT_VERSION_PROJECT_H_

#ifdef __cplusplus
extern "C" {
#endif

// Sec 0: Comment block of the file


// Sec 1: Include File
#include "mw_fim.h"


// Sec 2: Constant Definitions, Imported Symbols, miscellaneous
// the file ID
// xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx
// ^^^^ ^^^^ Zone (0~3)
//           ^^^^ ^^^^ Group (0~8), 0 is reserved for swap
//                     ^^^^ ^^^^ ^^^^ ^^^^ File ID, start from 0


/******************************
Declaration of data structure
******************************/
// Sec 3: structure, uniou, enum, linked list

/*
FIM version
*/
#define MW_FIM_VER11_PROJECT            0x00    // 0x00 ~ 0xFF
#define MW_FIM_VER12_PROJECT            0x00    // 0x00 ~ 0xFF
#define MW_FIM_VER13_PROJECT            0x00    // 0x00 ~ 0xFF
#define MW_FIM_VER14_PROJECT            0x00    // 0x00 ~ 0xFF
#define MW_FIM_VER15_PROJECT            0x00    // 0x00 ~ 0xFF
#define MW_FIM_VER16_PROJECT            0x00    // 0x00 ~ 0xFF
#define MW_FIM_VER17_PROJECT            0x00    // 0x00 ~ 0xFF
#define MW_FIM_VER18_PROJECT            0x00    // 0x00 ~ 0xFF

// Zone 2, user definie FIM area
#define MW_FIM_VER21_PROJECT            0x00    // 0x00 ~ 0xFF
#define MW_FIM_VER22_PROJECT            0x00    // 0x00 ~ 0xFF
#define MW_FIM_VER23_PROJECT            0x00    // 0x00 ~ 0xFF
#define MW_FIM_VER24_PROJECT            0x00    // 0x00 ~ 0xFF
#define MW_FIM_VER25_PROJECT            0x00    // 0x00 ~ 0xFF
#define MW_FIM_VER26_PROJECT            0x00    // 0x00 ~ 0xFF
#define MW_FIM_VER27_PROJECT            0x00    // 0x00 ~ 0xFF
#define MW_FIM_VER28_PROJECT            0x00    // 0x00 ~ 0xFF

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


#ifdef __cplusplus
}
#endif

#endif // _MW_FIM_DEFAULT_VERSION_PROJECT_H_
