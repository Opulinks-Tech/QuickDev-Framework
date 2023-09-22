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
*  nm_mngr_at.h
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

#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __NM_MNGR_AT_H__
#define __NM_MNGR_AT_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (NM_ENABLED == 1)

#define NM_AT_TEST_SSID                                 "hatest"
#define NM_AT_TEST_PWD                                  "12345678"

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
*   AT_CmdNmScanHandler
*
* DESCRIPTION:
*   at cmd for network manager scan request
*
* PARAMETERS
*   buf :           [IN] at cmd input
*   len :           [IN] at cmd input lens
*   mode :          [IN] at cmd type
*
* RETURNS
*   int :           handler result
*
*************************************************************************/
int AT_CmdNmScanHandler(char *buf, int len, int mode);

/*************************************************************************
* FUNCTION:
*   AT_CmdNmConnectHandler
*
* DESCRIPTION:
*   at cmd for network manager connect request
*
* PARAMETERS
*   buf :           [IN] at cmd input
*   len :           [IN] at cmd input lens
*   mode :          [IN] at cmd type
*
* RETURNS
*   int :           handler result
*
*************************************************************************/
int AT_CmdNmConnectHandler(char *buf, int len, int mode);

/*************************************************************************
* FUNCTION:
*   AT_CmdNmStopHandler
*
* DESCRIPTION:
*   at cmd for network manager connect request
*
* PARAMETERS
*   buf :           [IN] at cmd input
*   len :           [IN] at cmd input lens
*   mode :          [IN] at cmd type
*
* RETURNS
*   int :           handler result
*
*************************************************************************/
int AT_CmdNmStopHandler(char *buf, int len, int mode);

/*************************************************************************
* FUNCTION:
*   AT_CmdNmStopHandler
*
* DESCRIPTION:
*   at cmd for network manager connect request
*
* PARAMETERS
*   buf :           [IN] at cmd input
*   len :           [IN] at cmd input lens
*   mode :          [IN] at cmd type
*
* RETURNS
*   int :           handler result
*
*************************************************************************/
int AT_CmdNmResumeHandler(char *buf, int len, int mode);

/*************************************************************************
* FUNCTION:
*   AT_CmdNmQuickConnectSetHandler
*
* DESCRIPTION:
*   at cmd for network manager connect request
*
* PARAMETERS
*   buf :           [IN] at cmd input
*   len :           [IN] at cmd input lens
*   mode :          [IN] at cmd type
*
* RETURNS
*   int :           handler result
*
*************************************************************************/
int AT_CmdNmQuickConnectSetHandler(char *buf, int len, int mode);

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

#endif /* NM_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* __NM_MNGR_AT_H__ */
