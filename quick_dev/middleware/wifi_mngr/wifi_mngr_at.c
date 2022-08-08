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
*  wifi_mngr_at.c
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
#include "mw_fim_default_group11_project.h"
#include "opl_err.h"
#include "wifi_agent.h"
#include "wifi_mngr.h"
#include "wifi_mngr_at.h"
#include "wifi_pro_rec.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (WM_ENABLED == 1)

#define WM_AT_LOG                                       msg_print_uart1

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

#if (WM_AT_ENABLED == 1)
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
static void AT_CmdWmScanIndCb(T_OplErr tEvtRst)
{
    if(OPL_OK == tEvtRst)
    {
        WM_AT_LOG("[IND] Scan done success\r\n");        
    }
    else
    {
        WM_AT_LOG("[IND] Scan done fail\r\n");
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
static void AT_CmdWmConnIndCb(T_OplErr tEvtRst)
{
    if(OPL_OK == tEvtRst)
    {
        // start tcpip network
        lwip_net_start(WIFI_MODE_STA);

        WM_AT_LOG("[IND] Connect success\r\n");
    }
    else
    {
        WM_AT_LOG("[IND] Connect fail\r\n");
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
static void AT_CmdWmDisconnIndCb(T_OplErr tEvtRst)
{
    if(OPL_OK == tEvtRst)
    {
        WM_AT_LOG("[IND] Disconnect success\r\n");
    }
    else
    {
        WM_AT_LOG("[IND] Disconnect fail\r\n");
    }
}
#endif /* WM_AT_ENABLED */

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
void AT_UnsolicitedCallback(T_WmUslctedEvtType tEvtType, T_OplErr tEvtRst, uint8_t *pu8Data, uint32_t u32DataLen)
{
#if (WM_AT_ENABLED == 1)

    if(USLCTED_CB_EVT_WIFI_UP == tEvtType)
    {
        WM_AT_LOG("USLCTED WIFI UP\r\n");        
    }
    else if(USLCTED_CB_EVT_WIFI_DOWN == tEvtType)
    {
        WM_AT_LOG("USLCTED WIFI DOWN\r\n");
    }
    else if(USLCTED_CB_EVT_GOT_IP == tEvtType)
    {
        // find ip number in network interface
        uint32_t u32Ip = *((uint32_t*)pu8Data);
        uint8_t u8aIp[4] = {0};

        u8aIp[0] = (u32Ip >> 0) & 0xFF;
        u8aIp[1] = (u32Ip >> 8) & 0xFF;
        u8aIp[2] = (u32Ip >> 16) & 0xFF;
        u8aIp[3] = (u32Ip >> 24) & 0xFF;

        WM_AT_LOG("USLCTED GOT IP [%d.%d.%d.%d]\r\n", u8aIp[0], u8aIp[1], u8aIp[2], u8aIp[3]);
    }

#endif /* WM_AT_ENABLED */
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
int AT_CmdWmScanHandler(char *buf, int len, int mode)
{
    int iRet = 0;

#if (WM_AT_ENABLED == 1)

    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    wifi_scan_config_t tScanConfig = {0};

    if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
    {
        goto done;
    }

    if (argc > 4)
    {
        WM_AT_LOG("invalid param number\r\n");
        goto done;
    }

    tScanConfig.show_hidden = 1;
    tScanConfig.scan_type = WIFI_SCAN_TYPE_MIX;

    switch (mode)
    {
        case AT_CMD_MODE_SET:
        {
            uint8_t u8Ssid[WIFI_MAX_LENGTH_OF_SSID] = {0};

            if(strcmp(argv[1], " ") != 0 && strlen(argv[1]) != 0)
            {    
                memcpy(u8Ssid, argv[1], strlen(argv[1]));
                tScanConfig.ssid = u8Ssid;
            }
            else
            {
                tScanConfig.ssid = NULL;
            }

            WM_AT_LOG("Scan request {ssid %s, hidden %d, type %d}\r\n", tScanConfig.ssid,
                                                                        tScanConfig.show_hidden,
                                                                        tScanConfig.scan_type);

            if(OPL_OK != Opl_Wifi_Scan_Req(&tScanConfig, AT_CmdWmScanIndCb))
            {
                WM_AT_LOG("Dedicate scan request fail\r\n");
            }

            iRet = 1;

            break;
        }

        case AT_CMD_MODE_EXECUTION:
        {
            WM_AT_LOG("Scan request {hidden %d, type %d}\r\n", tScanConfig.show_hidden,
                                                               tScanConfig.scan_type);

            if(OPL_OK != Opl_Wifi_Scan_Req(&tScanConfig, AT_CmdWmScanIndCb))
            {
                WM_AT_LOG("Full scan request fail\r\n");
            }

            iRet = 1;
        
            break;
        }

        case AT_CMD_MODE_READ:
        {
            WM_AT_LOG("Get scan list\r\n");

            wifi_scan_list_t *pstScanList = NULL;

            pstScanList = (wifi_scan_list_t *)malloc(sizeof(wifi_scan_list_t));
			
            if(NULL == pstScanList)
            {
                WM_AT_LOG("malloc fail\r\n");

                return OPL_ERR_ALLOC_MEMORY_FAIL;
            }

            memset(pstScanList, 0, sizeof(wifi_scan_list_t));

            // get the information of scan list			
            WM_WaScanListGet(pstScanList);

            uint32_t cnt = 0;

            WM_AT_LOG("Scanned total num %d\r\n", pstScanList->num);

            for(; cnt < pstScanList->num; cnt ++)
            {
                WM_AT_LOG("(%d) %s %d\r\n", cnt, pstScanList->ap_record[cnt].ssid, pstScanList->ap_record[cnt].rssi);
            }

            free(pstScanList);

            iRet = 1;

            break;
        }
    }

done:
    if (iRet)
    {
        WM_AT_LOG("OK\r\n");
    }
    else
    {
        WM_AT_LOG("ERROR\r\n");
    }

#else
    WM_AT_LOG("WM at cmd not support\r\n");
#endif /* WM_AT_ENABLED */

    return iRet;
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
int AT_CmdWmConnectHandler(char *buf, int len, int mode)
{
    int iRet = 0;

#if (WM_AT_ENABLED == 1)

    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    T_WmConnConfig tConnConfig = {0};
    memset(&tConnConfig, 0, sizeof(T_WmConnConfig));

    if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
    {
        goto done;
    }

    if (argc > 4)
    {
        WM_AT_LOG("invalid param number\r\n");
        goto done;
    }

    switch (mode)
    {
        case AT_CMD_MODE_SET:
        {
            if(strcmp(argv[1], " ") != 0 && strlen(argv[1]) != 0)
            {    
                memcpy(tConnConfig.ssid, argv[1], strlen(argv[1]));
                tConnConfig.ssid_length = strlen(argv[1]);
            }

            if(strcmp(argv[2], " ") != 0 && strlen(argv[2]) != 0)
            {
                // parse each mac number in to 6 byte array
                sscanf(argv[2], "%x:%x:%x:%x:%x:%x", (uint32_t*)&tConnConfig.bssid[0],
                                                     (uint32_t*)&tConnConfig.bssid[1],
                                                     (uint32_t*)&tConnConfig.bssid[2],
                                                     (uint32_t*)&tConnConfig.bssid[3],
                                                     (uint32_t*)&tConnConfig.bssid[4],
                                                     (uint32_t*)&tConnConfig.bssid[5]);
            }

            if(strcmp(argv[3], " ") != 0 && strlen(argv[3]) != 0)
            {
                memcpy(tConnConfig.password, argv[3], strlen(argv[3]));
                tConnConfig.password_length = strlen(argv[3]);
            }

            WM_AT_LOG("Conn request {ssid %s(%d), bssid %x:%x:%x:%x:%x:%x, password %s(%d)}\r\n", tConnConfig.ssid,
                                                                                                       tConnConfig.ssid_length,
                                                                                                       tConnConfig.bssid[0],
                                                                                                       tConnConfig.bssid[1],
                                                                                                       tConnConfig.bssid[2],
                                                                                                       tConnConfig.bssid[3],
                                                                                                       tConnConfig.bssid[4],
                                                                                                       tConnConfig.bssid[5],
                                                                                                       tConnConfig.password,
                                                                                                       tConnConfig.password_length);
            
            if(OPL_OK != Opl_Wifi_Conn_Req(&tConnConfig, AT_CmdWmConnIndCb))
            {
                WM_AT_LOG("Dedicate conn request fail\r\n");
            }

            iRet = 1;

            break;
        }

        case AT_CMD_MODE_EXECUTION:
        {
            WM_AT_LOG("Conn request {NULL}\r\n");

            if(OPL_OK != Opl_Wifi_Conn_Req(NULL, AT_CmdWmConnIndCb))
            {
                WM_AT_LOG("Fast conn request fail\r\n");
            }

            iRet = 1;
        
            break;
        }

        case AT_CMD_MODE_READ:
        {
            WM_AT_LOG("Get stored auto-conn list num\r\n");

            uint8_t u8AutoConnListNum = 0;
            WM_WaAutoConnectListNumGet(&u8AutoConnListNum);

            WM_AT_LOG("Auto Connect Get Saved AP Num %d\r\n", u8AutoConnListNum);

            iRet = 1;

            break;
        }
    }

done:
    if (iRet)
    {
        WM_AT_LOG("OK\r\n");
    }
    else
    {
        WM_AT_LOG("ERROR\r\n");
    }

#else
    WM_AT_LOG("WM at cmd not support\r\n");
#endif /* WM_AT_ENABLED */

    return iRet;
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
int AT_CmdWmDisconnectHandler(char *buf, int len, int mode)
{
    int iRet = 0;

#if (WM_AT_ENABLED == 1)

    switch (mode)
    {
        case AT_CMD_MODE_EXECUTION:
        {
            WM_AT_LOG("Disconn request\r\n");

            if(OPL_OK != Opl_Wifi_Disc_Req(AT_CmdWmDisconnIndCb))
            {
                WM_AT_LOG("Disconn request fail\r\n");
            }

            iRet = 1;
        
            break;
        }
    }

    if (iRet)
    {
        WM_AT_LOG("OK\r\n");
    }
    else
    {
        WM_AT_LOG("ERROR\r\n");
    }

#else
    WM_AT_LOG("WM at cmd not support\r\n");
#endif /* WM_AT_ENABLED */

    return iRet;
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
int AT_CmdApProfileHandler(char *buf, int len, int mode)
{
    int iRet = 0;

#if (WM_AT_ENABLED == 1)

    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    T_PrApProfile *ptProList;
    uint32_t u32ProfileCnt = 0;

    switch (mode)
    {
        case AT_CMD_MODE_READ:
        {
            ptProList = Opl_Wifi_Profile_Get();
            u32ProfileCnt = WM_PrProfileCount();

            WM_AT_LOG("Profile count %d\r\n", u32ProfileCnt);
            
            uint8_t i = 0;
            for (; i < PR_AP_PROFILE_MAX_NUM; i++)
            {
                if(ptProList[i].u8Used)
                {
                    WM_AT_LOG("APInfo[%d]=%s, %s, s[%d], u[%d]\n", i,
                                                                   ptProList[i].u8Ssid,
                                                                   ptProList[i].u8Pwd,
                                                                   ptProList[i].u8SeqNo,
                                                                   ptProList[i].u8Used);
                }
            }

            iRet = 1;

            break;
        }
        case AT_CMD_MODE_EXECUTION:
        {
            WM_AT_LOG("Profile clear\r\n");

            Opl_Wifi_Profile_Clear();

            iRet = 1;

            break;
        }
        case AT_CMD_MODE_SET:
        {
            WM_AT_LOG("Profile insert\r\n");

            if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                goto done;
            }

            if (argc > 4)
            {
                WM_AT_LOG("invalid param number\r\n");
                goto done;
            }

            T_PrApProfile tApProfile = {0};
            memset(&tApProfile, 0, sizeof(T_PrApProfile));

            if(strcmp(argv[1], " ") != 0 && strlen(argv[1]) != 0)
            {    
                memcpy(tApProfile.u8Ssid, argv[1], strlen(argv[1]));
            }

            if(strcmp(argv[2], " ") != 0 && strlen(argv[2]) != 0)
            {
                memcpy(tApProfile.u8Pwd, argv[2], strlen(argv[2]));
            }

            WM_AT_LOG("ApProfile {ssid %s, pwd %s}\r\n", tApProfile.u8Ssid,
                                                         tApProfile.u8Pwd);

            Opl_Wifi_Profile_Ins(tApProfile);

            iRet = 1;

            break;
        }
    }

done:
    if (iRet)
    {
        WM_AT_LOG("OK\r\n");
    }
    else
    {
        WM_AT_LOG("ERROR\r\n");
    }

#else
    WM_AT_LOG("WM at cmd not support\r\n");
#endif /* WM_AT_ENABLED */

    return iRet;
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
int AT_CmdWmMngrStatusChkHandler(char *buf, int len, int mode)
{
    int iRet = 0;

#if (WM_AT_ENABLED == 1)

    switch (mode)
    {
        case AT_CMD_MODE_READ:
        {
            T_FsmDef *tWaFsmDef = WM_WaFsmDefGet();
            WM_AT_LOG("WA (C %d; H %d)\r\n", tWaFsmDef->ptFsmStateInfo.tCurrentState,
                                             tWaFsmDef->ptFsmStateInfo.tHistoryState);

            iRet = 1;

            break;
        }
    }

    if (iRet)
    {
        WM_AT_LOG("OK\r\n");
    }
    else
    {
        WM_AT_LOG("ERROR\r\n");
    }

#else
    WM_AT_LOG("WM at cmd not support\r\n");
#endif /* WM_AT_ENABLED */

    return iRet;
}

#endif /* WM_ENABLED */
