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
*  net_mngr_api.c
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
#include "fsm_kernel.h"
#include "net_mngr_api.h"
#include "net_mngr.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (NM_ENABLED == 1)

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

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

/*************************************************************************
* FUNCTION:
*   APP_NmWifiCnctReq
*
* DESCRIPTION:
*   trigger WI-FI quick connect set request, thaat is insert AP profile and enable auto connect
*
* PARAMETERS
*   ptNmWifiQCnctSet :
*                   [IN] WI-FI quick connect set configuration
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr APP_NmQuickCnctSetReq(T_NmWifiQCnctSet *ptNmWifiQCnctSet, T_NmQCnctSetIndCbFp fpIndCb)
{
    T_NmWifiCnctConfInt tNmWifiCnctConfInt;
    memset((void *)&tNmWifiCnctConfInt, 0, sizeof(T_NmWifiCnctConfInt));
    memcpy((void *)(tNmWifiCnctConfInt.u8aSsid), (void *)(ptNmWifiQCnctSet->u8aSsid), WIFI_MAX_LENGTH_OF_SSID);
    tNmWifiCnctConfInt.u8SsidLen = ptNmWifiQCnctSet->u8SsidLen;
    memcpy((void *)(tNmWifiCnctConfInt.u8aPwd), (void *)(ptNmWifiQCnctSet->u8aPwd), WIFI_LENGTH_PASSPHRASE);
    tNmWifiCnctConfInt.u8PwdLen = ptNmWifiQCnctSet->u8PwdLen;
    tNmWifiCnctConfInt.u8QuickSet = 1;

    return FSM_Run( APP_NmFsmDefGet(), 
                    NM_EVT_WIFI_CNCT_REQ, 
                    (uint8_t *)&tNmWifiCnctConfInt,
                    sizeof(T_NmWifiCnctConfInt),
                    (FsmIndicateCbFunc)fpIndCb
                  );
}

/*************************************************************************
* FUNCTION:
*   APP_NmWifiScanReq
*
* DESCRIPTION:
*   trigger WI-FI scan request, and carried the scan result in indicate callback
*
* PARAMETERS
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr APP_NmWifiScanReq(T_NmScanDoneIndCbFp fpIndCb)
{
    return FSM_Run( APP_NmFsmDefGet(), 
                    NM_EVT_WIFI_SCAN_REQ, 
                    NULL, 
                    0, 
                    (FsmIndicateCbFunc)fpIndCb
                  );
}

/*************************************************************************
* FUNCTION:
*   APP_NmWifiCnctReq
*
* DESCRIPTION:
*   trigger WI-FI connect request, and carried the connect result in indicate callback
*
* PARAMETERS
*   ptWmWifiCnctConfig :
*                   [IN] WI-FI connect configuration
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr APP_NmWifiCnctReq(T_NmWifiCnctConfig *ptWmWifiCnctConfig, T_NmCnctIndCbFp fpIndCb)
{
    T_NmWifiCnctConfInt tNmWifiCnctConfInt;
    memset((void *)&tNmWifiCnctConfInt, 0, sizeof(T_NmWifiCnctConfInt));
    memcpy((void *)&tNmWifiCnctConfInt, (void *)ptWmWifiCnctConfig, sizeof(T_NmWifiCnctConfig));
    tNmWifiCnctConfInt.u8QuickSet = 0;


    return FSM_Run( APP_NmFsmDefGet(), 
                    NM_EVT_WIFI_CNCT_REQ, 
                    (uint8_t *)&tNmWifiCnctConfInt,
                    sizeof(T_NmWifiCnctConfInt),
                    (FsmIndicateCbFunc)fpIndCb
                  );
}


/*************************************************************************
* FUNCTION:
*   APP_NmWifiStopReq
*
* DESCRIPTION:
*   Stop all WiFi activities (disconnect WiFi and disable Auto-connect)
*
* PARAMETERS
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr APP_NmWifiStopReq(T_NmStopIndCbFp fpIndCb)
{
    return FSM_Run( APP_NmFsmDefGet(), 
                    NM_EVT_WIFI_STOP_REQ, 
                    NULL, 
                    0, 
                    (FsmIndicateCbFunc)fpIndCb
                  );
}

/*************************************************************************
* FUNCTION:
*   APP_NmWifiResumeReq
*
* DESCRIPTION:
*   Resume WiFi (Enable auto connect), should only be called after WiFi stop
*
* PARAMETERS
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr APP_NmWifiResumeReq(T_NmResumeIndCbFp fpIndCb)
{
    return FSM_Run( APP_NmFsmDefGet(), 
                    NM_EVT_WIFI_RESUME_REQ, 
                    NULL, 
                    0, 
                    (FsmIndicateCbFunc)fpIndCb
                  );
}

#endif /* NM_ENABLED */
