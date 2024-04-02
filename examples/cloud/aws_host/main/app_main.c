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
//#include "cloud_ctrl.h"
//#include "cloud_kernel.h"
#include "hal_vic.h"
#include "log.h"
#include "pwr_save.h"
#include "rf_pwr.h"
#include "uart.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#define HOST_MODE_WAKEUP_PIN                            (GPIO_IDX_09) // wakeup slave
#define HOST_MODE_WAKEUP_HOST_PIN                       (GPIO_IDX_28) // slave wakeup host 
#define AP_NAME                                         ("AE_TP1")
#define AP_PASSWORD                                     ("")
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

uint8_t g_u8WifiStatus  = 0;
uint8_t g_u8BleStatus   = 0;
uint8_t g_u8ExampleMode = 0;
uint8_t g_u8SleepMode   = 0;
uint8_t g_u8SleepCount  = 0;
uint8_t g_u8ApNum       = 0; 
uint8_t g_u8UploadNum   = 0; 
static uint8_t g_u8UartRecvData[UART_DATA_BUF_LEN];
static int8_t i8AckMsg[UART_DATA_BUF_LEN] = {0};
T_CloudConnInfo g_tCloudConnInfo = {0};
T_UploadFileStruct *g_tUploadFileCA;
T_UploadFileStruct *g_tUploadFileCer;
T_UploadFileStruct *g_tUploadFilePrvKey;

static T_AppEvtHandlerTbl g_tAppEvtHandlerTbl[] = 
{
    {APP_EVT_AT_MSG_SEND,                   APP_EvtHandler_AtMsgSend},
    {APP_EVT_AT_MSG_RECV,                   APP_EvtHandler_AtMsgRecv},
    {APP_EVT_SLEEP_SLAVE,                   APP_EvtHandler_SleepSlave},
    {APP_EVT_WAKEUP_SLAVE,                  APP_EvtHandler_WakeupSlave},

    {APP_EVT_EXAMPLE_MODE,                  APP_EvtHandler_ExmapleMode}, 
    {APP_EVT_SMART_SLEEP,                   APP_EvtHandler_SmartSleep},
    {APP_EVT_IO_EXIT_SMART_SLEEP,           APP_EvtHandler_IoExitSmartSleep},

    {APP_EVT_CLOUD_MQTT_ESTAB_REQ,          APP_EvtHandler_CloudMqttEstablishReq},
    {APP_EVT_CLOUD_MQTT_UPLOAD_FILE,        APP_EvtHandler_CloudMqttUploadFile},

    {0xFFFFFFFF,                            NULL},
};

// host mode req command list in string (the indexing of string array must same as T_HostModeReqCmdList)
int8_t *i8HostModeReqCmdListStrTbl[] =
{
    "at+fwkver",
    "at+sleepmode",
    "at+blestart",
    "at+netscan", 
    "at+netconnect", 
    "at+netdisconnect",
    "at+fulmqttcert", 

    "",
    "",
    "",

    "at+mqttclientid",
    "at+mqttkpintvl",
    "at+mqttlastwill",
    "{\"Event\":\"Disconnect\"}",
    "at+mqttinit",
    "at+mqttconnect",
    "at+mqttdisconnect",
    "at+mqttsub",
    "at+mqttpub",
    "hello",
    "at+hostready",

    // add your command here
};

T_HostModeReqCmdList g_tHostModeBackupReqCmdIdx = AT_CMD_REQ_MAX;
T_HostModeAckCmdList g_tHostModeBackupAckCmdIdx = ACK_TAG_MAX;

// Sec 7: declaration of static function prototype

T_OplErr APP_HostModeSendReq(T_HostModeReqCmdList tHostModeReqCmdIdx, at_cmd_mode_t tReqCmdMode, uint8_t *payload, uint32_t payloadlen);
T_OplErr APP_HostModeAckParser(uint8_t *u8RecvData, uint32_t u32RecvDataLen, T_HostModeAckCmdList *tHostModeAckCmdIdx, uint8_t *u8Result);
void APP_HostModeDemoProgress(T_HostModeAckCmdList tHostModeAckCmdIdx, uint8_t *u8Result);

void APP_WakeupPinInit(void);
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

static void APP_TriggerBySlaveHandler(void const *argu)
{
    APP_SendMessage(APP_EVT_IO_EXIT_SMART_SLEEP, NULL, 0);
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
    T_HostModeAckCmdList tHostModeAckCmdIdx = ACK_TAG_MAX;
    uint8_t u8Result[UART_DATA_BUF_LEN] = {0};

    APP_HostModeAckParser(pData, u32DataLen, &tHostModeAckCmdIdx, u8Result);

    APP_HostModeDemoProgress(tHostModeAckCmdIdx, u8Result);
}

static void APP_EvtHandler_SleepSlave(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    T_HostModeReqCmdFmt tHostModeReqCmdFmt;

    if(g_u8SleepMode == 1)
    {
        strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "1");
    }
    else if(g_u8SleepMode == 2)
    {
        strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "2,10000"); 
    }
    else if(g_u8SleepMode == 3)
    {
        strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "3");
    }

    tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
    tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_SLEEP;
    tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;

    APP_SendMessage(APP_EVT_AT_MSG_SEND, (uint8_t*)&tHostModeReqCmdFmt, sizeof(T_HostModeReqCmdFmt));
}

static void APP_EvtHandler_WakeupSlave(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    APP_WakeupSlave();
}

static void APP_EvtHandler_ExmapleMode(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    memcpy(&g_u8ExampleMode, pData, sizeof(uint8_t));
    T_HostModeReqCmdFmt tHostModeReqCmdFmt;
    g_u8SleepCount = 0;
    g_u8UploadNum = 0;

    OPL_LOG_INFO(APP, "Eample mode : %d\n",g_u8ExampleMode);

    // host example start here
    switch (g_u8ExampleMode)
    {
        case 1: // sleep mode wakeup by io
        case 3: // event wifi disconnect
        case 4:
        case 5:
        {
            g_u8SleepMode = 1; //start with smart sleep

            if(g_u8BleStatus == 0)
            {
                strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "0");
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_BLE_START;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;
            }
            else
            {
                printf("BLE connect already\n");
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_WIFI_SCAN;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_EXECUTION;
            }
            break;
        }
        case 2: // event wakeup: ble connect and ble disconnect
        {
            g_u8SleepMode = 1; //start with smart sleep

            if(g_u8BleStatus == 0)
            {
                strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "0");
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_BLE_START;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;
            }
            else
            {
                OPL_LOG_INFO(APP, "Please Disconnect BLE");
            }
            break;
        }
        case 6: // event timer sleep time out wakeup
        {
            g_u8SleepMode = 2; //start with timer sleep

            if(g_u8BleStatus == 0)
            {
                strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "0");
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_BLE_START;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;
            }
            else
            {
                printf("BLE connect already\n");
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_WIFI_SCAN;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_EXECUTION;
            }
            break;
        }    

        default:
            break;
    }

    APP_SendMessage(APP_EVT_AT_MSG_SEND, (uint8_t*)&tHostModeReqCmdFmt, sizeof(T_HostModeReqCmdFmt));
}

static void APP_EvtHandler_IoExitSmartSleep(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    //Hal_Vic_GpioIntEn((E_GpioIdx_t)HOST_MODE_WAKEUP_HOST_PIN, 0);

    T_HostModeReqCmdFmt tHostModeReqCmdFmt;

    OPL_LOG_INFO(APP,"Trigger IO by slave");
    PS_ExitSmartSleep();
    
    tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
    tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_HOST_READY;
    tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_EXECUTION;

    APP_SendMessage(APP_EVT_AT_MSG_SEND, (uint8_t*)&tHostModeReqCmdFmt, sizeof(T_HostModeReqCmdFmt));
}

static void APP_SmartSleepIoCallback(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    APP_SendMessage(APP_EVT_IO_EXIT_SMART_SLEEP, NULL, 0);
}

static void APP_EvtHandler_SmartSleep(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "Smart Sleep");
    Hal_Pin_ConfigSet(HOST_MODE_WAKEUP_HOST_PIN, PIN_TYPE_GPIO_INPUT, PIN_DRIVING_FLOAT);  // OPL_POWER_SLEEP_CONTROL
    ps_set_wakeup_io((E_GpioIdx_t)HOST_MODE_WAKEUP_HOST_PIN, 1, INT_TYPE_RISING_EDGE, 0, (T_Gpio_CallBack)APP_SmartSleepIoCallback);
    PS_EnterSmartSleep(0);
}

static void APP_EvtHandler_CloudMqttEstablishReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    memcpy(&g_tCloudConnInfo, pData, sizeof(g_tCloudConnInfo));
    OPL_LOG_INFO(APP, "Host domain name: %s", g_tCloudConnInfo.u8aHostAddr);
    OPL_LOG_INFO(APP, "Host port: %d", g_tCloudConnInfo.u16HostPort);
    OPL_LOG_INFO(APP, "Host Auto connect: %d", g_tCloudConnInfo.u8AutoConn);

}

static void APP_EvtHandler_CloudMqttUploadFile(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    //int i = 0;
    if(g_u8UploadNum == 0)
    {
        g_tUploadFileCA = (T_UploadFileStruct *) malloc(u32DataLen+1);
        memset(g_tUploadFileCA, 0, u32DataLen + 1); //avoid get a strong strlen    
        memcpy(g_tUploadFileCA, pData, u32DataLen);

        OPL_LOG_INFO(APP, "File type: %d", g_tUploadFileCA->tFileType);
        OPL_LOG_INFO(APP, "File len %d", g_tUploadFileCA->u32DataLen);
    }
    else if(g_u8UploadNum == 1)
    {
        g_tUploadFileCer = (T_UploadFileStruct *) malloc(u32DataLen+1);
        memset(g_tUploadFileCer, 0, u32DataLen + 1);   
        memcpy(g_tUploadFileCer, pData, u32DataLen);

        OPL_LOG_INFO(APP, "File type: %d", g_tUploadFileCer->tFileType);
        OPL_LOG_INFO(APP, "File len %d", g_tUploadFileCer->u32DataLen);
    }
    else
    {
        g_tUploadFilePrvKey = (T_UploadFileStruct *) malloc(u32DataLen+1);
        memset(g_tUploadFilePrvKey, 0, u32DataLen + 1);   
        memcpy(g_tUploadFilePrvKey, pData, u32DataLen);

        OPL_LOG_INFO(APP, "File type: %d", g_tUploadFilePrvKey->tFileType);
        OPL_LOG_INFO(APP, "File len %d", g_tUploadFilePrvKey->u32DataLen);
    }
    
    g_u8UploadNum++;
}


//////////////////////////////////// 
//// APP function group
////////////////////////////////////

// add your application function here

T_OplErr APP_HostModeSendReq(T_HostModeReqCmdList tHostModeReqCmdIdx, at_cmd_mode_t tReqCmdMode, uint8_t *payload, uint32_t payloadlen)
{
    //int8_t i8AckMsg[UART_DATA_BUF_LEN] = {0};
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
    
    if(tHostModeReqCmdIdx < 7 || tHostModeReqCmdIdx > 9)
    {
        u32ReqMsgOffset += sprintf(cReqCmdPtr + u32ReqMsgOffset, "%s", (char *)i8HostModeReqCmdListStrTbl[tHostModeReqCmdIdx]);
    }
    else
    {
        u32ReqMsgOffset += sprintf(cReqCmdPtr + u32ReqMsgOffset, "%s", payload);
        //strcpy(&i8AckMsg[u32ReqMsgOffset], (char *)payload);
        //uint16_t len = strlen(&i8AckMsg[u32ReqMsgOffset]);
        //tracer_drct_printf("crt len %d\r\n", len);

        //u32ReqMsgOffset += len;
        //tracer_drct_printf("crt len %d\r\n", u32ReqMsgOffset);
    }

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

    if(tHostModeReqCmdIdx == 12 || tHostModeReqCmdIdx == 18)
    {
        osDelay(100);
    }

    g_tHostModeBackupReqCmdIdx = tHostModeReqCmdIdx;

    return OPL_OK;
}

T_OplErr APP_HostModeAckParser(uint8_t *u8RecvData, uint32_t u32RecvDataLen, T_HostModeAckCmdList *tHostModeAckCmdIdx, uint8_t *u8Result)
{
    uint8_t u8Count = 0;
    //int i;
    
    memset(g_u8UartRecvData, 0, sizeof(g_u8UartRecvData));
    memcpy(g_u8UartRecvData, u8RecvData, u32RecvDataLen);

    //for(i = 0; i<u32RecvDataLen ;i++)
    //{
    //    tracer_drct_printf("%02x ",g_u8UartRecvData[i]);
    //}
    //tracer_drct_printf("\n");

    if(g_u8UartRecvData[2] == 1) // command fail
    {
        return OPL_ERR;
    }

    uint8_t u8AckTagNum = g_u8UartRecvData[1];
    *tHostModeAckCmdIdx = (T_HostModeAckCmdList)u8AckTagNum;

    if(ACK_TAG_MAX == u8Count)
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
        /*
        case AT_CMD_ACK_BOOT_UP:
        {
            osTimerStop(g_tAppSleepSlaveTimer);
            osTimerStop(g_tAppWakeupSlaveTimer);

            break;
        }
        */
        case ACK_TAG_MODULE_READY:
        {
            OPL_LOG_INFO(APP, "DEVICE_READY\n");
            Hal_Uart_DataSend(UART_CONFIG_IDX, 0x0D);
            Hal_Uart_DataSend(UART_CONFIG_IDX, 0x0A);

            APP_WakeupPinInit();

            //OPL_LOG_INFO(APP, "Please enter MQTT three cerificates and host address");

            g_u8UploadNum = 0;
            break;
        }
        case ACK_TAG_SMART_SLEEP:
        {
            if(g_u8ExampleMode == 1)
            {
                // start wakeup salve timer after 8s
                osTimerStart(g_tAppWakeupSlaveTimer, 8000);
            }
            else
            {
                APP_SendMessage(APP_EVT_SMART_SLEEP, NULL, 0);
            }
            break;
        }
        case ACK_TAG_TIMER_SLEEP:
        case ACK_TAG_DEEP_SLEEP:
        {
            g_u8BleStatus = 0;
            g_u8WifiStatus = 0;
            if(g_u8ExampleMode == 1)
            {
                // start wakeup salve timer after 8s
                osTimerStart(g_tAppWakeupSlaveTimer, 8000);
            }
            else
            {
                APP_SendMessage(APP_EVT_SMART_SLEEP, NULL, 0);
            }

            break;
        }
        case ACK_TAG_MODULE_WAKEUP:
        {
            OPL_LOG_INFO(APP, "Slave wakeup from Smart sleep\n");
            
            if(g_u8ExampleMode == 1)
            {
                g_u8SleepMode = 2;
                osTimerStart(g_tAppSleepSlaveTimer, 1000); // enter timer sleep
            }
            break;
        }
        case ACK_TAG_TIMER_SLEEP_WAKEUP:
        {
            OPL_LOG_INFO(APP, "Slave wakeup from Timer wakeup\n");
            g_u8UploadNum = 0; //0219

            if(g_u8ExampleMode == 1)
            {
                g_u8SleepMode = 3;
                
                strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "0"); //auto adv
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_BLE_START;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;

                u8NeedToSend = 1;
            }
            //else if(g_u8ExampleMode == 6)
            //{
            //    OPL_LOG_INFO(APP, "------Exmaple 6 Done------");
            //}
            break;
        }
        case ACK_TAG_BLE_START_ADV_IND:
        {
            if(g_u8ExampleMode == 2)
            {
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_WIFI_SCAN;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_EXECUTION;

                u8NeedToSend = 1;
            }
            else
            {
                OPL_LOG_INFO(APP, "Please connect BLE");
            }
            break;
        }
        case ACK_TAG_BLE_CONNECTED_IND:
        {
            OPL_LOG_INFO(APP, "BLE connect\n");
            g_u8BleStatus = 1;
            if(g_u8ExampleMode != 2)
            {
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_WIFI_SCAN;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_EXECUTION;

                u8NeedToSend = 1;
            }
            else //g_u8ExampleMode == 2
            {
                g_u8SleepCount++;
                osTimerStart(g_tAppSleepSlaveTimer, 1000);
            }
            break;
        }
        case ACK_TAG_BLE_DISCONNECTED_IND:
        {
            OPL_LOG_INFO(APP, "BLE disconnect\n");
            g_u8BleStatus = 0;

            if(g_u8ExampleMode == 2)
            {
                if(g_u8SleepCount == 0)
                {
                    strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "0");
                    tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                    tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_BLE_START;
                    tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;

                    u8NeedToSend = 1;
                }
                else if(g_u8SleepCount == 2)
                {
                    //OPL_LOG_INFO(APP, "------Exmaple 2 Done------");
                    tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                    tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_DISC;
                    tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_EXECUTION;

                    u8NeedToSend = 1;
                }
                //else
                //{
                    //should not be here
                //}
            }
            break;
        }
        case ACK_TAG_WIFI_SCAN_REQ:
        { 
            int i;
            
            if(g_u8ApNum == 0) //Scan list ack message
            {
                g_u8ApNum = g_u8UartRecvData[3];
                OPL_LOG_INFO(APP, "AP num: %u\n", g_u8ApNum);
                tracer_drct_printf("BSSID\t\t\tCH\tRSSI\t\tSSID\t\t\n");
            }
            else //scan list AP message
            {   
                g_u8ApNum--;
                
                tracer_drct_printf( "%02x:%02x:%02x:%02x:%02x:%02x\t%d\t%d\t", 
                        g_u8UartRecvData[3], g_u8UartRecvData[4], g_u8UartRecvData[5],
                        g_u8UartRecvData[6], g_u8UartRecvData[7], g_u8UartRecvData[8],
                        g_u8UartRecvData[g_u8UartRecvData[9] + 10],
                        g_u8UartRecvData[g_u8UartRecvData[9] + 11] - 256);
                for(i = 0; i < g_u8UartRecvData[9]; i++)
                {
                    tracer_drct_printf("%c", g_u8UartRecvData[i + 10]);
                }
                tracer_drct_printf("\n");
                free(g_u8UartRecvData);

                if(g_u8ApNum == 0)
                { 
                    char ap_info[40] = {0};
                    strcpy(ap_info, AP_NAME);
                    
                    if(AP_PASSWORD != "")
                    {   
                        strcat(ap_info, ",");
                        strcat(ap_info, AP_PASSWORD);
                    }

                    strcpy((char *)tHostModeReqCmdFmt.u8aPayload, ap_info);
                    tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                    tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_WIFI_CONNECT;
                    tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;

                    u8NeedToSend = 1;
                }
            }

            break;
        }
        case ACK_TAG_WIFI_NETWORK_UP_IND:
        {
            g_u8WifiStatus = 1;

            OPL_LOG_INFO(APP, "Network Up");
            
            if(g_u8UploadNum == 0)
            {
                char rootca_info[20] = {0};
                sprintf(rootca_info, "%d,%d", g_tUploadFileCA->tFileType, g_tUploadFileCA->u32DataLen);      

                strcpy((char *)tHostModeReqCmdFmt.u8aPayload, rootca_info);
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_UPLOUD_FILE;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;

                APP_SendMessage(APP_EVT_AT_MSG_SEND, (uint8_t*)&tHostModeReqCmdFmt, sizeof(T_HostModeReqCmdFmt));

                //memset(tHostModeReqCmdFmt.u8aPayload, 0, 1800);
                //tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                //tracer_drct_printf("LEN1: %d\n",tHostModeReqCmdFmt.u16PayloadLen);
                strcpy((char *)tHostModeReqCmdFmt.u8aPayload, (char *)g_tUploadFileCA->pau8DataBuf);                

                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_FILE_ROOT_CA;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_EXECUTION;
            }
            else
            {
                strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "aws_test");
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_CILENT_ID;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;
            }

            u8NeedToSend = 1;
            
            break;
        }
        case ACK_TAG_WIFI_NETWORK_DOWN_IND:
        case ACK_TAG_WIFI_NETWORK_RESET_IND:
        {
            OPL_LOG_INFO(APP, "Wifi disconnect\n");
            g_u8WifiStatus = 0;

            break;
        }
        case ACK_TAG_CLOUD_MQTT_PUB_DATA_IND:
        {
            g_u8SleepCount++;
            osTimerStart(g_tAppSleepSlaveTimer, 5000);

            break;
        }
        case ACK_TAG_CLOUD_MQTT_RECV_DATA_IND:
        {
            uint8_t i;
            uint8_t topic_name_len = 0;
            uint8_t payload_len = 0;

            topic_name_len = g_u8UartRecvData[3];
            payload_len = g_u8UartRecvData[topic_name_len + 5];

            tracer_drct_printf("MQTT recv data:\n");
            tracer_drct_printf("Topic name:");
            for(i = 0;i < topic_name_len ;i++)
            {
                tracer_drct_printf("%c",g_u8UartRecvData[i + 5]);
            }
            tracer_drct_printf("\n");
            tracer_drct_printf("Payload:");
            for(i = 0;i < payload_len ;i++)
            {
                tracer_drct_printf("%c",g_u8UartRecvData[i + topic_name_len + 7]);
            }
            tracer_drct_printf("\n");

            if(g_u8ExampleMode == 5)
            {
                //OPL_LOG_INFO(APP, "------Example 5 Done------\n");
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_DISC;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_EXECUTION;

                u8NeedToSend = 1;
            }
            //else
            //{
            //    g_u8SleepCount++;
            //    osTimerStart(g_tAppSleepSlaveTimer, 5000);
            //}

            break;
        }
        case ACK_TAG_FILE_UPLOAD:
        {
            g_u8UploadNum++;

            if(g_u8UploadNum == 1)
            {
                char cer_info[20] = {0}; //client certificate
                sprintf(cer_info, "%d,%d", g_tUploadFileCer->tFileType, g_tUploadFileCer->u32DataLen);

                strcpy((char *)tHostModeReqCmdFmt.u8aPayload, cer_info);
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_UPLOUD_FILE;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;

                APP_SendMessage(APP_EVT_AT_MSG_SEND, (uint8_t*)&tHostModeReqCmdFmt, sizeof(T_HostModeReqCmdFmt));    

                strcpy((char *)tHostModeReqCmdFmt.u8aPayload, (char *)g_tUploadFileCer->pau8DataBuf);
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_FILE_CERTIFICATE;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_EXECUTION;
            }
            else if(g_u8UploadNum == 2)
            {
                char prvkey_info[20] = {0}; //client certificate
                sprintf(prvkey_info, "%d,%d", g_tUploadFilePrvKey->tFileType, g_tUploadFilePrvKey->u32DataLen);

                strcpy((char *)tHostModeReqCmdFmt.u8aPayload, prvkey_info);
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_UPLOUD_FILE;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;

                APP_SendMessage(APP_EVT_AT_MSG_SEND, (uint8_t*)&tHostModeReqCmdFmt, sizeof(T_HostModeReqCmdFmt));    

                strcpy((char *)tHostModeReqCmdFmt.u8aPayload, (char *)g_tUploadFilePrvKey->pau8DataBuf);
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_FILE_PRIVATE_KEY;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_EXECUTION;                
            }
            else if(g_u8UploadNum == 3) //upload done
            {
                strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "aws_test");
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_CILENT_ID;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;
            }
            else
            {
                // should not go in here
                OPL_LOG_WARN(APP, "Error\n")
            }

            u8NeedToSend = 1;
            break;
        }
        case ACK_TAG_CLOUD_MQTT_CLIENTID_SET:
        {
            OPL_LOG_INFO(APP, "Set client Id done\n");

            strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "300000");
            tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
            tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_KEEP_ALIVE;
            tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;

            u8NeedToSend = 1;
            break;
        }
        case ACK_TAG_CLOUD_MQTT_KEEP_ALIVE_SET_REQ:
        {
            OPL_LOG_INFO(APP, "Keep Alive Set Done\n");

            strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "AWSIOTLWTTEST,22");
            tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
            tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_LAST_WILL;
            tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;

            APP_SendMessage(APP_EVT_AT_MSG_SEND, (uint8_t*)&tHostModeReqCmdFmt, sizeof(T_HostModeReqCmdFmt));

            tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
            tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_LAST_WILL_MSG;
            tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_EXECUTION;

            u8NeedToSend = 1;
            break;
        }
        case ACK_TAG_CLOUD_MQTT_LASTWILL_SET:
        {
            OPL_LOG_INFO(APP, "Last Will Set Done\n");
            tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
            tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_INIT;
            tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_EXECUTION;

            u8NeedToSend = 1;
            break;
        }
        case ACK_TAG_CLOUD_MQTT_INIT_IND:
        {
            OPL_LOG_INFO(APP, "Cloud Init Done\n");

            char mqtt_info[200] = {0};
            sprintf(mqtt_info, "%s,%d,%d", g_tCloudConnInfo.u8aHostAddr, g_tCloudConnInfo.u16HostPort, g_tCloudConnInfo.u8AutoConn);         

            strcpy((char *)tHostModeReqCmdFmt.u8aPayload, mqtt_info);
            tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
            tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_CONN;
            tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;

            u8NeedToSend = 1;
            break;
        }
        case ACK_TAG_CLOUD_MQTT_CONNECT_IND:
        {
            // register tx topic
            strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "QD_FWK/MQTT_DEMO/SUB_Test,1");
            tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
            tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_SUB_TOPIC;
            tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;

            u8NeedToSend = 1;

            break;
        }
        case ACK_TAG_CLOUD_MQTT_SUB_TOPIC_IND:
        {
            OPL_LOG_INFO(APP, "Subscribe Topic Success\n");

            if(g_u8ExampleMode == 5)
            {
                g_u8SleepCount++;
                osTimerStart(g_tAppSleepSlaveTimer, 5000);
            }
            else
            {
                strcpy((char *)tHostModeReqCmdFmt.u8aPayload, "QD_FWK/MQTT_DEMO/PUB_Test,1,5");
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_PUBLISH;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_SET;

                APP_SendMessage(APP_EVT_AT_MSG_SEND, (uint8_t*)&tHostModeReqCmdFmt, sizeof(T_HostModeReqCmdFmt));

                osDelay(200);
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_CLOUD_PUBLISH_MSG;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_EXECUTION;

                u8NeedToSend = 1;
            }

            break;
        }
        case ACK_TAG_CLOUD_MQTT_DISCON_IND:
        {
            OPL_LOG_INFO(APP, "MQTT disconnect\n");
            g_u8UploadNum = 0;

            //if(g_u8ExampleMode == 3 || g_u8ExampleMode == 4)
            //{
            //    OPL_LOG_INFO(APP, "------Example %d Done------\n",g_u8ExampleMode);
            //}

            if(g_u8ExampleMode == 2 || g_u8ExampleMode == 4 || g_u8ExampleMode == 5)
            {
                tHostModeReqCmdFmt.u16PayloadLen = strlen((char *)tHostModeReqCmdFmt.u8aPayload);
                tHostModeReqCmdFmt.tHostModeReqCmdIdx = AT_CMD_REQ_WIFI_DISC;
                tHostModeReqCmdFmt.tAtMode = AT_CMD_MODE_EXECUTION;

                u8NeedToSend = 1;
            }
            break;
        }
        case ACK_TAG_HOST_READY:
        {
            //if(g_u8ExampleMode == 2 && g_u8SleepCount == 1)
            //{
            //    g_u8SleepCount++;
            //    osTimerStart(g_tAppSleepSlaveTimer, 1000);
            //}
            break;
        }
        case ACK_TAG_MAX:
        case ACK_TAG_INVALID:
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
#if (EXT_PA_ENABLED == 1)
    // Do not overwrite RF power setting if external PA enable
#else
    // initialize rf power setting
    RF_PwrSet(RF_CFG_DEF_PWR_SET);
#endif

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

void APP_UserAtInit(void)
{
    AT_CmdListAdd(1);
}

void APP_WakeupPinInit(void)
{
    uint32_t u32PinLevel = 0;

    // Get the status of GPIO (Low / High)
    u32PinLevel = Hal_Vic_GpioInput(HOST_MODE_WAKEUP_HOST_PIN);

    if(GPIO_LEVEL_LOW == u32PinLevel)
    {
        Hal_Vic_GpioCallBackFuncSet(HOST_MODE_WAKEUP_HOST_PIN, (T_Gpio_CallBack)APP_TriggerBySlaveHandler);
        Hal_Vic_GpioDirection(HOST_MODE_WAKEUP_HOST_PIN, GPIO_INPUT);
        Hal_Vic_GpioIntTypeSel(HOST_MODE_WAKEUP_HOST_PIN, INT_TYPE_RISING_EDGE);
        Hal_Vic_GpioIntInv(HOST_MODE_WAKEUP_HOST_PIN, 0);
        Hal_Vic_GpioIntMask(HOST_MODE_WAKEUP_HOST_PIN, 0);
        Hal_Vic_GpioIntEn(HOST_MODE_WAKEUP_HOST_PIN, 1);
    }
    else
    {
        Hal_Vic_GpioCallBackFuncSet(HOST_MODE_WAKEUP_HOST_PIN, (T_Gpio_CallBack)APP_TriggerBySlaveHandler);
        Hal_Vic_GpioDirection(HOST_MODE_WAKEUP_HOST_PIN, GPIO_INPUT);
        Hal_Vic_GpioIntTypeSel(HOST_MODE_WAKEUP_HOST_PIN, INT_TYPE_FALLING_EDGE);
        Hal_Vic_GpioIntInv(HOST_MODE_WAKEUP_HOST_PIN, 1);
        Hal_Vic_GpioIntMask(HOST_MODE_WAKEUP_HOST_PIN, 0);
        Hal_Vic_GpioIntEn(HOST_MODE_WAKEUP_HOST_PIN, 1);
    }
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

    APP_UserAtInit();
    
    // enter smart sleep after 5s
#if (PS_ENABLED == 1)
    PS_EnterSmartSleep(5000);
#endif

}
