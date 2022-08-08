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
*  wifi_autocon_at.c
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
#include "opl_err.h"
#include "wifi_autocon.h"
#include "wifi_autocon_at.h"
#include "wifi_mngr.h"
#include "wifi_mngr_api.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (WM_AC_ENABLED == 1)

#define AC_AT_LOG                                       msg_print_uart1

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

#if (WM_AC_AT_ENABLED == 1)
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
static void AT_CmdAcEnableIndCb(T_OplErr tEvtRst, uint8_t *pu8Data, uint32_t u32DataLen)
{
    if(OPL_OK == tEvtRst)
    {
        AC_AT_LOG("[IND] AC enable success\r\n");
    }
    else
    {
        AC_AT_LOG("[IND] AC enable fail\r\n");
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
static void AT_CmdAcDisableIndCb(T_OplErr tEvtRst)
{
    if(OPL_OK == tEvtRst)
    {
        AC_AT_LOG("[IND] AC disable success\r\n");
    }
    else
    {
        AC_AT_LOG("[IND] AC disable fail\r\n");
    }
}
#endif /* WM_AC_AT_ENABLED */

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
int AT_CmdWmAcSwitchHandler(char *buf, int len, int mode)
{
    int iRet = 0;

#if (WM_AC_AT_ENABLED == 1)

    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
    {
        goto done;
    }

    if (argc > 3)
    {
        AC_AT_LOG("invalid param number\r\n");
        goto done;
    }

    switch (mode)
    {
        case AT_CMD_MODE_SET:
        {
            if(strcmp(argv[1], "enable") == 0)
            {
                AC_AT_LOG("AC enable\r\n");
                if(OPL_OK != Opl_Wifi_AC_Enable_Req(AT_CmdAcEnableIndCb))
                {
                    AC_AT_LOG("AC enable request fail\r\n");
                }
            }
            else if(strcmp(argv[1], "disable") == 0)
            {
                uint8_t u8ActDisconnect = (uint8_t)atoi(argv[2]);

                if(1 < u8ActDisconnect)
                {
                    goto done;
                }

                AC_AT_LOG("AC disable (Act Disconnect %d)\r\n", u8ActDisconnect);

                if(OPL_OK != Opl_Wifi_AC_Disable_Req((bool)u8ActDisconnect, AT_CmdAcDisableIndCb))
                {
                    AC_AT_LOG("AC disable request fail\r\n");
                }
            }
            else
            {
                goto done;
            }

            iRet = 1;

            break;
        }
    }

done:
    if (iRet)
    {
        AC_AT_LOG("OK\r\n");
    }
    else
    {
        AC_AT_LOG("ERROR\r\n");
    }

#else
    AC_AT_LOG("AC at cmd not support\r\n");
#endif /* WM_AC_AT_ENABLED */

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
int AT_CmdWmAcStatusChkHandler(char *buf, int len, int mode)
{
    int iRet = 0;

#if (WM_AC_AT_ENABLED == 1)

    switch (mode)
    {
        case AT_CMD_MODE_READ:
        {
            T_FsmDef *tAcFsmDef = WM_AcFsmDefGet();

            AC_AT_LOG("AC (C %d; H %d)\r\n", tAcFsmDef->ptFsmStateInfo.tCurrentState,
                                             tAcFsmDef->ptFsmStateInfo.tHistoryState);

            iRet = 1;

            break;
        }
    }

    if (iRet)
    {
        AC_AT_LOG("OK\r\n");
    }
    else
    {
        AC_AT_LOG("ERROR\r\n");
    }

#else
    AC_AT_LOG("AC at cmd not support\r\n");
#endif /* WM_AC_AT_ENABLED */

    return iRet;
}

#endif /* WM_AC_ENABLED */
