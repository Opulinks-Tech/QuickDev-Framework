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

// #include "devinfo_svc.h"
#include "gap_svc.h"
#include "log.h"
#include "wifi_mngr_api.h"
#include "net_mngr_api.h"
#include "opl_data_hdl.h"
#include "opl_data_prot.h"
#include "opl_svc.h"
#include "ota_mngr.h"
#include "pwr_save.h"
#include "rf_pwr.h"
#include "wifi_mngr_api.h"
//#include "transfer.h"
#include "http_file.h"
//#include "http_ota.h"
#include "evt_group.h"


#if(OPL_DATA_ENABLED == 1)
#include "gatt_svc.h"
#endif

#if(CLOUD_ENABLED == 1)
#include "cloud_ctrl.h"
#include "cloud_kernel.h"
#endif

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous
#if defined(OPL2500_A0)
#define WAKEUP_PIN                                  (GPIO_IDX_09)
#define WAKEUP_HOST_PIN                             (GPIO_IDX_28) 
#else
#define WAKEUP_PIN                                  (GPIO_IDX_10)
#define WAKEUP_HOST_PIN                             (GPIO_IDX_23) 
#endif

#define APP_EG_BIT_WIFI_READY                       (0x00000001U)
#define APP_EG_BIT_BLE_READY                        (0x00000002U)
#define APP_MAX_QUEUE_SIZE                          (4)
#define TRIGGER_HOST_GPIO_RISING_EDGE               (1)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

T_AppAckQueue *g_tAppAckQueue;
T_AppHostState g_tAppHostState = APP_HOST_ST_NOT_WAIT; 
T_AppSleepInfo g_tPcStruct = {0};
#if(AWS_ENABLED == 1)
T_CloudPayloadFmt tCloudPayloadFmt = {0}; //for cloud ack info
#endif
osTimerId g_tAppWakeupTimer;
osTimerId g_tAppWaitHostRspTimer;
bool bSleepWillingness = false; // User want to enter sleep mode
bool bBleOffReqDone = false; // hen Ble off call back done will be true
bool bWifiOffReqDone = false;
bool bCloudOff = true;


// Sec 5: declaration of global function prototype

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

osThreadId g_tAppTaskId;
osMessageQId g_tAppQueueId;
osTimerId g_tAppSysTimer;


SHM_DATA static T_AppEvtHandlerTbl g_tAppEvtHandlerTbl[] = 
{
    {APP_EVT_BLE_START_ADV_REQ,             APP_EvtHandler_BleStateReq},
    {APP_EVT_BLE_STOP_REQ,                  APP_EvtHandler_BleStateReq},
    {APP_EVT_BLE_INIT_IND,                  APP_EvtHandler_BleStateRsp},
    {APP_EVT_BLE_START_ADV_IND,             APP_EvtHandler_BleStateRsp},
    {APP_EVT_BLE_STOP_IND,                  APP_EvtHandler_BleStateRsp},
    {APP_EVT_BLE_CONNECTED_IND,             APP_EvtHandler_BleStateRsp},
    {APP_EVT_BLE_DISCONNECTED_IND,          APP_EvtHandler_BleStateRsp},
    {APP_EVT_BLE_RECV_DATA_IND,             APP_EvtHandler_BleRecvDataRsp},
#if(OPL_DATA_ENABLED == 1)
    {APP_EVT_BLE_DATA_IND,                  APP_EvtHandler_BleDataInd},
#endif

    {APP_EVT_WIFI_SCAN_REQ,                 APP_EvtHandler_WifiScanReq},
    // {APP_EVT_WIFI_SCAN_LIST_GET_REQ,        APP_EvtHandler_WifiScanReq},
    {APP_EVT_WIFI_CONNECT_REQ,              APP_EvtHandler_WifiConnectReq},
    {APP_EVT_WIFI_DISCONNECT_REQ,           APP_EvtHandler_WifiDisconnectReq},
    {APP_EVT_WIFI_SCAN_DONE_IND,            APP_EvtHandler_WifiNetworkStateRsp},
    {APP_EVT_WIFI_NETWORK_INIT_IND,         APP_EvtHandler_WifiNetworkStateRsp},
    {APP_EVT_WIFI_NETWORK_UP_IND,           APP_EvtHandler_WifiNetworkStateRsp},
    {APP_EVT_WIFI_NETWORK_DOWN_IND,         APP_EvtHandler_WifiNetworkStateRsp},
    {APP_EVT_WIFI_NETWORK_RESET_IND,        APP_EvtHandler_WifiNetworkStateRsp},

#if(CLOUD_ENABLED == 1)
    {APP_EVT_CLOUD_MQTT_INIT_REQ,           APP_EvtHandler_CloudMqttInitReq},
    {APP_EVT_CLOUD_MQTT_ESTAB_REQ,          APP_EvtHandler_CloudMqttEstablishReq},
    {APP_EVT_CLOUD_MQTT_DISCONN_REQ,        APP_EvtHandler_CloudMqttDisconnectReq},
    {APP_EVT_CLOUD_MQTT_SUB_TOPIC_REQ,      APP_EvtHandler_CloudMqttSubTopicReq},
    {APP_EVT_CLOUD_MQTT_UNSUB_TOPIC_REQ,    APP_EvtHandler_CloudMqttUnSubTopicReq},
    {APP_EVT_CLOUD_MQTT_PUB_DATA_REQ,       APP_EvtHandler_CloudMqttPubDataReq},
    {APP_EVT_CLOUD_MQTT_INIT_IND,           APP_EvtHandler_CloudMqttStateRsp},
    {APP_EVT_CLOUD_MQTT_CONNECT_IND,        APP_EvtHandler_CloudMqttStateRsp},
    {APP_EVT_CLOUD_MQTT_DISCONN_IND,        APP_EvtHandler_CloudMqttStateRsp},
    {APP_EVT_CLOUD_MQTT_SUB_TOPIC_IND,      APP_EvtHandler_CloudMqttStateRsp},
    {APP_EVT_CLOUD_MQTT_UNSUB_TOPIC_IND,    APP_EvtHandler_CloudMqttStateRsp},
    {APP_EVT_CLOUD_MQTT_PUB_DATA_IND,       APP_EvtHandler_CloudMqttStateRsp},
    {APP_EVT_CLOUD_RECV_DATA_IND,           APP_EvtHandler_CloudMqttRecvDataRsp},
#endif

    //other
#if(OTA_ENABLED == 1)
    {APP_EVT_OTA_START_IND,                 APP_EvtHandler_OtaStateRsp},
    //{APP_EVT_OTA_DONE_IND,                  APP_EvtHandler_OtaStateRsp},
    {APP_EVT_OTA_FAIL_IND,                  APP_EvtHandler_OtaStateRsp},
#endif

    {APP_EVT_TIMER_SLEEP_TIMEOUT_WAKEUP,    APP_EvtHandler_TimerSleepTimeoutWakeup},
    {APP_EVT_PREPARE_SLEEP,                 APP_EvtHandler_PrepareSleep},
    {APP_EVT_WIFI_STOP_SLEEP_CHECK,         APP_EvtHandler_WifiStopSleepCheckHandler},
    {APP_EVT_IO_EXIT_SMART_SLEEP_IND,       APP_EvtHandler_IoExitSmartSleepRsp},
    {APP_EVT_IO_EXIT_TIMER_SLEEP_IND,       APP_EvtHandler_IoExitTimerSleepRsp},
    {APP_EVT_HOST_READY_IND,                APP_EvtHandler_HostReadyRsp},

    {APP_EVT_SYS_TIMER_TIMEOUT,             APP_EvtHandler_SysTimerTimeout},
    {APP_EVT_WAIT_HOST_TIMER_TIMEOUT,       APP_EvtHandler_WaitHostTimerTimeout},

    {0xFFFFFFFF,                            NULL},
};

EventGroupHandle_t g_tAppEventGroup;

// Sec 7: declaration of static function prototype

T_OplErr APP_ResponseScanList(T_AckTag tAckTagId);

void APP_TriggerHostWakeUp(void);
void APP_BleAdvDataInit(void);
void APP_BleScanRspDataInit(void);
void APP_SysInit(void);
void APP_DataInit(void);
void APP_TaskInit(void);
void APP_BleInit(void);
void APP_NetInit(void);
void APP_CldInit(void);
void APP_UserAtInit(void);

T_AppAckQueue *APP_NewQueue(void);
bool APP_QueueIsEmpty(T_AppAckQueue *q);
void APP_EnQueue(T_AppAckQueue *q, T_AckTag AckTag, T_AckStatus AckStatus, uint8_t *AckDataOne, uint16_t AckDataOneLen, uint8_t *AckDataTwo, uint16_t AckDataTwoLen);
void APP_DeQueue(T_AppAckQueue *q, bool bDeQueueAll);
static void APP_Sleep_Check(void);
/***********
C Functions
***********/
// Sec 8: C Functions

//////////////////////////////////// 
//// Queue Func
//////////////////////////////////// 

T_AppAckQueue *APP_NewQueue(void)
{
    T_AppAckQueue *q = malloc(sizeof(T_AppAckQueue));

    q->AckQueueHead = NULL;
    q->AckQueueTail = NULL;
    q->AckQueueSize = 0;

    return q;
}

bool APP_QueueIsEmpty(T_AppAckQueue *q)
{
    if(q->AckQueueHead)
    {
        return false;
    }

    return true;
}

void APP_EnQueue(T_AppAckQueue *q, T_AckTag AckTag, T_AckStatus AckStatus, uint8_t *AckDataOne, uint16_t AckDataOneLen, uint8_t *AckDataTwo, uint16_t AckDataTwoLen)
{
    T_AppAckInfoStruct *newNode = malloc(sizeof(T_AppAckInfoStruct));

    // Cloud recv data for transfer_MqttRecivedData
    if(AckTag == ACK_TAG_CLOUD_MQTT_RECV_DATA_IND)
    {
        newNode->AckTag = AckTag;
        newNode->AckStatus = AckStatus;
        newNode->TopicName = AckDataOne;
        newNode->TopicNameLen = AckDataOneLen;
        newNode->PayloadBuf = AckDataTwo;
        newNode->PayloadLen = AckDataTwoLen;
        newNode->next = NULL;
    }
    else
    {
        newNode->AckTag = AckTag;
        newNode->AckStatus = AckStatus;
        newNode->AckData = AckDataOne;
        newNode->AckDataLen = AckDataOneLen;
        newNode->next = NULL;
    }

    // First event in AckQueue
    if(!(q->AckQueueHead))
    {
        q->AckQueueHead = newNode;
        q->AckQueueTail = newNode;
        q->AckQueueSize = q->AckQueueSize + 1;
        OPL_LOG_INFO(APP,"Queue Size: %d",q->AckQueueSize);
        return;
    }

    //If AckQueue out of space
    if(q->AckQueueSize >= APP_MAX_QUEUE_SIZE)
    {
        // pop an oldest event
        APP_DeQueue(q, false);
    }

    q->AckQueueTail->next = newNode;
    q->AckQueueTail = newNode;
    q->AckQueueSize = q->AckQueueSize + 1;
    OPL_LOG_INFO(APP,"Queue Size: %d",q->AckQueueSize);
    return;
}

void APP_DeQueue(T_AppAckQueue *q, bool bDeQueueAll)
{
    if(APP_QueueIsEmpty(q))  return;

    if(bDeQueueAll)
    {
        while(q->AckQueueHead)
        {
            T_AppAckInfoStruct *popNode = q->AckQueueHead;

            if(popNode->AckTag == ACK_TAG_CLOUD_MQTT_RECV_DATA_IND)
            {
                Transfer_MqttRecivedData(popNode->TopicName, popNode->TopicNameLen, popNode->PayloadBuf, popNode->PayloadLen);
            }
            else
            {
                Transfer_AckCommand(popNode->AckTag, popNode->AckStatus, popNode->AckData, popNode->AckDataLen);
            }    

            q->AckQueueHead = popNode->next;
            free(popNode);
        }

        q->AckQueueHead = NULL;
        q->AckQueueTail = NULL;
        q->AckQueueSize = 0;
    }
    else
    {
        T_AppAckInfoStruct *popNode = q->AckQueueHead;

        q->AckQueueHead = popNode->next;
        free(popNode);

        q->AckQueueSize = q->AckQueueSize - 1;
    }
    return;
}

// indicate callback for each type request

//////////////////////////////////// 
//// Callback group
//////////////////////////////////// 

// add your callback function here

SHM_DATA void APP_NmUnsolicitedCallback(T_NmUslctdEvtType tEvtType, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // tEvtType refer to net_mngr_api.h
    switch(tEvtType)
    {
        case NM_USLCTD_EVT_NETWORK_INIT:
        {
            APP_SendMessage(APP_EVT_WIFI_NETWORK_INIT_IND, NULL, 0);

            break;
        }
        case NM_USLCTD_EVT_NETWORK_UP:
        {
            APP_SendMessage(APP_EVT_WIFI_NETWORK_UP_IND, pu8Data, u32DataLen);
#if(CLOUD_ENABLED == 1)
            // set network up (for cloud to checking wifi connection)
            Cloud_NetworkStatusSet(true);
#endif
            break;
        }
        case NM_USLCTD_EVT_NETWORK_DOWN:
        {
            APP_SendMessage(APP_EVT_WIFI_NETWORK_DOWN_IND, NULL, 0);
#if(CLOUD_ENABLED == 1)
            // set network down (for cloud to checking wifi connection)
            Cloud_NetworkStatusSet(false);
#endif
            break;
        }
        case NM_USLCTD_EVT_NETWORK_RESET:
        {
            APP_SendMessage(APP_EVT_WIFI_NETWORK_RESET_IND, NULL, 0);
#if(CLOUD_ENABLED == 1)
            // set network down (for cloud to checking wifi connection)
            Cloud_NetworkStatusSet(false);
#endif
            break;
        }
        default:
        {
            // should not be here

            break;
        }
    }
}

SHM_DATA void APP_BleUnsolicitedCallback(uint16_t u16EvtType, T_OplErr tEvtRst, uint8_t *pu8Data, uint32_t u32DataLen)
{
    switch(u16EvtType)
    {
        case USLCTED_CB_EVT_BLE_INIT:
        {       
            // initialize ble advertise data
            APP_BleAdvDataInit();

            // initialize ble scan response data
            APP_BleScanRspDataInit();

            APP_SendMessage(APP_EVT_BLE_INIT_IND, NULL, 0);

            break;
        }
        case USLCTED_CB_EVT_BLE_ENT_ADVERTISE:
        {
            APP_SendMessage(APP_EVT_BLE_START_ADV_IND, &tEvtRst, sizeof(T_OplErr));

            break;
        }
        case USLCTED_CB_EVT_BLE_STOP:
        {
            APP_SendMessage(APP_EVT_BLE_STOP_IND, &tEvtRst, sizeof(T_OplErr));

            break;
        }
        case USLCTED_CB_EVT_BLE_CONNECTED:
        {
            APP_SendMessage(APP_EVT_BLE_CONNECTED_IND, pu8Data, u32DataLen);

            break;
        }
        case USLCTED_CB_EVT_BLE_DISCONNECT:
        {
            APP_SendMessage(APP_EVT_BLE_DISCONNECTED_IND, NULL, 0);

            break;
        }
        default:
        {
            // should not be here
         
            break;
        }
    }
}

#if(CLOUD_ENABLED == 1)
SHM_DATA void APP_CloudStatusCallback(T_CloudStatus tCloudStatus, void *pData, uint32_t u32DataLen)
{
    switch(tCloudStatus)
    {
        case CLOUD_CB_STA_INIT_DONE:
        case CLOUD_CB_STA_INIT_FAIL:
        { 
#if(AWS_ENABLED == 1)  
            APP_SendMessage(APP_EVT_CLOUD_MQTT_INIT_IND, &tCloudStatus, sizeof(tCloudStatus));
#endif
            break;
        }   
         
        case CLOUD_CB_STA_CONN_DONE:
        case CLOUD_CB_STA_CONN_FAIL:
        {
#if(AWS_ENABLED == 1)
            APP_SendMessage(APP_EVT_CLOUD_MQTT_CONNECT_IND, &tCloudStatus, sizeof(tCloudStatus));
#endif
            break;
        }

        case CLOUD_CB_STA_RECONN_DONE:
        {
            break;
        }

        case CLOUD_CB_STA_DISCONN:
        {
#if(AWS_ENABLED == 1)
            APP_SendMessage(APP_EVT_CLOUD_MQTT_DISCONN_IND, NULL, 0);
#endif
            break;
        }

        case CLOUD_CB_STA_SUB_DONE:
        case CLOUD_CB_STA_SUB_FAIL:
        {
#if(AWS_ENABLED == 1)
            APP_SendMessage(APP_EVT_CLOUD_MQTT_SUB_TOPIC_IND, &tCloudStatus, sizeof(tCloudStatus));
#endif
            break;
        }

        case CLOUD_CB_STA_UNSUB_DONE:
        case CLOUD_CB_STA_UNSUB_FAIL:
        {
#if(AWS_ENABLED == 1)
            APP_SendMessage(APP_EVT_CLOUD_MQTT_UNSUB_TOPIC_IND, &tCloudStatus, sizeof(tCloudStatus));
#endif
            break;
        }

        case CLOUD_CB_STA_PUB_DONE:
        case CLOUD_CB_STA_PUB_FAIL:
        {
#if(AWS_ENABLED == 1)
            APP_SendMessage(APP_EVT_CLOUD_MQTT_PUB_DATA_IND, &tCloudStatus, sizeof(tCloudStatus));
#endif
            break;
        }

        case CLOUD_CB_STA_DATA_RECV:
        {
#if(AWS_ENABLED == 1)
            APP_SendMessage(APP_EVT_CLOUD_RECV_DATA_IND, pData, u32DataLen);
#endif
            break;
        }

        default:
            break;
    }
}
#endif

SHM_DATA static void APP_WifiScanReqIndCb(T_OplErr tEvtRst)
{
    // this function will be called while wifi manager scan progress finish
    if(OPL_OK != tEvtRst)
    {
        Transfer_AckCommand(ACK_TAG_WIFI_SCAN_REQ, ACK_STATUS_FAIL, NULL, 0);
    }
    else
    {
        if(OPL_OK != APP_ResponseScanList(ACK_TAG_WIFI_SCAN_REQ))
        {
            Transfer_AckCommand(ACK_TAG_WIFI_SCAN_REQ, ACK_STATUS_FAIL, NULL, 0);
        }
    }
}

SHM_DATA static void APP_WifiCnctReqIndCb(T_OplErr tEvtRst)
{
    if(OPL_OK != tEvtRst)
    {
        Transfer_AckCommand(ACK_TAG_WIFI_CONNECT_REQ, ACK_STATUS_FAIL, NULL, 0);
    }
}

SHM_DATA static void APP_WifiDisconnIndCb(T_OplErr tEvtRst)
{
    if(OPL_OK != tEvtRst)
    {
        Transfer_AckCommand(ACK_TAG_WIFI_DISCONNECT_REQ, ACK_STATUS_FAIL, NULL, 0);
    }
}

SHM_DATA static void APP_SysTimerTimeoutHandler(void const *argu)
{
    APP_SendMessage(APP_EVT_SYS_TIMER_TIMEOUT, NULL, 0);
}

SHM_DATA static void APP_WaitHostRspTimeoutHandler(void const *argu)
{
    APP_SendMessage(APP_EVT_WAIT_HOST_TIMER_TIMEOUT, NULL, 0);
}

//////////////////////////////////// 
//// Event handler group
////////////////////////////////////

// add your event handler function here

static void APP_EvtHandler_SysTimerTimeout(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    Transfer_AckCommand(ACK_TAG_MODULE_READY, ACK_STATUS_OK, NULL, 0);
}

static void APP_EvtHandler_WaitHostTimerTimeout(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP, "Wait Host Timer Timeout");

    g_tAppHostState = APP_HOST_ST_NOT_WAIT;
    APP_DeQueue(g_tAppAckQueue, true);
}

SHM_DATA static void APP_EvtHandler_BleStateReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    switch(u32EventId)
    {
        case APP_EVT_BLE_START_ADV_REQ:
        {
            uint8_t u8AutoAdv = *((uint8_t *)pData);

            if(OPL_OK != Opl_Ble_Start_Req(u8AutoAdv))
            {
                Transfer_AckCommand(ACK_TAG_BLE_START_ADV_REQ, ACK_STATUS_FAIL, NULL, 0);
            }

            break;
        }

        case APP_EVT_BLE_STOP_REQ:
        {
            if(OPL_OK != Opl_Ble_Stop_Req())
            {
                Transfer_AckCommand(ACK_TAG_BLE_STOP_REQ, ACK_STATUS_FAIL, NULL, 0);
            }

            break;
        }

        default:
            break;
    }
}

SHM_DATA static void APP_EvtHandler_BleStateRsp(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    switch(u32EventId)
    {
        case APP_EVT_BLE_INIT_IND:
        {
            //OPL_LOG_INFO(APP, "BLE ready");

            // set ble ready bit
            EG_StatusSet(g_tAppEventGroup, APP_EG_BIT_BLE_READY, 1);

            if(EG_StatusGet(g_tAppEventGroup, APP_EG_BIT_WIFI_READY))
            {
                OPL_LOG_INFO(APP, "OPL module ready");

                // Notify MCU OPL module is ready
                osTimerStart(g_tAppSysTimer, 1000);
                //Transfer_AckCommand(ACK_TAG_MODULE_READY, ACK_STATUS_OK, NULL, 0);
            }
            
            break;
        }
        case APP_EVT_BLE_START_ADV_IND:
        {
            T_OplErr eOplErr = *((T_OplErr *)pData);

            if(OPL_OK == eOplErr)
            {
                uint8_t u8aBleMac[6] = {0};
                Opl_Ble_MacAddr_Read(u8aBleMac);

                OPL_LOG_INFO(APP, "BLE advertising...Device mac [%0X:%0X:%0X:%0X:%0X:%0X]", u8aBleMac[0],
                                                                                            u8aBleMac[1],
                                                                                            u8aBleMac[2],
                                                                                            u8aBleMac[3],
                                                                                            u8aBleMac[4],
                                                                                            u8aBleMac[5]);                                                                                  

                Transfer_AckCommand(ACK_TAG_BLE_START_ADV_IND, ACK_STATUS_OK, NULL, 0);
            }
            else
            {
                OPL_LOG_WARN(APP, "Start BLE advertising fail!");                                                                           
                Transfer_AckCommand(ACK_TAG_BLE_START_ADV_REQ, ACK_STATUS_FAIL, NULL, 0);
            }

            break;
        }

        case APP_EVT_BLE_STOP_IND:
        {
            T_OplErr eOplErr = *((T_OplErr *)pData);
            
            if(true == bSleepWillingness)
            {
                bBleOffReqDone = true;
                APP_Sleep_Check();    
            }
            else
            {
                if(OPL_OK == eOplErr)
                {
                    Transfer_AckCommand(ACK_TAG_BLE_STOP_IND, ACK_STATUS_OK, NULL, 0);
                }
                else
                {
                    Transfer_AckCommand(ACK_TAG_BLE_STOP_REQ, ACK_STATUS_FAIL, NULL, 0);
                }
            }

            break;
        }

        case APP_EVT_BLE_CONNECTED_IND:
        {
            uint8_t u8aBlePeerMac[6] = {0};
            memcpy(u8aBlePeerMac, pData, sizeof(u8aBlePeerMac));

            OPL_LOG_INFO(APP, "BLE connected...Peer mac [%0X:%0X:%0X:%0X:%0X:%0X]", u8aBlePeerMac[0],
                                                                                    u8aBlePeerMac[1],
                                                                                    u8aBlePeerMac[2],
                                                                                    u8aBlePeerMac[3],
                                                                                    u8aBlePeerMac[4],
                                                                                    u8aBlePeerMac[5]);
            
            if(g_tAppHostState == APP_HOST_ST_NOT_WAIT)
            {
                //OPL exit smartsleep mode
                PS_ExitSmartSleep();

                Hal_Vic_GpioIntEn((E_GpioIdx_t)WAKEUP_PIN, 0);

                //save Ack tag, Ack status, Ack data, Ack datalen
                APP_EnQueue(g_tAppAckQueue, ACK_TAG_BLE_CONNECTED_IND, ACK_STATUS_OK, NULL, 0, NULL, NULL);

                // trigger mcu wakeup
                APP_TriggerHostWakeUp();

                //change Host state
                g_tAppHostState = APP_HOST_ST_WAIT;

                //start the timer 
                osTimerStart(g_tAppWaitHostRspTimer, 5000);
            }
            else if(g_tAppHostState == APP_HOST_ST_WAIT)
            {
                APP_EnQueue(g_tAppAckQueue, ACK_TAG_BLE_CONNECTED_IND, ACK_STATUS_OK, NULL, 0, NULL, NULL);
            }
            break;
        }

        case APP_EVT_BLE_DISCONNECTED_IND:
        {
            if(true == bSleepWillingness)
            {
                bBleOffReqDone = true;
                APP_Sleep_Check();
                break;    
            }
            else if(g_tAppHostState == APP_HOST_ST_NOT_WAIT)
            {
                //OPL exit smartsleep mode
                PS_ExitSmartSleep();

                Hal_Vic_GpioIntEn((E_GpioIdx_t)WAKEUP_PIN, 0);

                //save Ack tag, Ack status, Ack data, Ack datalen
                APP_EnQueue(g_tAppAckQueue, ACK_TAG_BLE_DISCONNECTED_IND, ACK_STATUS_OK, NULL, 0, NULL, NULL);                
                
                // trigger mcu wakeup
                APP_TriggerHostWakeUp();

                //change Host state
                g_tAppHostState = APP_HOST_ST_WAIT;

                osTimerStart(g_tAppWaitHostRspTimer, 5000);
            }
            else if(g_tAppHostState == APP_HOST_ST_WAIT)
            {
                APP_EnQueue(g_tAppAckQueue, ACK_TAG_BLE_DISCONNECTED_IND, ACK_STATUS_OK, NULL, 0, NULL, NULL);
            }
            break;
        }

        default:
            break;
    }
}

SHM_DATA static void APP_EvtHandler_BleRecvDataRsp(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    if(g_tAppHostState == APP_HOST_ST_NOT_WAIT)
    {
        //exit smartsleep mode
        PS_ExitSmartSleep();

        Hal_Vic_GpioIntEn((E_GpioIdx_t)WAKEUP_PIN, 0);

        //save Ack tag, Ack status, Ack data, Ack datalen
        APP_EnQueue(g_tAppAckQueue, ACK_TAG_BLE_RECV_DATA_IND, ACK_STATUS_OK, pData, u32DataLen, NULL, NULL);        
        
        // trigger mcu wakeup
        APP_TriggerHostWakeUp();

        //change Host state
        g_tAppHostState = APP_HOST_ST_WAIT;

        osTimerStart(g_tAppWaitHostRspTimer, 5000);
    }
    else if(g_tAppHostState == APP_HOST_ST_WAIT)
    {
        APP_EnQueue(g_tAppAckQueue, ACK_TAG_BLE_RECV_DATA_IND, ACK_STATUS_OK, pData, u32DataLen, NULL, NULL);        
    }
}

#if(OPL_DATA_ENABLED == 1)
static void APP_EvtHandler_BleDataInd(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_DataRecvHandler(pData, (uint16_t)u32DataLen);
}
#endif

SHM_DATA static void APP_EvtHandler_WifiScanReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    switch(u32EventId)
    {
        case APP_EVT_WIFI_SCAN_REQ:
        {
            if(OPL_OK != APP_NmWifiScanReq(APP_WifiScanReqIndCb))
            {
                Transfer_AckCommand(ACK_TAG_WIFI_SCAN_REQ, ACK_STATUS_FAIL, NULL, 0);
            }

            break;
        }

        default:
            break;
    }
}

SHM_DATA static void APP_EvtHandler_WifiConnectReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    T_NmWifiCnctConfig *tNmWifiCnctConfig = (T_NmWifiCnctConfig *)pData;

    if(OPL_OK != APP_NmWifiCnctReq(tNmWifiCnctConfig, APP_WifiCnctReqIndCb))
    {
        Transfer_AckCommand(ACK_TAG_WIFI_CONNECT_REQ, ACK_STATUS_FAIL, NULL, 0);
    }
}

SHM_DATA static void APP_EvtHandler_WifiDisconnectReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    if(OPL_OK != Opl_Wifi_Disc_Req(APP_WifiDisconnIndCb))
    {
        Transfer_AckCommand(ACK_TAG_WIFI_DISCONNECT_REQ,ACK_STATUS_FAIL, NULL, 0);
    }
    
}

SHM_DATA static void APP_EvtHandler_WifiNetworkStateRsp(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    switch(u32EventId)
    {
        case APP_EVT_WIFI_NETWORK_INIT_IND:
        {
            //OPL_LOG_INFO(APP, "WiFi ready");

            // set wifi ready bit
            EG_StatusSet(g_tAppEventGroup, APP_EG_BIT_WIFI_READY, 1);

            if(EG_StatusGet(g_tAppEventGroup, APP_EG_BIT_BLE_READY))
            {
                OPL_LOG_INFO(APP, "OPL module ready");

                // Notify MCU OPL module is ready
                osTimerStart(g_tAppSysTimer, 1000);
                //Transfer_AckCommand(ACK_TAG_MODULE_READY, ACK_STATUS_OK, NULL, 0);
            }

            break;
        }

        case APP_EVT_WIFI_NETWORK_UP_IND:
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

#if(OPL_DATA_ENABLED == 1)
            if (g_tAppHostState == APP_HOST_ST_NOT_WAIT)
            {
                //OPL exit smartsleep mode
                PS_ExitSmartSleep();

                Hal_Vic_GpioIntEn((E_GpioIdx_t)WAKEUP_PIN, 0);

                //save Ack tag, Ack status, Ack data, Ack datalen
                APP_EnQueue(g_tAppAckQueue, ACK_TAG_WIFI_NETWORK_UP_IND, ACK_STATUS_OK, NULL, 0, NULL, NULL);

                // trigger mcu wakeup
                APP_TriggerHostWakeUp();

                //change Host state
                g_tAppHostState = APP_HOST_ST_WAIT;

                osTimerStart(g_tAppWaitHostRspTimer, 5000);
            }
            else if (g_tAppHostState == APP_HOST_ST_WAIT)
            {
                APP_EnQueue(g_tAppAckQueue, ACK_TAG_WIFI_NETWORK_UP_IND, ACK_STATUS_OK, NULL, 0, NULL, NULL);
            }
#else
            Transfer_AckCommand(ACK_TAG_WIFI_NETWORK_UP_IND, ACK_STATUS_OK, NULL, 0);
#endif
#if(CLOUD_ENABLED == 1)
            // trigger cloud connection if cloud auto-connect flag sets
            if(true == Cloud_AutoConnectFlagGet())
            {
                Cloud_MsgSend(CLOUD_EVT_TYPE_ESTABLISH, NULL, 0);
            }
#endif            
            break;
        }

        case APP_EVT_WIFI_NETWORK_DOWN_IND:
        {
            OPL_LOG_INFO(APP, "Network disconnected");
            
            if (g_tAppHostState == APP_HOST_ST_NOT_WAIT)
            {
                //OPL exit smartsleep mode
                PS_ExitSmartSleep();

                Hal_Vic_GpioIntEn((E_GpioIdx_t)WAKEUP_PIN, 0);

                //save Ack tag, Ack status, Ack data, Ack datalen
                APP_EnQueue(g_tAppAckQueue, ACK_TAG_WIFI_NETWORK_DOWN_IND, ACK_STATUS_OK, NULL, 0, NULL, NULL);

                // trigger mcu wakeup
                APP_TriggerHostWakeUp();

                //change Host state
                g_tAppHostState = APP_HOST_ST_WAIT;

                osTimerStart(g_tAppWaitHostRspTimer, 5000);  
            }
            else if (g_tAppHostState == APP_HOST_ST_WAIT)
            {
                APP_EnQueue(g_tAppAckQueue, ACK_TAG_WIFI_NETWORK_DOWN_IND, ACK_STATUS_OK, NULL, 0, NULL, NULL);
            }
            break;
        }

        case APP_EVT_WIFI_NETWORK_RESET_IND:
        {
            Transfer_AckCommand(ACK_TAG_WIFI_NETWORK_RESET_IND, ACK_STATUS_OK, NULL, 0);
            break;
        }

        default:
            break;
    }
}


#if(AWS_ENABLED == 1)
// Cloud
SHM_DATA static void APP_EvtHandler_CloudMqttInitReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    if(OPL_OK != Cloud_MsgSend(CLOUD_EVT_TYPE_INIT, pData, u32DataLen))
    {
        AT_FAIL(ACK_TAG_CLOUD_MQTT_INIT_REQ, NULL, 0);
    }

}

SHM_DATA static void APP_EvtHandler_CloudMqttEstablishReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    if(OPL_OK != Cloud_MsgSend(CLOUD_EVT_TYPE_ESTABLISH, pData, u32DataLen))
    {
        Transfer_AckCommand(ACK_TAG_CLOUD_MQTT_ESTAB_REQ, ACK_STATUS_FAIL, NULL, 0);
    }
}

SHM_DATA static void APP_EvtHandler_CloudMqttDisconnectReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    if(OPL_OK != Cloud_MsgSend(CLOUD_EVT_TYPE_DISCONNECT, NULL, 0))
    {
        Transfer_AckCommand(ACK_TAG_CLOUD_MQTT_DISCON_REQ, ACK_STATUS_FAIL, NULL, 0);
    }
}

SHM_DATA static void APP_EvtHandler_CloudMqttSubTopicReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    if(OPL_OK != Cloud_MsgSend(CLOUD_EVT_TYPE_REGIS_TOPIC, pData, u32DataLen))
    {
        Transfer_AckCommand(ACK_TAG_CLOUD_MQTT_SUB_TOPIC_REQ, ACK_STATUS_FAIL, NULL, 0);
    }
}

SHM_DATA static void APP_EvtHandler_CloudMqttUnSubTopicReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    if(OPL_OK != Cloud_MsgSend(CLOUD_EVT_TYPE_UNREGIS_TOPIC, pData, u32DataLen))
    {
        Transfer_AckCommand(ACK_TAG_CLOUD_MQTT_UNSUB_TOPIC_REQ, ACK_STATUS_FAIL, NULL, 0);
    }
}

SHM_DATA static void APP_EvtHandler_CloudMqttPubDataReq(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    if(OPL_OK != Cloud_MsgSend(CLOUD_EVT_TYPE_POST, pData, u32DataLen))
    {
        AT_FAIL(ACK_TAG_CLOUD_MQTT_PUB_DATA_REQ, NULL, 0);
    }
}

SHM_DATA static void APP_EvtHandler_CloudMqttStateRsp(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    T_CloudStatus tCloudStatus;
    
    if(pData != NULL)
    {
        memcpy(&tCloudStatus, pData, sizeof(T_CloudStatus));
    }

    switch(u32EventId)
    {   
        case APP_EVT_CLOUD_MQTT_INIT_IND:
        {
            if(CLOUD_CB_STA_INIT_DONE == tCloudStatus)
            {
                AT_OK(ACK_TAG_CLOUD_MQTT_INIT_IND, NULL, 0);
            }
            else
            {
                AT_FAIL(ACK_TAG_CLOUD_MQTT_INIT_IND, NULL, 0);
            }
        }

        case APP_EVT_CLOUD_MQTT_CONNECT_IND:
        {
            bCloudOff = false;

            if(CLOUD_CB_STA_CONN_DONE == tCloudStatus)
            {
                AT_OK(ACK_TAG_CLOUD_MQTT_CONNECT_IND, NULL, 0);
            }
            else
            {
                AT_FAIL(ACK_TAG_CLOUD_MQTT_CONNECT_IND, NULL, 0);
            }

            break;
        }

        case APP_EVT_CLOUD_MQTT_DISCONN_IND:
        {
            OPL_LOG_INFO(APP, "Cloud disconnected");
            bCloudOff = true;
            
            if(true == bSleepWillingness)
            {
                APP_Sleep_Check();
                break;  
            }
            else if(g_tAppHostState == APP_HOST_ST_NOT_WAIT)
            {
                //OPL exit smartsleep mode
                PS_ExitSmartSleep();

                Hal_Vic_GpioIntEn((E_GpioIdx_t)WAKEUP_PIN, 0);
                
                //save Ack tag, Ack status, Ack data, Ack datalen
                APP_EnQueue(g_tAppAckQueue, ACK_TAG_CLOUD_MQTT_DISCON_IND, ACK_STATUS_OK, NULL, 0, NULL, NULL);                
                
                // trigger mcu wakeup
                APP_TriggerHostWakeUp();

                //change Host state
                g_tAppHostState = APP_HOST_ST_WAIT;

                osTimerStart(g_tAppWaitHostRspTimer, 5000);
            }
            else if (g_tAppHostState == APP_HOST_ST_WAIT)
            {
                APP_EnQueue(g_tAppAckQueue, ACK_TAG_CLOUD_MQTT_DISCON_IND, ACK_STATUS_OK, NULL, 0, NULL, NULL);
            }
            break;
        }
        case APP_EVT_CLOUD_MQTT_SUB_TOPIC_IND:
        {
            if(CLOUD_CB_STA_SUB_DONE == tCloudStatus)
            {
                AT_OK(ACK_TAG_CLOUD_MQTT_SUB_TOPIC_IND, NULL, 0);
            }
            else
            {
                AT_FAIL(ACK_TAG_CLOUD_MQTT_SUB_TOPIC_IND, NULL, 0);
            }

            break;
        }

        case APP_EVT_CLOUD_MQTT_UNSUB_TOPIC_IND:
        {
            if(CLOUD_CB_STA_UNSUB_DONE == tCloudStatus)
            {
                AT_OK(ACK_TAG_CLOUD_MQTT_UNSUB_TOPIC_IND, NULL, 0);
            }
            else
            {
                AT_FAIL(ACK_TAG_CLOUD_MQTT_UNSUB_TOPIC_IND, NULL, 0);
            }

            break;
        }

        case APP_EVT_CLOUD_MQTT_PUB_DATA_IND:
        {
            if(CLOUD_CB_STA_PUB_DONE == tCloudStatus)
            {
                AT_OK(ACK_TAG_CLOUD_MQTT_PUB_DATA_IND, NULL, 0);
            }
            else
            {
                AT_FAIL(ACK_TAG_CLOUD_MQTT_PUB_DATA_IND, NULL, 0);
            }
            
            break;
        }

        default:
            break;
    }
}

SHM_DATA static void APP_EvtHandler_CloudMqttRecvDataRsp(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    memcpy(&tCloudPayloadFmt, pData, sizeof(T_CloudPayloadFmt));

    if(g_tAppHostState == APP_HOST_ST_NOT_WAIT)
    {
        //exit smartsleep mode
        PS_ExitSmartSleep();

        Hal_Vic_GpioIntEn((E_GpioIdx_t)WAKEUP_PIN, 0);
        
        //save Ack tag, Ack status, Ack data, Ack datalen
        APP_EnQueue(g_tAppAckQueue, ACK_TAG_CLOUD_MQTT_RECV_DATA_IND, ACK_STATUS_OK, 
                    (uint8_t *)&tCloudPayloadFmt.u8TopicName, tCloudPayloadFmt.u16TopicNameLen, 
                    tCloudPayloadFmt.u8aPayloadBuf, tCloudPayloadFmt.u16PayloadLen);

        // trigger mcu wakeup
        APP_TriggerHostWakeUp();

        //change Host state
        g_tAppHostState = APP_HOST_ST_WAIT;

        osTimerStart(g_tAppWaitHostRspTimer, 5000);
    }
    else if (g_tAppHostState == APP_HOST_ST_WAIT)
    {
        APP_EnQueue(g_tAppAckQueue, ACK_TAG_CLOUD_MQTT_RECV_DATA_IND, ACK_STATUS_OK, 
                    (uint8_t *)&tCloudPayloadFmt.u8TopicName, tCloudPayloadFmt.u16TopicNameLen, 
                    tCloudPayloadFmt.u8aPayloadBuf, tCloudPayloadFmt.u16PayloadLen);
    }  
}
#endif
// *

#if(OTA_ENABLED == 1)
static void APP_EvtHandler_OtaStateRsp(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    switch(u32EventId)
    {
        case APP_EVT_OTA_START_IND:
        {
            if (g_tAppHostState == APP_HOST_ST_NOT_WAIT)
            {
                //OPL exit smartsleep mode
                PS_ExitSmartSleep();

                Hal_Vic_GpioIntEn((E_GpioIdx_t)WAKEUP_PIN, 0);

                //save Ack tag, Ack status, Ack data, Ack datalen
                APP_EnQueue(g_tAppAckQueue, ACK_TAG_OTA_START, ACK_STATUS_OK, NULL, 0, NULL, NULL);

                // trigger mcu wakeup
                APP_TriggerHostWakeUp();

                //change Host state
                g_tAppHostState = APP_HOST_ST_WAIT;

                osTimerStart(g_tAppWaitHostRspTimer, 5000);     
            }
            else if (g_tAppHostState == APP_HOST_ST_WAIT)
            {
                APP_EnQueue(g_tAppAckQueue, ACK_TAG_OTA_START, ACK_STATUS_OK, NULL, 0, NULL, NULL);
            }
            break;
        }

        case APP_EVT_OTA_FAIL_IND:
        {
            if (g_tAppHostState == APP_HOST_ST_NOT_WAIT)
            {
                //OPL exit smartsleep mode 
                PS_ExitSmartSleep();

                Hal_Vic_GpioIntEn((E_GpioIdx_t)WAKEUP_PIN, 0);

                //save Ack tag, Ack status, Ack data, Ack datalen 
                APP_EnQueue(g_tAppAckQueue, ACK_TAG_OTA_DONE, ACK_STATUS_FAIL, NULL, 0, NULL, NULL);

                // trigger mcu wakeup 
                APP_TriggerHostWakeUp();

                //change Host state
                g_tAppHostState = APP_HOST_ST_WAIT;

                osTimerStart(g_tAppWaitHostRspTimer, 5000);     
            }
            else if (g_tAppHostState == APP_HOST_ST_WAIT)
            {
                APP_EnQueue(g_tAppAckQueue, ACK_TAG_OTA_DONE, ACK_STATUS_FAIL, NULL, 0, NULL, NULL);
            }
            break;
        }

        default:
            break;
    }
}
#endif

static void APP_EvtHandler_WifiStopSleepCheckHandler(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    OPL_LOG_INFO(APP,"Wifi Stop success");

    if(true == bSleepWillingness)
    {
        bWifiOffReqDone = true;
        APP_Sleep_Check();    
    }
}

void APP_NmWifiStopCb(T_OplErr tEvtRst)
{
    APP_SendMessage(APP_EVT_WIFI_STOP_SLEEP_CHECK, NULL, 0);
    return;
}

static void APP_EvtHandler_TimerSleepTimeoutWakeup(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    //disable io wakeup
    Hal_Vic_GpioIntEn((E_GpioIdx_t)WAKEUP_PIN, 0);
    
    //save Ack tag, Ack status, Ack data, Ack datalen
    APP_EnQueue(g_tAppAckQueue, ACK_TAG_TIMER_SLEEP_WAKEUP, ACK_STATUS_OK, NULL, 0, NULL, NULL);                
    
    // trigger mcu wakeup
    APP_TriggerHostWakeUp();

    //change Host state
    g_tAppHostState = APP_HOST_ST_WAIT;

    osTimerStart(g_tAppWaitHostRspTimer, 5000);
}

SHM_DATA static void APP_EvtHandler_IoExitTimerSleepRsp(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    g_tAppHostState = APP_HOST_ST_NOT_WAIT;
    OPL_LOG_INFO(APP,"Timer sleep IO wakeup");
    AT_OK(ACK_TAG_TIMER_SLEEP_WAKEUP, NULL, 0);    
}

static void APP_TimerSleepCallback(PS_WAKEUP_TYPE type)
{
    if(type == PS_WAKEUP_TYPE_TIMEOUT)
    {
        APP_SendMessage(APP_EVT_TIMER_SLEEP_TIMEOUT_WAKEUP, NULL, 0);
    }
    else if(type == PS_WAKEUP_TYPE_IO)
    {
        APP_SendMessage(APP_EVT_IO_EXIT_TIMER_SLEEP_IND, NULL, 0);
    }
}

SHM_DATA static void APP_EvtHandler_IoExitSmartSleepRsp(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    PS_ExitSmartSleep();

    //disable io wakeup
    Hal_Vic_GpioIntEn((E_GpioIdx_t)WAKEUP_PIN, 0);

    g_tAppHostState = APP_HOST_ST_NOT_WAIT;
    OPL_LOG_INFO(APP,"Smart sleep IO wakeup");

    AT_OK(ACK_TAG_MODULE_WAKEUP, NULL, 0);
}

static void APP_SmartSleepIoCallback(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    APP_SendMessage(APP_EVT_IO_EXIT_SMART_SLEEP_IND, NULL, 0);
}

static void APP_EvtHandler_PrepareSleep(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    memcpy(&g_tPcStruct, pData, sizeof(g_tPcStruct));
    OPL_LOG_INFO(APP,"Sleep Mode: %d",g_tPcStruct.u8SleepMode);
    OPL_LOG_INFO(APP,"Sleep Time: %d", g_tPcStruct.u32SleepTime);
    
    bSleepWillingness = true; 

    switch(g_tPcStruct.u8SleepMode)
    {
        case 1: //SMART_SLEEP

        {
            APP_Sleep_Check();
            
            break;
        }
        
        case 2: //TIMER_SLEEP
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
            if(OPL_OK == APP_NmWifiStopReq(APP_NmWifiStopCb))
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

        case 3: //DEEP_SLEEP
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
            if(OPL_OK == APP_NmWifiStopReq(APP_NmWifiStopCb))
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
}

static void APP_Sleep_Check(void)
{
    switch(g_tPcStruct.u8SleepMode)
    {
        case 1: //SMART_SLEEP:
        {
            OPL_LOG_INFO(APP, "Smart Sleep");
            Hal_Pin_ConfigSet(WAKEUP_PIN, PIN_TYPE_GPIO_INPUT, PIN_DRIVING_FLOAT);
            ps_set_wakeup_io((E_GpioIdx_t) WAKEUP_PIN, 1, INT_TYPE_RISING_EDGE, 0, (T_Gpio_CallBack)APP_SmartSleepIoCallback);
            Transfer_AckCommand(ACK_TAG_SMART_SLEEP, ACK_STATUS_OK, NULL, 0); 
            PS_EnterSmartSleep(0);
            
            break;           
        }

        case 2: //TIMER_SLEEP:
        {
            //Check BLE & Wi-Fi module are both Idle
            if(bBleOffReqDone && bWifiOffReqDone && bCloudOff)
            {
                bBleOffReqDone = false;
                bWifiOffReqDone = false;
                bCloudOff = true;

                OPL_LOG_INFO(APP, "Timer Sleep");
                ps_smart_sleep(false); //turn off smart sleep to avoid simultaneous operation
                Hal_Pin_ConfigSet(WAKEUP_PIN, PIN_TYPE_GPIO_INPUT, PIN_DRIVING_FLOAT);
                ps_set_wakeup_io(WAKEUP_PIN, 1, INT_TYPE_RISING_EDGE, 0, (T_Gpio_CallBack)APP_TimerSleepCallback);
                ps_set_wakeup_cb(APP_TimerSleepCallback);
                Transfer_AckCommand(ACK_TAG_TIMER_SLEEP, ACK_STATUS_OK, NULL, 0);
                ps_timer_sleep(g_tPcStruct.u32SleepTime);
                
                break;
            }
            else
            {
                OPL_LOG_INFO(APP,"Enter Timer Sleep Postponed");
                return;
            }
        }                                                                                                                                                                                                                     

        case 3: //DEEP_SLEEP:
        {
            //Check BLE & Wi-Fi module are both off
            if(bBleOffReqDone && bWifiOffReqDone && bCloudOff)
            {
                bBleOffReqDone = false;
                bWifiOffReqDone = false;
                bCloudOff = true;

                OPL_LOG_INFO(APP, "Deep Sleep");
                Hal_Pin_ConfigSet(WAKEUP_PIN, PIN_TYPE_GPIO_INPUT, PIN_DRIVING_FLOAT);  // OPL_POWER_SLEEP_CONTROL
                ps_set_wakeup_io(WAKEUP_PIN, 1, INT_TYPE_RISING_EDGE, 0, NULL);
                Transfer_AckCommand(ACK_TAG_DEEP_SLEEP, ACK_STATUS_OK, NULL, 0);
                ps_deep_sleep();

                break;
            }
            else
            {
                OPL_LOG_INFO(APP,"Enter Deep Sleep PostPoned");
                return;
            }
        }
    }

    bSleepWillingness = false;
}

/*************************************************************************
* FUNCTION:
*   APP_EvtHandler_HostReadyRsp
*
* DESCRIPTION:
*   After confirming Host wakeup, send Ack data to Host.
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static void APP_EvtHandler_HostReadyRsp(uint32_t u32EventId, void *pData, uint32_t u32DataLen)
{
    osTimerStop(g_tAppWaitHostRspTimer);

    g_tAppHostState = APP_HOST_ST_NOT_WAIT;
    //OPL_LOG_INFO(APP,"Host state: NOT_WAIT");

    APP_DeQueue(g_tAppAckQueue, true);

    Transfer_AckCommand(ACK_TAG_HOST_READY, ACK_STATUS_OK, NULL, 0);
}

//////////////////////////////////// 
//// APP function group
////////////////////////////////////

// add your application function here

SHM_DATA T_OplErr APP_ResponseScanList(T_AckTag tAckTagId)
{
    wifi_scan_list_t *pstScanList = NULL;

    pstScanList = (wifi_scan_list_t *)malloc(sizeof(wifi_scan_list_t));
    
    if(NULL == pstScanList)
    {
        OPL_LOG_ERRO(APP, "malloc fail");

        return OPL_ERR_ALLOC_MEMORY_FAIL;
    }

    memset(pstScanList, 0, sizeof(wifi_scan_list_t));

    // get the information of scan list			
    Opl_Wifi_ScanList_Get(pstScanList);

    OPL_LOG_INFO(APP, "Scanned total num %d\r\n", pstScanList->num);
    
#if(OPL_DATA_ENABLED == 0)

    uint32_t cnt = 0;

    if(0 != pstScanList->num)
    {
        //scan ap num
        if(OPL_OK == Transfer_ScanNum(tAckTagId, (uint8_t)pstScanList->num))
        {
            for(; cnt < pstScanList->num; cnt ++)
            {
                Transfer_ScanList(tAckTagId, 
                                    pstScanList->ap_record[cnt].bssid, 
                                    pstScanList->ap_record[cnt].ssid,
                                    strlen((char *)pstScanList->ap_record[cnt].ssid),
                                    pstScanList->ap_record[cnt].channel,
                                    pstScanList->ap_record[cnt].rssi,
                                    pstScanList->ap_record[cnt].auth_mode);
            }
        }
        else
        {
            Transfer_AckCommand(tAckTagId, ACK_STATUS_FAIL, NULL, 0);
        }
    }
    else
    {
        Transfer_AckCommand(tAckTagId, ACK_STATUS_FAIL, NULL, 0);
    }
#endif
    free(pstScanList);

    return OPL_OK;
}

SHM_DATA void APP_TriggerHostWakeUp(void)
{
    OPL_LOG_INFO(APP, "Trigger gpio to wakeup host");

    //trigger GPIO 
#if (TRIGGER_HOST_GPIO_RISING_EDGE == 0) //falling 
    Hal_Vic_GpioOutput(WAKEUP_HOST_PIN, GPIO_LEVEL_LOW);
    osDelay(20);
    Hal_Vic_GpioOutput(WAKEUP_HOST_PIN, GPIO_LEVEL_HIGH);
#else //rising
    Hal_Vic_GpioOutput(WAKEUP_HOST_PIN, GPIO_LEVEL_HIGH);
    osDelay(20);
    Hal_Vic_GpioOutput(WAKEUP_HOST_PIN, GPIO_LEVEL_LOW);
#endif
}

SHM_DATA void APP_BleAdvDataInit(void)
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

SHM_DATA void APP_BleScanRspDataInit(void)
{
    // ble scan response data inititate

    // user modify
    // *

    char u8aBleName[BLE_ADV_SCAN_BUF_SIZE - 2];
    uint8_t u8BleNameLen = 0;
    uint8_t au8BleScanRspData[BLE_ADV_SCAN_BUF_SIZE];
    uint8_t u8aBleMac[6] = {0};

    Opl_Ble_MacAddr_Read(u8aBleMac);

    sprintf(u8aBleName, "%s_%02X%02X%02X", BLE_GAP_PF_DEVICE_NAME, 
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
    else
    {
        if(OPL_OK != GAP_Svc_DeviceName_Set((uint8_t *)u8aBleName, u8BleNameLen))
        {
            OPL_LOG_ERRO(APP, "Set dev name in GAP svc fail");
        }
    }

    // *
}

void APP_SysInit(void)
{
#if (EXT_PA_ENABLED == 1)
    // Do not overwrite RF power setting if external PA enable
#else
    // initialize rf power setting
    RF_PwrSet(RF_CFG_DEF_PWR_SET);
    // initialize rf power setting
#endif

    //initialize ota manager
#if(OTA_ENABLED == 1)
    OTA_Init();
#endif

    Transfer_Init();

    //Http file Task
#if(HTTP_ENABLED == 1)
    File_DownloadTaskInit();
#endif

    // user implement
}

void APP_DataInit(void)
{
    // user implement
}

SHM_DATA void APP_TaskInit(void)
{
    // create timer
    osTimerDef_t tSysTimerDef;

    tSysTimerDef.ptimer = APP_SysTimerTimeoutHandler;
    g_tAppSysTimer = osTimerCreate(&tSysTimerDef, osTimerOnce, NULL);
    if(g_tAppSysTimer == NULL)
    {
        OPL_LOG_ERRO(APP, "Create sys timer fail");
    }

    osTimerDef_t tWaitHostRspTimerDef;

    tWaitHostRspTimerDef.ptimer = APP_WaitHostRspTimeoutHandler;
    g_tAppWaitHostRspTimer = osTimerCreate(&tWaitHostRspTimerDef, osTimerOnce, NULL);
    if(g_tAppWaitHostRspTimer == NULL)
    {
        OPL_LOG_ERRO(APP, "Create timer fail");
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

    // create event group
    EG_Create(&g_tAppEventGroup);

    // user implement
    
    //create a new queue
    g_tAppAckQueue = APP_NewQueue(); 
}

void APP_BleInit(void)
{
    // assign unsolicited callback function
    Opl_Ble_Uslctd_CB_Reg(&APP_BleUnsolicitedCallback);

    // register service
    GAP_Svc_Init();
#if(OPL_DATA_ENABLED == 1)
    GATT_Svc_Init();
#endif

    //DevInfo_Svc_Init();

    // register opl service
#if(OPL_DATA_ENABLED == 1)    
    OPL_Svc_Init();
#endif

    // initialize the ble manager (auto-adv)
    Opl_Ble_Init_Req(false);

    // user implement
}

void APP_NetInit(void)
{
    // Network manager initialize (auto-connect enable)
    // turn off (auto-connect enable)
#if(OPL_DATA_ENABLED == 0)
    APP_NmInit(false, &APP_NmUnsolicitedCallback);
#else
    APP_NmInit(true, &APP_NmUnsolicitedCallback);
#endif
    // user implement
}

void APP_CldInit(void)
{
#if(CLOUD_ENABLED == 1)
    // initialize cloud status callback
    Cloud_StatusCallbackRegister(APP_CloudStatusCallback);

    Cloud_Init(NULL);
#endif
    // initialize http ota task
    // HTTP_OtaTaskInit();

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
SHM_DATA T_OplErr APP_SendMessage(uint32_t u32EventId, uint8_t *pu8Data, uint32_t u32DataLen)
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

    //
    OPL_LOG_INFO(APP, APP_FW_VER_STR);

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
