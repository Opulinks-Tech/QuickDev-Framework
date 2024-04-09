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
#include "mw_fim.h"
#include "mw_fim_default_group12_project.h"
#include "opl_err.h"

#if (WM_ENABLED == 1)
#include "wifi_mngr_at.h"
#endif

#if (NM_ENABLED == 1)
#include "net_mngr_at.h"
#endif

#if (WM_AC_ENABLED == 1)
#include "wifi_autocon_at.h"
#endif

#if (BM_ENABLED == 1)
#include "ble_mngr_at.h"
#endif

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#define AT_LOG                                          msg_print_uart1

// fot AT_CmdFimWriteHandler
#define AT_FIM_DATA_LENGTH                              (2)                       /* EX: 2 = FF */
#define AT_FIM_DATA_LENGTH_WITH_COMMA                   (AT_FIM_DATA_LENGTH + 1)  /* EX: 3 = FF, */

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
    {"at+ckcfg",            AT_CmdSysCkCfgHandler,               "CoolKit Configuration"},
    {"at+example",          AT_CmdExampleHandler,                "Host mode example:1 ~ 6"},
    {"at+fulmqttcert",      AT_CmdFulMqttCertHandler,            "Upload MQTT root CA/client cert/priv key file"},
    {"at+mqtturlset",       AT_CmdUrlSetHandler,                 "Mqtt url set"},
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

/*************************************************************************
* FUNCTION:
*   AT_CmdExampleHandler
*
* DESCRIPTION:
*   For host mode example
*
* PARAMETERS
*   1 : Example 1
*   ...
*
* RETURNS
*   none
*
*************************************************************************/
int AT_CmdExampleHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    switch (mode)
    {
        case AT_CMD_MODE_SET:
        {
            if(!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                goto done;
            }

            if(argc < 2)
            {
                goto done;
            }

            uint8_t u8ExMode = (uint8_t)atoi(argv[1]);

            if(OPL_OK != APP_SendMessage(APP_EVT_EXAMPLE_MODE, &u8ExMode, sizeof(u8ExMode)))
            {
                goto done;
            }

            iRet = 1;

            break;
        }            
    }

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

int AT_UploadFileRecvCB(uint32_t u32Type, uint8_t *u8aData, uint32_t u32DataLen, void *pParam)
{
    T_UploadFileStruct *ptUploadFileStruct = (T_UploadFileStruct *)pParam;

    int8_t iRet = 0;
    uint32_t i = 0;

    for(; i < u32DataLen; i ++)
    {
        ptUploadFileStruct->pau8DataBuf[ptUploadFileStruct->u32RecvLen + i] = u8aData[i];
    }

    ptUploadFileStruct->u32RecvLen += u32DataLen;
    //uint16_t len = strlen(ptUploadFileStruct->pau8DataBuf);
    //tracer_drct_printf("CB: %d\n",len);
    
    if(ptUploadFileStruct->u32RecvLen == ptUploadFileStruct->u32DataLen)
    {
        APP_SendMessage(APP_EVT_CLOUD_MQTT_UPLOAD_FILE, (uint8_t *)ptUploadFileStruct, sizeof(T_UploadFileStruct) + ptUploadFileStruct->u32DataLen);
    }
    else
    {
        goto done;
    }

    iRet = 1;
   
done:
    if(iRet == 0)
    {
        AT_LOG("ERROR\r\n");
    }

    return iRet;
}
/*************************************************************************
* FUNCTION:
*   AT_CmdFulMqttCertHandler
*
* DESCRIPTION:
*   Seting Mqtt host domain name
*
* PARAMETERS
*   
*
* RETURNS
*   none
*
*************************************************************************/
int AT_CmdFulMqttCertHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};
    uint32_t u32Datalen = 0;

    switch(mode)
    {
        case AT_CMD_MODE_SET:
        {
            if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                return false;
            }

            if(argc < 3)
            {
                goto done;
            }
            
            u32Datalen = (uint32_t)strtoul(argv[2], NULL, 0);

            if(0 == u32Datalen)
            {
                goto done;
            }
            
            T_UploadFileStruct *tUploadFileStruct = (T_UploadFileStruct *) malloc(sizeof(T_UploadFileStruct) + u32Datalen);

            if(tUploadFileStruct == NULL)
            {
                goto done;
            }
            
            memset(tUploadFileStruct, 0, sizeof(T_UploadFileStruct) + u32Datalen);

            tUploadFileStruct->tFileType = (T_UploadFileType)strtoul(argv[1], NULL, 0);
            tUploadFileStruct->u32DataLen = u32Datalen;
            tUploadFileStruct->u32RecvLen = 0;
            
            // register callback to process uart1 input
            agent_data_handle_reg(AT_UploadFileRecvCB, tUploadFileStruct);

#if defined(OPL1000_A2) || defined(OPL1000_A3)
            data_process_lock_patch(LOCK_APP, (tUploadFileStruct->u32DataLen));
#elif defined(OPL2500_A0)
            data_process_lock(LOCK_APP, (tUploadFileStruct->u32DataLen));
#endif
            iRet = 1;

            break;
        } 
        
        default:
            break;
    }
      
done:
    if(iRet == 0)
    {
        AT_LOG("ERROR\r\n");
    }

    return iRet;
}
/*************************************************************************
* FUNCTION:
*   AT_CmdUrlSetHandler
*
* DESCRIPTION:
*   Seting Mqtt host domain name
* 
*
* PARAMETERS
*   <host domain name>, <host port>,<bool autoconn flag>
*
* RETURNS
*   none
*
*************************************************************************/
int AT_CmdUrlSetHandler(char *buf, int len, int mode)
{
    int iRet = 0;
    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    switch(mode)
    {
        case AT_CMD_MODE_SET:
        {
            if(!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
            {
                AT_LOG("at_cmd_buf_to_argc_argv\n");
                goto done;
            }

            if(argc < 3)
            {
                goto done;
            }

            T_CloudConnInfo tCloudConnInfo = {0};

            // default using MBEDTLS_SSL_VERIFY_NONE type
            tCloudConnInfo.u8Security = 0;

            // copy port and host address
            memcpy(tCloudConnInfo.u8aHostAddr, argv[1], strlen(argv[1]));
            tCloudConnInfo.u16HostPort = (uint16_t)strtoul(argv[2], NULL, 0);

            if(argc >= 4)
            {
                // set auto connect flag
                tCloudConnInfo.u8AutoConn = (uint8_t)strtoul(argv[3], NULL, 0);
            }
            
            if(OPL_OK != APP_SendMessage(APP_EVT_CLOUD_MQTT_ESTAB_REQ, (uint8_t *)&tCloudConnInfo, sizeof(T_CloudConnInfo)))
            {
                goto done;
            }

            iRet = 1;

            break;
        }

        default:
            break;
    }

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
