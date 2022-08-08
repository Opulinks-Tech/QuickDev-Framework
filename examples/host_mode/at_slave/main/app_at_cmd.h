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
*  app_at_cmd.h
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

#ifndef __APP_AT_CMD_H__
#define __APP_AT_CMD_H__

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
void AT_CmdListAdd(uint8_t u8EnableCrLf);

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

#if 0
int AT_CmdFimWriteHandler(char *buf, int len, int mode);
int AT_CmdFimReadHandler(char *buf, int len, int mode);
#endif
int AT_CmdSysCkCfgHandler(char *buf, int len, int mode);
int AT_CmdOtaTestHandler(char *buf, int len, int mode);

int AT_CmdFwkVerHandler(char *buf, int len, int mode);
int AT_CmdEntSleepHandler(char *buf, int len, int mode);
int AT_CmdProvisionStartHandler(char *buf, int len, int mode);
int AT_CmdProvisionStopHandler(char *buf, int len, int mode);
int AT_CmdBleStatusHandler(char *buf, int len, int mode);
int AT_CmdWifiScanHandler(char *buf, int len, int mode);
int AT_CmdWifiConnectHandler(char *buf, int len, int mode);
int AT_CmdWifiQConnectHandler(char *buf, int len, int mode);
int AT_CmdWifiStatusHandler(char *buf, int len, int mode);
int AT_CmdCloudConnectHandler(char *buf, int len, int mode);
int AT_CmdCloudDisconnectHandler(char *buf, int len, int mode);
int AT_CmdCloudTxTopicHandler(char *buf, int len, int mode);
int AT_CmdCloudRxTopicHandler(char *buf, int len, int mode);
int AT_CmdCloudTxPostHandler(char *buf, int len, int mode);

/***********
C Functions
***********/
// Sec 8: C Functions

#ifdef __cplusplus
}
#endif

#endif /* __APP_AT_CMD_H__ */
