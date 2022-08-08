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
*  time_stmp.h
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

/*
tm Data Structure
+----------+------+---------------------------+-------+
|  Member  | Type |          Meaning          | Range |
+----------+------+---------------------------+-------+
| tm_sec   | int  | seconds after the minute  | 0-61* |
| tm_min   | int  | minutes after the hour    | 0-59  |
| tm_hour  | int  | hours since midnight      | 0-23  |
| tm_mday  | int  | day of the month          | 1-31  |
| tm_mon   | int  | months since January      | 1-12  |
| tm_year  | int  | years since 1900          |       |
| tm_wday  | int  | days since Sunday         | 0-6   |
| tm_yday  | int  | days since January 1      | 0-365 |
| tm_isdst | int  | Daylight Saving Time flag |       |
+----------+------+---------------------------+-------+
*/

// Sec 1: Include File

#include <time.h>
#include "log.h"
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __TIME_STMP_H__
#define __TIME_STMP_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (TIME_STMP_ENABLED == 1)

// module log
#if (TIME_STMP_LOG_ENABLED == 1)
#define TIME_STMP_LOG_DEBG(...)                         OPL_LOG_DEBG(TS, __VA_ARGS__)
#define TIME_STMP_LOG_INFO(...)                         OPL_LOG_INFO(TS, __VA_ARGS__)
#define TIME_STMP_LOG_WARN(...)                         OPL_LOG_WARN(TS, __VA_ARGS__)
#define TIME_STMP_LOG_ERRO(...)                         OPL_LOG_ERRO(TS, __VA_ARGS__)
#else
#define TIME_STMP_LOG_DEBG(...)
#define TIME_STMP_LOG_INFO(...)
#define TIME_STMP_LOG_WARN(...)
#define TIME_STMP_LOG_ERRO(...)
#endif

#ifndef TIME_STMP_INIT_SEC
#define TIME_STMP_INIT_SEC                              (0)
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
void TSTMP_TimestampSync(uint32_t u32Timestamp);

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
void TSTMP_DateTimeGet(struct tm *pSystemTime, float fTimeZone);

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
time_t TSTMP_TimestampGet(void);

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
void TSTMP_Init(void);

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

#endif /* TIME_STMP_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* __TIME_STMP_H__ */
