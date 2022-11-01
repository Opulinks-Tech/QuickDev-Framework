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

#include "event_loop.h"
#include "qd_config.h"
#include "qd_module.h"
#include "wifi_agent.h"
#include "wifi_pro_rec.h"
#include "wifi_mngr.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (WM_ENABLED == 1)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

static const T_FsmStateExctblEvent tWaNullExctblEvent[] = 
{
    {WA_EVT_INIT_REQ,           WM_WaInitRequestHandler},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tWaInitializingExctblEvent[] =
{
    {WA_EVT_INIT_IND,           WM_WaInitIndicateHandler},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tWaIdleExctblEvent[] =
{
    {WA_EVT_SCAN_REQ,           WM_WaScanRequestHandler},
    {WA_EVT_CONNECT_REQ,        WM_WaConnectRequestHandler},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tWaScaningExctblEvent[] =
{
    {WA_EVT_SCAN_IND,           WM_WaScanIndicateHandler},
    {WA_EVT_DISCONNECT_IND,     WM_WaDisconnectIndicateHandler},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tWaConnectingExctblEvent[] =
{
    {WA_EVT_CONNECT_PASS_IND,   WM_WaConnectIndicateHandler},
    {WA_EVT_CONNECT_FAIL_IND,   WM_WaConnectIndicateHandler},
    {WA_EVT_DISCONNECT_IND,     WM_WaDisconnectIndicateHandler},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tWaConnectedExctblEvent[] =
{
    {WA_EVT_SCAN_REQ,           WM_WaScanRequestHandler},
    {WA_EVT_DISCONNECT_REQ,     WM_WaDisconnectRequestHandler},
    {WA_EVT_DISCONNECT_IND,     WM_WaDisconnectIndicateHandler},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tWaConnectedScanExctblEvent[] =
{
    {WA_EVT_SCAN_IND,           WM_WaScanIndicateHandler},
    {WA_EVT_DISCONNECT_IND,     WM_WaDisconnectIndicateHandler},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tWaDisconnectingExctblEvent[] =
{
    {WA_EVT_DISCONNECT_IND,     WM_WaDisconnectIndicateHandler},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateTable tWaStateTbl[] =
{
    {tWaNullExctblEvent},
    {tWaInitializingExctblEvent},
    {tWaIdleExctblEvent},
    {tWaScaningExctblEvent},
    {tWaConnectingExctblEvent},
    {tWaConnectedExctblEvent},
    {tWaConnectedScanExctblEvent},
    {tWaDisconnectingExctblEvent},
};

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

static T_FsmDef g_tWaFsmDef;

static struct
{
    T_WaConnectType tConnectType;
    T_PrApProfile tApProfile;
} g_tApProfile;
// static T_PrApProfile g_tApProfile = {0};

static T_WaInitDoneIndCbFp      g_tWaInitDoneIndCbFp = NULL;
static T_WaScanDoneIndCbFp      g_tWaScanDoneIndCbFp = NULL;
static T_WaConnectIndCbFp       g_tWaConnectIndCbFp = NULL;
static T_WaDisconnectIndCbFp    g_tWaDisconnectIndCbFp = NULL;

// Sec 7: declaration of static function prototype

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
T_OplErr WM_WaIndCbSet(T_FsmEvent tFsmEvent, FsmIndicateCbFunc fpIndCb)
{
    switch(tFsmEvent)
    {
        case WA_EVT_INIT_REQ:
        {
            g_tWaInitDoneIndCbFp = (T_WaInitDoneIndCbFp)fpIndCb;
            return OPL_OK;
        }
        case WA_EVT_SCAN_REQ:
        {
            g_tWaScanDoneIndCbFp = (T_WaScanDoneIndCbFp)fpIndCb;
            return OPL_OK;
        }
        case WA_EVT_CONNECT_REQ:
        {
            g_tWaConnectIndCbFp = (T_WaConnectIndCbFp)fpIndCb;
            return OPL_OK;
        }
        case WA_EVT_DISCONNECT_REQ:
        {
            g_tWaDisconnectIndCbFp = (T_WaDisconnectIndCbFp)fpIndCb;
            return OPL_OK;
        }
        default:
            break;
    }

    return OPL_ERR_IND_CB_NOT_RELATED;
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
void WM_WaIndCbProc(T_FsmEvent tFsmEvent, T_OplErr tEvtRst)
{
    switch(tFsmEvent)
    {
        case WA_EVT_INIT_REQ:
        case WA_EVT_INIT_IND:
        {
            if (g_tWaInitDoneIndCbFp != NULL)
            {
                g_tWaInitDoneIndCbFp(tEvtRst);
            }
            break;
        }
        case WA_EVT_SCAN_REQ:
        case WA_EVT_SCAN_IND:
        {
            if (g_tWaScanDoneIndCbFp != NULL)
            {
                g_tWaScanDoneIndCbFp(tEvtRst);
            }
            break;
        }
        case WA_EVT_CONNECT_REQ:
        case WA_EVT_CONNECT_PASS_IND:
        case WA_EVT_CONNECT_FAIL_IND:
        {
            if (g_tWaConnectIndCbFp != NULL)
            {
                g_tWaConnectIndCbFp(tEvtRst);
            }
            break;
        }
        case WA_EVT_DISCONNECT_REQ:
        case WA_EVT_DISCONNECT_IND:
        {
            if (g_tWaDisconnectIndCbFp != NULL)
            {
                g_tWaDisconnectIndCbFp(tEvtRst);
            }
            break;
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
static T_OplErr WM_WaInitRequestHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    WM_LOG_DEBG("WA init req");

    // state change - INITING
    FSM_StateChange(&g_tWaFsmDef, WA_ST_INITING);

    // event loop initialization
    wifi_event_loop_init((wifi_event_cb_t)WM_WifiEventHandlerCb);

    // initialize WI-FI stack and register wifi init complete event handler
    wifi_init(NULL, NULL);

    // WI-FI operation start
    wifi_start();

    // initialize auto connect
    wifi_auto_connect_set_mode(WIFI_AUTO_CONNECT_ENABLE);

    // initialize ap number for auto connect
    wifi_auto_connect_set_ap_num(1);

    // TODO: Need timer to check wifi init is OK

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
static T_OplErr WM_WaInitIndicateHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    WM_LOG_DEBG("WA init ind");

    T_OplErr tEvtRst = OPL_OK;

    // state change - IDLE
    FSM_StateChange(&g_tWaFsmDef, WA_ST_IDLE);

    // process indicate callback
    WM_WaIndCbProc(tFsmEvent, tEvtRst);

    return tEvtRst;
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
static T_OplErr WM_WaScanRequestHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    WM_LOG_DEBG("Scan req");

    T_OplErr tEvtRst = OPL_OK;

    if (WA_ST_IDLE == tFsmState)
    {
        // state change - SCANING
        FSM_StateChange(&g_tWaFsmDef, WA_ST_SCANING);
    }
    else if (WA_ST_CONNECTED == tFsmState)
    {
        // state change - CONNECTED_SCAN
        FSM_StateChange(&g_tWaFsmDef, WA_ST_CONNECTED_SCAN);
    }

    int sRet;

    wifi_scan_config_t *stScanConfig = (wifi_scan_config_t *)pu8Data;

    sRet = wifi_scan_start(stScanConfig, NULL);

    if (0 != sRet)
    {
        WM_LOG_ERRO("Exec scan fail");

        // state change - IDLE
        FSM_StateChange(&g_tWaFsmDef, WA_ST_IDLE);

        tEvtRst = OPL_ERR_WIFI_SCAN_CMD_FAIL;
    }

    return tEvtRst;
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
static T_OplErr WM_WaScanIndicateHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    WM_LOG_DEBG("Scan ind");

    T_OplErr tEvtRst = OPL_OK;

    // update scanned info to auto-connect list
    tEvtRst = WM_WaUpdateScanInfoToAutoConnList();
    if (OPL_OK != tEvtRst)
    {
        WM_LOG_WARN("Update scan info to auto-conn list fail");
    }

    if (WA_ST_SCANING == tFsmState)
    {
        // state change - IDLE
        FSM_StateChange(&g_tWaFsmDef, WA_ST_IDLE);
    }
    else if (WA_ST_CONNECTED_SCAN == tFsmState)
    {
        // state change - CONNECTED
        FSM_StateChange(&g_tWaFsmDef, WA_ST_CONNECTED);
    }

    // process indicate callback
    WM_WaIndCbProc(tFsmEvent, tEvtRst);

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
static T_OplErr WM_WaConnectRequestHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    WM_LOG_DEBG("Connect req");

    int sRet=0;
    uint8_t u8CheckBssid[WIFI_MAC_ADDRESS_LENGTH] = {0};

    T_OplErr tEvtRst = OPL_OK;
    wifi_config_t tConnReqConfig = {0};
    T_WmConnConfig *ptConnConfig = (T_WmConnConfig *)pu8Data;

    // state change - CONNECTING
    FSM_StateChange(&g_tWaFsmDef, WA_ST_CONNECTING);

    // do dadicate connect
    if(0 != memcmp(u8CheckBssid, ptConnConfig->bssid, WIFI_MAC_ADDRESS_LENGTH))
    {
        // copy bssid to connect configure
        memcpy(tConnReqConfig.sta_config.bssid, ptConnConfig->bssid, WIFI_MAC_ADDRESS_LENGTH);

        WM_LOG_INFO("Connect config bssid=%X:%X:%X:%X:%X:%X", tConnReqConfig.sta_config.bssid[0],
                                                              tConnReqConfig.sta_config.bssid[1],
                                                              tConnReqConfig.sta_config.bssid[2],
                                                              tConnReqConfig.sta_config.bssid[3],
                                                              tConnReqConfig.sta_config.bssid[4],
                                                              tConnReqConfig.sta_config.bssid[5]);
        
        g_tApProfile.tConnectType = WA_CONN_TYPE_BSSID;
    }
    else
    {
        if (0 != ptConnConfig->ssid_length)
        {
            // copy ssid and lens to connect configure
            memcpy(tConnReqConfig.sta_config.ssid, ptConnConfig->ssid, ptConnConfig->ssid_length);
            tConnReqConfig.sta_config.ssid_length = ptConnConfig->ssid_length;

            WM_LOG_INFO("Connect config ssid=%s (lens %d)", (char *)tConnReqConfig.sta_config.ssid,
                                                            tConnReqConfig.sta_config.ssid_length);

            g_tApProfile.tConnectType = WA_CONN_TYPE_SSID;
        }
        else
        {
            WM_LOG_INFO("Connect config empty, doing fast connect");

            g_tApProfile.tConnectType = WA_CONN_TYPE_FAST;

            // do auto connect
            sRet = wifi_auto_connect_start();

            if (0 != sRet)
            {
                WM_LOG_ERRO("Exec auto connect fail");

                g_tApProfile.tConnectType = WA_CONN_TYPE_NONE;

                // state change - IDLE
                FSM_StateChange(&g_tWaFsmDef, WA_ST_IDLE);

                tEvtRst = OPL_ERR_WIFI_AC_CMD_FAIL;
            }

            return tEvtRst;
        }
    }

    // copy password and lens to connect configure
    if (0 != ptConnConfig->password_length)
    {
        memcpy(tConnReqConfig.sta_config.password, ptConnConfig->password, ptConnConfig->password_length);
        tConnReqConfig.sta_config.password_length = ptConnConfig->password_length;

        WM_LOG_INFO("Connect config pwd=%s (lens %d)", (char *)tConnReqConfig.sta_config.password, 
                                                       tConnReqConfig.sta_config.password_length);
    }
    else
    {
        WM_LOG_INFO("Connect config no pwd");
    }

    // set station mode
    wifi_set_config(WIFI_MODE_STA, &tConnReqConfig);
    sRet = wifi_connection_connect(&tConnReqConfig);

    if (0 != sRet)
    {
        WM_LOG_ERRO("Exec connect fail");

        g_tApProfile.tConnectType = WA_CONN_TYPE_NONE;

        // state change - IDLE
        FSM_StateChange(&g_tWaFsmDef, WA_ST_IDLE);

        tEvtRst = OPL_ERR_WIFI_CONNECT_CMD_FAIL;
    }
    else
    {
        // prepare the profile
        if(WA_CONN_TYPE_SSID == g_tApProfile.tConnectType)
        {
            memset(&g_tApProfile.tApProfile, 0, sizeof(g_tApProfile.tApProfile));
            memcpy(g_tApProfile.tApProfile.u8Ssid, tConnReqConfig.sta_config.ssid, tConnReqConfig.sta_config.ssid_length);
            memcpy(g_tApProfile.tApProfile.u8Pwd, tConnReqConfig.sta_config.password, tConnReqConfig.sta_config.password_length);
        }
        else if(WA_CONN_TYPE_BSSID == g_tApProfile.tConnectType)
        {
            memset(&g_tApProfile.tApProfile, 0, sizeof(g_tApProfile.tApProfile));

            // search the ssid base on bssid in scan list (hidden AP will not be saved)
            
            // get scan list
            wifi_scan_list_t *pstScanList = NULL;

            pstScanList = (wifi_scan_list_t *)malloc(sizeof(wifi_scan_list_t));

            if(NULL == pstScanList)
            {
                WM_LOG_ERRO("malloc fail");
                
                return OPL_ERR_ALLOC_MEMORY_FAIL;
            }

            Opl_Wifi_ScanList_Get(pstScanList);

            uint8_t i = 0;
            for(i = 0; i < pstScanList->num; i++)
            {
                if ((memcmp(pstScanList->ap_record[i].bssid, tConnReqConfig.sta_config.bssid, sizeof(tConnReqConfig.sta_config.bssid)) == 0))
                {
                    // matched
                    if(0 != strlen((char *)&pstScanList->ap_record[i].ssid))
                    {
                        // copy scanned ssid
                        memcpy(g_tApProfile.tApProfile.u8Ssid, pstScanList->ap_record[i].ssid, strlen((char *)&pstScanList->ap_record[i].ssid));

                        // copy password
                        memcpy(g_tApProfile.tApProfile.u8Pwd, tConnReqConfig.sta_config.password, tConnReqConfig.sta_config.password_length);

                        break;
                    }
                }
            }

            if(pstScanList->num == i)
            {
                WM_LOG_WARN("Ssid not found or it's hidden, not save");

                g_tApProfile.tConnectType = WA_CONN_TYPE_NONE;
            }
						
            free(pstScanList);
        }
    }

    return tEvtRst;
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
static T_OplErr WM_WaConnectIndicateHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    uint8_t u8Reason = *((uint8_t *)pu8Data);
    
    // incase of compiler warning
    if(u8Reason){}

    T_OplErr tEvtRst = OPL_OK;
    T_FsmEvent tEvent = WA_EVT_TOTAL;

    WM_LOG_DEBG("Connect ind (reason %d)", u8Reason);

    if (WA_EVT_CONNECT_PASS_IND == tFsmEvent)
    {
        WM_LOG_INFO("Wifi connected");

        // state change - CONNECTED
        FSM_StateChange(&g_tWaFsmDef, WA_ST_CONNECTED);

        tEvent = WA_EVT_CONNECT_PASS_IND;

        // insert the profile
        if(WA_CONN_TYPE_SSID == g_tApProfile.tConnectType || WA_CONN_TYPE_BSSID == g_tApProfile.tConnectType)
        {
            WM_PrInsert(g_tApProfile.tApProfile);

            g_tApProfile.tConnectType = WA_CONN_TYPE_NONE;
            memset(&g_tApProfile.tApProfile, 0, sizeof(g_tApProfile.tApProfile));
        }
    }
    else if (WA_EVT_CONNECT_FAIL_IND == tFsmEvent)
    {
        WM_LOG_WARN("Wifi connect fail");

        // state change - IDLE
        FSM_StateChange(&g_tWaFsmDef, WA_ST_IDLE);

        tEvtRst = OPL_ERR_WIFI_CONNECT_FAIL;
        tEvent = WA_EVT_CONNECT_FAIL_IND;
    }
    else
    {
        // invalid event case
        return OPL_ERR_CASE_INVALID;
    }

    // process indicate callback
    WM_WaIndCbProc(tEvent, tEvtRst);

    // process unsolicited callback
    if (WA_EVT_CONNECT_PASS_IND == tFsmEvent)
    {
        WM_UslctedCbRun(USLCTED_CB_EVT_WIFI_UP, tEvtRst, NULL, 0);
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
static T_OplErr WM_WaDisconnectRequestHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    WM_LOG_DEBG("Disconnect req");

    T_OplErr tEvtRst = OPL_OK;

    if (WA_ST_CONNECTED == tFsmState)
    {
        // state change - DISCONNECTING
        FSM_StateChange(&g_tWaFsmDef, WA_ST_DISCONNECTING);

        int sRet=0;

        sRet = wifi_connection_disconnect_ap();

        if(0 != sRet)
        {
            WM_LOG_ERRO("Exec disconnect fail");

            // state change - CONNECTED
            FSM_StateChange(&g_tWaFsmDef, WA_ST_CONNECTED);

            tEvtRst = OPL_ERR_WIFI_DISCONNECT_CMD_FAIL;
        }
    }

    return tEvtRst;
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
static T_OplErr WM_WaDisconnectIndicateHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    uint8_t u8Reason = *((uint8_t *)pu8Data);

    WM_LOG_DEBG("Disconnect ind (reason %d)", u8Reason);

    T_OplErr tEvtRst = OPL_OK;

    // reason 255 which means the wifi low layer timeout, back to idle state and call unsolicited callback
    if(255 == u8Reason)
    {
        // reset wifi
        wifi_reset();

        FSM_StateChange(&g_tWaFsmDef, WA_ST_IDLE);

        WM_UslctedCbRun(USLCTED_CB_EVT_WIFI_RESET, tEvtRst, pu8Data, u32DataLen);

        return OPL_OK;
    }

    switch(tFsmState)
    {
        case WA_ST_SCANING:
        {
            // scaning will keep processing if reason is not 255
            // state no change, call indicate callback directly
            WM_WaIndCbProc(tFsmEvent, tEvtRst);

            return OPL_OK;
        }
        case WA_ST_CONNECTING:
        {
            // state change - IDLE
            FSM_StateChange(&g_tWaFsmDef, WA_ST_IDLE);

            // // error handling with invalid case
            // if(0 == u8Reason)
            // {
            //     printf("[WA] reason 0, invalid case\r\n");

            //     // process indicate callback
            //     WM_WaIndCbProc(tFsmEvent, OPL_ERR_CASE_INVALID);

            //     return OPL_OK;
            // }
            // else
            // {
                WM_LOG_WARN("Wifi connect fail");

                WM_WaIndCbProc(WA_EVT_CONNECT_FAIL_IND, OPL_ERR_WIFI_CONNECT_FAIL);

                return OPL_OK;
            // }
        }
        case WA_ST_CONNECTED_SCAN:
            // state change - SCANING
            FSM_StateChange(&g_tWaFsmDef, WA_ST_SCANING);
            break;

        case WA_ST_CONNECTED:
        case WA_ST_DISCONNECTING:
            // state change - IDLE
            FSM_StateChange(&g_tWaFsmDef, WA_ST_IDLE);
            break;
    }

    // process indicate callback
    WM_WaIndCbProc(tFsmEvent, tEvtRst);

    // process unsolicited callback
    WM_UslctedCbRun(USLCTED_CB_EVT_WIFI_DOWN, tEvtRst, pu8Data, u32DataLen);

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
T_FsmDef *WM_WaFsmDefGet(void)
{
    return &g_tWaFsmDef;
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
void WM_WaInit(void)
{
    // name of WI-FI agent
    uint8_t u8aWaName[2] = "WA";

    // initialize the WI-FI agent state machine
    FSM_Init(&g_tWaFsmDef, u8aWaName, tWaStateTbl, WA_ST_NULL);

    // initialize the WI-FI agent indicate callback handler
    FSM_IndCbHdlInit(&g_tWaFsmDef, WM_WaIndCbSet, WM_WaIndCbProc);

    // initialize the ApProfile struct
    g_tApProfile.tConnectType = WA_CONN_TYPE_NONE;
    memset(&g_tApProfile.tApProfile, 0, sizeof(g_tApProfile.tApProfile));
}

#endif /* WM_ENABLED */
