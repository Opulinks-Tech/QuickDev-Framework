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
#include "ble_mngr.h"
#include "cmsis_os.h"
#include "opl_data_prot.h"
#include "opl_data_hdl.h"
#include "opl_svc.h"
#include "ota_mngr.h"
#include "gap_svc.h"
#include "gatt_svc.h"
#include "log.h"
#include "net_mngr_api.h"
#include "pwr_save.h"
#include "rf_pwr.h"
#include "wifi_mngr_api.h"
#include "app_at_cmd.h"
#include "ble_gap_if.h"
#include "ble_hci_if.h"
#include "wifi_agent.h"
#include "ps_public.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous
#define BleAdvIntvTransform     (1.6)
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

#if (OPL_DATA_CURRENT_MEASURE_ENABLED == 1)
osTimerId g_tAppWakeupTimer;
bool bBleUserPC = false; // User want to impl power consumption test 
uint8_t g_u8SleepModeStatus;
T_BlePCStruct g_tPcStruct = {0};
#endif

static T_AppEvtHandlerTbl g_tAppEvtHandlerTbl[] = 
{
    {APP_EVT_BLE_START_ADV,                 APP_EvtHandler_BleStartAdv},
    {APP_EVT_BLE_STOP_ADV,                  APP_EvtHandler_BleStopAdv},
    {APP_EVT_BLE_CONNECTED,                 APP_EvtHandler_BleConnected},
    {APP_EVT_BLE_DISCONNECTED,              APP_EvtHandler_BleDisconnected},
    {APP_EVT_BLE_DATA_IND,                  APP_EvtHandler_BleDataInd},
#if (OPL_DATA_CURRENT_MEASURE_ENABLED == 1)
    {APP_EVT_BLE_POWER_CONSUMPTION,         APP_EvtHandler_BlePowerConsumptionInd}, 
    {APP_EVT_WAKEUP_TIMER_START,            APP_EvtHandler_WakeUp},  
#endif
    {APP_EVT_NETWORK_UP,                    APP_EvtHandler_NetworkUp},
    {APP_EVT_NETWORK_DOWN,                  APP_EvtHandler_NetworkDown},
    {APP_EVT_NETWORK_RESET,                 APP_EvtHandler_NetworkReset},

    {APP_EVT_SYS_TIMER_TIMEOUT,             APP_EvtHandler_SysTimerTimeout},

    {0xFFFFFFFF,                            NULL},
};

// Sec 7: declaration of static function prototype

#if (OPL_DATA_CURRENT_MEASURE_ENABLED == 1)
static void Ble_Para_Set(uint16_t u16BleAdv_itvl);
static T_OplErr SkipDtimSet(uint32_t SkipDtim);
static void APP_Sleep_Check(void);
#endif

void APP_BleAdvDataInit(void);
void APP_BleScanRspDataInit(void);
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

#if (OPL_DATA_CURRENT_MEASURE_ENABLED == 1)
static void APP_WakeupCb(PS_WAKEUP_TYPE type)
{
    APP_SendMessage(APP_EVT_WAKEUP_TIMER_START, NULL, 0);
}

static void APP_WakeupTimeoutHandler(void const *argu)
{
    OPL_LOG_INFO(APP, "Timer Sleep");
    ps_timer_sleep(g_tPcStruct.u32SleepTime);
}
#endif

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

void APP_BleUnsolicitedCallback(uint16_t u16EvtType, T_OplErr tEvtRst, uint8_t *pu8Data, uint32_t u32DataLen)
{
    switch(u16EvtType)
    {
        case USLCTED_CB_EVT_BLE_INIT:
        {
            // initialize ble advertise data
            APP_BleAdvDataInit();

            // initialize ble scan response data
            APP_BleScanRspDataInit();

            break;
        }
        case USLCTED_CB_EVT_BLE_ENT_ADVERTISE:
        {
            APP_SendMessage(APP_EVT_BLE_START_ADV, NULL, 0);

            break;
        }
        case USLCTED_CB_EVT_BLE_EXI_ADVERTISE:
        {
            APP_SendMessage(APP_EVT_BLE_STOP_ADV, NULL, 0);

            break;
        }
        case USLCTED_CB_EVT_BLE_CONNECTED:
        {
            APP_SendMessage(APP_EVT_BLE_CONNECTED, pu8Data, u32DataLen);

            break;
        }
        case USLCTED_CB_EVT_BLE_DISCONNECT:
        {
            APP_SendMessage(APP_EVT_BLE_DISCONNECTED, NULL, 0);

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

//////////////////////////////////// 
//// Event handler group
////////////////////////////////////

// add your event handler function here

static void APP_EvtHandler_BleStartAdv(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    uint8_t u8aBleMac[6] = {0};
    Opl_Ble_MacAddr_Read(u8aBleMac);

    OPL_LOG_INFO(APP, "BLE advertising...Device mac [%0X:%0X:%0X:%0X:%0X:%0X]", u8aBleMac[0],
                                                                                u8aBleMac[1],
                                                                                u8aBleMac[2],
                                                                                u8aBleMac[3],
                                                                                u8aBleMac[4],
                                                                                u8aBleMac[5]);
}

static void APP_EvtHandler_BleStopAdv(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "BLE stop advertise");
}

static void APP_EvtHandler_BleConnected(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    uint8_t u8aBlePeerMac[6] = {0};
    memcpy(u8aBlePeerMac, pData, sizeof(u8aBlePeerMac));

    OPL_LOG_INFO(APP, "BLE connected...Peer mac [%0X:%0X:%0X:%0X:%0X:%0X]", u8aBlePeerMac[0],
                                                                            u8aBlePeerMac[1],
                                                                            u8aBlePeerMac[2],
                                                                            u8aBlePeerMac[3],
                                                                            u8aBlePeerMac[4],
                                                                            u8aBlePeerMac[5]);
}

static void APP_EvtHandler_BleDisconnected(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "BLE disconnected");

#if (OPL_DATA_CURRENT_MEASURE_ENABLED == 1)
    if(true == bBleUserPC)
    {
        APP_Sleep_Check();    
    }
#endif    
}

static void APP_EvtHandler_BleDataInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_DataRecvHandler(pData, (uint16_t)u32DataLen);
}

#if (OPL_DATA_CURRENT_MEASURE_ENABLED == 1)
static void APP_EvtHandler_BlePowerConsumptionInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    memcpy(&g_tPcStruct, pData, u32DataLen);

    switch(g_tPcStruct.u8SleepMode)
    {
        case SMART_SLEEP:
        {
            //Turn off BLE
            if(OPL_OK == Opl_Ble_Stop_Req()) //wait response
            {
                OPL_LOG_INFO(APP, "BLE Off Request Success");
            }
            else
            {
                OPL_LOG_INFO(APP, "BLE Off Request Fail");
                return;
            }

            break;
        }
        
        case TIMER_SLEEP:
        {
            //Turn off BLE
            if(OPL_OK == Opl_Ble_Stop_Req()) //wait response OK
            {
                OPL_LOG_INFO(APP, "BLE Off Request Success");
            }
            else
            {
                OPL_LOG_INFO(APP, "BLE Off Request Fail");
                return;
            }

            //Turn off Wi-Fi  //wait response OK
            if(OPL_OK == Opl_Wifi_AC_Disable_Req(true, NULL))
            {
                OPL_LOG_INFO(APP, "Wi-Fi Off Request Success");
            }
            else
            {
                OPL_LOG_INFO(APP, "Wi-Fi Off Request Fail");
                return;
            }

            break;
        }

        case DEEP_SLEEP:
        {
            //Turn off BLE
            if(OPL_OK == Opl_Ble_Stop_Req()) //wait response OK
            {
                OPL_LOG_INFO(APP, "BLE Off Request Success");
            }
            else
            {
                OPL_LOG_INFO(APP, "BLE Off Request Fail");
                return;
            }

            //Turn off Wi-Fi  //wait response OK
            if(OPL_OK == Opl_Wifi_AC_Disable_Req(true, NULL))
            {
                OPL_LOG_INFO(APP, "Wi-Fi Off Request Success");
            }
            else
            {
                OPL_LOG_INFO(APP, "Wi-Fi Off Request Fail");
                return;
            }

            break;
        }        
    }
    bBleUserPC = true; //User request current management flag 
}
#endif /*OPL_DATA_CURRENT_MEASURE_ENABLED*/

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

    int8_t rssi = 0;
    wifi_connection_get_rssi(&rssi);
    OPL_LOG_INFO(APP, "RSSI %d", rssi);
}

static void APP_EvtHandler_NetworkDown(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "Network disconnected");

#if (OPL_DATA_CURRENT_MEASURE_ENABLED == 1)
    if(true == bBleUserPC)
    {
        APP_Sleep_Check();    
    }
#endif
}

static void APP_EvtHandler_NetworkReset(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "Network reset");
}

#if (OPL_DATA_CURRENT_MEASURE_ENABLED == 1)
static void APP_EvtHandler_WakeUp(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "Wakeup time = %d", g_tPcStruct.u32WakeupTime);
    osTimerStart(g_tAppWakeupTimer, g_tPcStruct.u32WakeupTime);
}
#endif

static void APP_EvtHandler_SysTimerTimeout(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "Hello world !");
}

//////////////////////////////////// 
//// APP function group
////////////////////////////////////

// add your application function here
#if (OPL_DATA_CURRENT_MEASURE_ENABLED == 1)
static void APP_Sleep_Check(void)
{
    switch(g_tPcStruct.u8SleepMode)
    {
        case SMART_SLEEP:
        {
            if(BM_ST_IDLE == Opl_Ble_EntityGet()->u16CurrentState) //Maybe not check BLE IDLE
            {
                SkipDtimSet(g_tPcStruct.u32DtimPeriod);

                //Check Ble adv on/off
                if(g_tPcStruct.u8BleAdvOnOff)
                {
                    //Transform ble adv intv for ble spec (Because g_tPcStruct.u16BleAdvIntv (unit is 1ms)) 
                    g_tPcStruct.u16BleAdvIntv = g_tPcStruct.u16BleAdvIntv * BleAdvIntvTransform;
                    
                    //Set BLE adv parameter
                    Ble_Para_Set(g_tPcStruct.u16BleAdvIntv);

                    OPL_LOG_INFO(APP, "BLE ADV ON");
                    Opl_Ble_Start_Req(true);    
                }

                //Enter smart sleep directly
                OPL_LOG_INFO(APP, "Smart Sleep");
                ps_smart_sleep(1); 
            }
            
            break;
        }

        case TIMER_SLEEP:
        {
            //Check BLE & Wi-Fi module are both Idle
            if(BM_ST_IDLE == Opl_Ble_EntityGet()->u16CurrentState && WA_ST_IDLE == WM_WaFsmDefGet()->ptFsmStateInfo.tCurrentState)
            {
                osTimerDef_t tTimerDef;

                tTimerDef.ptimer = APP_WakeupTimeoutHandler;
                g_tAppWakeupTimer = osTimerCreate(&tTimerDef, osTimerOnce, NULL);
                if(g_tAppWakeupTimer == NULL)
                {
                    OPL_LOG_ERRO(APP, "Create timer fail");
                }
                ps_smart_sleep(false); //turn off smart sleep to avoid simultaneous operation
                ps_set_wakeup_cb(APP_WakeupCb);
                OPL_LOG_INFO(APP, "Timer Sleep");
                ps_timer_sleep(g_tPcStruct.u32SleepTime);

                break;
            }
            else
            {
                return;
            }
        }

        case DEEP_SLEEP:
        {
            //Check BLE & Wi-Fi module are both Idle
            if(BM_ST_IDLE == Opl_Ble_EntityGet()->u16CurrentState && WA_ST_IDLE == WM_WaFsmDefGet()->ptFsmStateInfo.tCurrentState)
            {
                OPL_LOG_INFO(APP, "Deep Sleep"); 
                ps_smart_sleep(false); //turn off smart sleep to avoid simultaneous operation
                ps_deep_sleep();

                break;
            }
            else
            {
                return;
            }
        }
    }

    bBleUserPC          = false;
}

static void Ble_Para_Set(uint16_t u16BleAdv_itvl)
{
    LE_GAP_ADVERTISING_PARAM_T para;

    para.interval_min = u16BleAdv_itvl;
    para.interval_max = u16BleAdv_itvl;
    para.type = LE_HCI_ADV_TYPE_ADV_IND;
    para.own_addr_type = LE_HCI_OWN_ADDR_PUBLIC;
    para.peer_addr_type = LE_HCI_ADV_PEER_ADDR_PUBLIC;
    MemSet(para.peer_addr, 0, 6);
    para.channel_map = 0x7;
    para.filter_policy = LE_HCI_ADV_FILT_NONE;


    printf("Min_intv=%d, Max_intv=%d\n", para.interval_min, para.interval_max);
    LeGapSetAdvParameter(&para);
}

static T_OplErr SkipDtimSet(uint32_t SkipDtim)
{
    uint32_t u32DtimInterval = 0;
    uint32_t u32BeaconInterval = 0;
    wifi_scan_info_t tInfo;

    // get the information of Wifi AP
    if(0 != wifi_sta_get_ap_info(&tInfo))
    {
        //WM_LOG_ERRO("Get AP info fail");
    }

    // beacon time (ms)
    u32BeaconInterval = tInfo.beacon_interval * tInfo.dtim_period;

    OPL_LOG_INFO(APP, "Beacon intvl %d", u32BeaconInterval);

    // error handle
    if (u32BeaconInterval == 0)
    {
        u32BeaconInterval = 100;
    }

    //set dtim
    u32DtimInterval = (SkipDtim + (u32BeaconInterval / 2)) / u32BeaconInterval;

    if(u32DtimInterval > 0)
    {
        u32DtimInterval = u32DtimInterval - 1;
    }

    if (u32DtimInterval > 255)
    {
        u32DtimInterval = 255;
    }

    OPL_LOG_INFO(APP, "Dtim intvl %d", u32DtimInterval);
    
    if (0 != wifi_config_set_skip_dtim(u32DtimInterval, false))
    {
        //WM_LOG_ERRO("Dtim skip intvl set fail");

        return OPL_ERR;
    }

    return OPL_OK;
}
#endif /*OPL_DATA_CURRENT_MEASURE_ENABLED*/

void APP_BleAdvDataInit(void)
{
    // ble advertise data inititate

    // user modify
    // *

    uint8_t u8Len;
    uint8_t au8BleAdvertData[] =
    {
        0x02,
        GAP_ADTYPE_FLAGS,
        GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
        // connection interval range
        0x05,
        GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE, 
        UINT16_LO(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
        UINT16_HI(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
        UINT16_LO(DEFAULT_DESIRED_MAX_CONN_INTERVAL),
        UINT16_HI(DEFAULT_DESIRED_MAX_CONN_INTERVAL),
        0x02,
        GAP_ADTYPE_POWER_LEVEL,
        0,
        0x11,
        GAP_ADTYPE_128BIT_COMPLETE,
        0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00
    };

    // error handle
    u8Len = sizeof(au8BleAdvertData);

    if (u8Len > BLE_ADV_SCAN_BUF_SIZE)
    {
        u8Len = BLE_ADV_SCAN_BUF_SIZE;
    }

    Opl_Ble_Advertise_Data_Set(au8BleAdvertData, u8Len);

    // *
}

void APP_BleScanRspDataInit(void)
{
    // ble scan response data inititate

    // user modify
    // *

    char u8aBleName[31];
    uint8_t u8BleNameLen = 0;
    uint8_t au8BleScanRspData[BLE_ADV_SCAN_BUF_SIZE];
    uint8_t u8aBleMac[6] = {0};

    Opl_Ble_MacAddr_Read(u8aBleMac);

    sprintf(u8aBleName, "%s_%0X:%0X:%0X:%0X", BLE_GAP_PF_DEVICE_NAME, 
                                              u8aBleMac[2],
                                              u8aBleMac[3],
                                              u8aBleMac[4],
                                              u8aBleMac[5]);

    u8BleNameLen = strlen(u8aBleName);

    au8BleScanRspData[0] = (u8BleNameLen + 1);
    au8BleScanRspData[1] = GAP_ADTYPE_LOCAL_NAME_COMPLETE;

    MemCopy((au8BleScanRspData + 2), u8aBleName, u8BleNameLen);

    if(OPL_OK != Opl_Ble_ScanRsp_Data_Set(au8BleScanRspData, (u8BleNameLen + 2)))
    {
        OPL_LOG_ERRO(APP, "Scan Rsp Data Set Fail");
    }

    // *
}

void APP_SysInit(void)
{
    // initialize rf power setting
    RF_PwrSet(RF_CFG_DEF_PWR_SET);

    // initialize ota manager
    OTA_Init();

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
    g_tAppSysTimer = osTimerCreate(&tTimerDef, osTimerPeriodic, NULL);
    if(g_tAppSysTimer == NULL)
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
}

void APP_BleInit(void)
{
    // assign unsolicited callback function
    Opl_Ble_Uslctd_CB_Reg(&APP_BleUnsolicitedCallback);

    // register service
    GAP_Svc_Init();
    GATT_Svc_Init();

    // register opl service
    OPL_Svc_Init();

    // initialize the ble manager (auto-adv)
    Opl_Ble_Init_Req(true);

    // user implement
}

void APP_NetInit(void)
{
    // Network manager initialize (auto-connect enable)
    APP_NmInit(true, &APP_NmUnsolicitedCallback);

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

    // start periodic timer
    osTimerStart(g_tAppSysTimer, 10000);
}
