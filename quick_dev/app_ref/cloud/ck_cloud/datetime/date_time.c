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
*  date_time.c
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
#include "date_time.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

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

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

/*************************************************************************
* FUNCTION:
*   ConvertDateToUnixTime
*
* DESCRIPTION:
*   convert date to unix timestamp
*
* PARAMETERS
*   date :          [IN] pointer to structure represeting the date and time
*
* RETURNS
*   time_t :        [OUT] unix timestamp
*
*************************************************************************/
time_t ConvertDateToUnixTime(const DateTime *date)
{
    uint16_t y;
    uint16_t m;
    uint16_t d;
    uint32_t t;

    //Year
    y = date->year;
    //Month of year
    m = date->month;
    //Day of month
    d = date->day;

    //January and February are counted as months 13 and 14 of the previous year
    if(m <= 2)
    {
       m += 12;
       y -= 1;
    }

    //Convert years to days
    t = (365 * y) + (y / 4) - (y / 100) + (y / 400);
    //Convert months to days
    t += (30 * m) + (3 * (m + 1) / 5) + d;
    //Unix time starts on January 1st, 1970
    t -= 719561;
    //Convert days to seconds
    t *= 86400;
    //Add hours, minutes and seconds
    t += (3600 * date->hours) + (60 * date->minutes) + date->seconds;

    OPL_LOG_DEBG(DATE, "tday = %u", t);

    //Return Unix time
    return t;
}
