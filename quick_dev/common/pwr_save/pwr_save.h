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
*  file_temp.h
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

#include "hal_vic.h"
#include "hal_pin.h"
#include "hal_pin_def.h"
#include "ps_public.h"
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __FILE_TEMP_H__
#define __FILE_TEMP_H__

#ifdef __cplusplus
extern "C" {
#endif

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
void PS_EnterDeepSleep(E_GpioIdx_t tGpioIdx, E_ItrType_t tItrType);

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
void PS_EnterTimerSleep(E_GpioIdx_t tGpioIdx, E_ItrType_t tItrType, T_Gpio_CallBack ptGpioCb, uint32_t u32SleepDuration, PS_WAKEUP_CALLBACK ptWakeupCb);

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   Enter smart sleep with duration
*
* PARAMETERS
*   1. u32EntryTime [In] : timeout time to enter smart sleep (0 = enter directly)
*
* RETURNS
*   none
*
*************************************************************************/
void PS_EnterSmartSleep(uint32_t u32EntryTime);

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   Enter smart sleep with duration
*
* PARAMETERS
*   1. u32EntryTime [In] : timeout time to enter smart sleep (0 enter directly)
*
* RETURNS
*   none
*
*************************************************************************/
void PS_ExitSmartSleep(void);

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

#endif /* __FILE_TEMP_H__ */
