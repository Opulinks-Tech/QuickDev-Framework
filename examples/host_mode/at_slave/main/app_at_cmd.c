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
*  app_at_cmd.c
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

#include "app_at_cmd.h"
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
#include "agent.h"
#endif
#include "cloud_ctrl.h"
#include "cloud_config.h"
#include "cloud_kernel.h"
#include "mw_fim.h"
#include "mw_fim_default_group12_project.h"
#include "net_mngr_api.h"
#include "opl_err.h"
#include "qd_fwk_ver.h"
#include "wifi_mngr_api.h"

#if (WM_ENABLED == 1 && WM_AT_ENABLED == 1)
#include "wifi_mngr_at.h"
#endif

#if (NM_ENABLED == 1 && NM_AT_ENABLED == 1)
#include "net_mngr_at.h"
#endif

#if (WM_AC_ENABLED == 1 && WM_AC_AT_ENABLED == 1)
#include "wifi_autocon_at.h"
#endif

#if (BM_ENABLED == 1 && BM_AT_ENABLED == 1)
#include "ble_mngr_at.h"
#endif


// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#define AT_LOG                                          msg_print_uart1

// fot AT_CmdFimWriteHandler
#define AT_FIM_DATA_LENGTH                              (2)                       /* EX: 2 = FF */
#define AT_FIM_DATA_LENGTH_WITH_COMMA                   (AT_FIM_DATA_LENGTH + 1)  /* EX: 3 = FF, */

#define AT_RETURN(result)                               {                                               \
                                                            if(1 == result)                             \
                                                            {                                           \
                                                                AT_LOG("OK\r\n");                       \
                                                            }                                           \
                                                            else                                        \
                                                            {                                           \
                                                                AT_LOG("ERROR\r\n");                    \
                                                            }                                           \
                                                                                                        \
                                                            return result;                         \
                                                        }                                               

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

// application AT cmd list
at_command_t g_taAppAtCmd[] =
{
    // user implement
    // *

#if 0
    {"at+writefim",        AT_CmdFimReadHandler,                "Write FIM data" },
    {"at+readfim",         AT_CmdFimWriteHandler,               "Read FIM data" },
#endif
    {"at+ckcfg",           AT_CmdSysCkCfgHandler,               "CoolKit Configuration"},

    // request cmd list
    {"at+fwkver",          AT_CmdFwkVerHandler,                 "Get QD_FWK version"},
    {"at+entsleep",        AT_CmdEntSleepHandler,               "Enter sleep mode"},
    {"at+provisionstart",  AT_CmdProvisionStartHandler,         "Start provision progress"},
    {"at+provisionstop",   AT_CmdProvisionStopHandler,          "Stop provision progress"},
    {"at+blests",          AT_CmdBleStatusHandler,              "Get ble status"},

    {"at+wifiscanap",      AT_CmdWifiScanHandler,               "WIFI scan request"},
    {"at+wificonnectap",   AT_CmdWifiConnectHandler,            "WIFI connect request"},
    {"at+wifiqconnectap",  AT_CmdWifiQConnectHandler,           "WIFI quick connect"},
    {"at+wifists",         AT_CmdWifiStatusHandler,             "Get wifi status"},
    
    {"at+cloudconn",       AT_CmdCloudConnectHandler,           "Cloud connect request"},
    {"at+clouddisc",       AT_CmdCloudDisconnectHandler,        "Cloud disconnect request"},
    {"at+cloudtxtopic",    AT_CmdCloudTxTopicHandler,           "Cloud register tx topic"},
    {"at+cloudrxtopic",    AT_CmdCloudRxTopicHandler,           "Cloud register rx topic"},
    {"at+cloudtxpost",     AT_CmdCloudTxPostHandler,            "Cloud send message"},

    // *

    // wifi manager - wifi_mngr_at.h
#if (WM_ENABLED == 1 && WM_AT_ENABLED == 1)
    {"at+scan",            AT_CmdWmScanHandler,                 "Send scan request to wifi agent"},
    {"at+connect",         AT_CmdWmConnectHandler,              "Send connect request to wifi agent"},
    {"at+disconnect",      AT_CmdWmDisconnectHandler,           "Send disconnect request to wifi agent"},
    {"at+profile",         AT_CmdApProfileHandler,              "Access profile recorder"},
    {"at+wmsta",           AT_CmdWmMngrStatusChkHandler,        "Check WI-FI status(WA)"},
#endif

    // network manager - net_mngr_at.h
#if (NM_ENABLED == 1 && NM_AT_ENABLED == 1)
    {"at+nmscan",          AT_CmdNmScanHandler,                 "Send scan request to network manager"},
    {"at+nmconnect",       AT_CmdNmConnectHandler,              "Send connect request to network manager"},
    {"at+nmqset",          AT_CmdNmQuickConnectSetHandler,      "Send quick connect set request to network manager"},
#endif

    // wifi auto-connect - wifi_autocon_at.h
#if (WM_AC_ENABLED == 1 && WM_AC_AT_ENABLED == 1)
    {"at+acswitch",        AT_CmdWmAcSwitchHandler,             "Switch Auto-Connect procedure"},
    {"at+acsta",           AT_CmdWmAcStatusChkHandler,          "Check WI-FI status(AC)"},
#endif

    // ble manager - ble_mngr_at.h
#if (BM_ENABLED == 1 && BM_AT_ENABLED == 1)
    {"at+bleswitch",       AT_CmdBmAdvSwitchHandler,            "BLE switch start / stop"},
#endif

    {NULL,                 NULL,                                NULL},
};

#if 0
typedef struct
{
    uint32_t u32Id;
    uint16_t u16Index;
    uint16_t u16DataTotalLen;

    uint32_t u32DataRecv;       // Calcuate the receive data
    uint32_t TotalSize;         // user need to input total bytes

    char     u8aReadBuf[8];
    uint8_t  *ResultBuf;
    uint32_t u32StringIndex;       // Indicate the location of reading string
    uint16_t u16Resultindex;       // Indicate the location of result string
    uint8_t  fIgnoreRestString;    // Set a flag for last one comma
    uint8_t  u8aTemp[1];
} T_AtFimParam;
#endif

typedef struct S_ATCmdCloudTxPostPayload
{
    uint8_t u8TopicIndex;
    uint16_t u16RecvIndex;
    uint32_t u32RecvLens;
    uint32_t u32DataLens;
    uint8_t *pu8DataBuf;
} T_ATCmdCloudTxPostPayload;


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

/// fim control

#if 0
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
int AT_CmdFimWrite(uint32_t u32Type, uint8_t *u8aData, uint32_t u32DataLen, void *pParam)
{
    T_AtFimParam *ptParam = (T_AtFimParam *)pParam;

    uint8_t iRet = 0;
    uint8_t u8acmp[] = ",\0";
    uint32_t i = 0;

    ptParam->u32DataRecv += u32DataLen;

    /* If previous segment is error then ignore the rest of segment */
    if (ptParam->fIgnoreRestString)
    {
        goto done;
    }

    for (i=0 ; i<u32DataLen; i++)
    {
        if (u8aData[i] != u8acmp[0])
        {
            if (ptParam->u32StringIndex >= AT_FIM_DATA_LENGTH)
            {
                ptParam->fIgnoreRestString = 1;
                goto done;
            }

            /* compare string. If not comma then store into array. */
            ptParam->u8aReadBuf[ptParam->u32StringIndex] = u8aData[i];
            ptParam->u32StringIndex++;
        }
        else
        {
            /* Convert string into Hex and store into array */
            ptParam->ResultBuf[ptParam->u16Resultindex] = (uint8_t)strtoul(ptParam->u8aReadBuf, NULL, 16);

            /* Result index add one */
            ptParam->u16Resultindex++;

            /* re-count when encounter comma */
            ptParam->u32StringIndex=0;
        }
    }

    /* If encounter the last one comma
       1. AT_FIM_DATA_LENGTH:
       Max character will pick up to compare.

       2. (ptParam->u16DataTotalLen - 1):
       If total length minus 1 is equal (ptParam->u16Resultindex) mean there is no comma at the rest of string.
    */
    if ((ptParam->u16Resultindex == (ptParam->u16DataTotalLen - 1)) && (ptParam->u32StringIndex >= AT_FIM_DATA_LENGTH))
    {
        ptParam->ResultBuf[ptParam->u16Resultindex] = (uint8_t)strtoul(ptParam->u8aReadBuf, NULL, 16);

        /* Result index add one */
        ptParam->u16Resultindex++;
    }

    /* Collect array data is equal to total lengh then write data to fim. */
    if (ptParam->u16Resultindex == ptParam->u16DataTotalLen)
    {
       	if (MW_FIM_OK == MwFim_FileWrite(ptParam->u32Id, ptParam->u16Index, ptParam->u16DataTotalLen, ptParam->ResultBuf))
        {
            msg_print_uart1("OK\r\n");
        }
        else
        {
            ptParam->fIgnoreRestString = 1;
        }
    }
    else
    {
        goto done;
    }

done:
    if (ptParam->TotalSize >= ptParam->u32DataRecv)
    {
        if (ptParam->fIgnoreRestString)
        {
            msg_print_uart1("ERROR\r\n");
        }

        if (ptParam != NULL)
        {
            if (ptParam->ResultBuf != NULL)
            {
                free(ptParam->ResultBuf);
            }
            free(ptParam);
            ptParam = NULL;
        }
    }

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
int AT_CmdFimWriteHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    /* Initialization the value */
    T_AtFimParam *tAtFimParam = (T_AtFimParam*)malloc(sizeof(T_AtFimParam));
    if (tAtFimParam == NULL)
    {
        goto done;
    }
    memset(tAtFimParam, 0, sizeof(T_AtFimParam));

    if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
    {
        goto done;
    }

    if (argc != 4)
    {
        msg_print_uart1("invalid param number\r\n");
        goto done;
    }

    /* save parameters to process uart1 input */
    tAtFimParam->u32Id = (uint32_t)strtoul(argv[1], NULL, 16);
    tAtFimParam->u16Index = (uint16_t)strtoul(argv[2], NULL, 0);
    tAtFimParam->u16DataTotalLen = (uint16_t)strtoul(argv[3], NULL, 0);

    /* If user input data length is 0 then go to error.*/
    if (tAtFimParam->u16DataTotalLen == 0)
    {
        goto done;
    }

    switch (mode)
    {
        case AT_CMD_MODE_SET:
        {
            tAtFimParam->TotalSize = ((tAtFimParam->u16DataTotalLen * AT_FIM_DATA_LENGTH_WITH_COMMA) - 1);

            /* Memory allocate a memory block for pointer */
            tAtFimParam->ResultBuf = (uint8_t *)malloc(tAtFimParam->u16DataTotalLen);
            if (tAtFimParam->ResultBuf == NULL)
                goto done;

            // register callback to process uart1 input
            agent_data_handle_reg(AT_CmdFimWrite, tAtFimParam);

            // redirect uart1 input to callback
#if defined(OPL1000_A2) || defined(OPL1000_A3)
            data_process_lock_patch(LOCK_APP, (tAtFimParam->TotalSize));
#elif defined(OPL2500_A0)
            data_process_lock(LOCK_APP, (tAtFimParam->TotalSize));
#endif

            break;
        }

        default:
            goto done;
    }

    iRet = 1;

done:
    if (iRet)
    {
        msg_print_uart1("OK\r\n");
    }
    else
    {
        msg_print_uart1("ERROR\r\n");
        if (tAtFimParam != NULL)
        {
            if (tAtFimParam->ResultBuf != NULL)
            {
		        free(tAtFimParam->ResultBuf);
            }
            free(tAtFimParam);
            tAtFimParam = NULL;
        }
    }

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
int AT_CmdFimReadHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};
    uint32_t i = 0;
    uint8_t *readBuf = NULL;
    uint32_t u32Id  = 0;
    uint16_t u16Index  = 0;
    uint16_t u16Size  = 0;

    if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
    {
        goto done;
    }

    if (argc != 4)
    {
        AT_LOG("invalid param number\r\n");
        goto done;
    }

    u32Id = (uint32_t)strtoul(argv[1], NULL, 16);
    u16Index = (uint16_t)strtoul(argv[2], NULL, 0);
    u16Size = (uint16_t)strtoul(argv[3], NULL, 0);

    if (u16Size == 0)
    {
        AT_LOG("invalid size[%d]\r\n", u16Size);
        goto done;
    }

    switch (mode)
    {
        case AT_CMD_MODE_SET:
        {
            readBuf = (uint8_t *)malloc(u16Size);
            if (NULL == readBuf)
            {
                AT_LOG("malloc fail\r\n");
                goto done;
            }

            if (MW_FIM_OK == MwFim_FileRead(u32Id, u16Index, u16Size, readBuf))
            {
                msg_print_uart1("%02X", readBuf[0]);
                for (i=1 ; i<u16Size; i++)
                {
                    msg_print_uart1(",%02X", readBuf[i]);
                }
            }
            else
            {
                goto done;
            }

            msg_print_uart1("\r\n");
            break;
        }

        default:
            goto done;
    }

    iRet = 1;

done:
    if (iRet)
    {
        msg_print_uart1("OK\r\n");
    }
    else
    {
        msg_print_uart1("ERROR\r\n");
    }

    if (readBuf != NULL)
        free(readBuf);

    return iRet;
}
#endif

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
int AT_CmdSysCkCfgHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    T_MwFim_GP12_HttpPostContent tDev = {0};

    switch (mode)
    {
        case AT_CMD_MODE_READ:
        {
            if(MwFim_FileRead(MW_FIM_IDX_GP12_PROJECT_DEVICE_AUTH_CONTENT, 0, MW_FIM_GP12_HTTP_POST_CONTENT_SIZE, (uint8_t*)&tDev) != MW_FIM_OK)
            {
                AT_LOG("Read fail\r\n");
                goto done;
            }

            AT_LOG("Apikey:  \t%s\r\n", tDev.ubaApiKey);
            AT_LOG("DeviceID:\t%s\r\n", tDev.ubaDeviceId);
            AT_LOG("ChipID:  \t%s\r\n", tDev.ubaChipId);
            AT_LOG("ModelID: \t%s\r\n", tDev.ubaModelId);
            break;
        }

        case AT_CMD_MODE_SET:
        {
            uint32_t u32APIKeySize = 0;
            uint32_t u32DeviceIDSize = 0;
            uint32_t u32ChipIDSize = 0;
            uint32_t u32ModelIDSize = 0;

            if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                AT_LOG("at_cmd_buf_to_argc_argv\n");
                goto done;
            }

            if(argc < 3)
            {
                goto done;
            }

            u32APIKeySize = sizeof(tDev.ubaApiKey);

            if(strlen(argv[1]) >= u32APIKeySize)
            {
                AT_LOG("Invalid APIKey Length\r\n");
                goto done;
            }

            u32DeviceIDSize = sizeof(tDev.ubaDeviceId);

            if(strlen(argv[2]) >= u32DeviceIDSize)
            {
                AT_LOG("Invalid ProductSecret Length\r\n");
                goto done;
            }

            u32ChipIDSize = sizeof(tDev.ubaChipId);

            if(strlen(argv[3]) >= u32ChipIDSize)
            {
                AT_LOG("Invalid DeviceName Length\r\n");
                goto done;
            }

            u32ModelIDSize = sizeof(tDev.ubaModelId);

            if(strlen(argv[4]) >= u32ModelIDSize)
            {
                AT_LOG("Invalid DeviceSecret Length\r\n");
                goto done;
            }

            strcpy(tDev.ubaApiKey, argv[1]);
            strcpy(tDev.ubaDeviceId, argv[2]);
            strcpy(tDev.ubaChipId, argv[3]);
            strcpy(tDev.ubaModelId, argv[4]);

            if(MwFim_FileWrite(MW_FIM_IDX_GP12_PROJECT_DEVICE_AUTH_CONTENT, 0, MW_FIM_GP12_HTTP_POST_CONTENT_SIZE, (uint8_t*)&tDev) != MW_FIM_OK)
            {
                AT_LOG("Write fail\r\n");
                goto done;
            }

            break;
        }

        default:
            goto done;
    }

    iRet = 1;

done:
    if(iRet)
    {
        AT_LOG("OK\r\n");
    }
    else
    {
        AT_LOG("ERROR\r\n");
    }

    return iRet;
}

int AT_CmdFwkVerHandler(char *buf, int len, int mode)
{
    int iRet = 0;

    if(AT_CMD_MODE_EXECUTION == mode)
    {
        AT_LOG("State: %s\r\n", QD_FWK_RELEASE_STATE);
        AT_LOG("Version: %s\r\n", QD_FWK_RELEASE_VER);
        AT_LOG("Date: %s\r\n", QD_FWK_RELEASE_DATE);
        iRet = 1;
    }

    AT_RETURN(iRet);
}

int AT_CmdEntSleepHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    if(AT_CMD_MODE_SET == mode)
    {
        if(!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
        {
            goto done;
        }

        if(argc < 2)
        {
            goto done;
        }

        uint8_t u8SleepMode = (uint8_t)strtoul(argv[1], NULL, 0);

        APP_SendMessage(APP_EVT_ENT_SLEEP_REQ, (uint8_t *)&u8SleepMode, sizeof(uint8_t));

        iRet = 1;
    }

done:
    AT_RETURN(iRet);
}

int AT_CmdProvisionStartHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    if(AT_CMD_MODE_SET == mode)
    {
        if(!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
        {
            goto done;
        }

        if(argc < 2)
        {
            goto done;
        }

        uint32_t u32OplPrvTimeout = (uint32_t)strtoul(argv[1], NULL, 0);

        APP_SendMessage(APP_EVT_OPL_PRV_START_REQ, (uint8_t *)&u32OplPrvTimeout, sizeof(uint32_t));

        iRet = 1;
    }

done:
    AT_RETURN(iRet);
}

int AT_CmdProvisionStopHandler(char *buf, int len, int mode)
{
    int iRet = 0;

    if(AT_CMD_MODE_EXECUTION == mode)
    {
        APP_SendMessage(APP_EVT_OPL_PRV_STOP_REQ, NULL, 0);

        iRet = 1;
    }

    AT_RETURN(iRet);
}

int AT_CmdBleStatusHandler(char *buf, int len, int mode)
{
    int iRet = 0;

    if(AT_CMD_MODE_EXECUTION == mode)
    {
        APP_SendMessage(APP_EVT_BLE_STATUS_REQ, NULL, 0);

        iRet = 1;
    }

    AT_RETURN(iRet);
}

int AT_CmdWifiScanHandler(char *buf, int len, int mode)
{
    int iRet = 0;

    if(AT_CMD_MODE_EXECUTION == mode)
    {
        APP_SendMessage(APP_EVT_WIFI_SCAN_REQ, NULL, 0);

        iRet = 1;
    }

    AT_RETURN(iRet);
}

int AT_CmdWifiConnectHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    if(AT_CMD_MODE_SET == mode)
    {
        T_NmWifiCnctConfig tNmCnctConf = {0};
        memset(&tNmCnctConf, 0, sizeof(T_NmWifiCnctConfig));

        if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
        {
            goto done;
        }

        if (argc > 4)
        {
            goto done;
        }

        if(strcmp(argv[1], " ") != 0 && strlen(argv[1]) != 0)
        {
            // parse each mac number in to 6 byte array
            // sscanf(argv[1], "%x:%x:%x:%x:%x:%x", (uint32_t*)&tNmCnctConf.u8aBssid[0],
            //                                         (uint32_t*)&tNmCnctConf.u8aBssid[1],
            //                                         (uint32_t*)&tNmCnctConf.u8aBssid[2],
            //                                         (uint32_t*)&tNmCnctConf.u8aBssid[3],
            //                                         (uint32_t*)&tNmCnctConf.u8aBssid[4],
            //                                         (uint32_t*)&tNmCnctConf.u8aBssid[5]);
            memcpy(tNmCnctConf.u8aSsid, argv[1], strlen(argv[1]));
            tNmCnctConf.u8SsidLen = strlen(argv[1]);
        }

        if(strcmp(argv[2], " ") != 0 && strlen(argv[2]) != 0)
        {
            memcpy(tNmCnctConf.u8aPwd, argv[2], strlen(argv[2]));
            tNmCnctConf.u8PwdLen = strlen(argv[2]);
        }
        
        APP_SendMessage(APP_EVT_WIFI_CONN_REQ, (uint8_t *)&tNmCnctConf, sizeof(T_NmWifiCnctConfig));

        iRet = 1;
    }

done:
    AT_RETURN(iRet);
}

int AT_CmdWifiQConnectHandler(char *buf, int len, int mode)
{
int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    if(AT_CMD_MODE_SET == mode)
    {
        T_NmWifiQCnctSet tNmWifiQCnctConf = {0};
        memset(&tNmWifiQCnctConf, 0, sizeof(T_NmWifiQCnctSet));

        if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
        {
            goto done;
        }

        if (argc > 4)
        {
            goto done;
        }

        if(strcmp(argv[1], " ") != 0 && strlen(argv[1]) != 0)
        {
            memcpy(tNmWifiQCnctConf.u8aSsid, argv[1], strlen(argv[1]));
            tNmWifiQCnctConf.u8SsidLen = strlen(argv[1]);
        }

        if(strcmp(argv[2], " ") != 0 && strlen(argv[2]) != 0)
        {
            memcpy(tNmWifiQCnctConf.u8aPwd, argv[2], strlen(argv[2]));
            tNmWifiQCnctConf.u8PwdLen = strlen(argv[2]);
        }
        
        APP_SendMessage(APP_EVT_WIFI_QCONN_REQ, (uint8_t *)&tNmWifiQCnctConf, sizeof(T_NmWifiQCnctSet));

        iRet = 1;
    }

done:
    AT_RETURN(iRet);
}

int AT_CmdWifiStatusHandler(char *buf, int len, int mode)
{
    int iRet = 0;

    if(AT_CMD_MODE_EXECUTION == mode)
    {
        APP_SendMessage(APP_EVT_WIFI_STATUS_REQ, NULL, 0);

        iRet = 1;
    }

    AT_RETURN(iRet);
}

int AT_CmdCloudConnectHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    if(AT_CMD_MODE_SET == mode)
    {
        if(!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
        {
            goto done;
        }

        if(argc < 2)
        {
            goto done;
        }

        T_CloudConnInfo tCloudConnInfo = {0};

        tCloudConnInfo.u8AutoConn = (uint8_t)strtoul(argv[1], NULL, 0);

        if(argc >= 4)
        {
            tCloudConnInfo.u16HostPort = (uint16_t)strtoul(argv[3], NULL, 0);
            memcpy(tCloudConnInfo.u8aHostAddr, argv[2], strlen(argv[2]));
        }

        // if have security value
        if(argc == 5)
        {
            tCloudConnInfo.u8Security = (uint8_t)strtoul(argv[4], NULL, 0);
        }
        else
        {
            // default using MBEDTLS_SSL_VERIFY_NONE type
            tCloudConnInfo.u8Security = 0;
        }
        
        APP_SendMessage(APP_EVT_CLOUD_CONNECT_REQ, (uint8_t *)&tCloudConnInfo, sizeof(T_CloudConnInfo));

        iRet = 1;
    }

done:
    AT_RETURN(iRet);
}

int AT_CmdCloudDisconnectHandler(char *buf, int len, int mode)
{
    int iRet = 0;

    if(AT_CMD_MODE_EXECUTION == mode)
    {
        APP_SendMessage(APP_EVT_CLOUD_DISCONNECT_REQ, NULL, 0);

        iRet = 1;
    }

    AT_RETURN(iRet);
}

int AT_CmdCloudTxTopicHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    if(AT_CMD_MODE_SET == mode)
    {
        if(!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
        {
            goto done;
        }

        if(argc < 3)
        {
            goto done;
        }

        T_CloudTopicRegInfo tCloudTopicRegInfo = {0};

        tCloudTopicRegInfo.u8TopicIndex = (uint8_t)strtoul(argv[1], NULL, 0);
        memcpy(tCloudTopicRegInfo.u8aTopicName, argv[2], strlen(argv[2]));
        
        if(OPL_OK == Cloud_TxTopicRegisterDyn(&tCloudTopicRegInfo))
        {
            iRet = 1;
        }
    }
    else if(AT_CMD_MODE_READ == mode)
    {
        T_CloudTopicRegInfo *tCloudTopicRegInfo;

        tCloudTopicRegInfo = Cloud_TxTopicGet();

        for(uint8_t u8Count; u8Count < 8; u8Count ++)
        {
            if(true == tCloudTopicRegInfo[u8Count].u8IsTopicRegisted)
            {
                char cTopicInfo[CLOUD_TOPIC_NAME_LEN + 2];
                uint32_t u32TopicInfoStrLen = 0;
                u32TopicInfoStrLen += sprintf(cTopicInfo, "%d,%s", u8Count, tCloudTopicRegInfo[u8Count].u8aTopicName);
                APP_HostModeResponseAck(AT_CMD_ACK_CLOUD_TX_TOPIC, (uint8_t *)cTopicInfo, u32TopicInfoStrLen);
            }
        }

        iRet = 1;
    }

done:
    AT_RETURN(iRet);
}

// TODO: move the callback to cloud_ctrl to handle
#if defined(HOST_MODE_MQTT)
void MQTT_RxTopicCallback(AWS_IoT_Client *pClient, char *pTopicName, uint16_t topicNameLen,
						  IoT_Publish_Message_Params *pParams, void *pClientData)
{
    OPL_LOG_INFO(AT, "sub: %.*s\t%.*s", topicNameLen, pTopicName, (int)pParams->payloadLen, (char *)pParams->payload);

    static char cRecvData[CLOUD_PAYLOAD_LEN];
    uint32_t u32RecvDataStrLen = 0;
    memset(cRecvData, 0, sizeof(cRecvData));

    u32RecvDataStrLen += sprintf(cRecvData, "%d:%s", (int)pParams->payloadLen, (char *)pParams->payload);
    APP_HostModeResponseAck(AT_CMD_ACK_CLOUD_RX_RECV, (uint8_t *)cRecvData, u32RecvDataStrLen);
}
#endif

int AT_CmdCloudRxTopicHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    if(AT_CMD_MODE_SET == mode)
    {
        if(!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
        {
            goto done;
        }

        if(argc < 3)
        {
            goto done;
        }

        T_CloudTopicRegInfo tCloudTopicRegInfo = {0};

        tCloudTopicRegInfo.u8TopicIndex = (uint8_t)strtoul(argv[1], NULL, 0);
        memcpy(tCloudTopicRegInfo.u8aTopicName, argv[2], strlen(argv[2]));

#if defined(HOST_MODE_TCP)
        if(OPL_OK == Cloud_RxTopicRegisterDyn(&tCloudTopicRegInfo))
        {
            iRet = 1;
        }
#elif defined(HOST_MODE_MQTT)
        // in mqtt protocol will need to carried the receive callback function
        tCloudTopicRegInfo.fpFunc = MQTT_RxTopicCallback;

        if(OPL_OK == Cloud_RxTopicRegisterDyn(&tCloudTopicRegInfo))
        {
            iRet = 1;
        }
#endif
    }
    else if(AT_CMD_MODE_READ == mode)
    {
        T_CloudTopicRegInfo *tCloudTopicRegInfo;

        tCloudTopicRegInfo = Cloud_RxTopicGet();

        for(uint8_t u8Count; u8Count < 8; u8Count ++)
        {
            if(true == tCloudTopicRegInfo[u8Count].u8IsTopicRegisted)
            {
                char cTopicInfo[CLOUD_TOPIC_NAME_LEN + 2];
                uint32_t u32TopicInfoStrLen = 0;
                u32TopicInfoStrLen += sprintf(cTopicInfo, "%d,%s", u8Count, tCloudTopicRegInfo[u8Count].u8aTopicName);
                APP_HostModeResponseAck(AT_CMD_ACK_CLOUD_RX_TOPIC, (uint8_t *)cTopicInfo, u32TopicInfoStrLen);
            }
        }

        iRet = 1;
    }

done:
    AT_RETURN(iRet);
}

int AT_CmdCloudTxPostPayloadRecvCB(uint32_t u32Type, uint8_t *u8aData, uint32_t u32DataLen, void *pParam)
{
    T_ATCmdCloudTxPostPayload *ptATCmdCloudTxPostPayload = (T_ATCmdCloudTxPostPayload *)pParam;

    uint8_t iRet = 0;
    uint32_t i = 0;

    for(; i < u32DataLen; i ++)
    {
        ptATCmdCloudTxPostPayload->pu8DataBuf[ptATCmdCloudTxPostPayload->u32RecvLens + i] = u8aData[i];
    }

    ptATCmdCloudTxPostPayload->u32RecvLens += u32DataLen;

    if(ptATCmdCloudTxPostPayload->u32RecvLens == ptATCmdCloudTxPostPayload->u32DataLens)
    {
        T_CloudPayloadFmt tCloudPayloadFmt = {0};

        tCloudPayloadFmt.u8TopicIndex = ptATCmdCloudTxPostPayload->u8TopicIndex;
        tCloudPayloadFmt.u32PayloadLen = ptATCmdCloudTxPostPayload->u32DataLens;
        memcpy(tCloudPayloadFmt.u8aPayloadBuf, ptATCmdCloudTxPostPayload->pu8DataBuf, ptATCmdCloudTxPostPayload->u32DataLens);

        //send recv data as payload and send to cloud
        Cloud_MsgSend(CLOUD_EVT_TYPE_POST, (uint8_t *)&tCloudPayloadFmt, sizeof(T_CloudPayloadFmt));

        if (ptATCmdCloudTxPostPayload != NULL)
        {
            if (ptATCmdCloudTxPostPayload->pu8DataBuf != NULL)
            {
                free(ptATCmdCloudTxPostPayload->pu8DataBuf);
            }
            free(ptATCmdCloudTxPostPayload);
            ptATCmdCloudTxPostPayload = NULL;
        }

        AT_LOG("+TXRSP:%d,%d:%s\r\n", tCloudPayloadFmt.u8TopicIndex, tCloudPayloadFmt.u32PayloadLen, tCloudPayloadFmt.u8aPayloadBuf);

        // char cTxPayload[CLOUD_PAYLOAD_LEN + 5];
        // sprintf(cTxPayload, "%d:%s", tCloudPayloadFmt.u32PayloadLen, tCloudPayloadFmt.u8aPayloadBuf);
        // APP_HostModeResponseAck(AT_CMD_ACK_CLOUD_TX_POST, (uint8_t *)cTxPayload);
    }

    return iRet;
}

int AT_CmdCloudTxPostHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    if(AT_CMD_MODE_SET == mode)
    {
        if(!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
        {
            goto done;
        }

        if(argc < 3)
        {
            goto done;
        }

        T_ATCmdCloudTxPostPayload *tATCmdCloudTxPostPayload = (T_ATCmdCloudTxPostPayload *)malloc(sizeof(T_ATCmdCloudTxPostPayload));
        if(tATCmdCloudTxPostPayload == NULL)
        {
            goto done;
        }

        tATCmdCloudTxPostPayload->u8TopicIndex = (uint8_t)strtoul(argv[1], NULL, 0);
        tATCmdCloudTxPostPayload->u32DataLens = (uint32_t)strtoul(argv[2], NULL, 0);
        tATCmdCloudTxPostPayload->u32RecvLens = 0;

        /* If user input data length is 0 then go to error.*/
        if (tATCmdCloudTxPostPayload->u32DataLens == 0)
        {
            free(tATCmdCloudTxPostPayload);
            goto done;
        }

        tATCmdCloudTxPostPayload->pu8DataBuf = (uint8_t *)malloc(tATCmdCloudTxPostPayload->u32DataLens);
        if(tATCmdCloudTxPostPayload->pu8DataBuf == NULL)
        {
            free(tATCmdCloudTxPostPayload);
            goto done;
        }

        // register callback to process uart1 input
        agent_data_handle_reg(AT_CmdCloudTxPostPayloadRecvCB, tATCmdCloudTxPostPayload);

        // redirect uart1 input to callback
#if defined(OPL1000_A2) || defined(OPL1000_A3)
        data_process_lock_patch(LOCK_APP, (tATCmdCloudTxPostPayload->u32DataLens));
#elif defined(OPL2500_A0)
        data_process_lock(LOCK_APP, (tATCmdCloudTxPostPayload->u32DataLens));
#endif

        iRet = 1;
    }

done:
    AT_RETURN(iRet);
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
void AT_CmdListAdd(uint8_t u8EnableCrLf)
{
    at_cmd_ext_tbl_reg(g_taAppAtCmd);

    at_cmd_crlf_term_set(u8EnableCrLf);

    set_echo_on(false);

#if (WM_ENABLED == 1 && WM_AT_ENABLED == 1)
    Opl_Wifi_Uslctd_CB_Reg(AT_UnsolicitedCallback);
#endif

}
