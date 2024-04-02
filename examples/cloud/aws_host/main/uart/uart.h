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
*  uart.h
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

#include "hal_uart.h"
#include "opl_err.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __UART_H__
#define __UART_H__

#ifdef __cplusplus
extern "C" {
#endif

// uart configuration
#define UART_CONFIG_IDX                 (UART_IDX_0)
#define UART_CONFIG_BAUDRATE            (115200)
#define UART_CONFIG_DATA_BITS           (DATA_BIT_8)
#define UART_CONFIG_PARITY              (PARITY_NONE)
#define UART_CONFIG_STOP_BITS           (STOP_BIT_1)
#define UART_CONFIG_FLOW_CTRL           (0)

// uart data setup
#define UART_DATA_HEADER                (0xEA)
#define UART_DATA_BUF_LEN               (1800)
#define UART_DATA_RECV_TIMEOUT          (1000)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

// enumeration of uart command
typedef enum E_UartCmdType
{
    UART_CMD_SLAVE_WAKEUP_SIGN = 0,
    UART_CMD_SMART_SLEEP_ENT_REQ,
    UART_CMD_SMART_SLEEP_ENT_RSP,
    UART_CMD_DEEP_SLEEP_ENT_REQ,
    UART_CMD_DEEP_SLEEP_ENT_RSP,
    UART_CMD_BLE_ADV_START_REQ,
    UART_CMD_BLE_ADV_START_RSP,
    UART_CMD_BLE_ADV_STOP_REQ,
    UART_CMD_BLE_ADV_STOP_RSP,
    UART_CMD_WIFI_CONNECT_REQ,
    UART_CMD_WIFI_CONNECT_RSP,
    UART_CMD_WIFI_DISCONNECT_REQ,
    UART_CMD_WIFI_DISCONNECT_RSP,
    UART_CMD_PING,
    UART_CMD_PONG,

    UART_CMD_MAX,
} T_UartCmdType;

// strcture of uart data
typedef struct S_UartData
{
    T_UartCmdType tUartCmdType;
    uint8_t u8DataLen;
    uint8_t u8aData[];
} T_UartData;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/*************************************************************************
* FUNCTION:
*   UART_MessageSend
*
* DESCRIPTION:
*   send the data through UART interface
*
* PARAMETERS
*   u8aData :       [In] the data to uart rx
*   u32Len :        [In] the length of data
*
* RETURNS
*   none
*
*************************************************************************/
void UART_MessageSend(uint8_t *u8aData, uint32_t u32Len);

/*************************************************************************
* FUNCTION:
*   UART_MessageReceive
*
* DESCRIPTION:
*   1. receive the data from uart rx
*   2. send the message into Uart0MessageQ
*
* PARAMETERS
*   1. ulData   : [In] the data from uart rx
*
* RETURNS
*   none
*
*************************************************************************/
void UART_MessageReceive(uint32_t ulData);

/*************************************************************************
* FUNCTION:
*   Iot_Uart0_Init
*
* DESCRIPTION:
*   the initial of Iot Uart0
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void UART_Init(void);

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

#endif /* __UART_H__ */
