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
*  rf_pwr.h
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

#include "opl_err.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __RF_PWR_H__
#define __RF_PWR_H__

#ifdef __cplusplus
extern "C" {
#endif

// .-----------------.----------------.------------------------.-----------------:----------------.
// |                 |  BLE Low Power | Peak Power11/5.5/2Mbps | Peak Power1Mbps | RSSI Offset    |
// :-----------------+----------------+------------------------:-----------------:----------------:
// | WIFI High power |  0xD0 (208)    |  +10dBm                |  +13dBm         |  -10           |
// :-----------------+----------------+------------------------:-----------------:----------------:
// | WIFI High power |  0xB0 (176)    |  +10dBm                |  +10dBm         |  -10           |
// :-----------------+----------------+------------------------:-----------------:----------------:
// | WIFI High power |  0xA0 (160)    |  +7dBm                 |  +10dBm         |  -13           |
// :-----------------+----------------+------------------------:-----------------:----------------:
// | WIFI High power |  0x90 (144)    |  +4dBm                 |  +7dBm          |  -16           |
// :-----------------+----------------+------------------------:-----------------:----------------:
// | WIFI Low power  |  0x40 (64)     |  -3dBm                 |  +0dBm          |  -22           |
// :-----------------+----------------+------------------------:-----------------:----------------:
// | WIFI Low power  |  0x20 (32)     |  -3dBm                 |  -3dBm          |  -22           |
// :-----------------+----------------+------------------------:-----------------:----------------:
// | WIFI Low power  |  0x00 (0)      |  -5dBm                 |  -5dBm          |  -25           |
// :-----------------+----------------+------------------------:-----------------:----------------:

#define RF_PWR_LVL_D0               (0xD0)
#define RF_PWR_LVL_B0               (0xB0)
#define RF_PWR_LVL_A0               (0xA0)
#define RF_PWR_LVL_90               (0x90)
#define RF_PWR_LVL_40               (0x40)
#define RF_PWR_LVL_20               (0x20)
#define RF_PWR_LVL_00               (0x00)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

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
T_OplErr RF_PwrRssiOffsetGet(int8_t *offset);

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
T_OplErr RF_PwrSet(uint8_t level);

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

#endif /* __RF_PWR_H__ */
