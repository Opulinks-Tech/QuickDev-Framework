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
*  bas_svc.c
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
#include "bas_svc.h"
#include "ble_mngr_api.h"
#include "ble_gatt_if.h"
#include "ble_uuid.h"
#include "log.h"

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

static uint16_t g_BasSvcUuid                      = BAS_SVC_SERVICE_UUID;

static uint16_t g_BasSvcBatLvlCharUuid            = BAS_SVC_BAT_LVL_CHAR_UUID;
static uint8_t  g_BasSvcBatLvlCharVal[]           = CHAR_DECL_UUID16_ATTR_VAL(LE_GATT_CHAR_PROP_RD | LE_GATT_CHAR_PROP_NTF, BAS_SVC_BAT_LVL_CHAR_UUID);
static uint8_t  g_BasSvcBatLvlVal                 = 0;
static uint16_t g_BasSvcBatLvlClientCfg           = 1;

// BAS service definition
static LE_GATT_ATTR_T g_BasSvcDb[BAS_SVC_IDX_TOTAL] =
{
    // BAS service declaration
    [BAS_SVC_IDX_SVC]                   = PRIMARY_SERVICE_DECL_UUID16(&g_BasSvcUuid),
    // BAS service rx characteristic
    [BAS_SVC_IDX_BAT_LVL_CHAR]          = CHARACTERISTIC_DECL_UUID16(g_BasSvcBatLvlCharVal),
    [BAS_SVC_IDX_BAT_LVL_VAL]           = CHARACTERISTIC_UUID16(&g_BasSvcBatLvlCharUuid, LE_GATT_PERMIT_AUTHOR_READ, sizeof(g_BasSvcBatLvlVal), sizeof(g_BasSvcBatLvlVal), &g_BasSvcBatLvlVal),
    [BAS_SVC_IDX_BAT_LVL_CFG]           = CHAR_CLIENT_CONFIG_DESCRIPTOR(LE_GATT_PERMIT_AUTHOR_READ | LE_GATT_PERMIT_AUTHOR_WRITE, &g_BasSvcBatLvlClientCfg)
};

static T_BmSvcHandle g_tBasSvcHandle = {0};

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

/*************************************************************************
* FUNCTION:
*   BAS_Svc_GattDispatchReadHandler
*
* DESCRIPTION:
*   dispatch read type from gatt protocol
*
* PARAMETERS
*   ind :           [IN] access read indicate data
*
* RETURNS
*   none
*
*************************************************************************/
static void BAS_Svc_GattDispatchReadHandler(LE_GATT_MSG_ACCESS_READ_IND_T *ind)
{
    // process the read access activity in each characteristic
    uint8_t u8AttErr = 0;
    uint16_t u16AttrId = ind->handle - g_tBasSvcHandle.ptSvcDef->startHdl;

    // printf("BAS Svc attId = %d op = %x offset = %d\r\n", u16AttrId, ind->flag, ind->offset);

    switch(u16AttrId)
    {
        case BAS_SVC_IDX_BAT_LVL_VAL:
        {
            // send message to app that the host required to read the battery
            APP_SendMessage(APP_EVT_BLE_REQ_BAT_LVL, NULL, 0);

            break;
        }
        case BAS_SVC_IDX_BAT_LVL_CFG:
        {
            uint16_t u16Enable;
            uint16_t u16Len = 0;

            LeGattGetAttrVal(g_tBasSvcHandle.ptSvcDef, BAS_SVC_IDX_BAT_LVL_CFG, &u16Len, &u16Enable);

            OPL_LOG_INFO(BAS, "Svc data out config val=%d", u16Enable);

            break;
        }

        default:
        {
            u8AttErr = LE_ATT_ERR_READ_NOT_PERMITTED;
            break;
        }
    }

    LeGattAccessReadRsp(ind->conn_hdl, ind->handle, u8AttErr);
}

/*************************************************************************
* FUNCTION:
*   BAS_Svc_GattDispatchWriteHandler
*
* DESCRIPTION:
*   dispatch write type from gatt protocol
*
* PARAMETERS
*   ind :           [IN] access write indicate data
*
* RETURNS
*   none
*
*************************************************************************/
static void BAS_Svc_GattDispatchWriteHandler(LE_GATT_MSG_ACCESS_WRITE_IND_T *ind)
{
    // process the write access activity in each characteristic
    uint8_t u8AttErr = 0;
    uint16_t u16AttrId = ind->handle - g_tBasSvcHandle.ptSvcDef->startHdl;

    switch(u16AttrId)
    {
        case BAS_SVC_IDX_BAT_LVL_CFG:
        {
            uint16_t u16Enable = *((uint16_t *)ind->pVal);

            if ((ind->len == 2) && (u16Enable <= 1))
            {
                LeGattChangeAttrVal(g_tBasSvcHandle.ptSvcDef, BAS_SVC_IDX_BAT_LVL_CFG, sizeof(u16Enable), &u16Enable);
            }
            else
            {
                u8AttErr = LE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }

            break;
        }

        default:
        {
            u8AttErr = LE_ATT_ERR_WRITE_NOT_PERMITTED;
            break;
        }
    }

    LeGattAccessWriteRsp(ind->conn_hdl, ind->flag, ind->handle, u8AttErr);
}

/*************************************************************************
* FUNCTION:
*   BAS_Svc_GattDispatchHandler
*
* DESCRIPTION:
*   dispatch charateristic from gatt protocol
*
* PARAMETERS
*   tId :           [IN] message ID 
*   tMsg :          [IN] message data
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
static T_OplErr BAS_Svc_GattDispatchHandler(MESSAGEID tId, MESSAGE tMsg)
{
    switch(tId)
    {
        case LE_GATT_MSG_ACCESS_READ_IND:
        {
            BAS_Svc_GattDispatchReadHandler((LE_GATT_MSG_ACCESS_READ_IND_T *)tMsg);
            break;
        }
        
        case LE_GATT_MSG_ACCESS_WRITE_IND:
        {
            BAS_Svc_GattDispatchWriteHandler((LE_GATT_MSG_ACCESS_WRITE_IND_T *)tMsg);
            break;
        }

        case LE_GATT_MSG_NOTIFY_CFM:
        {
            // notify successfully
            OPL_LOG_INFO(BAS, "Notify Confirm");
            break;
        }

        default:
        {
            return OPL_ERR_CASE_INVALID;
        }
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION:
*   BAS_Svc_BatteryLevelNotify
*
* DESCRIPTION:
*   update & notify battery level to host
*
* PARAMETERS
*   u8Percent :     [IN] percentage of battery level
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr BAS_Svc_BatteryLevelNotify(uint8_t u8Percent)
{
    LE_ERR_STATE status;

    uint16_t u16NotiAttrId = g_tBasSvcHandle.ptSvcDef->startHdl + BAS_SVC_IDX_BAT_LVL_VAL;

    // update battery level to attribute value
    LeGattChangeAttrVal(g_tBasSvcHandle.ptSvcDef, (uint16_t)BAS_SVC_IDX_BAT_LVL_VAL, sizeof(u8Percent), &u8Percent);

    // notify battery level to host
    status = LeGattCharValNotify(Opl_Ble_EntityGet()->conn_hdl, u16NotiAttrId, sizeof(u8Percent), &u8Percent);

    if (status != SYS_ERR_SUCCESS)
    {
        OPL_LOG_ERRO(BAS, "Ble sending data fail");
        return OPL_ERR;
    }

    return OPL_OK;
}

/*************************************************************************
* FUNCTION:
*   BAS_Svc_Init
*
* DESCRIPTION:
*   initiate battery service
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void BAS_Svc_Init(void)
{
    g_tBasSvcHandle.patSvcDb = g_BasSvcDb;
    g_tBasSvcHandle.u8SvcDbSize = BAS_SVC_IDX_TOTAL;
    g_tBasSvcHandle.ptSvcGattDispatchHandler = BAS_Svc_GattDispatchHandler;

    if(OPL_OK != Opl_Ble_Service_Assign(&g_tBasSvcHandle))
    {
        OPL_LOG_ERRO(BAS, "BAS Service assign fail\r\n");
    }
}
