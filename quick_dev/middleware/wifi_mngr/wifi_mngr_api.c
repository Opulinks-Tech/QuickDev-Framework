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
*  file_temp.c
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

#include "wifi_mngr_api.h"
#include "wifi_mngr.h"
#include "wifi_agent.h"

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

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Init_Req
*
* DESCRIPTION:
*   WI-FI manager initiate function, and carried the init result in indicate callback
*
* PARAMETERS
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_Init_Req(T_WaInitDoneIndCbFp fpIndCb)
{
    WM_TaskInit();

    return WM_SendMessage(WM_EVT_INIT_REQ, NULL, 0, (FsmIndicateCbFunc)fpIndCb);
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Scan_Req
*
* DESCRIPTION:
*   trigger WI-FI scan request, and carried the scan result in indicate callback
*
* PARAMETERS
*   ptWifiScanConfig :
*                   [IN] scan config structure
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_Scan_Req(wifi_scan_config_t *ptWifiScanConfig, T_WaScanDoneIndCbFp fpIndCb)
{
    return WM_SendMessage(WM_EVT_SCAN_REQ, (uint8_t *)ptWifiScanConfig, sizeof(wifi_scan_config_t), (FsmIndicateCbFunc)fpIndCb);
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Conn_Req
*
* DESCRIPTION:
*   trigger WI-FI connect request, and carried the connect result in indicate callback
*
* PARAMETERS
*   ptWifiConnConfig :
*                   [IN] connect config structure
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_Conn_Req(T_WmConnConfig *ptWifiConnConfig, T_WaConnectIndCbFp fpIndCb)
{
    return WM_SendMessage(WM_EVT_CONNECT_REQ, (uint8_t *)ptWifiConnConfig, sizeof(T_WmConnConfig), (FsmIndicateCbFunc)fpIndCb);
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Disc_Req
*
* DESCRIPTION:
*   trigger WI-FI disconnect request, and carried the disconnect result in indicate callback
*
* PARAMETERS
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_Disc_Req(T_WaDisconnectIndCbFp fpIndCb)
{
    return WM_SendMessage(WM_EVT_DISCONNECT_REQ, NULL, 0, (FsmIndicateCbFunc)fpIndCb);
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Uslctd_CB_Reg
*
* DESCRIPTION:
*   register unsolicited callback for WI-FI manager
*
* PARAMETERS
*   fpUslctdCb :    [IN] unsolicited callback function pointer
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_Uslctd_CB_Reg(T_WmUslctedCbFp fpUslctedCb)
{
    return WM_UslctedCbReg(fpUslctedCb);
}

#if (WM_AC_ENABLED == 1)
/*************************************************************************
* FUNCTION:
*   Opl_Wifi_AC_Enable_Req
*
* DESCRIPTION:
*   enable auto-connect request, and carried the enable result in indicate callback
*
* PARAMETERS
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_AC_Enable_Req(T_AcEnableIndCbFp fpIndCb)
{
    return WM_SendMessage(WM_EVT_AC_ENABLE_REQ, NULL, 0, (FsmIndicateCbFunc)fpIndCb);
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_AC_Disable_Req
*
* DESCRIPTION:
*   disable auto-connect request, and carried the disable result in indicate callback
*
* PARAMETERS
*   blActDisconnect :
*                   [IN] true -> will do WI-FI disconnect while disable auto-connect
*                        false -> won't do WI-FI disconnect while disable auto-connect
*   fpIndCb :       [IN] indicate callback function pointer (if required)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_AC_Disable_Req(bool blActDisconnect, T_AcDisableIndCbFp fpIndCb)
{
    if(true == blActDisconnect)
    {
        return WM_SendMessage(WM_EVT_AC_DISABLE_REQ, NULL, 0, (FsmIndicateCbFunc)fpIndCb);
    }
    else
    {
        return WM_SendMessage(WM_EVT_AC_DISABLE_NO_DISC_REQ, NULL, 0, (FsmIndicateCbFunc)fpIndCb);
    }
}

#endif /* WM_AC_ENABLED */

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Network_Status_Get
*
* DESCRIPTION:
*   get WI-FI manager status
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Opl_Wifi_Network_Status_Get(void)
{
    
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Profile_Ins
*
* DESCRIPTION:
*   insert WI-FI AP information into AP profile list
*
* PARAMETERS
*   tNewProfile :   [IN] AP information structure
*
* RETURNS
*   none
*
*************************************************************************/
void Opl_Wifi_Profile_Ins(T_PrApProfile tNewProfile)
{
    WM_PrInsert(tNewProfile);
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Profile_Get
*
* DESCRIPTION:
*   get WI-FI AP profile list
*
* PARAMETERS
*   none
*
* RETURNS
*   T_PrApProfilePtr :
*                   [OUT] AP profile list
*
*************************************************************************/
T_PrApProfilePtr Opl_Wifi_Profile_Get(void)
{
    return WM_PrGet();
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Profile_Clear
*
* DESCRIPTION:
*   clear WI-FI AP profile list
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void Opl_Wifi_Profile_Clear(void)
{
    WM_PrClear();
}

//// Get Type WI-FI Apis
/*************************************************************************
* FUNCTION:
*   Opl_Wifi_ApInfo_Get
*
* DESCRIPTION:
*   get AP information
*
* PARAMETERS
*   ptApRecord :    [OUT] AP information
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_ApInfo_Get(wifi_ap_record_t *ptApRecord)
{
    return WM_WaApInfoGet(ptApRecord);
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_MacAddr_Get
*
* DESCRIPTION:
*   get WI-FI mac address
*
* PARAMETERS
*   pu8Address :    [OUT] WI-FI mac address
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_MacAddr_Get(uint8_t *pu8Address)
{
    return WM_WaMacAddrGet(pu8Address);    
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_ManufName_Get
*
* DESCRIPTION:
*   get manufacturer name
*
* PARAMETERS
*   pu8Name :       [OUT] manufacturer name
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_ManufName_Get(uint8_t *pu8Name)
{
    return WM_WaManufNameGet(pu8Name);
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_ConfigSource_Get
*
* DESCRIPTION:
*   get configure source
*
* PARAMETERS
*   tWmWifiGetConfigSource :
                    [OUT] configure source structure
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_ConfigSource_Get(T_WmWifiGetConfigSource *tWmWifiGetConfigSource)
{
    return WM_WaConfigSourceGet(tWmWifiGetConfigSource->iface, tWmWifiGetConfigSource->type);
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Rssi_Get
*
* DESCRIPTION:
*   get current rssi value
*
* PARAMETERS
*   pi8Rssi :       [OUT] rssi value
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_Rssi_Get(int8_t *pi8Rssi)
{
    return WM_WaRssiGet(pi8Rssi);
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_ApNum_Get
*
* DESCRIPTION:
*   get AP number
*
* PARAMETERS
*   pu16Num :       [OUT] AP number
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_ApNum_Get(uint16_t *pu16Num)
{
    return WM_WaApNumGet(pu16Num);
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_ApRecord_Get
*
* DESCRIPTION:
*   get AP record
*
* PARAMETERS
*   pu16ApCount :   [OUT] count number of AP
*   ptScanInfo :    [OUT] scan information sturcture
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_ApRecord_Get(uint16_t *pu16ApCount, wifi_scan_info_t *ptScanInfo)
{
    return WM_WaApRecordGet(pu16ApCount, ptScanInfo);
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_AutoConnectApNum_Get
*
* DESCRIPTION:
*   get auto-connect AP number
*
* PARAMETERS
*   pu8Num :        [OUT] auto-connect AP number
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_AutoConnectApNum_Get(uint8_t *pu8Num)
{
    return WM_WaAutoConnectApNumGet(pu8Num);
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_AutoConnectApInfo_Get
*
* DESCRIPTION:
*   get specific auto-connect AP information
*
* PARAMETERS
*   u8Index :       [IN] index of auto-connect list
*   ptAutoConnInfo :
                    [OUT] auto-connect AP information structure
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_AutoConnectApInfo_Get(uint8_t u8Index, wifi_auto_connect_info_t *ptAutoConnInfo)
{
    return WM_WaAutoConnectApInfoGet(u8Index, ptAutoConnInfo);
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_AutoConnectListNum_Get
*
* DESCRIPTION:
*   get auto-connect list number
*
* PARAMETERS
*   pu8Num :        [OUT] auto-connect list number
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_AutoConnectListNum_Get(uint8_t *pu8Num)
{
    return WM_WaAutoConnectListNumGet(pu8Num);
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_ScanList_Get
*
* DESCRIPTION:
*   get scan list result
*
* PARAMETERS
*   ptScanList :    [OUT] scan list structure
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_ScanList_Get(wifi_scan_list_t *ptScanList)
{
    return WM_WaScanListGet(ptScanList);
}

//// Set Type WI-FI Apis
/*************************************************************************
* FUNCTION:
*   Opl_Wifi_MacAddr_Set
*
* DESCRIPTION:
*   set WI-FI mac address
*
* PARAMETERS
*   pu8Address :    [IN] WI-FI mac address
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_MacAddr_Set(uint8_t *pu8Address)
{
    return WM_WaMacAddrSet(pu8Address);   
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_ManufName_Set
*
* DESCRIPTION:
*   set manufacturer name
*
* PARAMETERS
*   pu8Name :       [IN] manufacturer name
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_ManufName_Set(uint8_t *pu8Name)
{
    return WM_WaManufNameSet(pu8Name);
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_ConfigSource_Set
*
* DESCRIPTION:
*   set configure source
*
* PARAMETERS
*   tWmWifiSetConfigSource :
                    [IN] configure source structure
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_ConfigSource_Set(T_WmWifiSetConfigSource *tWmWifiSetConfigSource)
{
    return WM_WaConfigSourceSet(tWmWifiSetConfigSource->iface, tWmWifiSetConfigSource->type);
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Skip_Dtim_Module_Reg
*
* DESCRIPTION:
*   register skip DTIM module
*
* PARAMETERS
*   u16ModuleId :   [OUT] skip DTIM module ID
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_Skip_Dtim_Module_Reg(uint16_t *u16ModuleId)
{
    return WM_WaSkipDtimModuleReg(u16ModuleId);
}

/*************************************************************************
* FUNCTION:
*   Opl_Wifi_Skip_Dtim_Set
*
* DESCRIPTION:
*   set skip DTIM 
*
* PARAMETERS
*   u16ModuleId :   [IN] registed skip DTIM module ID
*                        (must reigster by calling Opl_Wifi_Skip_Dtim_Module_Reg first)
*   u8Enable :      [IN] true -> skip DTIM
*                        false -> no skip DTIM
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Wifi_Skip_Dtim_Set(uint16_t u16ModuleId, uint8_t u8Enable)
{
    return WM_WaSkipDtimSet(u16ModuleId, u8Enable);
}

#endif /* WM_ENABLED */
