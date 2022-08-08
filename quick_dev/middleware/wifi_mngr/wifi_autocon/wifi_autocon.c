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
*  wifi_autocon.c
*
*  Project:
*  --------
*
*  Description:
*  ------------
*  This implement file is include the auto-connect function and api.
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
#include "qd_config.h"
#include "qd_module.h"
#include "wifi_autocon.h"
#include "wifi_mngr.h"
#include "wifi_mngr_api.h"
#include "wifi_agent.h"
#include "wifi_pro_rec.h"
#include "wifi_types.h"


// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (WM_AC_ENABLED == 1)

// the retry table size of AC timer
// #define WM_AC_INTERVAL_TABLE_SIZE       5

// not found the specifi SSID, to fill the RSSI level
#define WM_AC_PROFILE_RSSI_NOT_FOUND    (-999)

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
static T_FsmDef g_tAcFsmDef;
static uint8_t g_u8AcInitDone = 0;
static T_WmWifiConnectionState g_tAcWifiConnSt = WM_WIFI_ST_NOT_CONNECT;
static T_AcEnableIndCbFp g_tAcEnableCb;
static T_AcDisableIndCbFp g_tAcDisableCb;

static osTimerId g_tAcTriggerTimer;
static uint8_t g_u8AcIntervalIdx;
// static uint32_t g_u8aAcIntervalTbl[WM_AC_INTERVAL_TABLE_SIZE] =
// {
//     30000,
//     30000,
//     60000,
//     60000,
//     900000
// };

static int16_t g_s16aAcProfileMaxRssi[PR_AP_PROFILE_MAX_NUM];
static T_WmConnConfig g_taAcProfileInfo[PR_AP_PROFILE_MAX_NUM];
static uint32_t g_u32AcProfileIdx = 0;
static uint32_t g_u32AcProfileTotal = 0;

// Sec 7: declaration of static function prototype
uint8_t WM_AcIndCbSet(T_FsmEvent tFsmEvent, FsmIndicateCbFunc fpIndCb);
void WM_AcTimerTrigger(void const *argu);
void WM_AcConnResultCb(T_OplErr tEvtRst);
void WM_AcScanResultCb(T_OplErr tEvtRst);
void WM_AcDiscResultCb(T_OplErr tEvtRst);
void WM_AcDisCIndCb(T_WmUslctedEvtType tEvtType, T_OplErr tEvtRst, uint8_t *pu8Data, uint32_t u32DataLen);

T_FsmState WM_AcDoAutoConn(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_FsmState WM_AcDoScan(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_FsmState WM_AcDoDedicateConn(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_FsmState WM_AcDoHiddenConn(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_FsmState WM_AcDoDisconnect(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);

// Event table
T_OplErr WM_AcNull_EnableHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcIdle_DiscIndHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcIdle_AcTimerTimeoutHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcIdle_DisableHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcIdle_DisableNoDiscHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcFastConn_ConnSuccessHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcFastConn_ConnFailHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcFastConn_DisableHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcFastConn_DisableNoDiscHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcHiddenScan_ScanDoneHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcHiddenScan_DisableHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcHiddenScan_DisableNoDiscHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcDedicateConn_ConnSuccessHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcDedicateConn_ConnFailHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcDedicateConn_DisableHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcDedicateConn_DisableNoDiscHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcHiddenConn_ConnSuccessHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcHiddenConn_ConnFailHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcHiddenConn_DisableHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcHiddenConn_DisableNoDiscHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcDisaWait_ScanDoneHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcDisaWait_ConnFailHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcDisaWait_ConnSuccessHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcDisaWaitNoDisc_ScanDoneHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcDisaWaitNoDisc_ConnFailHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcDisaWaitNoDisc_ConnSuccessHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcDisaWaitDisc_DiscSuccessHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);
T_OplErr WM_AcWifiResetHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen);

static const T_FsmStateExctblEvent g_tAcNullExcTbl[] =
{
    {AC_EVT_ENABLE_AC_REQ,          WM_AcNull_EnableHandler},

    {FSM_EV_NULL_EVENT,             FSM_AT_NULL_ACTION}
};

static const T_FsmStateExctblEvent g_tAcIdleExcTbl[] =
{
    {AC_EVT_DISCONNECT_IND,         WM_AcIdle_DiscIndHandler},
    {AC_EVT_AC_TIMEOUT_IND,         WM_AcIdle_AcTimerTimeoutHandler},
    {AC_EVT_DISABLE_AC_REQ,         WM_AcIdle_DisableHandler},
    {AC_EVT_DISABLE_AC_NO_DISC_REQ, WM_AcIdle_DisableNoDiscHandler},
    {AC_EVT_WIFI_RESET_IND,         WM_AcWifiResetHandler},

    {FSM_EV_NULL_EVENT,             FSM_AT_NULL_ACTION}
};

static const T_FsmStateExctblEvent g_tAcFastConnExcTbl[] =
{
    {AC_EVT_CONNECT_SUCCESS_IND,    WM_AcFastConn_ConnSuccessHandler},
    {AC_EVT_CONNECT_FAIL_IND,       WM_AcFastConn_ConnFailHandler},
    {AC_EVT_DISABLE_AC_REQ,         WM_AcFastConn_DisableHandler},
    {AC_EVT_DISABLE_AC_NO_DISC_REQ, WM_AcFastConn_DisableNoDiscHandler},
    {AC_EVT_WIFI_RESET_IND,         WM_AcWifiResetHandler},

    {FSM_EV_NULL_EVENT,             FSM_AT_NULL_ACTION}
};

static const T_FsmStateExctblEvent g_tAcHiddeScanExcTbl[] =
{
    {AC_EVT_SCAN_DONE_IND,          WM_AcHiddenScan_ScanDoneHandler},
    {AC_EVT_DISABLE_AC_REQ,         WM_AcHiddenScan_DisableHandler},
    {AC_EVT_DISABLE_AC_NO_DISC_REQ, WM_AcHiddenScan_DisableNoDiscHandler},
    {AC_EVT_WIFI_RESET_IND,         WM_AcWifiResetHandler},

    {FSM_EV_NULL_EVENT,             FSM_AT_NULL_ACTION}
};

static const T_FsmStateExctblEvent g_tAcDedicateConnExcTbl[] =
{
    {AC_EVT_CONNECT_SUCCESS_IND,    WM_AcDedicateConn_ConnSuccessHandler},
    {AC_EVT_CONNECT_FAIL_IND,       WM_AcDedicateConn_ConnFailHandler},
    {AC_EVT_DISABLE_AC_REQ,         WM_AcDedicateConn_DisableHandler},
    {AC_EVT_DISABLE_AC_NO_DISC_REQ, WM_AcDedicateConn_DisableNoDiscHandler},
    {AC_EVT_WIFI_RESET_IND,         WM_AcWifiResetHandler},

    {FSM_EV_NULL_EVENT,             FSM_AT_NULL_ACTION}
};

static const T_FsmStateExctblEvent g_tAcHiddenConnExcTbl[] =
{
    {AC_EVT_CONNECT_SUCCESS_IND,    WM_AcHiddenConn_ConnSuccessHandler},
    {AC_EVT_CONNECT_FAIL_IND,       WM_AcHiddenConn_ConnFailHandler},
    {AC_EVT_DISABLE_AC_REQ,         WM_AcHiddenConn_DisableHandler},
    {AC_EVT_DISABLE_AC_NO_DISC_REQ, WM_AcHiddenConn_DisableNoDiscHandler},
    {AC_EVT_WIFI_RESET_IND,         WM_AcWifiResetHandler},

    {FSM_EV_NULL_EVENT,             FSM_AT_NULL_ACTION}    
};

static const T_FsmStateExctblEvent g_tAcDisaWaitIndExcTbl[] =
{
    {AC_EVT_SCAN_DONE_IND,          WM_AcDisaWait_ScanDoneHandler},
    {AC_EVT_CONNECT_FAIL_IND,       WM_AcDisaWait_ConnFailHandler},
    {AC_EVT_CONNECT_SUCCESS_IND,    WM_AcDisaWait_ConnSuccessHandler},
    {AC_EVT_WIFI_RESET_IND,         WM_AcWifiResetHandler},

    {FSM_EV_NULL_EVENT,             FSM_AT_NULL_ACTION}
};

static const T_FsmStateExctblEvent g_tAcDisaWaitIndNoDiscExcTbl[] =
{
    {AC_EVT_SCAN_DONE_IND,          WM_AcDisaWaitNoDisc_ScanDoneHandler},
    {AC_EVT_CONNECT_FAIL_IND,       WM_AcDisaWaitNoDisc_ConnFailHandler},
    {AC_EVT_CONNECT_SUCCESS_IND,    WM_AcDisaWaitNoDisc_ConnSuccessHandler},
    {AC_EVT_WIFI_RESET_IND,         WM_AcWifiResetHandler},

    {FSM_EV_NULL_EVENT,             FSM_AT_NULL_ACTION}
};

static const T_FsmStateExctblEvent g_tAcDisaWaitDiscExcTbl[] =
{
    {AC_EVT_DISC_SUCCESS_IND,       WM_AcDisaWaitDisc_DiscSuccessHandler},
    {AC_EVT_WIFI_RESET_IND,         WM_AcWifiResetHandler},

    {FSM_EV_NULL_EVENT,             FSM_AT_NULL_ACTION}
};

// State table
static const T_FsmStateTable g_tAcStateTbl[] =
{
    {g_tAcNullExcTbl},
    {g_tAcIdleExcTbl},
    {g_tAcFastConnExcTbl},
    {g_tAcHiddeScanExcTbl},
    {g_tAcDedicateConnExcTbl},
    {g_tAcHiddenConnExcTbl},
    {g_tAcDisaWaitIndExcTbl},
    {g_tAcDisaWaitIndNoDiscExcTbl},
    {g_tAcDisaWaitDiscExcTbl}
};


/***********
C Functions
***********/
// Sec 8: C Functions

/*************************************************************************
* FUNCTION
*   WM_AcInit
*
* DESCRIPTION
*   the initial of auto-connect
*
* PARAMETERS
*   none
*
* RETURNS
*   0: Success
*   Other: Fail
*
*************************************************************************/
int32_t WM_AcInit(void)
{
    osTimerDef_t timer_def;

    // error check
    if (g_u8AcInitDone == 1)
    {
        AC_LOG_WARN("AC has been inited");
        return -1;
    }

    // name of auto-connect
    uint8_t u8aAcName[2] = "AC";

    // initialize the Auto-Connect state machine
    FSM_Init(&g_tAcFsmDef, u8aAcName, g_tAcStateTbl, AC_ST_NULL);

    // initialize the Auto-Connect indicate callback handler
    FSM_IndCbHdlInit(&g_tAcFsmDef, WM_AcIndCbSet, NULL);

    // register the disc callback function
    WM_UslctedCbReg(WM_AcDisCIndCb);

    // callback function
    g_tAcEnableCb = NULL;
    g_tAcDisableCb = NULL;

    // interval index
    g_u8AcIntervalIdx = 0;

    // create the timeout function
    timer_def.ptimer = WM_AcTimerTrigger;
    g_tAcTriggerTimer = osTimerCreate(&timer_def, osTimerOnce, NULL);
    if (g_tAcTriggerTimer == NULL)
    {
        AC_LOG_ERRO("Create ac timer fail");
        return -1;
    }

    // profile information
    g_u32AcProfileIdx = 0;
    g_u32AcProfileTotal = 0;

    // init done
    g_u8AcInitDone = 1;

    // printf("WM: AC init is done\r\n");
    return 0;
}

/*************************************************************************
* FUNCTION
*   WM_AcFsmDefGet
*
* DESCRIPTION
*   get the FSM item of auto-connect
*
* PARAMETERS
*   none
*
* RETURNS
*   the FSM item of auto-connect
*
*************************************************************************/
T_FsmDef* WM_AcFsmDefGet(void)
{
    return &g_tAcFsmDef;
}

/*************************************************************************
* FUNCTION
*   WM_AcIndCbSet
*
* DESCRIPTION
*   set the callback function of auto-connect
*
* PARAMETERS
*   1. tFsmEvent [In] : the event ID
*   2. fpIndCb   [In] : the callback function pointer
*
* RETURNS
*   FSM_CHECK_IND_CB_VALID      : the valid behavior
*   FSM_CHECK_IND_CB_INVALID    : the in-valid behavior
*   FSM_CHECK_IND_CB_NOT_RELATE : the callback function is not related
*
*************************************************************************/
T_OplErr WM_AcIndCbSet(T_FsmEvent tFsmEvent, FsmIndicateCbFunc fpIndCb)
{
    switch (tFsmEvent)
    {
        case AC_EVT_ENABLE_AC_REQ:
        {
            g_tAcEnableCb = (T_AcEnableIndCbFp)fpIndCb;
            return OPL_OK;
        }

        case AC_EVT_DISABLE_AC_REQ:
        case AC_EVT_DISABLE_AC_NO_DISC_REQ:
        {
            g_tAcDisableCb = (T_AcDisableIndCbFp)fpIndCb;
            return OPL_OK;
        }

        default:
            break;
    }

    return OPL_ERR_IND_CB_NOT_RELATED;
}

/*************************************************************************
* FUNCTION
*   WM_AcTimerTrigger
*
* DESCRIPTION
*   the timeout function of AC timer
*
* PARAMETERS
*   1. argu [In] : the argument of timeout
*
* RETURNS
*   none
*
*************************************************************************/
void WM_AcTimerTrigger(void const *argu)
{
    // push AC_EVT_AC_TIMEOUT_IND msg
    WM_SendMessage(WM_EVT_AC_TIMEOUT_IND, NULL, 0, NULL);
}

/*************************************************************************
* FUNCTION
*   WM_AcConnResultCb
*
* DESCRIPTION
*   the result of fast connect or dedicate connect
*
* PARAMETERS
*   1. s8Status [In] : connect success of fail
*
* RETURNS
*   none
*
*************************************************************************/
void WM_AcConnResultCb(T_OplErr tEvtRst)
{
    // success
    if (OPL_OK == tEvtRst)
    {
        // reset the value of AC interval index
        g_u8AcIntervalIdx = 0;

        // reset the value of AC profile
        g_u32AcProfileIdx = 0;
        g_u32AcProfileTotal = 0;

        FSM_Run(WM_AcFsmDefGet(), AC_EVT_CONNECT_SUCCESS_IND, NULL, 0, NULL);
    }
    // fail
    else
    {
        FSM_Run(WM_AcFsmDefGet(), AC_EVT_CONNECT_FAIL_IND, NULL, 0, NULL);
    }
}

/*************************************************************************
* FUNCTION
*   WM_AcScanResultCb
*
* DESCRIPTION
*   the result of wifi scan
*
* PARAMETERS
*   1. s8Status [In] : connect success of fail
*
* RETURNS
*   none
*
*************************************************************************/
void WM_AcScanResultCb(T_OplErr tEvtRst)
{
    FSM_Run(WM_AcFsmDefGet(), AC_EVT_SCAN_DONE_IND, NULL, 0, NULL);
}

/*************************************************************************
* FUNCTION
*   WM_AcDiscResultCb
*
* DESCRIPTION
*   the result of wifi disconnect
*
* PARAMETERS
*   1. s8Status [In] : connect success of fail
*
* RETURNS
*   none
*
*************************************************************************/
void WM_AcDiscResultCb(T_OplErr tEvtRst)
{
    FSM_Run(WM_AcFsmDefGet(), AC_EVT_DISC_SUCCESS_IND, NULL, 0, NULL);
}

/*************************************************************************
* FUNCTION
*   WM_AcDisCIndCb
*
* DESCRIPTION
*   Disconnect IND
*
* PARAMETERS
*   1. tEvtType [In] : the event type of unsolicited IND
*   2. tEvtRst  [In] : the error code
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   none
*
*************************************************************************/
void WM_AcDisCIndCb(T_WmUslctedEvtType tEvtType, T_OplErr tEvtRst, uint8_t *pu8Data, uint32_t u32DataLen)
{
    if (tEvtType == USLCTED_CB_EVT_WIFI_UP)
    {
        g_tAcWifiConnSt = WM_WIFI_ST_CONNECTED;
    }
    else if (tEvtType == USLCTED_CB_EVT_WIFI_DOWN)
    {
        g_tAcWifiConnSt = WM_WIFI_ST_NOT_CONNECT;

        FSM_Run(WM_AcFsmDefGet(), AC_EVT_DISCONNECT_IND, NULL, 0, NULL);
    }
    else if (tEvtType == USLCTED_CB_EVT_WIFI_RESET)
    {
        g_tAcWifiConnSt = WM_WIFI_ST_NOT_CONNECT;

        FSM_Run(WM_AcFsmDefGet(), AC_EVT_WIFI_RESET_IND, pu8Data, u32DataLen, NULL);
    }
}

/*************************************************************************
* FUNCTION
*   WM_AcDoAutoConn
*
* DESCRIPTION
*   Do auto connect in every state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   the next state
*
*************************************************************************/
T_FsmState WM_AcDoAutoConn(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_OplErr tEvtRst;
    T_FsmState tNextState;

    // if the count is 0, bypass FastConn
    uint8_t u8Number = 0;
    WM_WaAutoConnectListNumGet(&u8Number);

    if (u8Number == 0)
    {
        tNextState = WM_AcDoScan(tFsmState, tFsmEvent, pu8Data, u32DataLen);
        goto done;
    }
    
    // execute the auto-connect req
    tEvtRst = FSM_Run(WM_WaFsmDefGet(), WA_EVT_CONNECT_REQ, NULL, 0, (FsmIndicateCbFunc)WM_AcConnResultCb);

    if (OPL_OK != tEvtRst)
    {
        if ((OPL_ERR_FSM_EVT_INVALID == tEvtRst) && (AC_ST_NULL == tFsmState))
        {
            // while the connection been connected, back to idle directly
            tNextState = AC_ST_IDLE;
            goto done;
        }
        else
        {
            // if fail, bypass FastConn
            tNextState = WM_AcDoScan(tFsmState, tFsmEvent, pu8Data, u32DataLen);
            goto done;

#if 0
            // check the count of profile
            if (0 != WM_PrProfileCount())
            {
                // if the count is not 0, do scan
                tNextState = WM_AcDoScan(tFsmState, tFsmEvent, pu8Data, u32DataLen);
                goto done;
            }
            else
            {
                // if the count is 0, bypass HiddenScan and DedicateConn

                // start the AC timer, then change the state to Idle
                osTimerStop(g_tAcTriggerTimer);
                osTimerStart(g_tAcTriggerTimer, WM_AC_RETRY_INTVL_TBL[g_u8AcIntervalIdx]);
                // osTimerStart(g_tAcTriggerTimer, g_u8aAcIntervalTbl[g_u8AcIntervalIdx]);

                tNextState = AC_ST_IDLE;
                goto done;
            }
#endif
        }
    }

    // change the state to FastConn
    tNextState = AC_ST_FAST_CONNECTING;
    goto done;

done:
    return tNextState;
}

/*************************************************************************
* FUNCTION
*   WM_AcDoScan
*
* DESCRIPTION
*   Do scan in every state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   the next state
*
*************************************************************************/
T_FsmState WM_AcDoScan(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_FsmState tNextState;
    wifi_scan_config_t tScanConfig = {0};

    if (0 == WM_PrProfileCount())
    {
        // if the count is 0, bypass HiddenScan and DedicateConn

        // start the AC timer, then change the state to Idle
        osTimerStop(g_tAcTriggerTimer);
        osTimerStart(g_tAcTriggerTimer, WM_AC_RETRY_INTVL_TBL[g_u8AcIntervalIdx]);
        // osTimerStart(g_tAcTriggerTimer, g_u8aAcIntervalTbl[g_u8AcIntervalIdx]);

        tNextState = AC_ST_IDLE;
        goto done;
    }

    // if the count is not 0, do scan
    // execute the scan req
    tScanConfig.show_hidden = 1;
    tScanConfig.scan_type = WIFI_SCAN_TYPE_MIX;
    if (OPL_OK == FSM_Run(WM_WaFsmDefGet(), WA_EVT_SCAN_REQ, (uint8_t*)&tScanConfig, sizeof(wifi_scan_config_t), (FsmIndicateCbFunc)WM_AcScanResultCb))
    {
        // if true, change the state to hidden scanning
        tNextState = AC_ST_HIDDEN_SCANNING;
        goto done;
    }
    else
    {
        // if scan fail, bypass HiddenScan and DedicateConn

        // start the AC timer, then change the state to Idle
        osTimerStop(g_tAcTriggerTimer);
        osTimerStart(g_tAcTriggerTimer, WM_AC_RETRY_INTVL_TBL[g_u8AcIntervalIdx]);
        // osTimerStart(g_tAcTriggerTimer, g_u8aAcIntervalTbl[g_u8AcIntervalIdx]);

        tNextState = AC_ST_IDLE;
        goto done;
    }

done:
    return tNextState;
}

/*************************************************************************
* FUNCTION
*   WM_AcDoDedicateConn
*
* DESCRIPTION
*   Do dedicate connect in every state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   the next state
*
*************************************************************************/
T_FsmState WM_AcDoDedicateConn(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_FsmState tNextState;
    T_PrApProfile *ptProList;
    T_PrApProfile *ptCurrProfile;
    wifi_scan_list_t *pstScanList = NULL;
    T_WmConnConfig tConnConfig = {0};

    int16_t s16TmpRssi;
    uint32_t i, j;

    // the first req
    if (0 == g_u32AcProfileIdx)
    {
        // get the information of Profile
        ptProList = WM_PrGet();

        pstScanList = (wifi_scan_list_t *)malloc(sizeof(wifi_scan_list_t));
			
        if(NULL == pstScanList)
        {
            AC_LOG_ERRO("malloc fail");

            return OPL_ERR_ALLOC_MEMORY_FAIL;
        }

        memset(pstScanList, 0, sizeof(wifi_scan_list_t));

        // get the information of scan list			
        WM_WaScanListGet(pstScanList);

        // update the max RSSI of profile list
        // TODO: Will need to maintain a flag to content is hidden AP included in scan list?
        for (i = 0, g_u32AcProfileTotal = 0; i < PR_AP_PROFILE_MAX_NUM; i++)
        {
            if(ptProList[i].u8Used != 0)
            {
                ptCurrProfile = ptProList + i;

                g_s16aAcProfileMaxRssi[g_u32AcProfileTotal] = WM_AC_PROFILE_RSSI_NOT_FOUND;
                
                memcpy(g_taAcProfileInfo[g_u32AcProfileTotal].ssid, ptCurrProfile->u8Ssid, WIFI_MAX_LENGTH_OF_SSID);
                g_taAcProfileInfo[g_u32AcProfileTotal].ssid_length = strlen((char*)g_taAcProfileInfo[g_u32AcProfileTotal].ssid);
                memcpy(g_taAcProfileInfo[g_u32AcProfileTotal].password, ptCurrProfile->u8Pwd, WIFI_LENGTH_PASSPHRASE);
                g_taAcProfileInfo[g_u32AcProfileTotal].password_length = strlen((char*)g_taAcProfileInfo[g_u32AcProfileTotal].password);

                for (j = 0; j < pstScanList->num; j++)
                {
                    if (0 == memcmp(ptCurrProfile->u8Ssid, pstScanList->ap_record[j].ssid, WIFI_MAX_LENGTH_OF_SSID))
                    {
                        if (pstScanList->ap_record[j].rssi > g_s16aAcProfileMaxRssi[g_u32AcProfileTotal])
                        {
                            g_s16aAcProfileMaxRssi[g_u32AcProfileTotal] = pstScanList->ap_record[j].rssi;
                            memcpy(g_taAcProfileInfo[g_u32AcProfileTotal].bssid, pstScanList->ap_record[j].bssid, WIFI_MAC_ADDRESS_LENGTH);
                        }
                    }
                }

                g_u32AcProfileTotal ++;
            }
        }

        free(pstScanList);

        // sort the SSID profile from max to min RSSI
        for (i = 0; i < g_u32AcProfileTotal; i++)
        {
            for (j = 0; j < (g_u32AcProfileTotal-1); j++)
            {
                if (g_s16aAcProfileMaxRssi[j] < g_s16aAcProfileMaxRssi[j+1])
                {
                    s16TmpRssi = g_s16aAcProfileMaxRssi[j+1];
                    memcpy(&tConnConfig, &g_taAcProfileInfo[j+1], sizeof(T_WmConnConfig));

                    g_s16aAcProfileMaxRssi[j+1] = g_s16aAcProfileMaxRssi[j];
                    memcpy(&g_taAcProfileInfo[j+1], &g_taAcProfileInfo[j], sizeof(T_WmConnConfig));

                    g_s16aAcProfileMaxRssi[j] = s16TmpRssi;
                    memcpy(&g_taAcProfileInfo[j], &tConnConfig, sizeof(T_WmConnConfig));
                }
            }

            // for debug
            AC_LOG_DEBG("Sort AP list %d {RSSI %d, %s, %X:%X:%X:%X:%X:%X, %s", i,
                                                                            g_s16aAcProfileMaxRssi[i],
                                                                            g_taAcProfileInfo[i].ssid,
                                                                            g_taAcProfileInfo[i].bssid[0],
                                                                            g_taAcProfileInfo[i].bssid[1],
                                                                            g_taAcProfileInfo[i].bssid[2],
                                                                            g_taAcProfileInfo[i].bssid[3],
                                                                            g_taAcProfileInfo[i].bssid[4],
                                                                            g_taAcProfileInfo[i].bssid[5],
                                                                            g_taAcProfileInfo[i].password);
        }
    }

    // execute the dedicate connect req
    for (i = g_u32AcProfileIdx; i < g_u32AcProfileTotal; i++)
    {
        // not found in scan list
        if (WM_AC_PROFILE_RSSI_NOT_FOUND == g_s16aAcProfileMaxRssi[i])
        {
            continue;
        }

        AC_LOG_INFO("Try dedicate connection (count %d)", i);

        // remove the ssid to do bssid dedicate connection
        memcpy(&tConnConfig, &g_taAcProfileInfo[i], sizeof(T_WmConnConfig));
        memset(tConnConfig.ssid, 0x00, WIFI_MAX_LENGTH_OF_SSID);
        tConnConfig.ssid_length = 0;

        if (OPL_OK == FSM_Run(WM_WaFsmDefGet(), WA_EVT_CONNECT_REQ, (uint8_t*)&tConnConfig, sizeof(T_WmConnConfig), (FsmIndicateCbFunc)WM_AcConnResultCb))
        {
            break;
        }
    }

    // the end
    if (i >= g_u32AcProfileTotal)
    {
        tNextState = AC_ST_HIDDEN_CONNECTING;
        goto done;
    }

    tNextState = AC_ST_DEDICATE_CONNECTING;
    goto done;

done:
    return tNextState;
}

/*************************************************************************
* FUNCTION
*   WM_AcDoHiddenConn
*
* DESCRIPTION
*   Do hidden connect in every state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   the next state
*
*************************************************************************/
T_FsmState WM_AcDoHiddenConn(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_FsmState tNextState;
    T_WmConnConfig tConnConfig = {0};

    uint32_t i;

    // execute the dedicate connect req
    for (i = g_u32AcProfileIdx; i < g_u32AcProfileTotal; i++)
    {
        AC_LOG_INFO("Try hidden connection (count %d)", i);

        // remove the bssid to do ssid dedicate connection
        memcpy(&tConnConfig, &g_taAcProfileInfo[i], sizeof(T_WmConnConfig));
        memset(tConnConfig.bssid, 0x00, WIFI_MAC_ADDRESS_LENGTH);

        if (OPL_OK == FSM_Run(WM_WaFsmDefGet(), WA_EVT_CONNECT_REQ, (uint8_t*)&tConnConfig, sizeof(T_WmConnConfig), (FsmIndicateCbFunc)WM_AcConnResultCb))
        {
            break;
        }
    }

    // the end
    if (i >= g_u32AcProfileTotal)
    {
        // if the end
        // start the AC timer, then change the state to Idle
        osTimerStop(g_tAcTriggerTimer);
        osTimerStart(g_tAcTriggerTimer, WM_AC_RETRY_INTVL_TBL[g_u8AcIntervalIdx]);
        // osTimerStart(g_tAcTriggerTimer, g_u8aAcIntervalTbl[g_u8AcIntervalIdx]);

        tNextState = AC_ST_IDLE;
        goto done;
    }

    tNextState = AC_ST_HIDDEN_CONNECTING;
    goto done;

done:
    return tNextState;
}

/*************************************************************************
* FUNCTIONf
*   WM_AcDoDisconnect
*
* DESCRIPTION
*   Do disconnect in every state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   the next state
*
*************************************************************************/
T_FsmState WM_AcDoDisconnect(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_FsmState tNextState;

    // execute the wifi disconnect req
    if (OPL_OK != FSM_Run(WM_WaFsmDefGet(), WA_EVT_DISCONNECT_REQ, NULL, 0, (FsmIndicateCbFunc)WM_AcDiscResultCb))
    {
        // if fail (it is not needed to disconnect), then change the state to Null
        // change the current state
        tNextState = AC_ST_NULL;
        goto done;
    }

    // change the current state
    tNextState = AC_ST_DISA_WAIT_DISC;
    goto done;

done:
    return tNextState;
}

/*************************************************************************
* FUNCTION
*   WM_AcNull_EnableHandler
*
* DESCRIPTION
*   AC enable in Null state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcNull_EnableHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_FsmState tNextState;

    // the init value of AC interval index
    g_u8AcIntervalIdx = 0;

    if(g_tAcWifiConnSt == WM_WIFI_ST_NOT_CONNECT)
    {
        // execute the auto-connect req
        tNextState = WM_AcDoAutoConn(tFsmState, tFsmEvent, pu8Data, u32DataLen);
    }
    else if(g_tAcWifiConnSt == WM_WIFI_ST_CONNECTED)
    {
        tNextState = AC_ST_IDLE;
    }

    // change the current state
    FSM_StateChange(&g_tAcFsmDef, tNextState);

    // execute the callback of AC enable
    if (g_tAcEnableCb != NULL)
    {
        g_tAcEnableCb(OPL_OK, &g_tAcWifiConnSt, sizeof(g_tAcWifiConnSt));
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcIdle_DiscIndHandler
*
* DESCRIPTION
*   Disconnet IND in Idle state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcIdle_DiscIndHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_FsmState tNextState;

    // reset the value of AC interval index
    g_u8AcIntervalIdx = 0;

    // execute the auto-connect req
    tNextState = WM_AcDoAutoConn(tFsmState, tFsmEvent, pu8Data, u32DataLen);

    // change the current state
    FSM_StateChange(&g_tAcFsmDef, tNextState);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcIdle_AcTimerTimeoutHandler
*
* DESCRIPTION
*   AC timeout IND in Idle state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcIdle_AcTimerTimeoutHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_FsmState tNextState;

    // update the next value of AC interval index
    if (g_u8AcIntervalIdx < (sizeof(WM_AC_RETRY_INTVL_TBL[0])/sizeof(WM_AC_RETRY_INTVL_TBL) - 1))
    // if (g_u8AcIntervalIdx < (WM_AC_INTERVAL_TABLE_SIZE - 1))
    {
        g_u8AcIntervalIdx++;
    }

    // execute the auto-connect req
    tNextState = WM_AcDoAutoConn(tFsmState, tFsmEvent, pu8Data, u32DataLen);

    // change the current state
    FSM_StateChange(&g_tAcFsmDef, tNextState);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcIdle_DisableHandler
*
* DESCRIPTION
*   AC disable in Idle state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcIdle_DisableHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_FsmState tNextState;

    // execute the wifi disconnect req
    tNextState = WM_AcDoDisconnect(tFsmState, tFsmEvent, pu8Data, u32DataLen);

    // change the current state
    FSM_StateChange(&g_tAcFsmDef, tNextState);

    // if tNextState is AC_ST_NULL, execute the callback function of AC disable
    if (tNextState == AC_ST_NULL)
    {
        // stop auto-connect trigger timer
        osTimerStop(g_tAcTriggerTimer);

        // execute the callback of AC disable
        if (g_tAcDisableCb != NULL)
        {
            g_tAcDisableCb(OPL_OK);
        }
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcIdle_DisableNoDiscHandler
*
* DESCRIPTION
*   AC disable without disconnect in Idle state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcIdle_DisableNoDiscHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // change the current state
    FSM_StateChange(&g_tAcFsmDef, AC_ST_NULL);

    // stop auto-connect trigger timer
    osTimerStop(g_tAcTriggerTimer);

    // execute the callback of AC disable
    if (g_tAcDisableCb != NULL)
    {
        g_tAcDisableCb(OPL_OK);
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcFastConn_ConnSuccessHandler
*
* DESCRIPTION
*   Connect success IND in FastConn state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcFastConn_ConnSuccessHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // change the current state
    FSM_StateChange(&g_tAcFsmDef, AC_ST_IDLE);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcFastConn_ConnFailHandler
*
* DESCRIPTION
*   Connect fail IND in FastConn state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcFastConn_ConnFailHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_FsmState tNextState;

    tNextState = WM_AcDoScan(tFsmState, tFsmEvent, pu8Data, u32DataLen);

    // change the current state
    FSM_StateChange(&g_tAcFsmDef, tNextState);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcFastConn_DisableHandler
*
* DESCRIPTION
*   AC disable in FastConn state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcFastConn_DisableHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // change the current state
    FSM_StateChange(&g_tAcFsmDef, AC_ST_DISA_WAIT_IND);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcFastConn_DisableNoDiscHandler
*
* DESCRIPTION
*   AC disable without disconnect in FastConn state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcFastConn_DisableNoDiscHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // change the current state
    FSM_StateChange(&g_tAcFsmDef, AC_ST_DISA_WAIT_IND_NO_DISC);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcHiddenScan_ScanDoneHandler
*
* DESCRIPTION
*   Scan done IND in HiddenScan state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcHiddenScan_ScanDoneHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_FsmState tNextState;

    // reset the value of AC profile
    g_u32AcProfileIdx = 0;
    g_u32AcProfileTotal = 0;

    tNextState = WM_AcDoDedicateConn(tFsmState, tFsmEvent, pu8Data, u32DataLen);

    if(AC_ST_HIDDEN_CONNECTING == tNextState)
    {
        g_u32AcProfileIdx = 0;

        tNextState = WM_AcDoHiddenConn(tFsmState, tFsmEvent, pu8Data, u32DataLen);
    }

    // change the current state
    FSM_StateChange(&g_tAcFsmDef, tNextState);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcHiddenScan_DisableHandler
*
* DESCRIPTION
*   AC disable in HiddenScan state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcHiddenScan_DisableHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // change the current state
    FSM_StateChange(&g_tAcFsmDef, AC_ST_DISA_WAIT_IND);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcHiddenScan_DisableNoDiscHandler
*
* DESCRIPTION
*   AC disable without disconnect in HiddenScan state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcHiddenScan_DisableNoDiscHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // change the current state
    FSM_StateChange(&g_tAcFsmDef, AC_ST_DISA_WAIT_IND_NO_DISC);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcDedicateConn_ConnSuccessHandler
*
* DESCRIPTION
*   Connect success IND in DedicateConn state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcDedicateConn_ConnSuccessHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // change the current state
    FSM_StateChange(&g_tAcFsmDef, AC_ST_IDLE);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcDedicateConn_ConnFailHandler
*
* DESCRIPTION
*   Connect fail IND in DedicateConn state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcDedicateConn_ConnFailHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_FsmState tNextState;

    // update the value of AC profile
    g_u32AcProfileIdx++;

    tNextState = WM_AcDoDedicateConn(tFsmState, tFsmEvent, pu8Data, u32DataLen);

    if(AC_ST_HIDDEN_CONNECTING == tNextState)
    {
        g_u32AcProfileIdx = 0;

        tNextState = WM_AcDoHiddenConn(tFsmState, tFsmEvent, pu8Data, u32DataLen);
    }

    // change the current state
    FSM_StateChange(&g_tAcFsmDef, tNextState);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcDedicateConn_DisableHandler
*
* DESCRIPTION
*   AC disable in DedicateConn state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcDedicateConn_DisableHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // change the current state
    FSM_StateChange(&g_tAcFsmDef, AC_ST_DISA_WAIT_IND);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcDedicateConn_DisableNoDiscHandler
*
* DESCRIPTION
*   AC disable without disconnect in DedicateConn state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcDedicateConn_DisableNoDiscHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // change the current state
    FSM_StateChange(&g_tAcFsmDef, AC_ST_DISA_WAIT_IND_NO_DISC);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcHiddenConn_ConnSuccessHandler
*
* DESCRIPTION
*   Connect success IND in HiddenConn state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcHiddenConn_ConnSuccessHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // change the current state
    FSM_StateChange(&g_tAcFsmDef, AC_ST_IDLE);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcHiddenConn_ConnFailHandler
*
* DESCRIPTION
*   Connect fail IND in HiddenConn state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcHiddenConn_ConnFailHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_FsmState tNextState;

    // update the value of AC profile
    g_u32AcProfileIdx++;

    tNextState = WM_AcDoHiddenConn(tFsmState, tFsmEvent, pu8Data, u32DataLen);

    // change the current state
    FSM_StateChange(&g_tAcFsmDef, tNextState);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcHiddenConn_DisableHandler
*
* DESCRIPTION
*   AC disable in HiddenConn state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcHiddenConn_DisableHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // change the current state
    FSM_StateChange(&g_tAcFsmDef, AC_ST_DISA_WAIT_IND);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcHiddenConn_DisableNoDiscHandler
*
* DESCRIPTION
*   AC disable without disconnect in HiddenConn state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcHiddenConn_DisableNoDiscHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // change the current state
    FSM_StateChange(&g_tAcFsmDef, AC_ST_DISA_WAIT_IND_NO_DISC);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcDisaWait_ScanDoneHandler
*
* DESCRIPTION
*   Scan done IND in DisaWait state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcDisaWait_ScanDoneHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // change the current state
    FSM_StateChange(&g_tAcFsmDef, AC_ST_NULL);

    // stop auto-connect trigger timer
    osTimerStop(g_tAcTriggerTimer);

    // execute the callback of AC disable
    if (g_tAcDisableCb != NULL)
    {
        g_tAcDisableCb(OPL_OK);
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcDisaWait_ConnFailHandler
*
* DESCRIPTION
*   Connect fail IND in DisaWait state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcDisaWait_ConnFailHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // change the current state
    FSM_StateChange(&g_tAcFsmDef, AC_ST_NULL);

    // stop auto-connect trigger timer
    osTimerStop(g_tAcTriggerTimer);

    // execute the callback of AC disable
    if (g_tAcDisableCb != NULL)
    {
        g_tAcDisableCb(OPL_OK);
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcDisaWait_ConnSuccessHandler
*
* DESCRIPTION
*   Connect success IND in DisaWait state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcDisaWait_ConnSuccessHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_FsmState tNextState;

    // execute the wifi disconnect req
    tNextState = WM_AcDoDisconnect(tFsmState, tFsmEvent, pu8Data, u32DataLen);

    // change the current state
    FSM_StateChange(&g_tAcFsmDef, tNextState);

    // if tNextState is AC_ST_NULL, execute the callback function of AC disable
    if (tNextState == AC_ST_NULL)
    {
        // stop auto-connect trigger timer
        osTimerStop(g_tAcTriggerTimer);
        
        // execute the callback of AC disable
        if (g_tAcDisableCb != NULL)
        {
            g_tAcDisableCb(OPL_OK);
        }
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcDisaWaitNoDisc_ScanDoneHandler
*
* DESCRIPTION
*   Scan done IND in DisaWaitNoDisc state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcDisaWaitNoDisc_ScanDoneHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // change the current state
    FSM_StateChange(&g_tAcFsmDef, AC_ST_NULL);

    // stop auto-connect trigger timer
    osTimerStop(g_tAcTriggerTimer);

    // execute the callback of AC disable
    if (g_tAcDisableCb != NULL)
    {
        g_tAcDisableCb(OPL_OK);
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcDisaWaitNoDisc_ConnFailHandler
*
* DESCRIPTION
*   Connect fail IND in DisaWaitNoDisc state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcDisaWaitNoDisc_ConnFailHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // change the current state
    FSM_StateChange(&g_tAcFsmDef, AC_ST_NULL);

    // stop auto-connect trigger timer
    osTimerStop(g_tAcTriggerTimer);

    // execute the callback of AC disable
    if (g_tAcDisableCb != NULL)
    {
        g_tAcDisableCb(OPL_OK);
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcDisaWaitNoDisc_ConnSuccessHandler
*
* DESCRIPTION
*   Connect success IND in DisaWaitNoDisc state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcDisaWaitNoDisc_ConnSuccessHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // change the current state
    FSM_StateChange(&g_tAcFsmDef, AC_ST_NULL);

    // stop auto-connect trigger timer
    osTimerStop(g_tAcTriggerTimer);

    // execute the callback of AC disable
    if (g_tAcDisableCb != NULL)
    {
        g_tAcDisableCb(OPL_OK);
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcDisaWaitDisc_DiscSuccessHandler
*
* DESCRIPTION
*   Disconnect success in DisaWaitDisc state
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcDisaWaitDisc_DiscSuccessHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // change the current state
    FSM_StateChange(&g_tAcFsmDef, AC_ST_NULL);

    // stop auto-connect trigger timer
    osTimerStop(g_tAcTriggerTimer);

    // execute the callback of AC disable
    if (g_tAcDisableCb != NULL)
    {
        g_tAcDisableCb(OPL_OK);
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION
*   WM_AcWifiResetHandler
*
* DESCRIPTION
*   Receive WIFI reset event from wifi agent
*
* PARAMETERS
*   1. tFsmState  [In] : the current state
*   2. tFsmEvent  [In] : the current event
*   3. pu8Data    [In] : the input parameter
*   4. u32DataLen [In] : the length of input parameter
*
* RETURNS
*   1. OPL_OK : success
*   2. others : fail
*
*************************************************************************/
T_OplErr WM_AcWifiResetHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    AC_LOG_INFO("Recv wifi reset evt, disable AC directly");

    // stop auto-connect trigger timer
    osTimerStop(g_tAcTriggerTimer);

    // reset the value of AC profile
    g_u32AcProfileIdx = 0;
    g_u32AcProfileTotal = 0;

    // reset the value of AC interval index
    g_u8AcIntervalIdx = 0;

    // change the current state
    FSM_StateChange(&g_tAcFsmDef, AC_ST_NULL);

    return OPL_OK;
}

#endif /* WM_AC_ENABLED */
