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
*  cloud_ota_http.h
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

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __CLOUD_OTA_HTTP_H__
#define __CLOUD_OTA_HTTP_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CLOUD_OTA_TASK_STACK_SIZE                       (1024)
#define CLOUD_OTA_TASK_PRIORITY                         (osPriorityNormal)
#define CLOUD_OTA_MSG_QUEUE_SIZE                        (20)

#define CLOUD_OTA_HTTP_URL_LEN                          (256)
#define CLOUD_OTA_HTTP_DATA_LEN                         (1024 + 1)

#define CLOUD_OTA_HTTP_CONNECTION_RETRY_COUNT           (3)
#define CLOUD_OTA_HTTP_CONNECTION_TIMEOUT               (5000)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

// ota event list
typedef enum E_CloudOtaEvt
{
    CLOUD_OTA_EVT_TYPE_DOWNLOAD         = 0x0000,
    CLOUD_OTA_EVT_TYPE_GET_VERSION,
} T_CloudOtaEvt;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/*************************************************************************
* FUNCTION:
*   Cloud_OtaTriggerReq
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
T_OplErr Cloud_OtaTriggerReq(uint32_t u32EventId, uint8_t *pu8Data, uint32_t u32DataLen);

/*************************************************************************
* FUNCTION:
*   Cloud_OtaTaskInit
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
void Cloud_OtaTaskInit(void);

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

#endif /* __CLOUD_OTA_HTTP_H__ */
