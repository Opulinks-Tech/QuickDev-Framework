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
*  sha_256.h
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
#include "app_main.h"
#include "scrt.h"
#include "time_stmp.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __SHA_256_H__
#define __SHA_256_H__

#ifdef __cplusplus
extern "C" {
#endif

#define SHA256_BUF_SIZE                                 (128)
#define SHA256_FOR_OTA_FORMAT                           "%s%u%s"

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
*   SHA256_Calucate
*
* DESCRIPTION:
*   calculate the value of SHA256 
*
* PARAMETERS
*   SHA256_Output : [OUT] an empty array to store the value of SHA256
*   len :           [IN] input the size value of array
*   uwSHA256_Buff : [IN] sha256 buffer
*
* RETURNS
*   int :           [IN] true -> setting complete
*                        false -> error
*
*************************************************************************/
int SHA256_Calucate(uint8_t SHA256_Output[], int len, unsigned char uwSHA256_Buff[]);

/*************************************************************************
* FUNCTION:
*   SHA256_ValueForOta
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
int SHA256_ValueForOta(time_t TimeStamp, uint8_t ubaSHA256CalcStrBuf[]);

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

#endif /* __SHA_256_H__ */
