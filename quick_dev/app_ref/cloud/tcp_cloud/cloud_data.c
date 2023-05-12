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
#include "qd_config.h"
#include "app_main.h"
#include "wifi_mngr_api.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

extern uint16_t g_u16CloudTcpWaitTcpAckSkipDtimId;

extern uint32_t g_u32PostWaitAck;

// Sec 5: declaration of global function prototype

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

uint8_t u8PostTemp[TCP_TX_BUF_SIZE] = {0};
uint8_t u8RecvTemp[TCP_RX_BUF_SIZE] = {0};

static uint32_t g_u32AckCnt = 0;

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

    //*u32OutDataLen += sprintf((char *)pOutData, "{\"P\":\"%s\"}", cDataBuf);
    *u32OutDataLen += sprintf((char *)pOutData, "%s", cDataBuf);    // post string only
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

    // *u32OutDataLen += sprintf((char *)pOutData, "{\"A\":\"%s\"}", cDataBuf);
    *u32OutDataLen += sprintf((char *)pOutData, "%s %d", cDataBuf, g_u32AckCnt++);
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

    // 2. send message to app

    Opl_Wifi_Skip_Dtim_Set(g_u16CloudTcpWaitTcpAckSkipDtimId, false);
    Cloud_TimerStart(CLOUD_TMR_WAIT_TCP_ACK, TCP_WAIT_TCP_ACK_TIME);

#if 0
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
            // Cloud_TimerStop(CLOUD_TMR_WAIT_ACK_DATA);
        }
    }
    else if(!lite_cjson_object_item(&tJson, "P", 1, &tPost))
    {
        Cloud_MsgSend(CLOUD_EVT_TYPE_ACK, (uint8_t *)tPost.value, tPost.value_length);
    }
#else   // For post data set

    if((0 != strlen((char *)u8PostTemp)) && (0 == strncmp((char *)u8PostTemp, (const char *)pInData, strlen((char *)u8PostTemp))))
    {
        if(g_u32PostWaitAck)
        {
            OPL_LOG_INFO(CLOUD, "Got ack data from post");

            // call got ack handler to restore the DTIM, stop timer, and post flag
            Cloud_GotAckHandler();
        }
    }
#if (TCP_RX_DATA_ECHO_ENABLE == 1)
    else if(0 == strncmp(TCP_RX_SPECIFIED_DATA, (const char *)pInData, strlen(TCP_RX_SPECIFIED_DATA)))
    {
        OPL_LOG_INFO(CLOUD, "Recv spec data from server, send echo");

        // call sending ack to reply echo message
        Cloud_MsgSend(CLOUD_EVT_TYPE_ACK, (uint8_t *)pInData, u32InDataLen);
    }
#endif

#if defined(OPL2500_A0)

    const char* pcDelimiter = "  ";
    char *pcArg;

    pcArg = strtok((char *)pInData, pcDelimiter);

    /*while (pcArg != NULL) 
    {
        printf("%s\n", pcArg);
        pcArg = strtok(NULL, pcDelimiter);           
    }*/

    if(0 == strncmp("pd", (const char *)pcArg, strlen(pcArg)))
    {
        int32_t i32PostDuration = 0;
        int32_t i32PostTotalCnt = 0;
        int32_t i32PostWaitAck = 0;
        pcArg = strtok(NULL, pcDelimiter);
        i32PostDuration = strtol(pcArg, NULL, 10);
        //printf("Duratin %d\n", i32PostDuration);
        if(i32PostDuration >= 0 && i32PostDuration <= 100)
        {
            if(0 == i32PostDuration)
            {
                i32PostTotalCnt = 1000;
                i32PostWaitAck = 0;
            }
            else
            {
                pcArg = strtok(NULL, pcDelimiter);
                i32PostTotalCnt = strtol(pcArg, NULL, 10);

                pcArg = strtok(NULL, pcDelimiter);
                i32PostWaitAck = strtol(pcArg, NULL, 10);
            }

            //printf("Post total count %d\n", i32PostTotalCnt);

            if(i32PostTotalCnt>=10 && i32PostTotalCnt<=1000000)
            {
                if(1 == i32PostWaitAck || 0 == i32PostWaitAck)
                {
                    T_PostSetMsg tPostSetMsg;

                    memset(&tPostSetMsg, 0, sizeof(T_PostSetMsg));

                    tPostSetMsg.u32PostDuration = (uint32_t)i32PostDuration;
                    tPostSetMsg.u32PostTotalCnt = (uint32_t)i32PostTotalCnt;
                    tPostSetMsg.u32PostWaitAck = (uint32_t)i32PostWaitAck;

                    APP_SendMessage(APP_EVT_POST_SET,(void*) &tPostSetMsg, sizeof(tPostSetMsg));
                }
                else
                {
                    OPL_LOG_WARN(CLOUD, "PostWaitAck should be 0 or 1");
                }
            }
            else
            {
                OPL_LOG_WARN(CLOUD, "Post total count should between 10~1000000");
            }

        }
        else
        {
            OPL_LOG_WARN(CLOUD, "Post duraiton should between 0~100");
        }
    }

#endif  // #if defined(OPL2500_A0)

#endif  // #if 0

#if (OPL_POWER_SLEEP_CONTROL == 1)
    T_SlpCtrlMsg tSlpCtrlMsg;

    memset(&tSlpCtrlMsg, 0, sizeof(T_SlpCtrlMsg));

    if(0 == strncmp("sson", (const char *)pInData, 4))
    {
        tSlpCtrlMsg.eSlpMode = SLP_MODE_SMART_SLEEP;
        tSlpCtrlMsg.u32SmrtSlpEn = 1;

        APP_SendMessage(APP_EVT_PWR_SLEEP_CONTROL,(void*) &tSlpCtrlMsg, sizeof(tSlpCtrlMsg));
    }
    else
    if(0 == strncmp("ssoff", (const char *)pInData, 5))
    {
        tSlpCtrlMsg.eSlpMode = SLP_MODE_SMART_SLEEP;
        tSlpCtrlMsg.u32SmrtSlpEn = 0;

        APP_SendMessage(APP_EVT_PWR_SLEEP_CONTROL,(void*) &tSlpCtrlMsg, sizeof(tSlpCtrlMsg));
    }
    else
    if(0 == strncmp("ts", (const char *)pInData, 2))
    {
        tSlpCtrlMsg.eSlpMode = SLP_MODE_TIMER_SLEEP;

        APP_SendMessage(APP_EVT_PWR_SLEEP_CONTROL,(void*) &tSlpCtrlMsg, sizeof(tSlpCtrlMsg));
    }
    if(0 == strncmp("ds", (const char *)pInData, 2))
    {
        tSlpCtrlMsg.eSlpMode = SLP_MODE_DEEP_SLEEP;

        APP_SendMessage(APP_EVT_PWR_SLEEP_CONTROL,(void*) &tSlpCtrlMsg, sizeof(tSlpCtrlMsg));
    }
#endif
    
}
