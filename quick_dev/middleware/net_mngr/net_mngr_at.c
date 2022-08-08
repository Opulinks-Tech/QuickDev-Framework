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
*  nm_mngr_at.c
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
#include "at_cmd.h"
#include "at_cmd_common.h"
#if defined(OPL1000_A2) || defined(OPL1000_A3)
#include "at_cmd_task_patch.h"
#include "at_cmd_data_process_patch.h"
#include "agent_patch.h"
#elif defined(OPL2500_A0)
#include "at_cmd_task.h"
#include "at_cmd_data_process.h"
#endif
#include "lwip_helper.h"
#include "net_mngr.h"
#include "net_mngr_at.h"
#include "net_mngr_api.h"
#include "opl_err.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (NM_ENABLED == 1)

#define NM_AT_LOG                                       msg_print_uart1

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
#if (NM_AT_ENABLED == 1)
/*************************************************************************
* FUNCTION:
*   AT_CmdNmScanIndCb
*
* DESCRIPTION:
*   indicate callback for network manager scan request
*
* PARAMETERS
*   tEvtRst :       [IN] see in opl_err.h
*
* RETURNS
*   none
*
*************************************************************************/
static void AT_CmdNmScanIndCb(T_OplErr tEvtRst)
{
    if(OPL_OK == tEvtRst)
    {
        NM_AT_LOG("[IND] Scan done success\r\n");        
    }
    else
    {
        NM_AT_LOG("[IND] Scan done fail\r\n");
    }
}

/*************************************************************************
* FUNCTION:
*   AT_CmdNmConnIndCb
*
* DESCRIPTION:
*   indicate callback for network manager connect request
*
* PARAMETERS
*   tEvtRst :       [IN] see in opl_err.h
*
* RETURNS
*   none
*
*************************************************************************/
static void AT_CmdNmConnIndCb(T_OplErr tEvtRst)
{
    if(OPL_OK == tEvtRst)
    {
        // start tcpip network
        //lwip_net_start(WIFI_MODE_STA);    // no need

        NM_AT_LOG("[IND] Connect success\r\n");
    }
    else
    {
        NM_AT_LOG("[IND] Connect fail\r\n");
    }
}

/*************************************************************************
* FUNCTION:
*   AT_CmdNmQConnSetIndCb
*
* DESCRIPTION:
*   indicate callback for network manager connect request
*
* PARAMETERS
*   tEvtRst :       [IN] see in opl_err.h
*
* RETURNS
*   none
*
*************************************************************************/
static void AT_CmdNmQConnSetIndCb(T_OplErr tEvtRst)
{ 
    if(OPL_OK == tEvtRst)
    {

        NM_AT_LOG("[IND] Quick connect set success\r\n");
    }
    else
    {
        NM_AT_LOG("[IND] Quick connect set fail\r\n");
    }
}
#endif

/*************************************************************************
* FUNCTION:
*   AT_CmdNmScanHandler
*
* DESCRIPTION:
*   at cmd for network manager scan request
*
* PARAMETERS
*   buf :           [IN] at cmd input
*   len :           [IN] at cmd input lens
*   mode :          [IN] at cmd type
*
* RETURNS
*   int :           handler result
*
*************************************************************************/
int AT_CmdNmScanHandler(char *buf, int len, int mode)
{
    int iRet = 0;

#if (NM_AT_ENABLED == 1)

    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
    {
        goto done;
    }

    if (argc > 2)
    {
        NM_AT_LOG("invalid param number\r\n");
        goto done;
    }

    
    switch (mode)
    {
        case AT_CMD_MODE_SET:
        case AT_CMD_MODE_EXECUTION:
        {
            NM_AT_LOG("NM Scan request \r\n");

            if(OPL_OK != APP_NmWifiScanReq(AT_CmdNmScanIndCb))
            {
                NM_AT_LOG("NM full scan request fail\r\n");
            }

            iRet = 1;
        
            break;
        }
    }

done:
    if (iRet)
    {
        NM_AT_LOG("OK\r\n");
    }
    else
    {
        NM_AT_LOG("ERROR\r\n");
    }

#else
    NM_AT_LOG("NM at cmd not support\r\n");
#endif /* NM_AT_ENABLED */

    return iRet;
}

/*************************************************************************
* FUNCTION:
*   AT_CmdNmConnectHandler
*
* DESCRIPTION:
*   at cmd for network manager connect request
*
* PARAMETERS
*   buf :           [IN] at cmd input
*   len :           [IN] at cmd input lens
*   mode :          [IN] at cmd type
*
* RETURNS
*   int :           handler result
*
*************************************************************************/
int AT_CmdNmConnectHandler(char *buf, int len, int mode)
{
    int iRet = 0;

#if (NM_AT_ENABLED == 1)

    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    T_NmWifiCnctConfig tNmCnctConf = {0};
    memset(&tNmCnctConf, 0, sizeof(T_NmWifiCnctConfig));

    if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
    {
        goto done;
    }

    if (argc > 4)
    {
        NM_AT_LOG("invalid param number\r\n");
        goto done;
    }

    switch (mode)
    {
        case AT_CMD_MODE_SET:
        {
            if(strcmp(argv[1], " ") != 0 && strlen(argv[1]) != 0)
            {
                // parse each mac number in to 6 byte array
                sscanf(argv[1], "%x:%x:%x:%x:%x:%x", (uint32_t*)&tNmCnctConf.u8aBssid[0],
                                                     (uint32_t*)&tNmCnctConf.u8aBssid[1],
                                                     (uint32_t*)&tNmCnctConf.u8aBssid[2],
                                                     (uint32_t*)&tNmCnctConf.u8aBssid[3],
                                                     (uint32_t*)&tNmCnctConf.u8aBssid[4],
                                                     (uint32_t*)&tNmCnctConf.u8aBssid[5]);
            }

            if(strcmp(argv[2], " ") != 0 && strlen(argv[2]) != 0)
            {
                memcpy(tNmCnctConf.u8aPwd, argv[2], strlen(argv[2]));
                tNmCnctConf.u8PwdLen = strlen(argv[2]);
            }

            NM_AT_LOG( "NM conn request {bssid %x:%x:%x:%x:%x:%x, password %s(%d)}\r\n", tNmCnctConf.u8aBssid[0],
                                                                                         tNmCnctConf.u8aBssid[1],
                                                                                         tNmCnctConf.u8aBssid[2],
                                                                                         tNmCnctConf.u8aBssid[3],
                                                                                         tNmCnctConf.u8aBssid[4],
                                                                                         tNmCnctConf.u8aBssid[5],
                                                                                         tNmCnctConf.u8aPwd,
                                                                                         tNmCnctConf.u8PwdLen);
            
            if(OPL_OK != APP_NmWifiCnctReq(&tNmCnctConf, AT_CmdNmConnIndCb))
            {
                NM_AT_LOG("Dedicate conn request fail\r\n");
            }

            iRet = 1;

            break;
        }

        case AT_CMD_MODE_EXECUTION:
        {
            NM_AT_LOG("NM conn request {NULL}\r\n");

            // connect with ssid
            tNmCnctConf.u8SsidLen = strlen((const char *)NM_AT_TEST_SSID);
            memcpy(tNmCnctConf.u8aSsid, NM_AT_TEST_SSID, tNmCnctConf.u8SsidLen);

            // copy password
            if(0 != strlen((const char *)NM_AT_TEST_PWD))
            {
                tNmCnctConf.u8PwdLen = strlen((const char *)NM_AT_TEST_PWD);
                memcpy(tNmCnctConf.u8aPwd, NM_AT_TEST_PWD, tNmCnctConf.u8PwdLen);
            }

#if 0
            APP_SendMessage((uint32_t)NM_EVT_WIFI_CNTC_REQ, (uint8_t *)&tNmCnctConf, sizeof(T_NmWifiCnctConfig));
#else
            if(OPL_OK != APP_NmWifiCnctReq(&tNmCnctConf, AT_CmdNmConnIndCb))
            {
                msg_print_uart1("[AT] Fast conn request fail\r\n");
            }
#endif
            iRet = 1;
        
            break;
        }
    }

done:
    if (iRet)
    {
        NM_AT_LOG("OK\r\n");
    }
    else
    {
        NM_AT_LOG("ERROR\r\n");
    }

#else
    NM_AT_LOG("NM at cmd not support\r\n");
#endif /* NM_AT_ENABLED */

    return iRet;
}



/*************************************************************************
* FUNCTION:
*   AT_CmdNmQuickConnectSettHandler
*
* DESCRIPTION:
*   at cmd for network manager quick connect set request
*
* PARAMETERS
*   buf :           [IN] at cmd input
*   len :           [IN] at cmd input lens
*   mode :          [IN] at cmd type
*
* RETURNS
*   int :           handler result
*
*************************************************************************/
int AT_CmdNmQuickConnectSetHandler(char *buf, int len, int mode)
{
    int iRet = 0;

#if (NM_AT_ENABLED == 1)

    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    T_NmWifiQCnctSet tNmQCnctConf = {0};
    memset(&tNmQCnctConf, 0, sizeof(T_NmWifiQCnctSet));

    if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
    {
        goto done;
    }

    if (argc > 2)
    {
        NM_AT_LOG("invalid param number\r\n");
        goto done;
    }

    switch (mode)
    {
        case AT_CMD_MODE_SET:    
        case AT_CMD_MODE_EXECUTION:
        {
            NM_AT_LOG("NM quick conn set request {NULL}\r\n");

            // connect with ssid
            tNmQCnctConf.u8SsidLen = strlen((const char *)NM_AT_TEST_SSID);
            memcpy(tNmQCnctConf.u8aSsid, NM_AT_TEST_SSID, tNmQCnctConf.u8SsidLen);

            // copy password
            if(0 != strlen((const char *)NM_AT_TEST_PWD))
            {
                tNmQCnctConf.u8PwdLen = strlen((const char *)NM_AT_TEST_PWD);
                memcpy(tNmQCnctConf.u8aPwd, NM_AT_TEST_PWD, tNmQCnctConf.u8PwdLen);
            }

            if(OPL_OK != APP_NmQuickCnctSetReq(&tNmQCnctConf, AT_CmdNmQConnSetIndCb))
            {
                msg_print_uart1("[AT] NM quick conn set request fail\r\n");
            }

            iRet = 1;
        
            break;
        }
    }

done:
    if (iRet)
    {
        NM_AT_LOG("OK\r\n");
    }
    else
    {
        NM_AT_LOG("ERROR\r\n");
    }

#else
    NM_AT_LOG("NM at cmd not support\r\n");
#endif /* NM_AT_ENABLED */

    return iRet;
}

#endif /* NM_ENABLED */
