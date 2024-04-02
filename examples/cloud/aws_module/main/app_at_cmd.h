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

// fot AT_CmdFimWriteHandler
#define AT_FIM_DATA_LENGTH                              (2)                       /* EX: 2 = FF */
#define AT_FIM_DATA_LENGTH_WITH_COMMA                   (AT_FIM_DATA_LENGTH + 1)  /* EX: 3 = FF, */
#define AT_BLE_ADV_DATA_LENGTH_MAX                      (200)     

#define AT_OK(ack_tag, payload, len)             Transfer_AckCommand(ack_tag, ACK_STATUS_OK, payload, len);
#define AT_FAIL(ack_tag, payload, len)           Transfer_AckCommand(ack_tag, ACK_STATUS_FAIL, payload, len);
#define AT_RETURN(ret)                           return ret;
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

int AT_CmdFimWriteHandler(char *buf, int len, int mode);
int AT_CmdFimReadHandler(char *buf, int len, int mode);

// * range for BLE commands
int AT_CmdBleStartHandler(char *buf, int len, int mode);
int AT_CmdBleStoptHandler(char *buf, int len, int mode);
//int AT_CmdBleAdvDataHandler(char *buf, int len, int mode);
int AT_CmdBleAppearanceHandler(char *buf, int len, int mode);
int AT_CmdBleFwRevHandler(char *buf, int len, int mode);
int AT_CmdBleModelNbHandler(char *buf, int len, int mode);
int AT_CmdBleDevMacHandler(char *buf, int len, int mode);
int AT_CmdBleNotifyHandler(char *buf, int len, int mode);
// *

// * range for WIFI commands
int AT_CmdWifiScanHandler(char *buf, int len, int mode);
int AT_CmdWifiConnectHandler(char *buf, int len, int mode);
int AT_CmdWifiDisconnectHandler(char *buf, int len, int mode);
int AT_CmdWifiApInfoHandler(char *buf, int len, int mode);
int AT_CmdWifiDevMacHandler(char *buf, int len, int mode);
int AT_CmdWifiResetHandler(char *buf, int len, int mode);
// *

// * range for Cloud commands
int AT_CmdMqttInitHandler(char *buf, int len, int mode);
int AT_CmdMqttConnectHandler(char *buf, int len, int mode);
int AT_CmdMqttDisconnectHandler(char *buf, int len, int mode);
int AT_CmdMqttKeepAliveIntvlHandler(char *buf, int len, int mode);
int AT_CmdMqttSubTopicHandler(char *buf, int len, int mode);
int AT_CmdMqttUnsubTopicHandler(char *buf, int len, int mode);
int AT_CmdMqttPubDataHandler(char *buf, int len, int mode);
int AT_CmdMqttClientIdHandler(char *buf, int len, int mode);
int AT_CmdMqttLastWillHandler(char *buf, int len, int mode);
// *

//* range for File Commands
int AT_CmdHttpPostHandler(char *buf, int len, int mode);
int AT_CmdHttpGetHandler(char *buf, int len, int mode);
int AT_CmdFulMqttCertHandler(char *buf, int len, int mode);
//*

// * range for System or else commands
int AT_CmdSleepHandler(char *buf, int len, int mode);
//int AT_CmdModuleOtaHandler(char *buf, int len, int mode);
// *

// * others
int AT_CmdAppFwVer(char *buf, int len, int mode);
int AT_CmdHostReadyHandler(char *buf, int len, int mode);

/***********
C Functions
***********/
// Sec 8: C Functions

#ifdef __cplusplus
}
#endif

#endif /* __APP_AT_CMD_H__ */
