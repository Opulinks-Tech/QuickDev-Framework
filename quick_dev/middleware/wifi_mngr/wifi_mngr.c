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
*  wifi_mngr.c
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

#include "cmsis_os.h"
#include "netif.h"
#include "opl_err.h"
#include "qd_config.h"
#include "qd_module.h"
#include "wifi_mngr.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (WM_ENABLED == 1)

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

osThreadId g_tWmTaskId;
osMessageQId g_tWmQueueId;
#if 0
osSemaphoreId g_tWmUslctedCbRegSemId;
#endif

static bool g_blWifiInited = false;

static T_WmUslctedCbFp g_tWmUslctedCbFp[WM_USLCTED_CB_REG_NUM] = {NULL};

// Sec 7: declaration of static function prototype

static uint8_t WM_UslctedCbIsReg(uint8_t u8Index);
static T_OplErr WM_ModuleEventMapping(T_WmEvent tWmEventId, T_FsmDef **tFsmDef, T_FsmEvent *tEventId);
static T_OplErr WM_CheckRequest(T_WmEvent tWmEventId);
static void WM_ProcessMessage(T_WmMessage *tMsg);

/***********
C Functions
***********/
// Sec 8: C Functions

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
int WM_WifiEventHandlerCb(wifi_event_id_t event_id, void *data, uint16_t length)
{
    switch(event_id)
    {
        case WIFI_EVENT_STA_START:
        {
            WM_LOG_DEBG("WIFI_EVENT_STA_START");

            /* Tcpip stack and net interface initialization,  dhcp client process initialization. */
            // lwip_network_init(WIFI_MODE_STA);

            WM_SendMessage(WM_EVT_INIT_IND, NULL, 0, NULL);
            break;
        }
        case WIFI_EVENT_SCAN_COMPLETE:
        {
            WM_LOG_DEBG("WIFI_EVENT_SCAN_COMPLETE");

            WM_SendMessage(WM_EVT_SCAN_IND, NULL, 0, NULL);
            break;
        }
        case WIFI_EVENT_STA_CONNECTED:
        {
            uint8_t u8Reason = *((uint8_t *)data);

            WM_LOG_DEBG("WIFI_EVENT_STA_CONNECTED (reason %d)", u8Reason);

            WM_SendMessage(WM_EVT_CONNECT_PASS_IND, &u8Reason, sizeof(uint8_t), NULL);
            break;
        }
        case WIFI_EVENT_STA_CONNECTION_FAILED:
        {
            uint8_t u8Reason = *((uint8_t *)data);

            WM_LOG_DEBG("WIFI_EVENT_STA_CONNECTION_FAILED (reason %d)", u8Reason);

            WM_SendMessage(WM_EVT_CONNECT_FAIL_IND, &u8Reason, sizeof(uint8_t), NULL);
            break;
        }
        case WIFI_EVENT_STA_DISCONNECTED:
        {
            uint8_t u8Reason = *((uint8_t *)data);

            WM_LOG_DEBG("WIFI_EVENT_STA_DISCONNECTED (reason %d)", u8Reason);

            WM_SendMessage(WM_EVT_DISCONNECT_IND, &u8Reason, sizeof(uint8_t), NULL);
            break;
        }

        case WIFI_EVENT_STA_GOT_IP:
        {
            WM_LOG_DEBG("WIFI_EVENT_STA_GOT_IP");

            // find ip number in network interface
            // uint32_t u32Ip = 0;
            struct netif *iface = netif_find("st1");

            // u8aIp[0] = (iface->ip_addr.u_addr.ip4.addr >> 0) & 0xFF;
            // u8aIp[1] = (iface->ip_addr.u_addr.ip4.addr >> 8) & 0xFF;
            // u8aIp[2] = (iface->ip_addr.u_addr.ip4.addr >> 16) & 0xFF;
            // u8aIp[3] = (iface->ip_addr.u_addr.ip4.addr >> 24) & 0xFF;

            // u32Ip = iface->ip_addr.u_addr.ip4.addr;

            // TODO: will trigger it at event loop task
            WM_UslctedCbRun(USLCTED_CB_EVT_GOT_IP, OPL_OK, (uint8_t*)&iface->ip_addr.u_addr.ip4.addr, sizeof(uint32_t));

			break;
        }

        // event no care
        // case WIFI_EVENT_INIT_COMPLETE:
        // case WIFI_EVENT_STA_STOP:
    }

    return 0;
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
T_OplErr WM_SendMessage(T_WmEvent tWmEventId, uint8_t *u8Data, uint32_t u32DataLen, FsmIndicateCbFunc fpIndCb)
{
    // printf("[WM] Send message (Event %d)\r\n", tWmEventId);

    T_WmMessage *tMsg = (T_WmMessage *)malloc(sizeof(T_WmMessage) + u32DataLen);

    if (NULL == tMsg)
    {
        WM_LOG_ERRO("malloc fail");
        return OPL_ERR_ALLOC_MEMORY_FAIL;
    }

    // check request isn't valid
    if (OPL_OK != WM_CheckRequest(tWmEventId))
    {
        WM_LOG_ERRO("Evt %d can't process", tWmEventId);
        free(tMsg);
        return OPL_ERR_FSM_EVT_INVALID;
    }

    // check wifi modules init status
    if (WM_EVT_INIT_REQ == tWmEventId && true == g_blWifiInited)
    {
        WM_LOG_WARN("WM has been inited");
        return OPL_ERR_FSM_REINIT;
    }

    // prepare the data
    tMsg->tWmEventId = tWmEventId;
    tMsg->fpIndCb = fpIndCb;

    if (u32DataLen != 0)
    {
        tMsg->u32DataLen = u32DataLen;
        memcpy(tMsg->u8aData, u8Data, u32DataLen);
    }

    // send message
    if (osOK != osMessagePut(g_tWmQueueId, (uint32_t)tMsg, 0))
    {
        WM_LOG_ERRO("Send msg fail");
        free(tMsg);
        return OPL_ERR_RTOS_SEND_QMSG_FAIL;
    }

    return OPL_OK;
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
T_OplErr WM_UslctedCbReg(T_WmUslctedCbFp fpUslctedCb)
{
    uint8_t u8Count = 0;

    while (u8Count != WM_USLCTED_CB_REG_NUM)
    {
#if 0
        osSemaphoreWait(g_tWmUslctedCbRegSemId, osWaitForever);
#endif

        if (false == WM_UslctedCbIsReg(u8Count))
        {
            g_tWmUslctedCbFp[u8Count] = fpUslctedCb;

#if 0
            osSemaphoreRelease(g_tWmUslctedCbRegSemId);
#endif

            return OPL_OK;
        }

#if 0
        osSemaphoreRelease(g_tWmUslctedCbRegSemId);
#endif

        u8Count ++;
    }

    return OPL_ERR_USLCTED_CB_REG_INVALID;
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
void WM_UslctedCbReset(void)
{
    memset(g_tWmUslctedCbFp, NULL, sizeof(g_tWmUslctedCbFp));
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
void WM_UslctedCbRun(T_WmUslctedEvtType tEvtType, T_OplErr tEvtRst, uint8_t *pu8Data, uint32_t u32DataLen)
{
    uint8_t u8Count = 0;

    while (u8Count != WM_USLCTED_CB_REG_NUM)
    {
        if (true == WM_UslctedCbIsReg(u8Count))
        {
            g_tWmUslctedCbFp[u8Count](tEvtType, tEvtRst, pu8Data, u32DataLen);
        }
        else
        {
            break;
        }

        u8Count ++;
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
static uint8_t WM_UslctedCbIsReg(uint8_t u8Index)
{
    if (NULL != g_tWmUslctedCbFp[u8Index])
    {
        return true;
    }

    return false;
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
static T_OplErr WM_ModuleEventMapping(T_WmEvent tWmEventId, T_FsmDef **tFsmDef, T_FsmEvent *tEventId)
{
    switch (tWmEventId)
    {
        case WM_EVT_INIT_REQ:
        {
            *tFsmDef = WM_WaFsmDefGet();
            *tEventId = WA_EVT_INIT_REQ;
            break;
        }
        case WM_EVT_INIT_IND:
        {
            *tFsmDef = WM_WaFsmDefGet();
            *tEventId = WA_EVT_INIT_IND;
            break;
        }
        case WM_EVT_SCAN_REQ:
        {
            *tFsmDef = WM_WaFsmDefGet();
            *tEventId = WA_EVT_SCAN_REQ;
            break;
        }
        case WM_EVT_SCAN_IND:
        {
            *tFsmDef = WM_WaFsmDefGet();
            *tEventId = WA_EVT_SCAN_IND;
            break;
        }
        case WM_EVT_CONNECT_REQ:
        {
            *tFsmDef = WM_WaFsmDefGet();
            *tEventId = WA_EVT_CONNECT_REQ;
            break;
        }
        case WM_EVT_CONNECT_PASS_IND:
        {
            *tFsmDef = WM_WaFsmDefGet();
            *tEventId = WA_EVT_CONNECT_PASS_IND;
            break;
        }
        case WM_EVT_CONNECT_FAIL_IND:
        {
            *tFsmDef = WM_WaFsmDefGet();
            *tEventId = WA_EVT_CONNECT_FAIL_IND;
            break;
        }
        case WM_EVT_DISCONNECT_REQ:
        {
            *tFsmDef = WM_WaFsmDefGet();
            *tEventId = WA_EVT_DISCONNECT_REQ;
            break;
        }
        case WM_EVT_DISCONNECT_IND:
        {
            *tFsmDef = WM_WaFsmDefGet();
            *tEventId = WA_EVT_DISCONNECT_IND;
            break;
        }

#if (WM_AC_ENABLED == 1)
        case WM_EVT_AC_ENABLE_REQ:
        {
            *tFsmDef = WM_AcFsmDefGet();
            *tEventId = AC_EVT_ENABLE_AC_REQ;
            break;
        }
        case WM_EVT_AC_DISABLE_REQ:
        {
            *tFsmDef = WM_AcFsmDefGet();
            *tEventId = AC_EVT_DISABLE_AC_REQ;
            break;
        }
        case WM_EVT_AC_DISABLE_NO_DISC_REQ:
        {
            *tFsmDef = WM_AcFsmDefGet();
            *tEventId = AC_EVT_DISABLE_AC_NO_DISC_REQ;
            break;
        }
        case WM_EVT_AC_TIMEOUT_IND:
        {
            *tFsmDef = WM_AcFsmDefGet();
            *tEventId = AC_EVT_AC_TIMEOUT_IND;
            break;
        }
#endif

        default:
        {
            WM_LOG_ERRO("Invalid evt %d to assign module evt", tWmEventId);
            return OPL_ERR;
        }
    }

    return OPL_OK;
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
static T_OplErr WM_CheckRequest(T_WmEvent tWmEventId)
{
    T_FsmDef *tFsmDef = NULL;
    T_FsmEvent tEventId = FSM_EV_NULL_EVENT;

    if (WM_EVT_INIT_REQ != tWmEventId)
    {
        // assign FSM module and request event
        if (OPL_OK != WM_ModuleEventMapping(tWmEventId, &tFsmDef, &tEventId))
        {
            return OPL_ERR;
        }

        // state test (pre-check the event is valid to be process)
        if (OPL_OK != FSM_Test(tFsmDef, tEventId))
        {
            return OPL_ERR_FSM_EVT_INVALID;
        }
    }

    return OPL_OK;
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
static void WM_ProcessMessage(T_WmMessage *tMsg)
{
    T_FsmDef *tFsmDef = NULL;
    T_FsmEvent tEventId = FSM_EV_NULL_EVENT;

    // printf("[WM] Process message (Event %d)\r\n", tMsg->tWmEventId);

    if (WM_EVT_INIT_REQ == tMsg->tWmEventId)
    {
        // tcpip stack and net interface initialization,  dhcp client process initialization.
        lwip_network_init(WIFI_MODE_STA);
        
        // initiate WI-FI agent FSM
        WM_WaInit();

        // initiate WI-FI profile recorder
        WM_PrInit();

#if (WM_AC_ENABLED == 1)
        // initiate WI-FI autoconnect FSM
        WM_AcInit();
#endif

        // rise init status flag
        g_blWifiInited = true;
    }

    // assign FSM module and request event
    WM_ModuleEventMapping(tMsg->tWmEventId, &tFsmDef, &tEventId);

    T_OplErr tEvtRst = FSM_Run(tFsmDef, tEventId, tMsg->u8aData, tMsg->u32DataLen, tMsg->fpIndCb);

    // notify to application while the result not equal OPL_OK
    if (tEvtRst != OPL_OK)
    {        
        // process indicate callback
        if (NULL != tFsmDef->ptFsmIndCbProc)
        {
            tFsmDef->ptFsmIndCbProc(tEventId, tEvtRst);
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
void WM_TaskHandler(void *args)
{
    osEvent tEvent;
    T_WmMessage *tMsg;

    for (;;)
    {
        // wait event
        tEvent = osMessageGet(g_tWmQueueId, osWaitForever);

        if (tEvent.status == osEventMessage)
        {
            tMsg = (T_WmMessage *)tEvent.value.p;

            // process event message
            WM_ProcessMessage(tMsg);

            if (tMsg != NULL)
            {
                free(tMsg);
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
void WM_TaskInit(void)
{
    osThreadDef_t tTaskDef;
    osMessageQDef_t tQueueDef;
#if 0
    osSemaphoreDef_t tSemaphoreDef;
#endif

    // create message queue
    tQueueDef.item_sz = sizeof(T_WmMessage);
    tQueueDef.queue_sz = WM_QUEUE_SIZE;
    g_tWmQueueId = osMessageCreate(&tQueueDef, NULL);

    if (g_tWmQueueId == NULL)
    {
        WM_LOG_ERRO("Create msg queue fail")
    }

#if 0
    // create semaphore
    tSemaphoreDef.dummy = 0;
    g_tWmUslctedCbRegSemId = osSemaphoreCreate(&tSemaphoreDef, 1);

    if (g_tWmUslctedCbRegSemId == NULL)
    {
        WM_LOG_ERRO("Create uslcted semaphore fail");
    }
#endif

    // create task
    tTaskDef.name = "WIFI Manager";
    tTaskDef.stacksize = WM_TASK_STACK_SIZE;
    tTaskDef.tpriority = WM_TASK_PRIORITY;
    tTaskDef.pthread = WM_TaskHandler;
    g_tWmTaskId = osThreadCreate(&tTaskDef, NULL);

    if (g_tWmTaskId == NULL)
    {
        WM_LOG_ERRO("Create task fail");
    }

    WM_LOG_DEBG("Create task ok");
}

#endif /* WM_ENABLED */
