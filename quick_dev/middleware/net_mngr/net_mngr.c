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
*  net_mngr.c
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
#include "lwip_helper.h"
#include "hal_wdt.h"
#include "net_mngr.h"
#include "net_mngr_api.h"
#include "wifi_mngr_api.h"
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (NM_ENABLED == 1)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

static const T_FsmStateExctblEvent tNmNullExctblEvent[] = 
{
    {NM_EVT_WIFI_INIT_IND,      APP_NmInitIndHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tNmIdleExctblEvent[] = 
{
    {NM_EVT_WIFI_SCAN_REQ,      APP_NmScanReqHndlr},
    {NM_EVT_WIFI_CNCT_REQ,      APP_NmCnctReqHndlr},
    {NM_EVT_AC_EN_TIMEOUT,      APP_NmAcEnTimeoutHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};


static const T_FsmStateExctblEvent tNmWaitScanExctblEvent[] = 
{
    {NM_EVT_WIFI_SCAN_IND,      APP_NmScanDoneHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};


static const T_FsmStateExctblEvent tNmWaitCnctExctblEvent[] = 
{
    {NM_EVT_WIFI_CNCT_PASS_IND, APP_NmCnctIndHndlr},
    {NM_EVT_WIFI_CNCT_FAIL_IND, APP_NmCnctIndHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};


static const T_FsmStateExctblEvent tNmWifiUpExctblEvent[] = 
{
    {NM_EVT_GOT_IP,             APP_NmGotIpHndlr},
    {NM_EVT_WIFI_DOWN_IND,      APP_NmWiFiDownHndlr},
    {NM_EVT_DHCP_TIMEOUT,       APP_NmDhcpTimeoutHndlr},
    //{NM_EVT_WIFI_CNCT_REQ,      APP_NmCnctReqHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tNmGotIpExctblEvent[] = 
{
    {NM_EVT_WIFI_DOWN_IND,      APP_NmWiFiDownHndlr},
    {NM_EVT_WIFI_SCAN_REQ,      APP_NmScanReqHndlr},
    {NM_EVT_WIFI_CNCT_REQ,      APP_NmCnctReqHndlr},
    {NM_EVT_AC_EN_TIMEOUT,      APP_NmAcEnTimeoutHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tNmDhcpWaitDiscExctblEvent[] = 
{
    {NM_EVT_WIFI_DISC_IND,      APP_NmWiFiDiscIndHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tNmCnctWaitDiscExctblEvent[] = 
{
    {NM_EVT_WIFI_DISC_IND,      APP_NmWiFiDiscIndHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tNmWiFiUpWaitScanExctblEvent[] = 
{
    {NM_EVT_WIFI_SCAN_IND,      APP_NmScanDoneHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tNmWifiUpAcptScanExctblEvent[] = 
{
    {NM_EVT_GOT_IP,             APP_NmGotIpHndlr},
    {NM_EVT_WIFI_DOWN_IND,      APP_NmWiFiDownHndlr},
    {NM_EVT_DHCP_TIMEOUT,       APP_NmDhcpTimeoutHndlr},
    {NM_EVT_WIFI_SCAN_REQ,      APP_NmScanReqHndlr},
    {NM_EVT_WIFI_CNCT_REQ,      APP_NmCnctReqHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tNmGotIpWaitScanExctblEvent[] = 
{
    {NM_EVT_WIFI_SCAN_IND,      APP_NmScanDoneHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};


static const T_FsmStateExctblEvent tNmIdleWaitAcEnExctblEvent[] = 
{
    {NM_EVT_WIFI_AC_EN_IND,     APP_NmAcEnIndHndlr},
    {NM_EVT_WIFI_SCAN_REQ,      APP_NmScanReqHndlr},
    {NM_EVT_WIFI_CNCT_REQ,      APP_NmCnctReqHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};


static const T_FsmStateExctblEvent tNmAcEnExctblEvent[] = 
{
    {NM_EVT_WIFI_UP_IND,        APP_NmWiFiUpHndlr},
    {NM_EVT_WIFI_SCAN_REQ,      APP_NmScanReqHndlr},
    {NM_EVT_WIFI_CNCT_REQ,      APP_NmCnctReqHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tNmAcWifiUpExctblEvent[] = 
{
    {NM_EVT_GOT_IP,             APP_NmGotIpHndlr},
    {NM_EVT_WIFI_SCAN_REQ,      APP_NmScanReqHndlr},
    {NM_EVT_WIFI_CNCT_REQ,      APP_NmCnctReqHndlr},
    {NM_EVT_WIFI_DOWN_IND,      APP_NmWiFiDownHndlr},
    {NM_EVT_DHCP_TIMEOUT,       APP_NmDhcpTimeoutHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};


static const T_FsmStateExctblEvent tNmAcDhcpWaitDiscExctblEvent[] = 
{
    {NM_EVT_WIFI_DISC_IND,      APP_NmWiFiDiscIndHndlr},
    {NM_EVT_WIFI_SCAN_REQ,      APP_NmScanReqHndlr},
    {NM_EVT_WIFI_CNCT_REQ,      APP_NmCnctReqHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};


static const T_FsmStateExctblEvent tNmAcScanWaitDiscExctblEvent[] = 
{
    {NM_EVT_WIFI_DISC_IND,      APP_NmWiFiDiscIndHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};


static const T_FsmStateExctblEvent tNmAcCnctWaitDiscExctblEvent[] = 
{
    {NM_EVT_WIFI_DISC_IND,      APP_NmWiFiDiscIndHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};


static const T_FsmStateExctblEvent tNmAcGotIpExctblEvent[] = 
{
    {NM_EVT_WIFI_DOWN_IND,      APP_NmWiFiDownHndlr},
    {NM_EVT_WIFI_SCAN_REQ,      APP_NmScanReqHndlr},
    {NM_EVT_WIFI_CNCT_REQ,      APP_NmCnctReqHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};


static const T_FsmStateExctblEvent tNmScanWaitAcEnExctblEvent[] = 
{
    {NM_EVT_WIFI_AC_EN_IND,     APP_NmAcEnIndHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tNmCnctWaitAcEnExctblEvent[] = 
{
    {NM_EVT_WIFI_AC_EN_IND,     APP_NmAcEnIndHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tNmScanWaitAcDisaExctblEvent[] = 
{
    {NM_EVT_WIFI_AC_DISA_IND,   APP_NmAcDisaIndHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tNmCnctWaitAcDisaExctblEvent[] = 
{
    {NM_EVT_WIFI_AC_DISA_IND,   APP_NmAcDisaIndHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tNmGotIpWaitAcEnExctblEvent[] = 
{
    {NM_EVT_WIFI_AC_EN_IND,     APP_NmAcEnIndHndlr},
    {NM_EVT_WIFI_SCAN_REQ,      APP_NmScanReqHndlr},
    {NM_EVT_WIFI_CNCT_REQ,      APP_NmCnctReqHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tNmQSetWaitAcEnExctblEvent[] = 
{
    {NM_EVT_WIFI_AC_EN_IND,     APP_NmAcEnIndHndlr},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};


static const T_FsmStateTable tNmStateTbl[] =
{
    {tNmNullExctblEvent},
    {tNmIdleExctblEvent},

    {tNmWaitScanExctblEvent},
    {tNmWaitCnctExctblEvent},

    {tNmWifiUpExctblEvent},
    {tNmGotIpExctblEvent},

    {tNmDhcpWaitDiscExctblEvent},
    {tNmCnctWaitDiscExctblEvent},

    {tNmWiFiUpWaitScanExctblEvent},
    {tNmWifiUpAcptScanExctblEvent},

    {tNmGotIpWaitScanExctblEvent},

    {tNmIdleWaitAcEnExctblEvent},

    {tNmAcEnExctblEvent},
    {tNmAcWifiUpExctblEvent},

    {tNmAcDhcpWaitDiscExctblEvent},
    
    {tNmAcScanWaitDiscExctblEvent},
    {tNmAcCnctWaitDiscExctblEvent},

    {tNmAcGotIpExctblEvent},

    {tNmScanWaitAcEnExctblEvent},
    {tNmCnctWaitAcEnExctblEvent},

    {tNmScanWaitAcDisaExctblEvent},
    {tNmCnctWaitAcDisaExctblEvent},

    {tNmGotIpWaitAcEnExctblEvent},

    {tNmQSetWaitAcEnExctblEvent},
};

// Network manager init status
typedef struct S_NmInitStatus
{
    uint8_t u8IsInit;                           // init status
    uint8_t u8IsCnctDrct;                       // connect directly in initiate
    uint8_t u8Ssid[WIFI_MAX_LENGTH_OF_SSID];    // connect directly SSID
    uint8_t u8Pwd[WIFI_LENGTH_PASSPHRASE];      // connect directly PWD
} T_NmInitStatus;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable
// DHCP timer
osTimerId	g_tDhcpTmr;
osTimerId   g_tAcEnTmr;

// Auto connect enable flag
uint32_t g_u32AcEnable = 0;

// WiFi connect retry handle
uint32_t g_u32WifiCnctTimeout = 0;
uint32_t g_u32WifiCnctStartTime = 0;
uint32_t g_u32WiFiCnctTotalRetryTime = 0;
uint32_t g_u32WiFiCnctFailMaxTime = 0;
uint32_t g_u32WiFiCnctRetry = 0;

// WiFi connect config for WiFi manager
T_WmConnConfig g_tCnctReqConfig = {0};
uint32_t g_u32CnctQuickSet = 0;

// Network manager unsolicted callback function pointer
T_NmUslctdCbFp g_fpNmUslctdCb = NULL;

// Sec 5: declaration of global function prototype
//extern T_OplErr APP_SendMessage(uint32_t pu32Idx);

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

static T_FsmDef g_tNmFsmDef;

// Network manager init status
static T_NmInitStatus tNmInitStatus = {0};

static T_NmScanDoneIndCbFp  g_tNmScanDoneIndCbFp = NULL;
static T_NmCnctIndCbFp      g_tNmConnectIndCbFp = NULL;

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

// ****************** Indicate callback for WiFi manager

// WiFi maianger init done
void _WmInitDoneIndCbFp(T_OplErr tEvtRst)
{
    // NM_LOG_DEBG("Wifi mngr init done");

    APP_SendMessage((uint32_t)NM_EVT_WIFI_INIT_IND, NULL, 0);
}


// WiFi scan done
void _WmScanDoneIndCbFp(T_OplErr tEvtRst)
{
    // NM_LOG_DEBG("Wifi scan done");

    APP_SendMessage((uint32_t)NM_EVT_WIFI_SCAN_IND, NULL, 0);
}

// WiFi connect done
void _WmCnctDoneIndCbFp(T_OplErr tEvtRst)
{
    // NM_LOG_DEBG("Wifi conn done [%d]", tEvtRst);

    if(OPL_OK == tEvtRst)
    {
        APP_SendMessage((uint32_t)NM_EVT_WIFI_CNCT_PASS_IND, NULL, 0);
    }
    else
    {
        APP_SendMessage((uint32_t)NM_EVT_WIFI_CNCT_FAIL_IND, NULL, 0);
    }
}


// WiFi disconnect done
void _WmDiscDoneIndCbFp(T_OplErr tEvtRst)
{
    // NM_LOG_DEBG("Wifi disc done");

    APP_SendMessage((uint32_t)NM_EVT_WIFI_DISC_IND, NULL, 0);
}


// WiFi maianger auto-connect enable done
void _AcEnIndCbFp(T_OplErr tEvtRst, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // NM_LOG_DEBG("Auto-connect enabled");

    APP_SendMessage((uint32_t)NM_EVT_WIFI_AC_EN_IND, pu8Data, u32DataLen);
}

// WiFi maianger auto-connect disable done
void _AcDisaIndCbFp(T_OplErr tEvtRst)
{
    // NM_LOG_DEBG("Auto-connect disabled");

    APP_SendMessage((uint32_t)NM_EVT_WIFI_AC_DISA_IND, NULL, 0);
}


// WiFi manager unsolicited callback 
void _WmUnsolicitedCbFp(T_WmUslctedEvtType tEvtType, T_OplErr tEvtRst, uint8_t *pu8Data, uint32_t u32DataLen)
{
    // NM_LOG_DEBG("WM Unsolicited callback (EvtType %d) (EvtRst %d)\r\n", tEvtType, tEvtRst);

    if(USLCTED_CB_EVT_WIFI_UP == tEvtType)
    {
        if(OPL_OK == tEvtRst)
        {
            // start tcpip network
            //lwip_net_start(WIFI_MODE_STA);
            APP_SendMessage((uint32_t)NM_EVT_WIFI_UP_IND, NULL, 0);
        }
    }
    else if(USLCTED_CB_EVT_WIFI_DOWN == tEvtType)
    {
        APP_SendMessage((uint32_t)NM_EVT_WIFI_DOWN_IND, pu8Data, u32DataLen);
    }
    else if(USLCTED_CB_EVT_GOT_IP == tEvtType)
    {
        
        APP_SendMessage((uint32_t)NM_EVT_GOT_IP, pu8Data, u32DataLen);
#if 0
        // find ip number in network interface
        uint32_t u32Ip = *((uint32_t*)pu8Data);
        uint8_t u8aIp[4] = {0};

        u8aIp[0] = (u32Ip >> 0) & 0xFF;
        u8aIp[1] = (u32Ip >> 8) & 0xFF;
        u8aIp[2] = (u32Ip >> 16) & 0xFF;
        u8aIp[3] = (u32Ip >> 24) & 0xFF;

        printf("ip = (%d) %d.%d.%d.%d\r\n", u32Ip, u8aIp[0], u8aIp[1], u8aIp[2], u8aIp[3]);
#endif
    }
    else if(USLCTED_CB_EVT_WIFI_RESET == tEvtType)
    {
        APP_SendMessage((uint32_t)NM_EVT_WIFI_RESET, pu8Data, u32DataLen);
    }
}

// ****************** Timer timeout callback

// DHCP timeout callback
static void _DhcpTimeoutCb(void const *argu)
{
    NM_LOG_WARN("DHCP timeout");

    APP_SendMessage((uint32_t)NM_EVT_DHCP_TIMEOUT, NULL, 0);
}


// AC enable timeout callback
static void _AcEnTimeoutCb(void const *argu)
{
    NM_LOG_WARN("AC enable timeout");

    APP_SendMessage((uint32_t)NM_EVT_AC_EN_TIMEOUT, NULL, 0);
}


// ****************** Other interal function
void _IpGet(uint8_t *pu8Ip)
{
    struct netif *iface = netif_find("st1");

    *pu8Ip      = (iface->ip_addr.u_addr.ip4.addr >> 0) & 0xFF;
    *(pu8Ip+1)  = (iface->ip_addr.u_addr.ip4.addr >> 8) & 0xFF;
    *(pu8Ip+2)  = (iface->ip_addr.u_addr.ip4.addr >> 16) & 0xFF;
    *(pu8Ip+3)  = (iface->ip_addr.u_addr.ip4.addr >> 24) & 0xFF;
}

uint32_t _IpIsGot(void)
{
    uint8_t u8aIp[4];
    u8aIp[0] = 0;
    u8aIp[1] = 0;
    u8aIp[2] = 0;
    u8aIp[3] = 0;

    _IpGet(u8aIp);

    if( 0 == u8aIp[0] &&
        0 == u8aIp[1] &&
        0 == u8aIp[2] &&
        0 == u8aIp[3] )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void APP_NmUslctdCbRun(T_NmEventList tEvtType, uint8_t *pu8Data, uint32_t u32DataLen)
{
    if( NM_EVT_WIFI_RESET == tEvtType )
    {
        NM_LOG_INFO("Reset from WIFI");

        // Stop DHCP Timer
        osTimerStop(g_tDhcpTmr);

        // Stop AC enable Timer
        osTimerStop(g_tAcEnTmr);

        // state change - IDLE
        FSM_StateChange(&g_tNmFsmDef, NM_ST_IDLE);

        NM_LOG_INFO("AC enable flag [%d]", g_u32AcEnable);

        if(g_u32AcEnable)
        {
            // Start AC enable Timer
            osTimerStart(g_tAcEnTmr, NM_WIFI_AC_EN_DEF_TIMEOUT);
        }

    }

    if( NULL != g_fpNmUslctdCb)
    {
        if( NM_EVT_WIFI_DOWN_IND == tEvtType )
        {
            g_fpNmUslctdCb(NM_USLCTD_EVT_NETWORK_DOWN, pu8Data, u32DataLen);
        }
        else
        if( NM_EVT_GOT_IP == tEvtType )
        {
            g_fpNmUslctdCb(NM_USLCTD_EVT_NETWORK_UP, pu8Data, u32DataLen);
        }
        else
        if ( NM_EVT_WIFI_RESET == tEvtType )
        {
            g_fpNmUslctdCb(NM_USLCTD_EVT_NETWORK_RESET, pu8Data, u32DataLen);
        }
        
    }
}

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
T_OplErr APP_WmIndCbSet(T_FsmEvent tFsmEvent, FsmIndicateCbFunc fpIndCb)
{
    switch(tFsmEvent)
    {
        case NM_EVT_WIFI_SCAN_REQ:
        {
            g_tNmScanDoneIndCbFp = (T_NmScanDoneIndCbFp)fpIndCb;
            return OPL_OK;
        }
        case NM_EVT_WIFI_CNCT_REQ:
        {
            g_tNmConnectIndCbFp = (T_NmCnctIndCbFp)fpIndCb;
            return OPL_OK;
        }
        default:
            break;
    }

    return OPL_ERR_IND_CB_NOT_RELATED;
}


/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void APP_NmIndCbProc(T_FsmEvent tFsmEvent, T_OplErr tEvtRst)
{
    switch(tFsmEvent)
    {
        // callback of wifi scan request
        case NM_EVT_WIFI_SCAN_IND:
        {
            if (g_tNmScanDoneIndCbFp != NULL)
            {
                g_tNmScanDoneIndCbFp(tEvtRst);
            }
            break;
        }
        
        // callback of wifi connect
        case NM_EVT_WIFI_CNCT_FAIL_IND:
        case NM_EVT_GOT_IP:
        case NM_EVT_WIFI_DISC_IND:
        {
            if (g_tNmConnectIndCbFp != NULL)
            {
                g_tNmConnectIndCbFp(tEvtRst);
            }
            break;
        }
    }
}


/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static T_OplErr APP_NmInitIndHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    NM_LOG_DEBG("NM init ind");

    T_OplErr tEvtRst = OPL_OK;

    // state change - IDLE
    FSM_StateChange(&g_tNmFsmDef, NM_ST_IDLE);

    // process indicate callback
    //WM_WaIndCbProc(tFsmEvent, tEvtRst);

    // Cloud init?

    // if the init request with connect directly, then clear profile and insert the assigned one
    if(1 == tNmInitStatus.u8IsCnctDrct)
    {
        NM_LOG_INFO("NM init and cnct");

        T_PrApProfile tApProfile = {0};

        // copy scanned ssid
        memcpy(tApProfile.u8Ssid, tNmInitStatus.u8Ssid, WIFI_MAX_LENGTH_OF_SSID);

        // copy password
        memcpy(tApProfile.u8Pwd, tNmInitStatus.u8Pwd, WIFI_LENGTH_PASSPHRASE);

        // clear all profile record
        Opl_Wifi_Profile_Clear();

        // insert the new one
        Opl_Wifi_Profile_Ins(tApProfile);
    }

    NM_LOG_INFO("AC enable flag [%d]", g_u32AcEnable);

    if(g_u32AcEnable)
    {
        Opl_Wifi_AC_Enable_Req(_AcEnIndCbFp);

        // state change - IDLE_WAIT_AC_EN
        FSM_StateChange(&g_tNmFsmDef, NM_ST_IDLE_WAIT_AC_EN);
    }

    return tEvtRst;
}


/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static T_OplErr APP_NmScanReqHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
	T_OplErr tEvtRst = OPL_OK;
	
    NM_LOG_DEBG("Scan req");

    if( NM_ST_IDLE_WAIT_AC_EN == g_tNmFsmDef.ptFsmStateInfo.tCurrentState ||
        NM_ST_GOT_IP_WAIT_AC_EN == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        // state change
        FSM_StateChange(&g_tNmFsmDef, NM_ST_SCAN_WAIT_AC_EN);
    }
    else
    if( NM_ST_AC_EN == g_tNmFsmDef.ptFsmStateInfo.tCurrentState ||
        NM_ST_AC_WIFI_UP == g_tNmFsmDef.ptFsmStateInfo.tCurrentState ||
        NM_ST_AC_GOT_IP == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        if(NM_ST_AC_WIFI_UP == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
        {
            // Stop DHCP Timer
            osTimerStop(g_tDhcpTmr);
        }

        tEvtRst = Opl_Wifi_AC_Disable_Req(false, _AcDisaIndCbFp);

        if(OPL_OK != tEvtRst)
        {
            NM_LOG_WARN("Wifi ac disable req fail [%d]", tEvtRst);

            // TODO: ??
        }
        else
        {
            // state change
            FSM_StateChange(&g_tNmFsmDef, NM_ST_SCAN_WAIT_AC_DISA);
        }
    }
    else
    if(NM_ST_AC_DHCP_WAIT_DISC == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        // state change
        FSM_StateChange(&g_tNmFsmDef, NM_ST_AC_SCAN_WAIT_DISC);
    }
    else
    if( NM_ST_IDLE == g_tNmFsmDef.ptFsmStateInfo.tCurrentState ||
        NM_ST_GOT_IP == g_tNmFsmDef.ptFsmStateInfo.tCurrentState ||
        NM_ST_WIFI_UP_ACPT_SCAN == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        wifi_scan_config_t tScanConfig = {0};
    	
        tScanConfig.show_hidden = 1;
        tScanConfig.scan_type = WIFI_SCAN_TYPE_MIX;

        tEvtRst = Opl_Wifi_Scan_Req(&tScanConfig, _WmScanDoneIndCbFp);

        if(OPL_OK != tEvtRst)
        {
            NM_LOG_WARN("Wifi scan req fail [%d]", tEvtRst);

            // TODO: ??
        }
        else
        {
        	// state change
            if(NM_ST_WIFI_UP_ACPT_SCAN == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
            {
                // Stop DHCP Timer
                osTimerStop(g_tDhcpTmr);
                FSM_StateChange(&g_tNmFsmDef, NM_ST_WIFI_UP_WAIT_SCAN);
            }
            else
            {
                // Stop AC enable Timer
                osTimerStop(g_tAcEnTmr);

                if(NM_ST_GOT_IP == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
                {
                    FSM_StateChange(&g_tNmFsmDef, NM_ST_GOT_IP_WAIT_SCAN);
                }
                else    // Shoule be NM_ST_IDLE
                {
                    FSM_StateChange(&g_tNmFsmDef, NM_ST_WAIT_SCAN);
                }
            }
        }
    }

    return tEvtRst;
}


/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static T_OplErr APP_NmCnctReqHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_OplErr tEvtRst = OPL_OK;
    //T_WmConnConfig tCnctReqConfig = {0};
    T_NmWifiCnctConfInt *ptNmWifiCnctConfig = (T_NmWifiCnctConfInt *)pu8Data;

    NM_LOG_DEBG("Connect req");

#if 0
    printf( "BSSID[%x:%x:%x:%x:%x:%x] \r\n", 
            ptNmWifiCnctConfig->u8aBssid[0],
            ptNmWifiCnctConfig->u8aBssid[1],
            ptNmWifiCnctConfig->u8aBssid[2],
            ptNmWifiCnctConfig->u8aBssid[3],
            ptNmWifiCnctConfig->u8aBssid[4],
            ptNmWifiCnctConfig->u8aBssid[5] );

    printf("Password[%s], len[%d] \r\n", ptNmWifiCnctConfig->u8aPwd, ptNmWifiCnctConfig->u8PwdLen);
#endif    

    memset((void *)&g_tCnctReqConfig, 0, sizeof(T_WmConnConfig));
    g_u32CnctQuickSet = ptNmWifiCnctConfig->u8QuickSet;

    g_u32WiFiCnctTotalRetryTime = 0;
    g_u32WiFiCnctFailMaxTime = 0;
    g_u32WifiCnctStartTime = osKernelSysTick();
    g_u32WiFiCnctRetry = 0;

    if(0 == ptNmWifiCnctConfig->u8Timeout)
    {
        g_u32WifiCnctTimeout = NM_WIFI_CNCT_DEF_TIMEOUT;
    }
    else
    {
        g_u32WifiCnctTimeout = ptNmWifiCnctConfig->u8Timeout * 1000;
    }


    if(0 != memcmp(g_tCnctReqConfig.bssid, ptNmWifiCnctConfig->u8aBssid, WIFI_MAC_ADDRESS_LENGTH))
    {
        memcpy(g_tCnctReqConfig.bssid, ptNmWifiCnctConfig->u8aBssid, WIFI_MAC_ADDRESS_LENGTH);

        NM_LOG_INFO("BSSID[%X:%X:%X:%X:%X:%X]", g_tCnctReqConfig.bssid[0],
                                                     g_tCnctReqConfig.bssid[1],
                                                     g_tCnctReqConfig.bssid[2],
                                                     g_tCnctReqConfig.bssid[3],
                                                     g_tCnctReqConfig.bssid[4],
                                                     g_tCnctReqConfig.bssid[5] );
    }
    else
    {
        if (0 != ptNmWifiCnctConfig->u8SsidLen)
        {
            memcpy(g_tCnctReqConfig.ssid, ptNmWifiCnctConfig->u8aSsid, ptNmWifiCnctConfig->u8SsidLen);
            g_tCnctReqConfig.ssid_length = ptNmWifiCnctConfig->u8SsidLen;

            NM_LOG_INFO("SSID[%d] length[%d]", g_tCnctReqConfig.ssid, g_tCnctReqConfig.ssid_length);
        }
        else
        {
            // TODO: should not happend
            NM_LOG_INFO("No BSSID, NO SSID, len[%d]", ptNmWifiCnctConfig->u8SsidLen);
        }
    }

    

    if(0 == ptNmWifiCnctConfig->u8PwdLen)  //password len = 0
    {
        NM_LOG_INFO("pwd lens = 0");
        g_tCnctReqConfig.password_length = 0;
        memset((char *)g_tCnctReqConfig.password, 0 , WIFI_LENGTH_PASSPHRASE);
    }
    else
    {
        g_tCnctReqConfig.password_length = ptNmWifiCnctConfig->u8PwdLen;
        memcpy(g_tCnctReqConfig.password, ptNmWifiCnctConfig->u8aPwd, ptNmWifiCnctConfig->u8PwdLen);
    }

#if 0
    printf( "BSSID[%x:%x:%x:%x:%x:%x] \r\n", 
            g_tCnctReqConfig.bssid[0],
            g_tCnctReqConfig.bssid[1],
            g_tCnctReqConfig.bssid[2],
            g_tCnctReqConfig.bssid[3],
            g_tCnctReqConfig.bssid[4],
            g_tCnctReqConfig.bssid[5] );

    printf("Password[%s], len[%d] \r\n", g_tCnctReqConfig.password, g_tCnctReqConfig.password_length);
    printf("SSID len[%d] \r\n", g_tCnctReqConfig.ssid_length);
#endif   

    if(NM_ST_IDLE_WAIT_AC_EN == g_tNmFsmDef.ptFsmStateInfo.tCurrentState ||
        NM_ST_GOT_IP_WAIT_AC_EN == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        // state change
        FSM_StateChange(&g_tNmFsmDef, NM_ST_CNCT_WAIT_AC_EN);
    }
    else
    if( NM_ST_AC_EN == g_tNmFsmDef.ptFsmStateInfo.tCurrentState ||
        NM_ST_AC_WIFI_UP == g_tNmFsmDef.ptFsmStateInfo.tCurrentState ||
        NM_ST_AC_GOT_IP == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        if(NM_ST_AC_WIFI_UP == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
        {
            // Stop DHCP Timer
            osTimerStop(g_tDhcpTmr);
        }

        tEvtRst = Opl_Wifi_AC_Disable_Req(true, _AcDisaIndCbFp);

        if(OPL_OK != tEvtRst)
        {
            NM_LOG_WARN("Wifi ac disable req fail [%d]", tEvtRst);

            // TODO: ??
        }
        else
        {
            // state change
            FSM_StateChange(&g_tNmFsmDef, NM_ST_CNCT_WAIT_AC_DISA);
        }
    }
    else
    if( NM_ST_GOT_IP == g_tNmFsmDef.ptFsmStateInfo.tCurrentState || 
        //NM_ST_WIFI_UP == g_tNmFsmDef.ptFsmStateInfo.tCurrentState ||
        NM_ST_WIFI_UP_ACPT_SCAN == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        if(NM_ST_GOT_IP == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
        {
            // Stop AC enable Timer
            osTimerStop(g_tAcEnTmr);
        }
        else    // Shoulde be NM_ST_WIFI_UP_ACPT_SCAN
        {
            // Stop DHCP Timer
            osTimerStop(g_tDhcpTmr);
        }

        tEvtRst = Opl_Wifi_Disc_Req(_WmDiscDoneIndCbFp);

        if(OPL_OK == tEvtRst)
        {
            // state change
            FSM_StateChange(&g_tNmFsmDef, NM_ST_CNCT_WAIT_DISC);
        }
        else
        {
            // TODO: ??
        }
    }
    else
    if(NM_ST_AC_DHCP_WAIT_DISC == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        //tEvtRst = Opl_Wifi_Disc_Req(_WmDiscDoneIndCbFp);

        if(OPL_OK == tEvtRst)
        {
            // state change
            FSM_StateChange(&g_tNmFsmDef, NM_ST_AC_CNCT_WAIT_DISC);
        }
        else
        {
            // TODO: ??
        }
    }
    else
    if(NM_ST_IDLE == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        // Stop AC enable Timer
        osTimerStop(g_tAcEnTmr);

        if(g_u32CnctQuickSet)
        {
            g_u32CnctQuickSet = 0;

            T_PrApProfile tApProfile = {0};

            // copy scanned ssid
            memcpy(tApProfile.u8Ssid, g_tCnctReqConfig.ssid, g_tCnctReqConfig.ssid_length);

            // copy password
            memcpy(tApProfile.u8Pwd, g_tCnctReqConfig.password, g_tCnctReqConfig.password_length);

            // clear all profile record
            Opl_Wifi_Profile_Clear();

            // insert the new one
            Opl_Wifi_Profile_Ins(tApProfile);

            if(!g_u32AcEnable)
            {
                g_u32AcEnable = 1;
                NM_LOG_INFO("Turn on AC enable flag [%d]", g_u32AcEnable);
            }

            Opl_Wifi_AC_Enable_Req(_AcEnIndCbFp);

            // state change - QSET_WAIT_AC_E
            FSM_StateChange(&g_tNmFsmDef, NM_ST_QSET_WAIT_AC_EN);
        }
        else
        {
        tEvtRst = Opl_Wifi_Conn_Req(&g_tCnctReqConfig, _WmCnctDoneIndCbFp);

        if(OPL_OK == tEvtRst)
        {
            // state change
            FSM_StateChange(&g_tNmFsmDef, NM_ST_WAIT_CNCT);
        }
        else
        {
            // TODO: ??
        }
    }
    }

    return tEvtRst;
}



/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static T_OplErr APP_NmScanDoneHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
	T_OplErr tEvtRst = OPL_OK;

    NM_LOG_DEBG("Scan done ind");

	if(NM_ST_WIFI_UP_WAIT_SCAN == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        if(_IpIsGot())
        {
            FSM_StateChange(&g_tNmFsmDef, NM_ST_GOT_IP);
        }
        else
        {
            FSM_StateChange(&g_tNmFsmDef, NM_ST_WIFI_UP_ACPT_SCAN);
        }
    }
    else
    if(NM_ST_GOT_IP_WAIT_SCAN == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        FSM_StateChange(&g_tNmFsmDef, NM_ST_GOT_IP);
    }
    else // Should be NM_ST_WAIT_SCAN
    {
        // state change
        FSM_StateChange(&g_tNmFsmDef, NM_ST_IDLE);
    }

    

    if(NM_ST_WIFI_UP_ACPT_SCAN == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        // Start DHCP Timer
        osTimerStart(g_tDhcpTmr, NM_WIFI_DHCP_DEF_TIMEOUT);

        // Start Lwip network
        lwip_net_start(WIFI_MODE_STA);
    }
    else    // NM_ST_GOT_IP or NM_ST_IDLE
    {
        NM_LOG_INFO("AC enable flag [%d]", g_u32AcEnable);

        if(g_u32AcEnable)
        {
#if 0
            Opl_Wifi_AC_Enable_Req(_AcEnIndCbFp);

            if(NM_ST_GOT_IP == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
            {
                FSM_StateChange(&g_tNmFsmDef, NM_ST_GOT_IP_WAIT_AC_EN);
            }
            else    // Should be NM_ST_IDLE
            {
                // state change - IDLE_WAIT_AC_EN
                FSM_StateChange(&g_tNmFsmDef, NM_ST_IDLE_WAIT_AC_EN);
            }
#else
            // Start AC enable Timer
            osTimerStart(g_tAcEnTmr, NM_WIFI_AC_EN_DEF_TIMEOUT);
#endif
        }
    }

    // Notify caller
    APP_NmIndCbProc(tFsmEvent, tEvtRst);

    return tEvtRst;
}


/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static T_OplErr APP_NmCnctIndHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_OplErr tEvtRst = OPL_OK;
    uint32_t u32WiFiCnctOnceTime = 0;
    uint32_t u32CurrentTime = 0;
    uint32_t u32DhcpTimeout = 0;

    // !! To prevent sysyem always busy, watch dog can't exec;
    Hal_Wdt_Clear();

    u32CurrentTime = osKernelSysTick();

    g_u32WiFiCnctRetry++;

    if(u32CurrentTime >= g_u32WifiCnctStartTime)
    {
        u32WiFiCnctOnceTime = u32CurrentTime - g_u32WifiCnctStartTime;
    }
    else // overflow
    {
        u32WiFiCnctOnceTime = 0xFFFFFFFF - g_u32WifiCnctStartTime + u32CurrentTime + 1;
    }

    g_u32WiFiCnctTotalRetryTime = g_u32WiFiCnctTotalRetryTime + u32WiFiCnctOnceTime;

    if (NM_EVT_WIFI_CNCT_FAIL_IND == tFsmEvent)
    {
        g_u32WifiCnctStartTime = u32CurrentTime;
   
        if(u32WiFiCnctOnceTime > g_u32WiFiCnctFailMaxTime)
        {
            g_u32WiFiCnctFailMaxTime = u32WiFiCnctOnceTime;
        }
        
        if( g_u32WiFiCnctTotalRetryTime + g_u32WiFiCnctFailMaxTime > g_u32WifiCnctTimeout ||    // remaining time < once exec time
            g_u32WiFiCnctRetry > NM_WIFI_CNCT_RETRY_MAX                                            // Exceed max retry times
          )
        {
            // Stop retry, notify caller
            APP_NmIndCbProc(tFsmEvent, OPL_ERR_CASE_INVALID);

            // state change
            FSM_StateChange(&g_tNmFsmDef, NM_ST_IDLE);

            NM_LOG_INFO("AC enable flag [%d]", g_u32AcEnable);

            if(g_u32AcEnable)
            {
#if 0
                Opl_Wifi_AC_Enable_Req(_AcEnIndCbFp);

                // state change - IDLE_WAIT_AC_EN
                FSM_StateChange(&g_tNmFsmDef, NM_ST_IDLE_WAIT_AC_EN);
#else
                // Start AC enable Timer
                osTimerStart(g_tAcEnTmr, NM_WIFI_AC_EN_DEF_TIMEOUT);
#endif
            }
        }
        else
        {
            NM_LOG_INFO("Connect retry [%d]", g_u32WiFiCnctRetry);

            // continue retry
            tEvtRst = Opl_Wifi_Conn_Req(&g_tCnctReqConfig, _WmCnctDoneIndCbFp);

            if(OPL_OK == tEvtRst)
            {
                // state not change
            }
            else
            {
                // TODO: ??
            }
        }
    }
    else
    if (NM_EVT_WIFI_CNCT_PASS_IND == tFsmEvent)
    {
#if 0
        uint8_t u8aIp[4];
        u8aIp[0] = 123;
        u8aIp[1] = 123;
        u8aIp[2] = 123;
        u8aIp[3] = 123;

        _IpGet(u8aIp);

        printf("[NM] CNCT_PASS IP[%d.%d.%d,%d] \r\n", u8aIp[0], u8aIp[1], u8aIp[2], u8aIp[3]);
#endif        

        // state change - NM_ST_WIFI_UP
        FSM_StateChange(&g_tNmFsmDef, NM_ST_WIFI_UP);

        if(g_u32WiFiCnctTotalRetryTime + NM_WIFI_DHCP_DEF_TIMEOUT > g_u32WifiCnctTimeout)  // remaining time < DHCP default timeout
        {
            u32DhcpTimeout = NM_WIFI_DHCP_DEF_TIMEOUT;
        }
        else
        {
            u32DhcpTimeout = g_u32WifiCnctTimeout - g_u32WiFiCnctTotalRetryTime;
        }

        // TODO: Notify App to display WIFI LED?

        // Start DHCP Timer
        osTimerStart(g_tDhcpTmr, u32DhcpTimeout);

        // Start Lwip network
        lwip_net_start(WIFI_MODE_STA);
    }
    else
    {
        // TODO: ??
    }

    return tEvtRst;
}



/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static T_OplErr APP_NmAcEnIndHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_OplErr tEvtRst = OPL_OK;

    NM_LOG_DEBG("AC enable ind");

    // state change
    if(NM_ST_SCAN_WAIT_AC_EN == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        tEvtRst = Opl_Wifi_AC_Disable_Req(false, _AcDisaIndCbFp);

        if(OPL_OK != tEvtRst)
        {
            NM_LOG_WARN("Wifi ac disable req fail [%d]", tEvtRst);

            // TODO: ??
        }
        else
        {
            FSM_StateChange(&g_tNmFsmDef, NM_ST_SCAN_WAIT_AC_DISA);
        }
    }
    else
    if(NM_ST_CNCT_WAIT_AC_EN == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        tEvtRst = Opl_Wifi_AC_Disable_Req(true, _AcDisaIndCbFp);

        if(OPL_OK != tEvtRst)
        {
            NM_LOG_WARN("Wifi ac disable req fail [%d]", tEvtRst);

            // TODO: ??
        }
        else
        {
            FSM_StateChange(&g_tNmFsmDef, NM_ST_CNCT_WAIT_AC_DISA);
        }
    }
    else
    if(NM_ST_GOT_IP_WAIT_AC_EN == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
    	FSM_StateChange(&g_tNmFsmDef, NM_ST_AC_GOT_IP);
    }
    else    // Should be NM_ST_IDLE_WAIT_AC_EN or NM_ST_QSET_WAIT_AC_EN
    {
        if(NM_ST_QSET_WAIT_AC_EN == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
        {
            // Indicate app quick set done, the quick connect set apply same global callback
            if (g_tNmConnectIndCbFp != NULL)
            {
                g_tNmConnectIndCbFp(OPL_OK);
            }

        }

    	if( WM_WIFI_ST_NOT_CONNECT == (T_WmWifiConnectionState)*pu8Data)
        {
            FSM_StateChange(&g_tNmFsmDef, NM_ST_AC_EN);
        }
        else    // Should be WM_WIFI_ST_CONNECTED
        {
            if(_IpIsGot())
            {
                FSM_StateChange(&g_tNmFsmDef, NM_ST_AC_GOT_IP);
            }
            else
            {
                FSM_StateChange(&g_tNmFsmDef, NM_ST_AC_WIFI_UP);

                // Start DHCP Timer
                osTimerStart(g_tDhcpTmr, NM_WIFI_DHCP_DEF_TIMEOUT);

                // Start Lwip network
                lwip_net_start(WIFI_MODE_STA);
            }
        }
    }

    return tEvtRst;
}


/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static T_OplErr APP_NmWiFiUpHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    NM_LOG_DEBG("Wifi up ind");

#if 0
        uint8_t u8aIp[4];
        u8aIp[0] = 123;
        u8aIp[1] = 123;
        u8aIp[2] = 123;
        u8aIp[3] = 123;

        _IpGet(u8aIp);

        printf("[NM] WiFi Up IP[%d.%d.%d,%d] \r\n", u8aIp[0], u8aIp[1], u8aIp[2], u8aIp[3]);
#endif     

	// state change
	if(NM_ST_AC_EN == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
	{
		FSM_StateChange(&g_tNmFsmDef, NM_ST_AC_WIFI_UP);
	}
	else
	{
		FSM_StateChange(&g_tNmFsmDef, NM_ST_WIFI_UP);
	}

	// TODO: Notify App to display WIFI LED?

	// Start DHCP Timer
	osTimerStart(g_tDhcpTmr, NM_WIFI_DHCP_DEF_TIMEOUT);

	// Start Lwip network
	lwip_net_start(WIFI_MODE_STA);

	return OPL_OK;
}


/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static T_OplErr APP_NmWiFiDownHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    NM_LOG_DEBG("Wifi down ind");

	// State change
	switch(g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
	{
		case NM_ST_AC_WIFI_UP:
		case NM_ST_AC_GOT_IP:
		{
			FSM_StateChange(&g_tNmFsmDef, NM_ST_AC_EN);
			break;
		}
		case NM_ST_WIFI_UP:
        case NM_ST_WIFI_UP_ACPT_SCAN:
		case NM_ST_GOT_IP:
		{
			FSM_StateChange(&g_tNmFsmDef, NM_ST_IDLE);

            NM_LOG_INFO("AC enable flag [%d]", g_u32AcEnable);

		    if(g_u32AcEnable)
		    {
#if 0
		        Opl_Wifi_AC_Enable_Req(_AcEnIndCbFp);

		        // state change - IDLE_WAIT_AC_EN
		        FSM_StateChange(&g_tNmFsmDef, NM_ST_IDLE_WAIT_AC_EN);
#else
                
                if(NM_ST_GOT_IP == g_tNmFsmDef.ptFsmStateInfo.tHistoryState)
                {
                    // Stop previous AC enable Timer first
                    osTimerStop(g_tAcEnTmr);
                }

                // Start AC enable Timer
                osTimerStart(g_tAcEnTmr, NM_WIFI_AC_EN_DEF_TIMEOUT);
#endif
		    }
		}
		default:
			break;
	}

	// TODO: Notify App to display LED?

	return OPL_OK;
}


/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static T_OplErr APP_NmGotIpHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    NM_LOG_DEBG("Got IP ind");
#if 0
        uint8_t u8aIp[4];
        u8aIp[0] = 123;
        u8aIp[1] = 123;
        u8aIp[2] = 123;
        u8aIp[3] = 123;

        _IpGet(u8aIp);

        printf("[NM] Got IP[%d.%d.%d,%d] \r\n", u8aIp[0], u8aIp[1], u8aIp[2], u8aIp[3]);
#endif   
    // Stop DHCP Timer
    osTimerStop(g_tDhcpTmr);

	// state change
	if(NM_ST_AC_WIFI_UP == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
	{
		FSM_StateChange(&g_tNmFsmDef, NM_ST_AC_GOT_IP);
	}
	else   // NM_ST_WIFI_UP or NM_ST_WIFI_UP_ACPT_SCAN
	{
		FSM_StateChange(&g_tNmFsmDef, NM_ST_GOT_IP);

        NM_LOG_INFO("AC enable flag [%d]", g_u32AcEnable);

	    if(g_u32AcEnable)
	    {
#if 0
	        Opl_Wifi_AC_Enable_Req(_AcEnIndCbFp);

	        // state change - IDLE_WAIT_AC_EN
	        FSM_StateChange(&g_tNmFsmDef, NM_ST_GOT_IP_WAIT_AC_EN);
#else
            // Start AC enable Timer
            osTimerStart(g_tAcEnTmr, NM_WIFI_AC_EN_DEF_TIMEOUT);
#endif
    	}
	}

	// TODO: Notify App to display network LED?

    // Notify caller (only user connect)
    if(NM_ST_WIFI_UP == g_tNmFsmDef.ptFsmStateInfo.tHistoryState)
    {
        APP_NmIndCbProc(tFsmEvent, OPL_OK);
    }

	return OPL_OK;
}


/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static T_OplErr APP_NmAcDisaIndHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_OplErr tEvtRst = OPL_OK;

    NM_LOG_DEBG("AC disable done");

    if(NM_ST_SCAN_WAIT_AC_DISA == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        wifi_scan_config_t tScanConfig = {0};
        
        tScanConfig.show_hidden = 1;
        tScanConfig.scan_type = WIFI_SCAN_TYPE_MIX;

        tEvtRst = Opl_Wifi_Scan_Req(&tScanConfig, _WmScanDoneIndCbFp);

        if(OPL_OK != tEvtRst)
        {
            NM_LOG_WARN("Wifi scan req fail [%d]", tEvtRst);

            // TODO: ??
        }
        else
        {
            
            if(NM_ST_AC_WIFI_UP == g_tNmFsmDef.ptFsmStateInfo.tHistoryState)
            {
                FSM_StateChange(&g_tNmFsmDef, NM_ST_WIFI_UP_WAIT_SCAN);
            }
            else
            if(NM_ST_AC_GOT_IP == g_tNmFsmDef.ptFsmStateInfo.tHistoryState)
            {
                FSM_StateChange(&g_tNmFsmDef, NM_ST_GOT_IP_WAIT_SCAN);
            }
            else 
            {
                FSM_StateChange(&g_tNmFsmDef, NM_ST_WAIT_SCAN);
            }
        }
    }
    else    // NM_ST_CNCT_WAIT_AC_DISA
    {
        if(g_u32CnctQuickSet)
        {
            g_u32CnctQuickSet = 0;

            T_PrApProfile tApProfile = {0};

            // copy scanned ssid
            memcpy(tApProfile.u8Ssid, g_tCnctReqConfig.ssid, g_tCnctReqConfig.ssid_length);

            // copy password
            memcpy(tApProfile.u8Pwd, g_tCnctReqConfig.password, g_tCnctReqConfig.password_length);

            // clear all profile record
            Opl_Wifi_Profile_Clear();

            // insert the new one
            Opl_Wifi_Profile_Ins(tApProfile);

            if(!g_u32AcEnable)
            {
                g_u32AcEnable = 1;
                NM_LOG_INFO("Turn on AC enable flag [%d]", g_u32AcEnable);
            }

            Opl_Wifi_AC_Enable_Req(_AcEnIndCbFp);

            // state change - QSET_WAIT_AC_E
            FSM_StateChange(&g_tNmFsmDef, NM_ST_QSET_WAIT_AC_EN);
        }
    else
    {
        tEvtRst = Opl_Wifi_Conn_Req(&g_tCnctReqConfig, _WmCnctDoneIndCbFp);

        if(OPL_OK == tEvtRst)
        {
            // state change
            FSM_StateChange(&g_tNmFsmDef, NM_ST_WAIT_CNCT);
        }
        else
        {
            // TODO: ??
        }
    }
    }

    return tEvtRst;
}


/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static T_OplErr APP_NmDhcpTimeoutHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_OplErr tEvtRst = OPL_OK;

    NM_LOG_DEBG("DHCP timeout ind");

    tEvtRst = Opl_Wifi_Disc_Req(_WmDiscDoneIndCbFp);

    if(OPL_OK != tEvtRst)
    {
        NM_LOG_WARN("Wifi disconnect req fail [%d]", tEvtRst);

        // TODO: ??

        return tEvtRst;
    }

    // state change
    if(NM_ST_AC_WIFI_UP == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        FSM_StateChange(&g_tNmFsmDef, NM_ST_AC_DHCP_WAIT_DISC);
    }
    else    // NM_ST_WIFI_UP or NM_ST_WIFI_UP_ACPT_SCAN
    {
        FSM_StateChange(&g_tNmFsmDef, NM_ST_DHCP_WAIT_DISC);
    }

    return tEvtRst;
}

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static T_OplErr APP_NmAcEnTimeoutHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    NM_LOG_DEBG("AC enable timeout ind");

    Opl_Wifi_AC_Enable_Req(_AcEnIndCbFp);

    if(NM_ST_GOT_IP == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        FSM_StateChange(&g_tNmFsmDef, NM_ST_GOT_IP_WAIT_AC_EN);
    }
    else    // Should be NM_ST_IDLE
    {
        // state change - IDLE_WAIT_AC_EN
        FSM_StateChange(&g_tNmFsmDef, NM_ST_IDLE_WAIT_AC_EN);
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static T_OplErr APP_NmWiFiDiscIndHndlr(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_OplErr tEvtRst = OPL_OK;    

    NM_LOG_DEBG("Disconnect ind");

    if(NM_ST_CNCT_WAIT_DISC == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        if(g_u32CnctQuickSet)
        {
            g_u32CnctQuickSet = 0;

            T_PrApProfile tApProfile = {0};

            // copy scanned ssid
            memcpy(tApProfile.u8Ssid, g_tCnctReqConfig.ssid, g_tCnctReqConfig.ssid_length);

            // copy password
            memcpy(tApProfile.u8Pwd, g_tCnctReqConfig.password, g_tCnctReqConfig.password_length);

            // clear all profile record
            Opl_Wifi_Profile_Clear();

            // insert the new one
            Opl_Wifi_Profile_Ins(tApProfile);

            if(!g_u32AcEnable)
            {
                g_u32AcEnable = 1;
                NM_LOG_INFO("Turn on AC enable flag [%d]", g_u32AcEnable);
            }

            Opl_Wifi_AC_Enable_Req(_AcEnIndCbFp);

            // state change - QSET_WAIT_AC_E
            FSM_StateChange(&g_tNmFsmDef, NM_ST_QSET_WAIT_AC_EN);
        }
        else
        {

        tEvtRst = Opl_Wifi_Conn_Req(&g_tCnctReqConfig, _WmCnctDoneIndCbFp);

        if(OPL_OK != tEvtRst)
        {
            NM_LOG_WARN("Wifi conn req fail [%d]", tEvtRst);

            // TODO: ??
        }
        else
        {
            FSM_StateChange(&g_tNmFsmDef, NM_ST_WAIT_CNCT);
            }
        }
    }
    else
    if(NM_ST_AC_SCAN_WAIT_DISC == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        tEvtRst = Opl_Wifi_AC_Disable_Req(false, _AcDisaIndCbFp);

        if(OPL_OK != tEvtRst)
        {
            NM_LOG_WARN("Wifi ac disable req fail [%d]", tEvtRst);

            // TODO: ??
        }
        else
        {
            FSM_StateChange(&g_tNmFsmDef, NM_ST_SCAN_WAIT_AC_DISA);
        }
    }
    else
    if(NM_ST_AC_CNCT_WAIT_DISC == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        tEvtRst = Opl_Wifi_AC_Disable_Req(true, _AcDisaIndCbFp);

        if(OPL_OK != tEvtRst)
        {
            NM_LOG_WARN("Wifi ac disable req fail [%d]", tEvtRst);

            // TODO: ??
        }
        else
        {
            FSM_StateChange(&g_tNmFsmDef, NM_ST_CNCT_WAIT_AC_DISA);
        }
    }
    else
    if(NM_ST_AC_DHCP_WAIT_DISC == g_tNmFsmDef.ptFsmStateInfo.tCurrentState)
    {
        FSM_StateChange(&g_tNmFsmDef, NM_ST_AC_EN);
    }
    else    // Should be NM_ST_DHCP_WAIT_DISC
    {
        uint8_t u8ShouldNotify = 0;

        if(NM_ST_WIFI_UP == g_tNmFsmDef.ptFsmStateInfo.tHistoryState)
        {
            // Only user connect should be notified
            u8ShouldNotify = 1;
        }

        FSM_StateChange(&g_tNmFsmDef, NM_ST_IDLE);

        if(u8ShouldNotify)
        {
            // Got IP fail and disconnect done, notify caller
            APP_NmIndCbProc(tFsmEvent, OPL_ERR_CASE_INVALID);
        }

        NM_LOG_INFO("AC enable flag [%d]", g_u32AcEnable);

        if(g_u32AcEnable)
        {
#if 0
            Opl_Wifi_AC_Enable_Req(_AcEnIndCbFp);

            // state change - IDLE_WAIT_AC_EN
            FSM_StateChange(&g_tNmFsmDef, NM_ST_IDLE_WAIT_AC_EN);
#else
            // Start AC enable Timer
            osTimerStart(g_tAcEnTmr, NM_WIFI_AC_EN_DEF_TIMEOUT);
#endif
        }
    }


    return tEvtRst;
}


/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
T_FsmDef *APP_NmFsmDefGet(void)
{
    return &g_tNmFsmDef;
}

/*************************************************************************
* FUNCTION:
*   APP_NmEventProc
*
* DESCRIPTION:
*   network manager event process
*   (context in app_main, should runs in app_main task)
*
* PARAMETERS
*   u32EventId :    [IN] network manager event id
*   u8Data :        [IN] data parse to network manager
*   u32DataLen :    [IN] data lens parse to network manager
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr APP_NmEventProc(uint32_t u32EventId, uint8_t *u8Data, uint32_t u32DataLen)
{
    T_OplErr tEvtRst = OPL_OK;

    if((NM_EVT_BEGIN <= u32EventId) && (u32EventId <= NM_EVT_TOTAL))
    {
        tEvtRst = FSM_Run( APP_NmFsmDefGet(),
                           u32EventId,
                           u8Data,
                           u32DataLen,
                           NULL );

        // Unsolicited callback
        if(OPL_OK == tEvtRst)
        {
            if( u32EventId == NM_EVT_WIFI_DOWN_IND ||
                u32EventId == NM_EVT_GOT_IP ||
                u32EventId == NM_EVT_WIFI_RESET )
            {
                APP_NmUslctdCbRun( (T_NmEventList)u32EventId,
                                    u8Data,
                                    u32DataLen );
            }
        }

        return tEvtRst;
    }
    else
    {
        return OPL_ERR_RTOS_EVT_NOT_FOUND;
    }
}

/*************************************************************************
* FUNCTION:
*   APP_NmInit
*
* DESCRIPTION:
*   network manager initiate function (optional to assign unsolicited callback)
*
* PARAMETERS
*   u8AcEnable :    [IN] enable auto-connect
*   fpUslctdCb :    [IN] unsolicited callback function pointer
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr APP_NmInit(uint8_t u8AcEnable, T_NmUslctdCbFp fpUslctdCb)
{
    if(0 == tNmInitStatus.u8IsInit)
    {
        osTimerDef_t tTmrDhcpDef, tTmrAcEnDef;
        // name of newtowrk manager
        uint8_t u8aNmName[2] = "NM";

        // initialize the newtowrk manager state machine
        FSM_Init(&g_tNmFsmDef, u8aNmName, tNmStateTbl, NM_ST_NULL);

        // initialize the newtowrk manager indicate callback handler
        FSM_IndCbHdlInit(&g_tNmFsmDef, APP_WmIndCbSet, APP_NmIndCbProc);

        // AC Enable flag
        g_u32AcEnable = u8AcEnable;

        // Assign unsolicted callback function pointer
        g_fpNmUslctdCb = fpUslctdCb;

        //
        // WIFI Manager Initialize
        //

        // regist the wifi unsolicited callback
        Opl_Wifi_Uslctd_CB_Reg(_WmUnsolicitedCbFp);

        // initialize the wifi manager
        Opl_Wifi_Init_Req(_WmInitDoneIndCbFp);

        // Create DHCP timer
        tTmrDhcpDef.ptimer = _DhcpTimeoutCb;
        g_tDhcpTmr = osTimerCreate(&tTmrDhcpDef, osTimerOnce, NULL);
        if (g_tDhcpTmr == NULL)
        {
            NM_LOG_ERRO("Create dhcp timer fail");
        }

        // Create AC enable timer
        tTmrAcEnDef.ptimer = _AcEnTimeoutCb;
        g_tAcEnTmr = osTimerCreate(&tTmrAcEnDef, osTimerOnce, NULL);
        if (g_tAcEnTmr == NULL)
        {
            NM_LOG_ERRO("Create ac enable timer fail");
        }

        tNmInitStatus.u8IsInit = 1;

        NM_LOG_DEBG("NM init ok");

        return OPL_OK;
    }
    else
    {
        NM_LOG_WARN("NM already inited");

        return OPL_ERR_FSM_REINIT;
    }
}

/*************************************************************************
* FUNCTION:
*   APP_NmInitAndCnct
*
* DESCRIPTION:
*   network manager initiate function and trigger WI-FI connect direclty (optional to assign unsolicited callback)
*   (SSID & PWD connection only, and will clear all profile record only store the assigned one)
*
* PARAMETERS
*   fpUslctdCb :    [IN] unsolicited callback function pointer
*   ptNmWifiCnctConfig :
*                   [IN] WI-FI connect config
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr APP_NmInitAndCnct(T_NmUslctdCbFp fpUslctdCb, T_NmWifiCnctConfig *ptNmWifiCnctConfig)
{
    T_OplErr tEvtRst = OPL_OK;

    if(0 == ptNmWifiCnctConfig->u8SsidLen)
    {
        NM_LOG_ERRO("cnct config ssid empty, only allow ssid connection");

        return OPL_ERR_PARAM_INVALID;
    }

    tEvtRst == APP_NmInit(true, fpUslctdCb);

    if(OPL_OK == tEvtRst)
    {
        tNmInitStatus.u8IsCnctDrct = 1;

        memcpy(tNmInitStatus.u8Ssid, ptNmWifiCnctConfig->u8aSsid, ptNmWifiCnctConfig->u8SsidLen);
        memcpy(tNmInitStatus.u8Pwd, ptNmWifiCnctConfig->u8aPwd, ptNmWifiCnctConfig->u8PwdLen);
    }

    return tEvtRst;
}

#endif /* NM_ENABLED */
