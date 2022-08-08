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
#include "app_at_cmd.h"
#if defined(OPL1000_A2) || defined(OPL1000_A3)
#include "at_cmd_data_process_patch.h"
#elif defined(OPL2500_A0)
#include "at_cmd_data_process.h"
#endif
#include "cmsis_os.h"
#include "hal_vic.h"
#include "log.h"
#include "pwr_save.h"
#include "rf_pwr.h"
#include "uart.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#define HOST_MODE_WAKEUP_PIN                            (GPIO_IDX_04)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef struct S_HostModeReqCmdFmt
{
    T_HostModeReqCmdList tHostModeReqCmdIdx;
    at_cmd_mode_t tAtMode;
    uint16_t u16PayloadLen;
    uint8_t u8aPayload[HOST_MODE_DATA_LEN];
} T_HostModeReqCmdFmt;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

osThreadId g_tAppTaskId;
osMessageQId g_tAppQueueId;
osTimerId g_tAppSleepSlaveTimer;
osTimerId g_tAppWakeupSlaveTimer;

static T_AppEvtHandlerTbl g_tAppEvtHandlerTbl[] = 
{
    {APP_EVT_AT_MSG_SEND,                   APP_EvtHandler_AtMsgSend},
    {APP_EVT_AT_MSG_RECV,                   APP_EvtHandler_AtMsgRecv},
    {APP_EVT_SLEEP_SLAVE,                   APP_EvtHandler_SleepSlave},
    {APP_EVT_WAKEUP_SLAVE,                  APP_EvtHandler_WakeupSlave},

    {0xFFFFFFFF,                            NULL},
};

// host mode req command list in string (the indexing of string array must same as T_HostModeReqCmdList)
int8_t *i8HostModeReqCmdListStrTbl[] =
{
    "at+fwkver",
    "at+entsleep",
    "at+provisionstart",
    "at+provisionstop",
    "at+blests",
    "at+wifiscanap",
    "at+wificonnectap",
    "at+wifiqconnectap",
    "at+wifists",
    "at+cloudconn",
    "at+clouddisc",
    "at+cloudtxtopic",
    "at+cloudrxtopic",
    "at+cloudtxpost",

    // add your command here
};

// host mode ack command list in string (the indexing of string array must same as T_HostModeAckCmdList)
int8_t *i8HostModeAckCmdListStrTbl[] = 
{
    "+READY",
    "+WAKEUP",
    "+PROVISION",
    "+BLESTS",
    "+WIFISCAN",
    "+WIFICONN",
    "+WIFIDISCONN",
    "+NETWORKUP",
    "+NETWORKDOWN",
    "+NETWORKRESET",
    "+WIFISTS",
    "+CLOUDUP",
    "+CLOUDDOWN",
    "+CLOUDKEEPALIVE",
    "+CLOUDTXTOPIC",
    "+CLOUDRXTOPIC",
    "+TXRSP",
    "+RXDATA",
    "+TEST",
    "+NOTSUPPORT",
    "OK",
    "ERROR",

    // add your command here
};

T_HostModeReqCmdList g_tHostModeBackupReqCmdIdx = AT_CMD_REQ_MAX;
T_HostModeAckCmdList g_tHostModeBackupAckCmdIdx = AT_CMD_ACK_MAX;

uint8_t g_u8WifiStatus = 0;

// Sec 7: declaration of static function prototype

T_OplErr APP_HostModeSendReq(T_HostModeReqCmdList tHostModeReqCmdIdx, at_cmd_mode_t tReqCmdMode, uint8_t *payload, uint32_t payloadlen);
T_OplErr APP_HostModeAckParser(uint8_t *u8RecvData, uint32_t u32RecvDataLen, T_HostModeAckCmdList *tHostModeAckCmdIdx, uint8_t *u8Result);
void APP_HostModeDemoProgress(T_HostModeAckCmdList tHostModeAckCmdIdx, uint8_t *u8Result);

void APP_WakeupSlave(void);
void APP_SysInit(void);
void APP_DataInit(void);
void APP_TaskInit(void);
void APP_BleInit(void);
void APP_NetInit(void);
void APP_CldInit(void);

/***********
C Functions
***********/
// Sec 8: C Functions

// indicate callback for each type request

//////////////////////////////////// 
//// Callback group
//////////////////////////////////// 

// add your callback function here

static void APP_SleepSlaveTimerTimeoutHandler(void const *argu)
{
    APP_SendMessage(APP_EVT_SLEEP_SLAVE, NULL, 0);
}

static void APP_WakeupSlaveTimerTimeoutHandler(void const *argu)
{
    APP_SendMessage(APP_EVT_WAKEUP_SLAVE, NULL, 0);
}

//////////////////////////////////// 
//// Event handler group
////////////////////////////////////

// add your event handler function here


static void APP_EvtHandler_AtMsgSend(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    T_HostModeReqCmdFmt tHostModeReqCmdFmt = *((T_HostModeReqCmdFmt *)pData);

    APP_HostModeSendReq(tHostModeReqCmdFmt.tHostModeReqCmdIdx, tHostModeReqCmdFmt.tAtMode, tHostModeReqCmdFmt.u8aPayload, tHostModeReqCmdFmt.u16PayloadLen);
}

static void APP_EvtHandler_AtMsgRecv(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    T_HostModeAckCmdList tHostModeAckCmdIdx = AT_CMD_ACK_MAX;
    uint8_t u8Result[UART_DATA_BUF_LEN] = {0};

    APP_HostModeAckParser(pData, u32DataLen, &tHostModeAckCmdIdx, u8Result);

    APP_HostModeDemoProgress(tHostModeAckCmdIdx, u8Result);
}

static void APP_EvtHandler_SleepSlave(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    T_HostModeReqCmdFmt tHostModeReqCmdFmt;

    // trigger sleep mode
    // 1 = smart sleep
    // 2 = timer sleep
    // 3 = deep sleep
    strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "1");
    tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
    tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_SLEEP;
    tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;

    APP_SendMessage(APP_EVT_AT_MSG_SEND, (uint8_t*)&tHostModeReqCmdFmt, sizeof(T_HostModeReqCmdFmt));
}

static void APP_EvtHandler_WakeupSlave(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    APP_WakeupSlave();
}

//////////////////////////////////// 
//// APP function group
////////////////////////////////////

// add your application function here

T_OplErr APP_HostModeSendReq(T_HostModeReqCmdList tHostModeReqCmdIdx, at_cmd_mode_t tReqCmdMode, uint8_t *payload, uint32_t payloadlen)
{
    int8_t i8AckMsg[UART_DATA_BUF_LEN] = {0};
    char *cReqCmdPtr = (char *)&i8AckMsg;

    uint32_t u32ReqMsgOffset = 0;

    if(AT_CMD_REQ_MAX <= tHostModeReqCmdIdx)
    {
        OPL_LOG_WARN(APP, "REQ cmd index %d", tHostModeReqCmdIdx)
        return OPL_ERR;
    }

    if(AT_CMD_REQ_EMPTY == tHostModeReqCmdIdx)
    {
        u32ReqMsgOffset += sprintf(cReqCmdPtr + u32ReqMsgOffset, "%s", payload);
        
        goto send;
    }

    u32ReqMsgOffset += sprintf(cReqCmdPtr + u32ReqMsgOffset, "%s", (char *)i8HostModeReqCmdListStrTbl[tHostModeReqCmdIdx]);

    if(AT_CMD_MODE_SET == tReqCmdMode && NULL != payload)
    {
        u32ReqMsgOffset += sprintf(cReqCmdPtr + u32ReqMsgOffset, "=%s", payload);

        // memcpy(cReqCmdPtr + u32ReqMsgOffset, payload, payloadlen);
        // u32ReqMsgOffset += payloadlen;
    }
    else if(AT_CMD_MODE_READ == tReqCmdMode)
    {
        u32ReqMsgOffset += sprintf(cReqCmdPtr + u32ReqMsgOffset, "?");
    }
    else if(AT_CMD_MODE_TESTING == tReqCmdMode)
    {
        u32ReqMsgOffset += sprintf(cReqCmdPtr + u32ReqMsgOffset, "=?");
    }

    u32ReqMsgOffset += sprintf(cReqCmdPtr + u32ReqMsgOffset, "\r\n");

send:
    UART_MessageSend((uint8_t *)i8AckMsg, u32ReqMsgOffset);

    g_tHostModeBackupReqCmdIdx = tHostModeReqCmdIdx;

    return OPL_OK;
}

T_OplErr APP_HostModeAckParser(uint8_t *u8RecvData, uint32_t u32RecvDataLen, T_HostModeAckCmdList *tHostModeAckCmdIdx, uint8_t *u8Result)
{
    uint8_t u8Count = 0;
    static uint8_t u8UartRecvData[UART_DATA_BUF_LEN];
    memset(u8UartRecvData, 0, sizeof(u8UartRecvData));
    memcpy(u8UartRecvData, u8RecvData, u32RecvDataLen);

    for(; u8Count < AT_CMD_ACK_MAX; u8Count ++)
    {
        if(strstr((char *)u8UartRecvData, (char *)i8HostModeAckCmdListStrTbl[u8Count]) != NULL)
        {
            uint16_t u16RecvDataLen = strlen((char *)u8UartRecvData);
            uint16_t u16AckCmdHdrLen = strlen((char *)i8HostModeAckCmdListStrTbl[u8Count]);

            *tHostModeAckCmdIdx = (T_HostModeAckCmdList)u8Count;

            OPL_LOG_INFO(APP, "Recv %s cmd", u8UartRecvData);

            // if have result
            if(strstr((char *)u8UartRecvData, ":") != NULL)
            {
                memcpy(u8Result, (u8UartRecvData + u16AckCmdHdrLen + 1), (u16RecvDataLen - u16AckCmdHdrLen));
                OPL_LOG_INFO(APP, "with rst %s (%d)", u8Result, u16RecvDataLen - u16AckCmdHdrLen);
            }

            break;
        }
    }

    if(AT_CMD_ACK_MAX == u8Count)
    {
        return OPL_ERR; // command not found
    }
    else
    {
        return OPL_OK;
    }
}

void APP_HostModeDemoProgress(T_HostModeAckCmdList tHostModeAckCmdIdx, uint8_t *u8Result)
{
    T_HostModeReqCmdFmt tHostModeReqCmdFmt;
    uint8_t u8NeedToSend = 0;

    switch(tHostModeAckCmdIdx)
    {
        case AT_CMD_ACK_DEVICE_READY:
        {
            // might cause from slave auto-connect
            if(1 != g_u8WifiStatus)
            {
                // start provision
                strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "60");
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_PROVISION_START;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;

                u8NeedToSend = 1;
            }

            break;
        }
        case AT_CMD_ACK_WAKEUP_FROM_SLEEP:
        {
            strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "1,10");
            tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
            tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_TX_POST;
            tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;

            u8NeedToSend = 1;

            break;
        }
        case AT_CMD_ACK_PROVISION:
        {
            uint8_t u8Rst = (uint8_t)atoi((char *)u8Result);

            if(0 == u8Rst)
            {
                // provision success

                // wait until recv Network Up ack
            }
            else if(1 == u8Rst)
            {
                // provision timeout

                // re-start provision
                strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "60");
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_PROVISION_START;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;

                u8NeedToSend = 1;
            }
            else if(2 == u8Rst)
            {
                // provision stop
            }

            break;
        }
        case AT_CMD_ACK_BLE_STATUS:
        case AT_CMD_ACK_WIFI_SCAN:
        case AT_CMD_ACK_WIFI_CONN:
        case AT_CMD_ACK_WIFI_DISCONN:
            break;
        case AT_CMD_ACK_NETWORK_UP:
        {
            g_u8WifiStatus = 1;

            // send cloud estalbish event
#if defined(HOST_MODE_TCP)
            strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "1,192.168.0.100,8883");
#elif defined(HOST_MODE_MQTT)
            strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "1,broker.emqx.io,8883");
#endif

            tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
            tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_CONN;
            tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;

            u8NeedToSend = 1;
            break;
        }
        case AT_CMD_ACK_NETWORK_DOWN:
        case AT_CMD_ACK_NETWORK_RESET:
        {
            g_u8WifiStatus = 0;

            break;
        }
        case AT_CMD_ACK_WIFI_STATUS:
        case AT_CMD_ACK_CLOUD_CONNECT:
        case AT_CMD_ACK_CLOUD_DISCONNECT:
        case AT_CMD_ACK_CLOUD_KEEPALIVE_INTERVAL:
        case AT_CMD_ACK_CLOUD_TX_TOPIC:
        case AT_CMD_ACK_CLOUD_RX_TOPIC:
        case AT_CMD_ACK_CLOUD_TX_POST:
            break;
        case AT_CMD_ACK_CLOUD_RX_RECV:
        {
            // shows the received data
        }
        case AT_CMD_ACK_TEST:
        case AT_CMD_ACK_NOT_SUPPORT:
            break;

        case AT_CMD_ACK_OK:
        {
            if(g_tHostModeBackupReqCmdIdx == AT_CMD_REQ_CLOUD_CONN)
            {
                // wait cloud real connected
                osDelay(5000);

#if defined(HOST_MODE_TCP)
                // start sleep slave timer after 1s
                osTimerStart(g_tAppSleepSlaveTimer, 1000);
#elif defined(HOST_MODE_MQTT)
                // register tx topic
                strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "1,MQTT/TEST/PUB/1");
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_TX_TOPIC;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;

                u8NeedToSend = 1;
#endif
            }
            else if(g_tHostModeBackupReqCmdIdx == AT_CMD_REQ_CLOUD_TX_TOPIC)
            {
#if defined(HOST_MODE_MQTT)
                // register rx topic
                strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "1,MQTT/TEST/SUB/1");
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_RX_TOPIC;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;

                u8NeedToSend = 1;
#endif
            }
            else if(g_tHostModeBackupReqCmdIdx == AT_CMD_REQ_CLOUD_RX_TOPIC)
            {
                // start sleep slave timer after 1s
                osTimerStart(g_tAppSleepSlaveTimer, 1000);
            }
            else if(g_tHostModeBackupReqCmdIdx == AT_CMD_REQ_CLOUD_TX_POST)
            {
                strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "HelloWorld");
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_EMPTY;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_EXECUTION;

                u8NeedToSend = 1;

                // start sleep slave timer after 5s
                osTimerStart(g_tAppSleepSlaveTimer, 5000);
            }
            else if(g_tHostModeBackupReqCmdIdx == AT_CMD_REQ_SLEEP)
            {
                // start wakeup salve timer after 10s
                osTimerStart(g_tAppWakeupSlaveTimer, 10000);
            }

            break;
        }

        case AT_CMD_ACK_ERROR:
        {
            break;
        }

        default:
            break;
    }

    if(1 == u8NeedToSend)
    {
        // process to send the command to slave
        APP_SendMessage(APP_EVT_AT_MSG_SEND, (uint8_t*)&tHostModeReqCmdFmt, sizeof(T_HostModeReqCmdFmt));
    }
}

void APP_WakeupSlave(void)
{
    OPL_LOG_INFO(APP, "Trigger gpio to wakeup slave");

    // trigger GPIO 4 to wakeup slave
    Hal_Vic_GpioOutput(HOST_MODE_WAKEUP_PIN, GPIO_LEVEL_HIGH);
    osDelay(50);
    Hal_Vic_GpioOutput(HOST_MODE_WAKEUP_PIN, GPIO_LEVEL_LOW);
}

void APP_SysInit(void)
{
    // initialize rf power setting
    RF_PwrSet(RF_CFG_DEF_PWR_SET);

    // user implement

    UART_Init();
}

void APP_DataInit(void)
{
    // user implement
}

void APP_TaskInit(void)
{
    // create timer
    osTimerDef_t tTimerDef;

    tTimerDef.ptimer = APP_SleepSlaveTimerTimeoutHandler;
    g_tAppSleepSlaveTimer = osTimerCreate(&tTimerDef, osTimerOnce, NULL);
    if(g_tAppSleepSlaveTimer == NULL)
    {
        OPL_LOG_ERRO(APP, "Create sleep slave timer fail");
    }

    tTimerDef.ptimer = APP_WakeupSlaveTimerTimeoutHandler;
    g_tAppWakeupSlaveTimer = osTimerCreate(&tTimerDef, osTimerOnce, NULL);
    if(g_tAppWakeupSlaveTimer == NULL)
    {
        OPL_LOG_ERRO(APP, "Create ping slave timer fail");
    }

    // create message queue
    osMessageQDef_t tQueueDef;

    tQueueDef.item_sz = sizeof(T_AppMsgStruct);
    tQueueDef.queue_sz = APP_QUEUE_SIZE;
    g_tAppQueueId = osMessageCreate(&tQueueDef, NULL);

    if(g_tAppQueueId == NULL)
    {
        OPL_LOG_ERRO(APP, "Create queue fail");
    }

    // create task
    osThreadDef_t tTaskDef;
    
    tTaskDef.name = "App Main";
    tTaskDef.stacksize = APP_TASK_STACK_SIZE;
    tTaskDef.tpriority = APP_TASK_PRIORITY;
    tTaskDef.pthread = APP_Main;
    g_tAppTaskId = osThreadCreate(&tTaskDef, NULL);

    if(g_tAppTaskId == NULL)
    {
        OPL_LOG_ERRO(APP, "Create task fail");
    }

    OPL_LOG_INFO(APP, "Create task ok");

    // user implement
}

void APP_BleInit(void)
{
    // user implement
}

void APP_NetInit(void)
{
    // user implement
}

void APP_CldInit(void)
{
    // user implement
}

//////////////////////////////////// 
//// APP task group
//////////////////////////////////// 
/*************************************************************************
* FUNCTION:
*   APP_SendMessage
*
* DESCRIPTION:
*   Send message queue to APP task
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
T_OplErr APP_SendMessage(uint32_t u32EventId, uint8_t *pu8Data, uint32_t u32DataLen)
{
    if(NULL == g_tAppQueueId)
    {
        OPL_LOG_WARN(APP, "Queue not init");

        return OPL_ERR_RTOS_QMSG_NOT_INIT;
    }

    T_AppMsgStruct *ptMsg = (T_AppMsgStruct *)malloc(sizeof(T_AppMsgStruct) + u32DataLen);

    if(NULL == ptMsg)
    {
        OPL_LOG_ERRO(APP, "Alloc WM message fail");
        return OPL_ERR_ALLOC_MEMORY_FAIL;
    }

    // prepare the data
    ptMsg->u32EventId = u32EventId;
    ptMsg->u32DataLen = u32DataLen;

    if(0 != ptMsg->u32DataLen)
    {
        memcpy(ptMsg->pau8Data, pu8Data, u32DataLen);
    }

    // send message
    if(osOK != osMessagePut(g_tAppQueueId, (uint32_t)ptMsg, 0))
    {
        OPL_LOG_ERRO(APP, "Send message fail");
        free(ptMsg);

        return OPL_ERR_RTOS_SEND_QMSG_FAIL;
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION:
*   APP_EventProcess
*
* DESCRIPTION:
*   Message processor
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
T_OplErr APP_EventProcess(T_AppMsgStruct *ptMsg)
{
    // event in user app define list
    if((APP_EVT_BEGIN <= ptMsg->u32EventId) && (ptMsg->u32EventId <= APP_EVT_TOTAL))
    {   
        uint32_t i = 0;
        while(g_tAppEvtHandlerTbl[i].u32EventId != 0xFFFFFFFF)
        {
            // matched
            if(g_tAppEvtHandlerTbl[i].u32EventId == ptMsg->u32EventId)
            {
                g_tAppEvtHandlerTbl[i].fpFunc(ptMsg->u32EventId, ptMsg->pau8Data, ptMsg->u32DataLen);
                break;
            }

            i ++;
        }

        // not match
        if(g_tAppEvtHandlerTbl[i].u32EventId == 0xFFFFFFFF)
        {
            OPL_LOG_WARN(APP, "can't find event in event table");

            return OPL_ERR_RTOS_EVT_NOT_FOUND;
        }

        return OPL_OK;
    }
    else
    {
        return OPL_ERR_RTOS_EVT_NOT_FOUND;
    }
}

/*************************************************************************
* FUNCTION:
*   AppInit
*
* DESCRIPTION:
*   the initial of application
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void APP_Main(void *args)
{
    osEvent tEvent;
    T_AppMsgStruct *ptMsg;

    for(;;)
    {
        // wait event
        tEvent = osMessageGet(g_tAppQueueId, osWaitForever);

        if (tEvent.status == osEventMessage)
        {
            ptMsg = (T_AppMsgStruct *)tEvent.value.p;

            if(OPL_ERR_RTOS_EVT_NOT_FOUND == APP_EventProcess(ptMsg))
            {
#if (NM_ENABLED == 1)
                // while event not found in user define, try network manager
                APP_NmEventProc(ptMsg->u32EventId, ptMsg->pau8Data, ptMsg->u32DataLen);
#endif
            }

            if(ptMsg != NULL)
            {
                free(ptMsg);
            }
        }
    }
}

/*************************************************************************
* FUNCTION:
*   AppInit
*
* DESCRIPTION:
*   the initial of application
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void APP_MainInit(void)
{
    // create main task
    OPL_LOG_INFO(APP, "App Main Init");

    APP_SysInit();

    APP_DataInit();

    APP_TaskInit();

    APP_BleInit();

    APP_NetInit();

    APP_CldInit();
    
    // enter smart sleep after 5s
#if (PS_ENABLED == 1)
    PS_EnterSmartSleep(5000);
#endif

}
