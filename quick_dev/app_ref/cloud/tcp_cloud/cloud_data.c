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
*  cloud_data.c
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

#include "cloud_data.h"
#include "cloud_ctrl.h"
#include "cloud_kernel.h"
#include "infra_cjson/infra_cjson.h"

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

uint8_t u8PostTemp[TCP_TX_BUF_SIZE] = {0};
uint8_t u8RecvTemp[TCP_RX_BUF_SIZE] = {0};

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

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
void Cloud_DataConstruct(uint8_t *pInData, uint32_t u32InDataLen, uint8_t *pOutData, uint32_t *u32OutDataLen)
{
    // user implement
    // 1. construct post data
    char cDataBuf[TCP_TX_BUF_SIZE] = {0};
    
    memcpy(cDataBuf, pInData, u32InDataLen);

    memset(u8PostTemp, 0, sizeof(u8PostTemp));
    memcpy(u8PostTemp, pInData, u32InDataLen);

    *u32OutDataLen += sprintf((char *)pOutData, "{\"P\":\"%s\"}", cDataBuf);
}

/*************************************************************************
* FUNCTION:
*   Cloud_AckDataConstruct
*
* DESCRIPTION:
*   constructing the ack data
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
void Cloud_AckDataConstruct(uint8_t *pInData, uint32_t u32InDataLen, uint8_t *pOutData, uint32_t *u32OutDataLen)
{
    // user implement
    // 1. construct post data
    char cDataBuf[TCP_TX_BUF_SIZE] = {0};
    
    memcpy(cDataBuf, pInData, u32InDataLen);

    *u32OutDataLen += sprintf((char *)pOutData, "{\"A\":\"%s\"}", cDataBuf);
}

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
void Cloud_DataParser(uint8_t *pInData, uint32_t u32InDataLen)
{
    // user implement
    // 1. handle data parser

    lite_cjson_t tJson = {0};
    lite_cjson_t tRecv = {0};
    lite_cjson_t tPost = {0};

    if(lite_cjson_parse((char*)pInData, u32InDataLen, &tJson))
    {
        OPL_LOG_WARN(CLOUD, "json parse error");
        return;
    }

    if(!lite_cjson_object_item(&tJson, "R", 1, &tRecv))
    {
        if(0 != strstr(tRecv.value, (char *)u8PostTemp))
        {
            // Cloud_TimerStop(CLOUD_TMR_DATA_POST);
        }
    }
    else if(!lite_cjson_object_item(&tJson, "P", 1, &tPost))
    {
        Cloud_MsgSend(CLOUD_EVT_TYPE_ACK, (uint8_t *)tPost.value, tPost.value_length);
    }

    // 2. send message to app
}
