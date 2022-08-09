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
*  bat_aux.c
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

#include "bat_aux.h"
#include "boot_sequence.h"
#include "cmsis_os.h"
#include "hal_auxadc.h"
#include "hal_vic.h"
#include "hal_pin.h"
#include "hal_pin_def.h"
#include "mw_fim_default_group12_project.h"
#include "mw_fim_default_group16_project.h"
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (BAT_MEAS_ENABLED == 1)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

extern uint32_t g_ulHalAux_AverageCount;

// Sec 5: declaration of global function prototype

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

T_BatAux BatteryVoltage;
float g_fVoltageOffset = 0;

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

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
float Bat_AuxAdc_Convert_To_Percentage(void)
{
    float fVBat;
    float fAverageVBat;

#if defined(OPL1000_A2)
    Hal_Pin_ConfigSet(BAT_MEAS_IO_PORT, PIN_TYPE_GPIO_OUTPUT_HIGH, PIN_DRIVING_FLOAT);
    osDelay(3);
    Hal_Aux_IoVoltageGet(BAT_MEAS_IO_PORT, &fVBat);
    Hal_Pin_ConfigSet(BAT_MEAS_IO_PORT, PIN_TYPE_NONE, PIN_DRIVING_FLOAT);
#elif defined(OPL1000_A3) || defined(OPL2500_A0)
    Hal_Aux_AdcMiniVolt_Get(HAL_AUX_SRC_VBAT, 0, &fVBat);
#endif

    if (fVBat > MAXIMUM_VOLTAGE_DEF)
        fVBat = MAXIMUM_VOLTAGE_DEF;
    if (fVBat < MINIMUM_VOLTAGE_DEF)
        fVBat = MINIMUM_VOLTAGE_DEF;

    // error handle: if the average count = 0
    if (BAT_IO_MOVING_AVERAGE_COUNT == 0)
    {
        fAverageVBat = fVBat;
    }
    // compute the moving average value
    else
    {
        // update the new average count
        if (BatteryVoltage.ulSensorVbatMovingAverageCount < BAT_IO_MOVING_AVERAGE_COUNT)
            BatteryVoltage.ulSensorVbatMovingAverageCount++;

        // compute the new moving average value
        fAverageVBat = BatteryVoltage.fSensorVbatCurrentValue * (BatteryVoltage.ulSensorVbatMovingAverageCount - 1);
        fAverageVBat = (fAverageVBat + fVBat) / BatteryVoltage.ulSensorVbatMovingAverageCount;
    }

    // the value is updated when the new value is the lower one.
    if (fAverageVBat < BatteryVoltage.fSensorVbatCurrentValue)
        BatteryVoltage.fSensorVbatCurrentValue = fAverageVBat;

    return ((BatteryVoltage.fSensorVbatCurrentValue - MINIMUM_VOLTAGE_DEF)/(MAXIMUM_VOLTAGE_DEF - MINIMUM_VOLTAGE_DEF))*100;
}

float Bat_AuxAdc_Get(void)
{
    float fVBat;
    float fAverageVBat;

    g_ulHalAux_AverageCount = BAT_IO_VOLTAGE_GET_AVERAGE_COUNT;

#if defined(OPL1000_A2)
    Hal_Pin_ConfigSet(BAT_MEAS_IO_PORT, PIN_TYPE_GPIO_OUTPUT_HIGH, PIN_DRIVING_FLOAT);
    osDelay(3);
    Hal_Aux_IoVoltageGet(BAT_MEAS_IO_PORT, &fVBat);
    Hal_Pin_ConfigSet(BAT_MEAS_IO_PORT, PIN_TYPE_NONE, PIN_DRIVING_FLOAT);
#elif defined(OPL1000_A3) || defined(OPL2500_A0)
    Hal_Aux_AdcMiniVolt_Get(HAL_AUX_SRC_VBAT, 0, &fVBat);
#endif

    fVBat -= g_fVoltageOffset;

    fAverageVBat = fVBat;

    // the value is updated when the new value is the lower one.
    BatteryVoltage.fSensorVbatCurrentValue = fAverageVBat;

    return (BatteryVoltage.fSensorVbatCurrentValue);
}

void Bat_AuxAdc_Init(void)
{
    // cold boot
    if (0 == Boot_CheckWarmBoot())
    {
        BatteryVoltage.ulSensorVbatMovingAverageCount = 0;
        BatteryVoltage.fSensorVbatCurrentValue = MAXIMUM_VOLTAGE_DEF;
    }

    if (MW_FIM_OK != MwFim_FileRead(MW_FIM_IDX_GP16_PROJECT_VOLTAGE_OFFSET, 0, MW_FIM_GP16_VOLTAGE_OFFSET_SIZE, (uint8_t *)&g_fVoltageOffset))
    {
        // if fail, return fail
        BAT_MEAS_LOG_ERRO("Read fim fail\r\n");
        g_fVoltageOffset = VOLTAGE_OFFSET;
    }

    // Calibration data
    g_ulHalAux_AverageCount = BAT_IO_VOLTAGE_GET_AVERAGE_COUNT;

#if defined(OPL1000_A3) || defined(OPL2500_A0)
    Hal_Aux_AdcCal_Init();
#endif
}

#endif /* BAT_MEAS_ENABLED */
