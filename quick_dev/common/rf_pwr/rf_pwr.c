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
*  rf_pwr.c
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

#include "log.h"
#include "opl_err.h"
#include "rf_pwr.h"
#if defined(OPL1000_A2) || defined(OPL1000_A3)
#include "sys_cfg_patch.h"
#elif defined(OPL2500_A0)
#include "sys_cfg.h"
#endif
#include "sys_common_api.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

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
*   Get RSSI offset
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
T_OplErr RF_PwrRssiOffsetGet(int8_t *offset)
{
    int ret = 0;
    uint8_t level = 0;

    ret = sys_get_config_rf_power_level(&level);

    if(0 != ret)
    {
        return OPL_ERR;
    }

    switch(level)
    {
        case RF_PWR_LVL_D0:
            *offset = -10;
            break;
        case RF_PWR_LVL_B0:
            *offset = -10;
            break;
        case RF_PWR_LVL_A0:
            *offset = -13;
            break;
        case RF_PWR_LVL_90:
            *offset = -16;
            break;
        case RF_PWR_LVL_40:
            *offset = -22;
            break;
        case RF_PWR_LVL_20:
            *offset = -22;
            break;
        case RF_PWR_LVL_00:
            *offset = -25;
            break;
        default:
            return OPL_ERR;
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   Set RF power (0x00 - 0xFF)
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
T_OplErr RF_PwrSet(uint8_t level)
{
    int ret = 0;

    if( RF_PWR_LVL_D0 == level ||
        RF_PWR_LVL_B0 == level ||
        RF_PWR_LVL_A0 == level ||
        RF_PWR_LVL_90 == level ||
        RF_PWR_LVL_40 == level ||
        RF_PWR_LVL_20 == level ||
        RF_PWR_LVL_00 == level)
    {
#if defined(OPL1000_A2) || defined(OPL1000_A3)
        T_RfCfg tCfg;
        tCfg.u8HighPwrStatus = level;

        ret = sys_cfg_rf_init_patch(&tCfg);

        if(0 != ret)
        {
            ret = sys_set_config_rf_power_level(level);
        }
#elif defined(OPL2500_A0)
        ret = sys_set_config_rf_power_level(level);
#endif

        OPL_LOG_INFO(RF, "RF Power level %X set %s", level, (ret == 0 ? "ok" : "fail"));

        if(0 != ret)
        {
            return OPL_ERR;
        }
        else
        {
            return OPL_OK;
        }
    }
    else
    {
        return OPL_ERR;
    }
}
