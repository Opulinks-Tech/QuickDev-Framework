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
*  pwr_save.c
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
#include "log.h"
#include "pwr_save.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

osTimerId g_tPsSmartSleepTimer = NULL;

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

void PS_EnterDeepSleep(E_GpioIdx_t tGpioIdx, E_ItrType_t tItrType)
{
    Hal_Pin_ConfigSet(tGpioIdx, PIN_TYPE_GPIO_INPUT, PIN_DRIVING_FLOAT);
    ps_set_wakeup_io(tGpioIdx, 1, tItrType, 0, NULL);

    ps_deep_sleep();

    // system will go deep sleep, and will reboot after io wake up
}

void PS_EnterTimerSleep(E_GpioIdx_t tGpioIdx, E_ItrType_t tItrType, T_Gpio_CallBack ptGpioCb, uint32_t u32SleepDuration, PS_WAKEUP_CALLBACK ptWakeupCb)
{
    Hal_Pin_ConfigSet(tGpioIdx, PIN_TYPE_GPIO_INPUT, PIN_DRIVING_FLOAT);
    ps_set_wakeup_io(tGpioIdx, 1, tItrType, 0, ptGpioCb);
    ps_set_wakeup_cb(ptWakeupCb);

    ps_timer_sleep(u32SleepDuration);

    // system will go timer sleep, and will call IoWakeupCb and TmrWakeupCb while io trigger or timer timeout
}

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
static void PS_SmartSleepTimerTimeoutHandler(void const *argu)
{
    OPL_LOG_INFO(PS, "Enter smart sleep");
    ps_smart_sleep(1);
}

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
void PS_EnterSmartSleep(uint32_t u32EntryTime)
{
    if(0 != u32EntryTime)
    {
        if(NULL == g_tPsSmartSleepTimer)
        {
            osTimerDef_t tTimerDef;

            tTimerDef.ptimer = PS_SmartSleepTimerTimeoutHandler;
            g_tPsSmartSleepTimer = osTimerCreate(&tTimerDef, osTimerOnce, NULL);
            if(g_tPsSmartSleepTimer == NULL)
            {
                OPL_LOG_ERRO(PS, "Create smart sleep timer fail");

                return;
            }
        }

        osTimerStart(g_tPsSmartSleepTimer, u32EntryTime);
    }
    else
    {
        OPL_LOG_INFO(PS, "Enter smart sleep");
        ps_smart_sleep(1);
    }
}

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
void PS_ExitSmartSleep(void)
{
    if(NULL != g_tPsSmartSleepTimer)
    {
        osTimerDelete(g_tPsSmartSleepTimer);

        g_tPsSmartSleepTimer = NULL;
    }

    ps_smart_sleep(0);
}
