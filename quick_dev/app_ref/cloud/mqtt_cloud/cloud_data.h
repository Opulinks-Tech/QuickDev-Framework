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
*  cloud_data.h
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
#include "opl_err.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __CLOUD_DATA_H__
#define __CLOUD_DATA_H__

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
*   Cloud_DataConstruct
*
* DESCRIPTION:
*   constructing the post data
*
* PARAMETERS
*   pInData :       [IN] input data
*   u32InDataLen :  [IN] input data lens
*   pOutData :      [OUT] output data
*   u32OutDataLen : [OUT] output data lens
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_DataConstruct(uint8_t *pInData, uint32_t u32InDataLen, uint8_t *pOutData, uint32_t *u32OutDataLen);

/*************************************************************************
* FUNCTION:
*   Cloud_DataParser
*
* DESCRIPTION:
*   parsing received data and activate to application
*
* PARAMETERS
*   pInData :       [IN] received data
*   u32InDataLen :  [IN] received data lens
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_DataParser(uint8_t *pInData, uint32_t u32InDataLen);

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

#endif /* __CLOUD_DATA_H__ */
