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
*  time_stmp.c
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

#include "time_stmp.h"
#include "cmsis_os.h"
#include "opl_err.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (TIME_STMP_ENABLED == 1)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

uint32_t g_ulTstmpSystemSecondInit;      // System Clock Time Initialize
uint32_t g_ulTstmpTimestampInit;         // GMT Time Initialize

uint32_t TSTMP_CurrentSysTimeGet(void);

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
uint32_t TSTMP_CurrentSysTimeGet(void)
{
    uint32_t ulTick;
    int32_t  dwOverflow;
    uint32_t ulsecond;

    uint32_t ulSecPerTickOverflow;
    uint32_t ulSecModTickOverflow;
    uint32_t ulMsModTickOverflow;

    uint32_t ulSecPerTick;
    uint32_t ulSecModTick;

    osKernelSysTickEx(&ulTick, &dwOverflow);

    // the total time of TickOverflow is 4294967295 + 1 ms
    ulSecPerTickOverflow = (0xFFFFFFFF / osKernelSysTickFrequency) * dwOverflow;
    ulSecModTickOverflow = (((0xFFFFFFFF % osKernelSysTickFrequency) + 1) * dwOverflow) / osKernelSysTickFrequency;
    ulMsModTickOverflow = (((0xFFFFFFFF % osKernelSysTickFrequency) + 1) * dwOverflow) % osKernelSysTickFrequency;

    ulSecPerTick = (ulTick / osKernelSysTickFrequency);
    ulSecModTick = (ulTick % osKernelSysTickFrequency + ulMsModTickOverflow) / osKernelSysTickFrequency;

    ulsecond = (ulSecPerTickOverflow + ulSecModTickOverflow) + (ulSecPerTick + ulSecModTick);

    return ulsecond;
}

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
void TSTMP_TimestampSync(uint32_t u32Timestamp)
{
    g_ulTstmpSystemSecondInit = TSTMP_CurrentSysTimeGet();
    g_ulTstmpTimestampInit = u32Timestamp;
    return;
}

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
void TSTMP_DateTimeGet(struct tm *pSystemTime, float fTimeZone)
{
    time_t rawtime;
    struct tm * timeinfo;

    rawtime = TSTMP_TimestampGet() + 3600 * fTimeZone;
    timeinfo = localtime(&rawtime);
    memcpy(pSystemTime, timeinfo, sizeof(struct tm));
    pSystemTime->tm_year = pSystemTime->tm_year + 1900;
    pSystemTime->tm_mon = pSystemTime->tm_mon + 1;

    TIME_STMP_LOG_DEBG("Current time: %d-%d-%d %d:%d:%d\n", pSystemTime->tm_year, pSystemTime->tm_mon, pSystemTime->tm_mday, pSystemTime->tm_hour, pSystemTime->tm_min, pSystemTime->tm_sec);
}

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
time_t TSTMP_TimestampGet(void)
{
    time_t rawtime;
    uint32_t ulTmpSystemSecond = 0;
    uint32_t ulDeltaSystemSecond = 0;

    ulTmpSystemSecond = TSTMP_CurrentSysTimeGet();
    ulDeltaSystemSecond =  (ulTmpSystemSecond - g_ulTstmpSystemSecondInit);
    rawtime = g_ulTstmpTimestampInit + ulDeltaSystemSecond;

    return rawtime;
}

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
void TSTMP_Init(void)
{
    TSTMP_TimestampSync(TIME_STMP_INIT_SEC);
}

#endif /* TIME_STMP_ENABLED */
