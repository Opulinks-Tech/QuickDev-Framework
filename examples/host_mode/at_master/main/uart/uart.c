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
*  app_main.c
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

#include "app_main.h"
#include "boot_sequence.h"
#include "cmsis_os.h"
// #include "mw_fim_default_group01.h"
#include "log.h"
#include "sys_os_config.h"
#include "uart.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

// #define UART0_RCV_TIMEOUT       100     // ms

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

static osSemaphoreId g_tUartTxSemaphoreId;
static osTimerId g_tUartRxTimeoutId;

static uint8_t g_u8aUartRecvData[UART_DATA_BUF_LEN];     // Receiving Data content
static uint16_t g_u16UartRecvDataCnt = 0;
// static uint16_t g_u16UartDataLen = 0;

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

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
static void UART_RxReceivedTimeoutHandler(void const *argu)
{
    OPL_LOG_WARN(UART, "received timeout\r\n");

    // reset
    memset(g_u8aUartRecvData, 0, sizeof(uint8_t)*UART_DATA_BUF_LEN);
    g_u16UartRecvDataCnt = 0;
    // g_u16UartDataLen = 0;
}

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
void UART_MessageSend(uint8_t *u8aData, uint32_t u32Len)
{
    int i;

    osSemaphoreWait(g_tUartTxSemaphoreId, osWaitForever);

    OPL_LOG_INFO(UART, "[UART] Send to MCU : %s (len %d)", u8aData, u32Len);

    for (i = 0; i < u32Len; i ++)
    {
        // printf("%s", u8aData[i]);
        Hal_Uart_DataSend(UART_CONFIG_IDX, (uint32_t)u8aData[i]);
    }

    Hal_Uart_DataSend(UART_CONFIG_IDX, 0x0D);
    Hal_Uart_DataSend(UART_CONFIG_IDX, 0x0A);

    osSemaphoreRelease(g_tUartTxSemaphoreId);
}

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
void UART_MessageReceive(uint32_t ulData)
{
    osTimerStop(g_tUartRxTimeoutId);

    // header
    if (0 == g_u16UartRecvDataCnt)
    {
        // error header
        if (0x2B != ulData && 0x45 != ulData && 0x4F != ulData)
        {
            goto done;
        }
    }
    else if (g_u16UartRecvDataCnt != 0 && (0x0A == ulData || 0x0D == ulData))
    {
        // g_u8aUartRecvData[g_u16UartRecvDataCnt] = ulData;

        OPL_LOG_INFO(UART, "Recv %s", g_u8aUartRecvData);

        APP_SendMessage(APP_EVT_AT_MSG_RECV, g_u8aUartRecvData, g_u16UartRecvDataCnt);

        goto done;
    }

    g_u8aUartRecvData[g_u16UartRecvDataCnt] = ulData;
    g_u16UartRecvDataCnt++;

    osTimerStart(g_tUartRxTimeoutId, UART_DATA_RECV_TIMEOUT);

    // normal case
    return;

    // reset case
done:
    memset(g_u8aUartRecvData, 0, sizeof(uint8_t)*UART_DATA_BUF_LEN);
    g_u16UartRecvDataCnt = 0;
    // g_u16UartDataLen = 0;
    return;
}

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
void UART_Init(void)
{
    osSemaphoreDef_t tSemaphoreDef;
    osTimerDef_t tTimerDef;

    // Init UART0
    // cold boot
    if (0 == Boot_CheckWarmBoot())
    {
        // Initialize Receiving Data
        memset(g_u8aUartRecvData, 0, sizeof(uint8_t)*UART_DATA_BUF_LEN);
        g_u16UartRecvDataCnt = 0;
        // g_u16UartDataLen = 0;
        
        // create the UART TX semaphore
        tSemaphoreDef.dummy = 0;                            // reserved, it is no used
        g_tUartTxSemaphoreId = osSemaphoreCreate(&tSemaphoreDef, 1);
        if (g_tUartTxSemaphoreId == NULL)
        {
            OPL_LOG_ERRO(UART, "Create UART TX semaphore fail\r\n");
        }
        
        // create the UART RX timeout timer
        tTimerDef.ptimer = UART_RxReceivedTimeoutHandler;
        g_tUartRxTimeoutId = osTimerCreate(&tTimerDef, osTimerOnce, NULL);
        if(g_tUartRxTimeoutId == NULL)
        {
            OPL_LOG_ERRO(UART, "Create UART RX timeout timer fail\r\n");
        }
    }

    Hal_Uart_Init(UART_CONFIG_IDX,
                  UART_CONFIG_BAUDRATE,
                  UART_CONFIG_DATA_BITS,
                  UART_CONFIG_PARITY,
                  UART_CONFIG_STOP_BITS,
                  UART_CONFIG_FLOW_CTRL);

    // set the rx callback function and enable the rx interrupt
    Hal_Uart_RxCallBackFuncSet(UART_CONFIG_IDX, UART_MessageReceive);

#if defined(OPL1000_A2) || defined(OPL1000_A3)
    Hal_Uart_RxIntEn(UART_CONFIG_IDX, 1);
#elif defined(OPL2500_A0)
    Hal_Uart_RxIntEn(UART_CONFIG_IDX, IRQ_PRIORITY_UART0, ENABLE);
#endif
}
