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
*  ble_mngr_at.c
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
#include "ble_mngr_api.h"
#include "ble_mngr_at.h"
#include "opl_err.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (BM_ENABLED == 1)

#define BM_AT_LOG                                       msg_print_uart1

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
int AT_CmdBmAdvSwitchHandler(char *buf, int len, int mode)
{
    int iRet = 0;

#if (BM_AT_ENABLED == 1)

    int argc = 0;
    char *argv[AT_MAX_CMD_ARGS] = {0};

    if (!at_cmd_buf_to_argc_argv(buf, &argc, argv, AT_MAX_CMD_ARGS))
    {
        goto done;
    }

    if (argc > 3)
    {
        BM_AT_LOG("invalid param number\r\n");
        goto done;
    }

    switch (mode)
    {
        case AT_CMD_MODE_SET:
        {
            if(strcmp(argv[1], "enable") == 0)
            {
                uint8_t u8AutoAdv = (uint8_t)atoi(argv[2]);

                if(1 < u8AutoAdv)
                {
                    goto done;
                }

                BM_AT_LOG("BLE switch enable (auto adv %d)\r\n", u8AutoAdv);

                if(OPL_OK != Opl_Ble_Start_Req(u8AutoAdv))
                {
                    BM_AT_LOG("BLE switch enable request fail\r\n");
                }
            }
            else if(strcmp(argv[1], "disable") == 0)
            {
                BM_AT_LOG("BLE switch disable\r\n");

                if(OPL_OK != Opl_Ble_Stop_Req())
                {
                    BM_AT_LOG("BLE switch disable request fail\r\n");
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
        BM_AT_LOG("OK\r\n");
    }
    else
    {
        BM_AT_LOG("ERROR\r\n");
    }

#else
    BM_AT_LOG("BM at cmd not support\r\n");
#endif /* BM_AT_ENABLED */

    return iRet;
}

#endif /* BM_ENABLED */
