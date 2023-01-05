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
*  http_ota.h
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
#include "httpclient_exp.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __HTTP_OTA_H__
#define __HTTP_OTA_H__

#ifdef __cplusplus
extern "C" {
#endif

// map to structure prototype
#if defined(HTTPCLIENT_EXP)
#define HTTPCLIENT_T                                    httpclient_exp_t
#define HTTPCLIENT_DATA_T                               httpclient_exp_data_t
#else
#define HTTPCLIENT_T                                    httpclient_t
#define HTTPCLIENT_DATA_T                               httpclient_data_t
#endif

#define HTTP_OTA_TASK_STACK_SIZE                       (1024)
#define HTTP_OTA_TASK_PRIORITY                         (osPriorityNormal)
#define HTTP_OTA_MSG_QUEUE_SIZE                        (20)

#define HTTP_OTA_HTTP_URL_LEN                          (256)
#define HTTP_OTA_HTTP_DATA_LEN                         (1024 + 1)

#define HTTP_OTA_HTTP_CONNECTION_RETRY_COUNT           (3)
#define HTTP_OTA_HTTP_CONNECTION_TIMEOUT               (5000)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

// ota event list
typedef enum E_HttpOtaEvt
{
    HTTP_OTA_EVT_TYPE_DOWNLOAD         = 0x0000,
    HTTP_OTA_EVT_TYPE_GET_VERSION,
} T_HttpOtaEvt;

// cloud kernel event message
typedef struct S_HttpOtaMsg
{
    uint32_t u32EventId;
    uint32_t u32DataLen;
    uint8_t u8aData[];
} T_HttpOtaMsg;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/*************************************************************************
* FUNCTION:
*   HTTP_OtaInProgress
*
* DESCRIPTION:
*   check is in ota progress
*
* PARAMETERS
*   none
*
* RETURNS
*   bool :          [OUT] true: in OTA progress
*                         false: in idle
*
*************************************************************************/
bool HTTP_OtaInProgress(void);

/*************************************************************************
* FUNCTION:
*   HTTP_OtaTriggerReq
*
* DESCRIPTION:
*   send OTA request to OTA task
*
* PARAMETERS
*   u32EventId :    [IN] event ID
*   pu8Data :       [IN] message data
*   u32DataLen :    [IN] messgae data lens
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr HTTP_OtaTriggerReq(uint32_t u32EventId, uint8_t *pu8Data, uint32_t u32DataLen);

/*************************************************************************
* FUNCTION:
*   HTTP_OtaTaskInit
*
* DESCRIPTION:
*   OTA task initiate function
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void HTTP_OtaTaskInit(void);

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

#endif /* __HTTP_OTA_H__ */
