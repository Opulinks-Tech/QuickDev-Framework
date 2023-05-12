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
#include "cmsis_os.h"
#include "log.h"
#include "lwip_helper.h"
#include "net_mngr_api.h"
#include "pwr_save.h"
#include "rf_pwr.h"
#include "wifi_mngr_api.h"
#include "hal_i2c.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous
#define INIT_AND_CNCT

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

osThreadId g_tAppTaskId;
osMessageQId g_tAppQueueId;
osTimerId g_tAppSysTimer;
osTimerId g_tDeepSleepTimer;
osTimerId g_tWakeUpTimer;

static T_AppEvtHandlerTbl g_tAppEvtHandlerTbl[] = 
{
    {APP_EVT_NETWORK_UP,                    APP_EvtHandler_NetworkUp},
    {APP_EVT_NETWORK_DOWN,                  APP_EvtHandler_NetworkDown},
    {APP_EVT_NETWORK_RESET,                 APP_EvtHandler_NetworkReset},

    {APP_EVT_SYS_TIMER_TIMEOUT,             APP_EvtHandler_SysTimerTimeout},

    {APP_EVT_I2C_SLAVE_READY,               APP_EvtHandler_I2CSlaveReady},
    {APP_EVT_DEEP_SLEEP_TIMER_TIMEOUT,      APP_EvtHandler_DeepSleepTimerTimeout},
    {APP_EVT_WAKE_UP_TIMER_TIMEOUT,         APP_EvtHandler_WakeUpTimerTimeout},

    {0xFFFFFFFF,                            NULL},
};

// Sec 7: declaration of static function prototype

void APP_SysInit(void);
void APP_DataInit(void);
void APP_TaskInit(void);
void APP_BleInit(void);
void APP_NetInit(void);
void APP_CldInit(void);
void APP_UserAtInit(void);

/***********
C Functions
***********/
// Sec 8: C Functions

// indicate callback for each type request

//////////////////////////////////// 
//// Callback group
//////////////////////////////////// 

// add your callback function here

void APP_NmUnsolicitedCallback(T_NmUslctdEvtType tEvtType, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // tEvtType refer to net_mngr_api.h
    switch(tEvtType)
    {
        case NM_USLCTD_EVT_NETWORK_UP:
        {
            APP_SendMessage(APP_EVT_NETWORK_UP, pu8Data, u32DataLen);

            break;
        }
        case NM_USLCTD_EVT_NETWORK_DOWN:
        {
            APP_SendMessage(APP_EVT_NETWORK_DOWN, NULL, 0);

            break;
        }
        case NM_USLCTD_EVT_NETWORK_RESET:
        {
            APP_SendMessage(APP_EVT_NETWORK_RESET, NULL, 0);

            break;
        }
        default:
        {
            // should not be here

            break;
        }
    }
}

static void APP_SysTimerTimeoutHandler(void const *argu)
{
    APP_SendMessage(APP_EVT_SYS_TIMER_TIMEOUT, NULL, 0);
}

static void APP_DeepSleepTimerTimeoutHandler(void const *argu)
{
    APP_SendMessage(APP_EVT_DEEP_SLEEP_TIMER_TIMEOUT, NULL, 0);
}

static void APP_WakeUpTimerTimeoutHandler(void const *argu)
{
    APP_SendMessage(APP_EVT_WAKE_UP_TIMER_TIMEOUT, NULL, 0);
}

//////////////////////////////////// 
//// Event handler group
////////////////////////////////////

// add your event handler function here

static void APP_EvtHandler_NetworkUp(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "Network connected");

    // find ip number in network interface
    uint32_t u32Ip = *((uint32_t*)pData);
    uint8_t u8aIp[4] = {0};

    u8aIp[0] = (u32Ip >> 0) & 0xFF;
    u8aIp[1] = (u32Ip >> 8) & 0xFF;
    u8aIp[2] = (u32Ip >> 16) & 0xFF;
    u8aIp[3] = (u32Ip >> 24) & 0xFF;

    OPL_LOG_INFO(APP, "WI-FI IP [%d.%d.%d.%d]", u8aIp[0], u8aIp[1], u8aIp[2], u8aIp[3]);
}

static void APP_EvtHandler_NetworkDown(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "Network disconnected");
}

static void APP_EvtHandler_NetworkReset(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "Network reset");
}

static void APP_EvtHandler_SysTimerTimeout(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
#if 0
    // user implement
    OPL_LOG_INFO(APP, "Timeout, try I2C");
    APP_SendMessage(APP_EVT_I2C_SLAVE_READY, NULL, 0);  // timer time out ready
#endif
}

uint32_t g_u32SuccessCnt = 0;
uint32_t g_u32FailCnt = 0;

static void APP_EvtHandler_I2CSlaveReady(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    int i;
    uint8_t ubaWriteCmd[3];
    uint8_t ubaReadCmd[3];

    OPL_LOG_INFO(APP, "====I2C slave ready====");

    for(i=0; i<5; i++)
    {
        ubaWriteCmd[0] = 0x40;

        Hal_I2c_MasterTrasmit(&ubaWriteCmd[0], 1, 1);

        OPL_LOG_INFO(APP, "I2C tx[%d]=[0x%02x]", i, ubaWriteCmd[0]);

        osDelay(5); // delay due to EEPROM spec.

        memset(ubaReadCmd, 0x00, 3);

        // Query if slave is ok
        Hal_I2c_MasterReceive(&ubaReadCmd[0], 1, 1);

        osDelay(2);

        OPL_LOG_INFO(APP, "I2C rx[0x%02X]", ubaReadCmd[0]);

        if(0x41 == ubaReadCmd[0])
        {
            OPL_LOG_INFO(APP, "====Slave has respondsed====");
            OPL_LOG_INFO(APP, "Success count[%d]", ++g_u32SuccessCnt);

            // Start deep sleep timer
            osTimerStop(g_tDeepSleepTimer);
            osTimerStart(g_tDeepSleepTimer, 10000);

            break;
        }
    }

    if(i >= 5)
    {
        OPL_LOG_WARN(APP, "Slave doesn't respond!!!!!!");
        OPL_LOG_INFO(APP, "Fail count[%d]", ++g_u32FailCnt);
    }
}

static void APP_EvtHandler_DeepSleepTimerTimeout(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "====Ask Slave to Enter Deep Sleep====");
#if 0 // Apply GPIO 9 falling edge to ask slave to enter deep sleep
    

    Hal_Gpio_Output(WAKEUP_IO_PORT, GPIO_LEVEL_HIGH);

    OPL_LOG_INFO(APP, "GPIO[%d] high", WAKEUP_IO_PORT);

    osDelay(20);

    Hal_Gpio_Output(WAKEUP_IO_PORT, GPIO_LEVEL_LOW);

    OPL_LOG_INFO(APP, "GPIO[%d] low", WAKEUP_IO_PORT);

    osDelay(20);

    /*Hal_Gpio_Output(WAKEUP_IO_PORT, GPIO_LEVEL_HIGH);

    OPL_LOG_INFO(APP, "GPIO[%d] high", WAKEUP_IO_PORT);*/

#else   // Apply I2C command 0x80 to ask slave to enter deep sleep
    uint8_t ubaWriteCmd[3];

    ubaWriteCmd[0] = 0x80;

    Hal_I2c_MasterTrasmit(&ubaWriteCmd[0], 1, 1);

    OPL_LOG_INFO(APP, "I2C tx=[0x%02x]", ubaWriteCmd[0]);
#endif
    // Start wakeup timer
    osTimerStop(g_tWakeUpTimer);
    osTimerStart(g_tWakeUpTimer, 11000);
}

static void APP_EvtHandler_WakeUpTimerTimeout(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "====Wake up Slave from Deep Sleep====");

#if 0
    // Config GPIO 24 as ouput
    Hal_Pin_Config(PIN_TYPE_GPIO_OUT_HIGH_IO24 | PIN_DRVCRNT_IO24_12mA | PIN_INMODE_IO24_PULL_UP);

    Hal_Gpio_Output(GPIO_IDX_24, GPIO_LEVEL_HIGH);

    OPL_LOG_INFO(APP, "GPIO[%d] high", GPIO_IDX_24);

    osDelay(20);

    Hal_Gpio_Output(GPIO_IDX_24, GPIO_LEVEL_LOW);

    OPL_LOG_INFO(APP, "GPIO[%d] low", GPIO_IDX_24);


    osDelay(20);

    Hal_Gpio_Output(GPIO_IDX_24, GPIO_LEVEL_HIGH);

    OPL_LOG_INFO(APP, "GPIO[%d] high", GPIO_IDX_24);

    //osDelay(20);

    // Change back GPIO24 as I2C SDA
    Hal_Pin_Config(PIN_TYPE_I2C_SDA_IO_IO24 | PIN_DRVCRNT_IO24_12mA | PIN_INMODE_IO24_PULL_UP);

    // Start workaround timer
    osTimerStop(g_tAppSysTimer);
    osTimerStart(g_tAppSysTimer, 1000); // Wait for 1 sec for I2C ready
    //osTimerStart(g_tAppSysTimer, 1300);
#else
    Hal_Gpio_Output(WAKEUP_IO_PORT, GPIO_LEVEL_HIGH);

    OPL_LOG_INFO(APP, "GPIO[%d] high", WAKEUP_IO_PORT);

    osDelay(20);

    Hal_Gpio_Output(WAKEUP_IO_PORT, GPIO_LEVEL_LOW);

    OPL_LOG_INFO(APP, "GPIO[%d] low", WAKEUP_IO_PORT);


    osDelay(20);

    /*Hal_Gpio_Output(WAKEUP_IO_PORT, GPIO_LEVEL_HIGH);

    OPL_LOG_INFO(APP, "GPIO[%d] high", WAKEUP_IO_PORT);*/
#endif
    // Start workaround timer
    //osTimerStop(g_tAppSysTimer);
    //osTimerStart(g_tAppSysTimer, 13000);
}

//////////////////////////////////// 
//// APP function group
////////////////////////////////////

// add your application function here

void APP_SysInit(void)
{
#if (EXT_PA_ENABLED == 1)
    // Do not overwrite RF power setting if external PA enable
#else
    // initialize rf power setting
    RF_PwrSet(RF_CFG_DEF_PWR_SET);
#endif

    // user implement
}

void APP_DataInit(void)
{
    // user implement
}

void APP_TaskInit(void)
{
    // create timer
    osTimerDef_t tTimerDef;

    tTimerDef.ptimer = APP_SysTimerTimeoutHandler;
    g_tAppSysTimer = osTimerCreate(&tTimerDef, osTimerOnce, NULL);
    if(g_tAppSysTimer == NULL)
    {
        OPL_LOG_ERRO(APP, "Create sys timer fail");
    }

    tTimerDef.ptimer = APP_DeepSleepTimerTimeoutHandler;
    g_tDeepSleepTimer = osTimerCreate(&tTimerDef, osTimerOnce, NULL);
    if(g_tDeepSleepTimer == NULL)
    {
        OPL_LOG_ERRO(APP, "Create sys timer fail");
    }

    tTimerDef.ptimer = APP_WakeUpTimerTimeoutHandler;
    g_tWakeUpTimer = osTimerCreate(&tTimerDef, osTimerOnce, NULL);
    if(g_tWakeUpTimer == NULL)
    {
        OPL_LOG_ERRO(APP, "Create sys timer fail");
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
    //APP_SendMessage(APP_EVT_I2C_SLAVE_READY, NULL, 0);
}

void APP_BleInit(void)
{
    // user implement
}

void APP_NetInit(void)
{


#ifdef INIT_AND_CNCT
    // Network manager initialize and trigger quick connect
    T_NmWifiCnctConfig tWifiCnctConfig = {0};

    memset(&tWifiCnctConfig, 0, sizeof(T_NmWifiCnctConfig));

    // connect with ssid
    tWifiCnctConfig.u8SsidLen = strlen((const char *)APP_DEF_AP_SSID);
    memcpy(tWifiCnctConfig.u8aSsid, APP_DEF_AP_SSID, tWifiCnctConfig.u8SsidLen);
    // connect with bssid
    // memcpy(tWifiCnctConfig.u8aBssid, APP_DEF_AP_BSSID, WIFI_MAC_ADDRESS_LENGTH);

    // copy password
    if(0 != strlen((const char *)APP_DEF_AP_PWD))
    {
        tWifiCnctConfig.u8PwdLen = strlen((const char *)APP_DEF_AP_PWD);
        memcpy(tWifiCnctConfig.u8aPwd, APP_DEF_AP_PWD, tWifiCnctConfig.u8PwdLen);
    }
#endif
    

#ifdef INIT_AND_CNCT
    if(OPL_OK != APP_NmInitAndCnct(&APP_NmUnsolicitedCallback, &tWifiCnctConfig))
#else
    if(OPL_OK != APP_NmInit(false, &APP_NmUnsolicitedCallback))
#endif
    {
        OPL_LOG_ERRO(APP, "Net manager init fail");

        return;
    }

    OPL_LOG_INFO(APP, "Net manager init ok, try connect to %s..", APP_DEF_AP_SSID);

    // user implement
}

void APP_CldInit(void)
{
    // user implement
}

void APP_UserAtInit(void)
{
    // add at cmd and enable CR/LF
    AT_CmdListAdd(1);  // #define CRLF_ENABLE (1)
}


static void App_SlaveReadyGpioCallBack(E_GpioIdx_t tGpioIdx)
{
    // send the result to the task of blewifi control.
    APP_SendMessage(APP_EVT_I2C_SLAVE_READY, NULL, 0);
}


void App_SlaveReadyGpioInit(E_GpioIdx_t tGpioIdx)
{
    Hal_Vic_GpioCallBackFuncSet(tGpioIdx, App_SlaveReadyGpioCallBack);
    Hal_Vic_GpioDirection(tGpioIdx, GPIO_INPUT);
    Hal_Vic_GpioIntTypeSel(tGpioIdx, INT_TYPE_FALLING_EDGE);
    Hal_Vic_GpioIntMask(tGpioIdx, 0);
    Hal_Vic_GpioIntEn(tGpioIdx, 1);
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

    //APP_BleInit();

    //APP_NetInit();

    APP_CldInit();

    APP_UserAtInit();
#if 0
    // For the 1st time
    osTimerStop(g_tAppSysTimer);
    osTimerStart(g_tAppSysTimer, 10000);
#else
    App_SlaveReadyGpioInit(SLAVE_READY_IO_PORT);
#endif

    // user implement
    
    // enter smart sleep after 5s
#if (PS_ENABLED == 1)
    PS_EnterSmartSleep(5000);
#endif
}
