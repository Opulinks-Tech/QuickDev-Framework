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
*  bat_aux.h
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

#include "log.h"
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __BAT_AUX_H__
#define __BAT_AUX_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (BAT_MEAS_ENABLED == 1)

// module log
#if (BAT_MEAS_LOG_ENABLED == 1)
#define BAT_MEAS_LOG_DEBG(...)                          OPL_LOG_DEBG(BAT, __VA_ARGS__)
#define BAT_MEAS_LOG_INFO(...)                          OPL_LOG_INFO(BAT, __VA_ARGS__)
#define BAT_MEAS_LOG_WARN(...)                          OPL_LOG_WARN(BAT, __VA_ARGS__)
#define BAT_MEAS_LOG_ERRO(...)                          OPL_LOG_ERRO(BAT, __VA_ARGS__)
#else
#define BAT_MEAS_LOG_DEBG(...)
#define BAT_MEAS_LOG_INFO(...)
#define BAT_MEAS_LOG_WARN(...)
#define BAT_MEAS_LOG_ERRO(...)
#endif

#ifndef BAT_MEAS_IO_PORT
#define BAT_MEAS_IO_PORT                                (GPIO_IDX_02)
#endif

#define BAT_IO_VOLTAGE_GET_AVERAGE_COUNT                (30)
#define BAT_IO_MOVING_AVERAGE_COUNT                     (1)

#define MAXIMUM_VOLTAGE_DEF                             (3.0f)    //(3.0f)
#define MINIMUM_VOLTAGE_DEF                             (1.5f)    //(1.5f)
#define VOLTAGE_OFFSET                                  (0.0f)

#define BATTERY_HIGH_VOL                                (2.9)
#define BATTERY_LOW_VOL                                 (2.5)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef struct S_BatAux
{
    int ulSensorVbatMovingAverageCount;
    float fSensorVbatCurrentValue;
} T_BatAux;

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
void Bat_AuxAdc_Init(void);
float Bat_AuxAdc_Convert_To_Percentage(void);
float Bat_AuxAdc_Get(void);

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

#endif /* BAT_MEAS_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* __BAT_AUX_H__ */
