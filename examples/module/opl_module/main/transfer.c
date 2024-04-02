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
*  transfer.c
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
#include "transfer.h"
#include "hal_uart.h"
#include "cmsis_os.h"
#include "httpclient.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable
static osSemaphoreId g_tUartTxSemaphoreId;

static uint32_t g_u32FileLengthIdx = 0;

// Sec 5: declaration of global function prototype

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

/*************************************************************************
* FUNCTION:
*   _Transfer_SendHeader
*
* DESCRIPTION:
*   transfer header "+" (internal)
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
SHM_DATA void _Transfer_SendHeader(void)
{
    // send the ack tag "+"
    uint8_t u8AckTag = 0x2B;
    Hal_Uart_DataSend(UART_IDX_1, (uint32_t)u8AckTag);
}

/*************************************************************************
* FUNCTION:
*   _Transfer_SendPayload
*
* DESCRIPTION:
*   transfer data payload (internal)
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
SHM_DATA void _Transfer_SendPayload(uint8_t *pu8Data, uint16_t u16Len)
{
    uint16_t u16Count = 0;

    if(0 == u16Len || NULL == pu8Data)
    {
        // no data required to be sent
        return;
    }

    for(; u16Count < u16Len; u16Count ++)
    {
        Hal_Uart_DataSend(UART_IDX_1, (uint32_t)pu8Data[u16Count]);
    }
}

/*************************************************************************
* FUNCTION:
*   _Transfer_SendNextLine
*
* DESCRIPTION:
*   transfer next line mark "\r\n" (internal)
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
SHM_DATA void _Transfer_SendNextLine(void)
{
    // sending the line tag "\r\n"
    uint8_t u8LineTag[2] = {0x0D, 0x0A};
    Hal_Uart_DataSend(UART_IDX_1, (uint32_t)u8LineTag[0]);
    Hal_Uart_DataSend(UART_IDX_1, (uint32_t)u8LineTag[1]);
}

/*************************************************************************
* FUNCTION:
*   _Transfer_SumCal
*
* DESCRIPTION:
*   calculate chech sum (internal)
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
SHM_DATA uint16_t _Transfer_SumCal(uint8_t *pu8Data, uint16_t u16Len)
{
    uint16_t u16Count = 0;
    uint16_t u16SumCal = 0;
    
    for(; u16Count < u16Len; u16Count ++)
    {
        u16SumCal += pu8Data[u16Count];
    }

    return u16SumCal;
}

/*************************************************************************
* FUNCTION:
*   Transfer_File
*
* DESCRIPTION:
*   transfer file
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
SHM_DATA T_OplErr Transfer_File(int method, uint8_t *pu8Data, uint16_t u16Len, uint16_t u16FileTotalLen)
{
    uint16_t u16Sum = 0;

    T_TransferFileFormat tTransferFile = {0};
    
    tTransferFile.reserve = 0;

    switch(method)
    {
        case HTTPCLIENT_POST:
        {
            tTransferFile.ack_hdr.ack_tag = ACK_TAG_FILE_HTTP_POST_RSP;
            break;
        }
        case HTTPCLIENT_GET:
        {
            tTransferFile.ack_hdr.ack_tag = ACK_TAG_FILE_HTTP_GET_RSP;
            break;
        } 
        default:
        {
            return OPL_ERR_PARAM_INVALID;
        }
    }

    if(0 == u16FileTotalLen)
    {
        // error, file lens should not be 0
        return OPL_ERR_PARAM_INVALID;
    }

    if(0 == g_u32FileLengthIdx)
    {
        // begin
        tTransferFile.ack_hdr.ack_status = ACK_STATUS_BEGIN;
        g_u32FileLengthIdx += (uint32_t)u16Len;
    }
    else
    {
        if(u16FileTotalLen > (u16Len + g_u32FileLengthIdx))
        {
            // in progress
            tTransferFile.ack_hdr.ack_status = ACK_STATUS_PROGRESS;
            g_u32FileLengthIdx += (uint32_t)u16Len;
        }
        else if(u16FileTotalLen == (u16Len + g_u32FileLengthIdx))
        {
            // final
            tTransferFile.ack_hdr.ack_status = ACK_STATUS_END;

            // restore FileTypeIdx due to the final page of file has been handled in this scope
            g_u32FileLengthIdx = 0;
        }
        else
        {
            // should not be here, but handle error
            tTransferFile.ack_hdr.ack_status = ACK_STATUS_FAIL;
        }
    }
    
    u16Sum += _Transfer_SumCal(&(tTransferFile.reserve), sizeof(tTransferFile.reserve));
    u16Sum += _Transfer_SumCal((uint8_t *)&u16FileTotalLen, sizeof(u16FileTotalLen));
    u16Sum += _Transfer_SumCal((uint8_t *)&u16Len, sizeof(u16Len));
    u16Sum += _Transfer_SumCal(pu8Data, u16Len);
    
    tTransferFile.file_total_len = u16FileTotalLen;
    tTransferFile.ack_payload.payload_len = u16Len;
    tTransferFile.ack_payload.payload = pu8Data;
    tTransferFile.ack_payload.sum = u16Sum;

    // if(NULL != g_tUartTxSemaphoreId)
    //     osSemaphoreWait(g_tUartTxSemaphoreId, osWaitForever);

    // send ack tag first
    _Transfer_SendHeader();

    // send header (tag, status)
    _Transfer_SendPayload((uint8_t *)&tTransferFile.ack_hdr, sizeof(tTransferFile.ack_hdr));

    // send reserved byte
    _Transfer_SendPayload((uint8_t *)&tTransferFile.reserve, sizeof(tTransferFile.reserve));

    // send file total lens
    _Transfer_SendPayload((uint8_t *)&tTransferFile.file_total_len, sizeof(tTransferFile.file_total_len));

    // send payload len
    _Transfer_SendPayload((uint8_t *)&tTransferFile.ack_payload.payload_len, sizeof(tTransferFile.ack_payload.payload_len));

    // send payload
    _Transfer_SendPayload(tTransferFile.ack_payload.payload, tTransferFile.ack_payload.payload_len);

    // send sum
    _Transfer_SendPayload((uint8_t *)&tTransferFile.ack_payload.sum, sizeof(tTransferFile.ack_payload.sum));

    // send next line
    _Transfer_SendNextLine();

    // if(NULL != g_tUartTxSemaphoreId)
    //     osSemaphoreRelease(g_tUartTxSemaphoreId);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION:
*   Transfer_AckCommand
*
* DESCRIPTION:
*   transfer ack command
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
SHM_DATA void Transfer_AckCommand(T_AckTag tAckTagId, T_AckStatus tAckStatus, uint8_t *pu8Data, uint16_t u16Len)
{
    //printf("Send Ack: Tag = %02x\n", tAckTagId);

    uint16_t u16Sum = 0;

    T_TransferCommandAckFormat tTransferCommand = {0};

    tTransferCommand.ack_hdr.ack_tag = tAckTagId;
    tTransferCommand.ack_hdr.ack_status = tAckStatus;
    
    u16Sum += _Transfer_SumCal((uint8_t *)&u16Len, sizeof(u16Len));
    u16Sum += _Transfer_SumCal(pu8Data, u16Len);

    tTransferCommand.ack_payload.payload_len = u16Len;
    tTransferCommand.ack_payload.payload = pu8Data;
    tTransferCommand.ack_payload.sum = u16Sum;

    if(NULL != g_tUartTxSemaphoreId)
        osSemaphoreWait(g_tUartTxSemaphoreId, osWaitForever);

    // send ack tag first
    _Transfer_SendHeader();

    // send ack header (tag, status)
    _Transfer_SendPayload((uint8_t *)&tTransferCommand.ack_hdr, sizeof(tTransferCommand.ack_hdr));

    // send payload len
    _Transfer_SendPayload((uint8_t *)&tTransferCommand.ack_payload.payload_len, sizeof(tTransferCommand.ack_payload.payload_len));

    // send payload (if have)
    if(NULL != pu8Data && 0 != u16Len)
    {
        _Transfer_SendPayload(tTransferCommand.ack_payload.payload, tTransferCommand.ack_payload.payload_len);
    }

    // send sum
    _Transfer_SendPayload((uint8_t *)&tTransferCommand.ack_payload.sum, sizeof(tTransferCommand.ack_payload.sum));

    // send next line
    _Transfer_SendNextLine();
    
    if(NULL != g_tUartTxSemaphoreId)
        osSemaphoreRelease(g_tUartTxSemaphoreId);
}

/*************************************************************************
* FUNCTION:
*   Transfer_MqttRecivedData
*
* DESCRIPTION:
*   transfer mqtt received data
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
SHM_DATA T_OplErr Transfer_MqttRecivedData(uint8_t *pu8TopicName, uint16_t u16TopicNameLen, uint8_t *pu8Data, uint16_t u16Len)
{
    uint16_t u16Sum = 0;
    T_TransferMqttAckFormat tTransferMqtt = {0};

    if(NULL == pu8TopicName || 0 == u16TopicNameLen)
    {
        // error, topic name must have
        return OPL_ERR_PARAM_INVALID;
    }

    if(NULL == pu8Data || 0 == u16Len)
    {
        // error, payload must have
        return OPL_ERR_PARAM_INVALID;
    }

    tTransferMqtt.ack_hdr.ack_tag = ACK_TAG_CLOUD_MQTT_RECV_DATA_IND;
    tTransferMqtt.ack_hdr.ack_status = ACK_STATUS_OK;

    //printf("Send Ack: Tag = %02x\n", tTransferMqtt.ack_hdr.ack_tag);=

    u16Sum += _Transfer_SumCal((uint8_t *)&u16TopicNameLen, sizeof(u16TopicNameLen));
    u16Sum += _Transfer_SumCal(pu8TopicName, u16TopicNameLen);
    u16Sum += _Transfer_SumCal((uint8_t *)&u16Len, sizeof(u16Len));
    u16Sum += _Transfer_SumCal(pu8Data, u16Len);

    tTransferMqtt.mqtt_payload.topic_name_len = u16TopicNameLen;
    tTransferMqtt.mqtt_payload.topic_name = pu8TopicName;
    tTransferMqtt.mqtt_payload.ack_payload.payload_len = u16Len;
    tTransferMqtt.mqtt_payload.ack_payload.payload = pu8Data;
    tTransferMqtt.mqtt_payload.ack_payload.sum = u16Sum;
    
    if(NULL != g_tUartTxSemaphoreId)
        osSemaphoreWait(g_tUartTxSemaphoreId, osWaitForever);

    // send ack tag first
    _Transfer_SendHeader();

    // send ack header (tag, status)
    _Transfer_SendPayload((uint8_t *)&tTransferMqtt.ack_hdr, sizeof(tTransferMqtt.ack_hdr));

    // send topic name len
    _Transfer_SendPayload((uint8_t *)&tTransferMqtt.mqtt_payload.topic_name_len, sizeof(tTransferMqtt.mqtt_payload.topic_name_len));

    // send topic name
    _Transfer_SendPayload(tTransferMqtt.mqtt_payload.topic_name, tTransferMqtt.mqtt_payload.topic_name_len);

    // send payload len
    _Transfer_SendPayload((uint8_t *)&tTransferMqtt.mqtt_payload.ack_payload.payload_len, sizeof(tTransferMqtt.mqtt_payload.ack_payload.payload_len));

    // send payload
    _Transfer_SendPayload(tTransferMqtt.mqtt_payload.ack_payload.payload, tTransferMqtt.mqtt_payload.ack_payload.payload_len);

    // send sum
    _Transfer_SendPayload((uint8_t *)&tTransferMqtt.mqtt_payload.ack_payload.sum, sizeof(tTransferMqtt.mqtt_payload.ack_payload.sum));

    // send next line
    _Transfer_SendNextLine();

    if(NULL != g_tUartTxSemaphoreId)
        osSemaphoreRelease(g_tUartTxSemaphoreId);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION:
*   Transfer_ScanList
*
* DESCRIPTION:
*   transfer scan list
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
SHM_DATA T_OplErr Transfer_ScanList(T_AckTag tAckTagId, uint8_t *pu8Bssid, uint8_t *pu8Ssid, uint8_t u8SsidLen, uint8_t u8Channel, int8_t i8Rssi, uint8_t u8AuthMode)
{
    uint16_t u16Sum = 0;
    T_AckHdr tAckHdr = {0};

    if(NULL == pu8Bssid)
    {
        // error, bssid must have
        return OPL_ERR_PARAM_INVALID;
    }

    tAckHdr.ack_tag = tAckTagId;
    tAckHdr.ack_status = ACK_STATUS_OK;

    u16Sum += _Transfer_SumCal(pu8Bssid, 6);
    u16Sum += _Transfer_SumCal(pu8Ssid, u8SsidLen);
    u16Sum += _Transfer_SumCal(&u8SsidLen, sizeof(u8SsidLen));
    u16Sum += _Transfer_SumCal(&u8Channel, sizeof(u8Channel));
    u16Sum += _Transfer_SumCal((uint8_t *)&i8Rssi, sizeof(i8Rssi));
    u16Sum += _Transfer_SumCal(&u8AuthMode, sizeof(u8AuthMode));

    if(NULL != g_tUartTxSemaphoreId)
        osSemaphoreWait(g_tUartTxSemaphoreId, osWaitForever);

    // send ack tag first
    _Transfer_SendHeader();

    // send ack header (tag, status)
    _Transfer_SendPayload((uint8_t *)&tAckHdr, sizeof(tAckHdr));

    // send bssid
    _Transfer_SendPayload(pu8Bssid, 6);

    // send ssid len
    _Transfer_SendPayload(&u8SsidLen, sizeof(u8SsidLen));

    // send ssid
    _Transfer_SendPayload(pu8Ssid, u8SsidLen);

    // send channel
    _Transfer_SendPayload(&u8Channel, sizeof(u8Channel));

    // send rssi
    _Transfer_SendPayload((uint8_t *)&i8Rssi, sizeof(i8Rssi));

    // send auth mode
    _Transfer_SendPayload(&u8AuthMode, sizeof(u8AuthMode));

    // send sum
    _Transfer_SendPayload((uint8_t *)&u16Sum, sizeof(u16Sum));

    // send next line
    _Transfer_SendNextLine();

    if(NULL != g_tUartTxSemaphoreId)
        osSemaphoreRelease(g_tUartTxSemaphoreId);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION:
*   Transfer_ScanNum
*
* DESCRIPTION:
*   transfer scan ap number
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
SHM_DATA T_OplErr Transfer_ScanNum(T_AckTag tAckTagId, uint8_t ApNum)
{
    uint16_t u16Sum = 0;
    T_AckHdr tAckHdr = {0};

    if(ApNum <= 0)
    {
        // error, AP Number must bigger than 0
        return OPL_ERR_PARAM_INVALID;
    }

    tAckHdr.ack_tag = tAckTagId;
    tAckHdr.ack_status = ACK_STATUS_OK;

    u16Sum += _Transfer_SumCal((uint8_t *)&ApNum, sizeof(ApNum));

    if(NULL != g_tUartTxSemaphoreId)
        osSemaphoreWait(g_tUartTxSemaphoreId, osWaitForever);

    // send ack tag first
    _Transfer_SendHeader();

    // send ack header (tag, status)
    _Transfer_SendPayload((uint8_t *)&tAckHdr, sizeof(tAckHdr));

    // send AP number
    _Transfer_SendPayload((uint8_t *)&ApNum, sizeof(ApNum));

    // send sum
    _Transfer_SendPayload((uint8_t *)&u16Sum, sizeof(u16Sum));

     // send next line
    _Transfer_SendNextLine();

    if(NULL != g_tUartTxSemaphoreId)
        osSemaphoreRelease(g_tUartTxSemaphoreId);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION:
*   Transfer_Init
*
* DESCRIPTION:
*   initialize transfer module
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Transfer_Init(void)
{
    osSemaphoreDef_t tSemaphoreDef;

    // create the UART TX semaphore
    tSemaphoreDef.dummy = 0;                            // reserved, it is no used
    g_tUartTxSemaphoreId = osSemaphoreCreate(&tSemaphoreDef, 1);
    if (g_tUartTxSemaphoreId == NULL)
    {
        OPL_LOG_ERRO(UART, "Create UART TX semaphore fail\r\n");
    }
}
