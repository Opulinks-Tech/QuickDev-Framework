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

#include <time.h>

#include "cloud_kernel.h"
#include "log.h"
#include "opl_err.h"

#if (CLOUD_TX_DATA_BACKUP_ENABLED == 1)
#include "ring_buffer.h"
#endif /* CLOUD_TX_DATA_BACKUP_ENABLED */

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __CLOUD_DATA_H__
#define __CLOUD_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

#define BUFFER_SIZE                                     (768)

#define POST_DATA_MACADDRESS_FORMAT                     "\"mac\":\"%02x%02x%02x%02x%02x%02x\""
#define OTA_DATA_URL                                    "%s?deviceid=%s&ts=%u&sign=%s"
#define CKS_FW_VERSION_FORMAT                           "%d.%d.%03d"

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

#if defined(DOOR_SENSOR)
enum _coollink_ws_result_t {
    COOLLINK_WS_RES_OK = 0,
    COOLLINK_WS_RES_ERR = -1
};

typedef enum _coollink_ws_result_t coollink_ws_result_t;

typedef coollink_ws_result_t(*coollink_ws_command_callback_t)(uint8_t *szOutBuf, uint16_t out_length, uint8_t u8IsNeedToPost);

struct _coollink_ws_command_t {
    const char *pattern;
    coollink_ws_command_callback_t callback;
};

typedef struct _coollink_ws_command_t coollink_ws_command_t;
#endif

#if defined(MAGIC_LED)
extern uint64_t g_u64WaitSeqId;
#endif
extern uint64_t g_u64TmpSeqId;
extern uint8_t g_u8SeqIdOffset;

// typedef struct
// {
//     uint8_t u8aUrl[128];
//     uint8_t u8aSendBuf[1024];
//     uint16_t u16SendBufLen;
// } xIotDataPostInfo_t;

// typedef struct {
//   time_t TimeStamp;
//   uint8_t DoorStatus;
//   uint8_t ContentType;
//   uint8_t ubaMacAddr[6];
//   char Battery[8];
//   int rssi;
//   char FwVersion[24];
// } HttpPostData_t;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

#if defined(DOOR_SENSOR)
coollink_ws_result_t Coollink_ws_process_update(uint8_t *szOutBuf, uint16_t out_length, uint8_t u8IsNeedToPost);
coollink_ws_result_t Coollink_ws_process_upgrade(uint8_t *szOutBuf, uint16_t out_length, uint8_t u8IsNeedToPost);
coollink_ws_result_t Coollink_ws_process_error(uint8_t *szOutBuf, uint16_t out_length, uint8_t u8IsNeedToPost);

static const coollink_ws_command_t coollink_ws_action_commands[] = {

    { .pattern = "update",      .callback = Coollink_ws_process_update},
    { .pattern = "upgrade",     .callback = Coollink_ws_process_upgrade},
};
#endif

// Sec 5: declaration of global function prototype

/*************************************************************************
* FUNCTION:
*   UpdateBatteryContent
*
* DESCRIPTION:
*   update battery content to global variable for data post
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void UpdateBatteryContent(void);

/*************************************************************************
* FUNCTION:
*   Cloud_KeepAliveHandle
*
* DESCRIPTION:
*   process keep alive
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Cloud_KeepAliveHandle(void);

/*************************************************************************
* FUNCTION:
*   Cloud_SendCloudRspHandle
*
* DESCRIPTION:
*   send response data to cloud handler
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
#if defined(MAGIC_LED)
void Cloud_SendCloudRspHandle(uint8_t u8CtrlSource);
#else
void Cloud_SendCloudRspHandle(void);
#endif

/*************************************************************************
* FUNCTION:
*   Cloud_DataConstruct
*
* DESCRIPTION:
*   construct post data (weak type)
*
* PARAMETERS
*   pInData :       [IN] input data
*   u32InDataLen :  [IN] input data len
*   pOutData :      [OUT] constructed data
*   pu32OutDataLen :[OUT] constructed data lens
*
* RETURNS
*   none
*
*************************************************************************/
#if defined(DOOR_SENSOR)
void Cloud_DataConstruct(uint8_t *pInData, uint32_t u32InDataLen, uint8_t *pOutData, uint32_t *pu32OutDataLen);
#endif

/*************************************************************************
* FUNCTION:
*   Cloud_ConstructPostDataAndSend
*
* DESCRIPTION:
*   constructing the post data
*
* PARAMETERS
*   ptProperity :   [IN] data pop from ring buffer
*
* RETURNS
*   int8_t :        [OUT] cloud send result
*
*************************************************************************/
int8_t Cloud_ConstructPostDataAndSend(T_RingBufData *ptProperity);

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
#if defined(DOOR_SENSOR)
int8_t Cloud_DataParser(uint8_t *pInData, uint16_t u32InDataLen);
#endif

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
