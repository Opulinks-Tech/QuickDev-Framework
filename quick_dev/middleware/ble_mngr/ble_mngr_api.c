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
*  ble_mngr_api.c
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

#include "ble_mngr.h"
#include "ble_mngr_api.h"
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (BM_ENABLED == 1)

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
*   Opl_Ble_Init_Req
*
* DESCRIPTION:
*   ble manager initiate function
*
* PARAMETERS
*   u8AutoAdvEn :   [IN] true -> will enable auto-advertise while disconnect
*                        (also will enable advertise after init)
*                        false -> won't enable auto-advertise while disconnect
*                        (also won't enable advertise after init)
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Ble_Init_Req(uint8_t u8AutoAdvEn)
{
    BM_TaskInit();

    return BM_SendMessage(BM_EVT_INIT_REQ, (uint8_t *)&u8AutoAdvEn, sizeof(uint8_t), 500);
}

/*************************************************************************
* FUNCTION:
*   Opl_Ble_Start_Req
*
* DESCRIPTION:
*   start ble advertising
*
* PARAMETERS
*   u8AutoAdvEn :   [IN] enable auto-advertise while disconnect
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Ble_Start_Req(uint8_t u8AutoAdvEn)
{
    return BM_SendMessage(BM_EVT_START_REQ, (uint8_t *)&u8AutoAdvEn, sizeof(uint8_t), 0);
}

/*************************************************************************
* FUNCTION:
*   Opl_Ble_Stop_Req
*
* DESCRIPTION:
*   stop ble advertising
*
* PARAMETERS
*   none
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Ble_Stop_Req(void)
{
    return BM_SendMessage(BM_EVT_STOP_REQ, NULL, 0, 0);
}

/*************************************************************************
* FUNCTION:
*   Opl_Ble_Stop_Req
*
* DESCRIPTION:
*   register unsolicited callback for ble manager
*
* PARAMETERS
*   fpUslctdCb :    [IN] unsolicited callback function pointer
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Ble_Uslctd_CB_Reg(T_BmUslctedCbFp fpUslctdCb)
{
    return BM_UslctedCbReg(fpUslctdCb);
}

/*************************************************************************
* FUNCTION:
*   Opl_Ble_Send_Message
*
* DESCRIPTION:
*   send message to ble task
*
* PARAMETERS
*   u16Event :      [IN] event id (define by user)
*   u8Data :        [IN] data send to ble manager
*   u32DataLen :    [IN] data lens send to ble manager
*   u32DelayMs :    [IN] delay time to trigger event
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Ble_Send_Message(uint16_t u16Evt, uint8_t *u8Data, uint32_t u32DataLen, uint32_t u32DelayMs)
{
    return BM_SendMessage(u16Evt, (uint8_t *)u8Data, u32DataLen, u32DelayMs);
}

/*************************************************************************
* FUNCTION:
*   Opl_Ble_Service_Assign
*
* DESCRIPTION:
*   register ble service
*
* PARAMETERS
*   ptBmSvcHandle : [IN] ble service structure
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Ble_Service_Assign(T_BmSvcHandle *ptBmSvcHandle)
{
    return BM_ServiceAssign(ptBmSvcHandle);
}

/*************************************************************************
* FUNCTION:
*   Opl_Ble_Advertise_Data_Set
*
* DESCRIPTION:
*   setting ble advertise data
*
* PARAMETERS
*   pau8AdvData :   [IN] advertise data
*   u8AdvDataLen :  [IN] advertise data len
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Ble_Advertise_Data_Set(uint8_t *pau8AdvData, uint8_t u8AdvDataLen)
{
    return BM_AdvDataSet(pau8AdvData, u8AdvDataLen);
}

/*************************************************************************
* FUNCTION:
*   Opl_Ble_ScanRsp_Data_Set
*
* DESCRIPTION:
*   setting ble scan response data
*
* PARAMETERS
*   pau8ScanRspData :
*                   [IN] scan response data
*   u8ScanRspDataLen :
*                   [IN] scan response data len
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Ble_ScanRsp_Data_Set(uint8_t *pau8ScanRspData, uint8_t u8ScanRspDataLen)
{
    return BM_ScanRspDataSet(pau8ScanRspData, u8ScanRspDataLen);
}

/*************************************************************************
* FUNCTION:
*   Opl_Ble_EntityGet
*
* DESCRIPTION:
*   get ble task entity
*
* PARAMETERS
*   none
*
* RETURNS
*   BLE_APP_DATA_T :
*                   return ble task entity
*
*************************************************************************/
BLE_APP_DATA_T *Opl_Ble_EntityGet(void)
{
    return BM_EntityGet();
}

/*************************************************************************
* FUNCTION:
*   Opl_Ble_MacAddr_Write
*
* DESCRIPTION:
*   write ble mac address
*
* PARAMETERS
*   pau8Data :      [IN] mac address array
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Ble_MacAddr_Write(uint8_t *pau8Data)
{
    uint8_t ubaMacAddr[6];

    ubaMacAddr[5] = pau8Data[0];
    ubaMacAddr[4] = pau8Data[1];
    ubaMacAddr[3] = pau8Data[2];
    ubaMacAddr[2] = pau8Data[3];
    ubaMacAddr[1] = pau8Data[4];
    ubaMacAddr[0] = pau8Data[5];

    if(0 != ble_set_config_bd_addr(ubaMacAddr))
    {
        return OPL_ERR;
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION:
*   Opl_Ble_MacAddr_Read
*
* DESCRIPTION:
*   read ble mac address
*
* PARAMETERS
*   pau8Data :      [OUT] mac address array
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr Opl_Ble_MacAddr_Read(uint8_t *pau8Data)
{
    uint8_t u8aMacAddr[6];

    if(0 != ble_get_config_bd_addr(u8aMacAddr))
    {
        return OPL_ERR;
    }

    pau8Data[5] = u8aMacAddr[0];
    pau8Data[4] = u8aMacAddr[1];
    pau8Data[3] = u8aMacAddr[2];
    pau8Data[2] = u8aMacAddr[3];
    pau8Data[1] = u8aMacAddr[4];
    pau8Data[0] = u8aMacAddr[5];

    return OPL_OK;
}

#endif /* BM_ENABLED */
