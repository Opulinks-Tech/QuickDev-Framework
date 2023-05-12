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
*  wifi_agent_api.c
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

#include "rf_pwr.h"
#include "qd_config.h"
#include "qd_module.h"
#include "wifi_mngr.h"
#include "wifi_agent_api.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (WM_ENABLED == 1)

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

static uint16_t g_u16SkipDtimModuleIdField = 0;
static uint16_t g_u16SkipDtimModuleStField = 0;

// Sec 7: declaration of static function prototype

uint32_t _WM_WaBeaconIntervalGet(void);

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
uint32_t _WM_WaBeaconIntervalGet(void)
{
    uint32_t u32BeaconInterval = 0;

    wifi_scan_info_t tInfo;

    // get the information of Wifi AP
    if(0 != wifi_sta_get_ap_info(&tInfo))
    {
        WM_LOG_ERRO("Get AP info fail");
    }

    // beacon time (ms)
    u32BeaconInterval = tInfo.beacon_interval * tInfo.dtim_period;

    WM_LOG_DEBG("Beacon intvl %d", u32BeaconInterval);

    // error handle
    if (u32BeaconInterval == 0)
    {
        u32BeaconInterval = 100;
    }

    return u32BeaconInterval;
}

//// Get Type Apis
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
// void WM_WaScanListGet(uint8_t u8FilterType, wifi_scan_info_t *ptApList, uint8_t *pu8ApListNum)
// {
//     switch(u8FilterType)
//     {
//         case WA_SCAN_LIST_FILTER_MAX_RSSI:
//         case WA_SCAN_LIST_FILTER_MIN_RSSI:
//         case WA_SCAN_LIST_FILTER_MAX_RSSI_SORT:
//         case WA_SCAN_LIST_FILTER_MIN_RSSI_SORT:
//         case WA_SCAN_LIST_FILTER_NONE:
//     }
// }

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
T_OplErr WM_WaApInfoGet(wifi_ap_record_t *ptApRecord)
{
    wifi_sta_get_ap_info(ptApRecord);

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
T_OplErr WM_WaMacAddrGet(uint8_t *pu8Address)
{
    if(0 != wifi_config_get_mac_address(WIFI_MODE_STA , pu8Address))
    {
        return OPL_ERR;
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
T_OplErr WM_WaManufNameGet(uint8_t *pu8Name)
{
    if(NULL == pu8Name)
    {
        return OPL_ERR_PARAM_INVALID;
    }

    memset(pu8Name, 0, STA_INFO_MAX_MANUF_NAME_SIZE);

    if(0 != wifi_nvm_sta_info_read(WIFI_NVM_STA_INFO_MANUFACTURE_NAME, STA_INFO_MAX_MANUF_NAME_SIZE, pu8Name))
    {
        WM_LOG_ERRO("Wifi get manuf name fail");
        return OPL_ERR;
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
T_OplErr WM_WaConfigSourceGet(mac_iface_t tIface, mac_source_type_t *tType)
{
    if(0 != mac_addr_get_config_source(tIface, tType))
    {
        return OPL_ERR;
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
T_OplErr WM_WaRssiGet(int8_t *pi8Rssi)
{	
    int8_t i8Offset = 0;

    if(0 != wifi_connection_get_rssi(pi8Rssi))
    {
        return OPL_ERR;
    }

    if(OPL_OK != RF_PwrRssiOffsetGet(&i8Offset))
    {
        return OPL_ERR;
    }

    *pi8Rssi += i8Offset;

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
T_OplErr WM_WaApNumGet(uint16_t *pu16Num)
{
    if(0 != wifi_scan_get_ap_num(pu16Num))
    {
        return OPL_ERR;
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
T_OplErr WM_WaApRecordGet(uint16_t *pu16ApCount, wifi_scan_info_t *ptScanInfo)
{
    if(0 != wifi_scan_get_ap_records(pu16ApCount, ptScanInfo))
    {
        return OPL_ERR;
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
T_OplErr WM_WaAutoConnectApNumGet(uint8_t *pu8Num)
{
    if(0 != wifi_auto_connect_get_ap_num(pu8Num))
    {
        return OPL_ERR;
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
T_OplErr WM_WaAutoConnectApInfoGet(uint8_t u8Index, wifi_auto_connect_info_t *ptAutoConnInfo)
{
    if(0 != wifi_auto_connect_get_ap_info(u8Index , ptAutoConnInfo))
    {
        return OPL_ERR;
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
T_OplErr WM_WaAutoConnectListNumGet(uint8_t *pu8Num)
{
    if(0 != wifi_auto_connect_get_saved_ap_num(pu8Num))
    {
        return OPL_ERR;
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
T_OplErr WM_WaScanListGet(wifi_scan_list_t *ptScanList)
{
    if(0 != wifi_scan_get_ap_list(ptScanList))
    {
        return OPL_ERR;
    }

    return OPL_OK;
}

//// Set Type Apis
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
T_OplErr WM_WaMacAddrSet(uint8_t *pu8Address)
{
    if(0 != wifi_config_set_mac_address(WIFI_MODE_STA, pu8Address))
    {
        return OPL_ERR;
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
T_OplErr WM_WaManufNameSet(uint8_t *pu8Name)
{
    uint8_t u8Len;

    if(NULL == pu8Name)
    {
        return OPL_ERR_PARAM_INVALID;
    }

    u8Len = strlen((char *)pu8Name);

    if(u8Len > STA_INFO_MAX_MANUF_NAME_SIZE)
        u8Len = STA_INFO_MAX_MANUF_NAME_SIZE;

    if(0 != wifi_nvm_sta_info_write(WIFI_NVM_STA_INFO_MANUFACTURE_NAME, u8Len, pu8Name))
    {
        WM_LOG_ERRO("Wifi set manuf name fail");
        return OPL_ERR;
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
T_OplErr WM_WaConfigSourceSet(mac_iface_t tIface, mac_source_type_t tType)
{
    if(0 != mac_addr_set_config_source(tIface, tType))
    {
        return OPL_ERR;
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
T_OplErr WM_WaSkipDtimModuleReg(uint16_t *u16ModuleId)
{
    uint8_t i = 0;

    for(; i < 16; i ++)
    {
        if(0 == (g_u16SkipDtimModuleIdField & (1 << i)))
        {
            *u16ModuleId = (1 << i);

            // assign regist to id field
            g_u16SkipDtimModuleIdField |= (1 << i);

            // initiate state field
            g_u16SkipDtimModuleStField |= (1 << i);

            WM_LOG_INFO("Dtim module id %X (total %X)", *u16ModuleId, g_u16SkipDtimModuleIdField);

            return OPL_OK;
        }
    }

    WM_LOG_ERRO("Dtim module can't regist");

    return OPL_ERR;
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
T_OplErr WM_WaSkipDtimSet(uint16_t u16ModuleId, uint8_t u8Enable)
{
    uint32_t u32DtimInterval = 0;

    // validate module id
    if(u16ModuleId == (u16ModuleId & g_u16SkipDtimModuleIdField))
    {
        if(0 == WM_DTIM_PERIOD_TIME)
        {
            u32DtimInterval = 0;
        }
        else if(true == u8Enable)
        {
            g_u16SkipDtimModuleStField |= u16ModuleId;

            // while all modules set enable, then change skip dtim time
            if(g_u16SkipDtimModuleStField == g_u16SkipDtimModuleIdField)
            {
                uint32_t u32BeaconInterval = _WM_WaBeaconIntervalGet();
                // u32DtimInterval = (WM_DTIM_PERIOD_TIME / _WM_WaBeaconIntervalGet());
                u32DtimInterval = (WM_DTIM_PERIOD_TIME + (u32BeaconInterval / 2)) / u32BeaconInterval;
                
                if(u32DtimInterval > 0)
                {
                    u32DtimInterval = u32DtimInterval - 1;
                }
            }
            // rest will not change the dtim value
            else
            {
                WM_LOG_INFO("Some module no disable, dtim skip intvl no change (%X)", g_u16SkipDtimModuleStField);
                return OPL_OK;
            }
        }
        else if(false == u8Enable)
        {
            g_u16SkipDtimModuleStField &= ~u16ModuleId;

            u32DtimInterval = 0;
        }
        else
        {
            // invalid data of control argument (u8Enable)
            return OPL_ERR_PARAM_INVALID;
        }
    }
    else
    {
        // invalid module id
        return OPL_ERR_PARAM_INVALID;
    }

    // the max is 8 bits
    if (u32DtimInterval > 255)
    {
        u32DtimInterval = 255;
    }

    WM_LOG_INFO("Dtim skip intvl %d", u32DtimInterval);
    
    if (0 != wifi_config_set_skip_dtim(u32DtimInterval, false))
    {
        WM_LOG_ERRO("Dtim skip intvl set fail");

        return OPL_ERR;
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
T_OplErr WM_WaUpdateScanInfoToAutoConnList(void)
{
    uint8_t u8AutoCount;
    uint16_t u16ApCount = 0;
    uint16_t i, j;

    wifi_scan_info_t *ptApList = NULL;
    wifi_auto_connect_info_t *ptWifiAutoConnectInfo = NULL;

    // if no AP founded, don't update the auto-connect list
    WM_WaApNumGet(&u16ApCount);

    if(0 == u16ApCount)
    {
        WM_LOG_WARN("No AP found");

        return OPL_ERR_WIFI_SCAN_NO_AP;
    }

    // get scanned AP record
    ptApList = (wifi_scan_info_t *)malloc(sizeof(wifi_scan_info_t) * u16ApCount);

    if(NULL == ptApList)
    {
        WM_LOG_ERRO("malloc AP list fail");

        return OPL_ERR_ALLOC_MEMORY_FAIL;
    }

    // get scanned AP record
    WM_WaApRecordGet(&u16ApCount, ptApList);

    //////////////////////////////////////////////////////////////////////////////////

    // if the count of auto-connection list is empty, don't update the auto-connect list
    WM_WaAutoConnectListNumGet(&u8AutoCount);

    if(0 == u8AutoCount)
    {
        WM_LOG_DEBG("Auto-conn saved AP number = 0, bypass update ch");
        
        free(ptApList);

        return OPL_OK;
    }

    ptWifiAutoConnectInfo = (wifi_auto_connect_info_t *)malloc(sizeof(wifi_auto_connect_info_t));

    if(NULL == ptWifiAutoConnectInfo)
    {
        WM_LOG_ERRO("malloc auto-conn info fail");

        free(ptApList);

        return OPL_ERR_ALLOC_MEMORY_FAIL;
    }

    // compare and update the auto-connect list
    for (i=0; i<u8AutoCount; i++)
    {
        WM_WaAutoConnectApInfoGet(i, ptWifiAutoConnectInfo);

        for (j=0; j<u16ApCount; j++)
        {
            if (0 == memcmp(ptApList[j].bssid, ptWifiAutoConnectInfo->bssid, sizeof(ptWifiAutoConnectInfo->bssid)))
            {
                // if the channel is not the same, update it
                if (ptApList[j].channel != ptWifiAutoConnectInfo->ap_channel)
                {
                    if(0 != wifi_auto_connect_update_ch(i, ptApList[j].channel))
                    {
                        WM_LOG_WARN("Update auto-conn ch fail");

                        free(ptWifiAutoConnectInfo);
                        free(ptApList);

                        return OPL_ERR;
                    }
                }
            }
        }
    }

    free(ptWifiAutoConnectInfo);
    free(ptApList);

    return OPL_OK;
}

#endif /* WM_ENABLED */
