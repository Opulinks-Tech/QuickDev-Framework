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
*  gatt_svc.c
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

#include "ble_mngr_api.h"
#include "ble_gatt_if.h"
#include "ble_uuid.h"
#include "gatt_svc.h"
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

// This is used for GATT service
static uint16_t gGattSvcUuid = ATT_SVC_GENERIC_ATTRIBUTE;

static uint16_t gGattServiceChangeUuid      = ATT_CHAR_SERVICE_CHANGED;
static uint8_t  gGattServiceChangeCharVal[] = CHAR_DECL_UUID16_ATTR_VAL(LE_GATT_CHAR_PROP_IND, ATT_CHAR_SERVICE_CHANGED);
static uint8_t  gGattServiceChangeVal[4]    = {0, 0, 0, 0};
static uint16_t gGattServiceChangeClientCfg = 0;

static LE_GATT_ATTR_T gGattSvcDb[GATT_SVC_IDX_TOTAL] =
{
    // GATT Service Declaration
    [GATT_SVC_IDX_SVC]                 = PRIMARY_SERVICE_DECL_UUID16(&gGattSvcUuid),
    // GATT Service Change Characteristic 
    [GATT_SVC_IDX_SERVICE_CHANGE_CHAR] = CHARACTERISTIC_DECL_UUID16(gGattServiceChangeCharVal),
    [GATT_SVC_IDX_SERVICE_CHANGE_VAL]  = CHARACTERISTIC_UUID16(&gGattServiceChangeUuid, 0, 0, sizeof(gGattServiceChangeVal), gGattServiceChangeVal),
    [GATT_SVC_IDX_SERVICE_CHANGE_CFG]  = CHAR_CLIENT_CONFIG_DESCRIPTOR(LE_GATT_PERMIT_AUTHOR_READ | LE_GATT_PERMIT_AUTHOR_WRITE, &gGattServiceChangeClientCfg)
};

static T_BmSvcHandle g_tGattSvcHandle = {0};

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

/*************************************************************************
* FUNCTION:
*   GATT_Svc_GattDispatchReadHandler
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
static void GATT_Svc_GattDispatchReadHandler(LE_GATT_MSG_ACCESS_READ_IND_T *ind)
{
    uint8_t attErr = 0;
    uint16_t attrid = ind->handle - g_tGattSvcHandle.ptSvcDef->startHdl;
    // printf("BleAppHandleGattServiceRead attId = %d offset = %d\r\n", attrid, ind->offset);

    switch (attrid)
    {
        case GATT_SVC_IDX_SERVICE_CHANGE_CFG:
        break;

        default:
            attErr = LE_ATT_ERR_READ_NOT_PERMITTED;
        break;
    }

    LeGattAccessReadRsp(ind->conn_hdl, ind->handle, attErr);
}

/*************************************************************************
* FUNCTION:
*   GATT_Svc_GattDispatchWriteHandler
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
static void GATT_Svc_GattDispatchWriteHandler(LE_GATT_MSG_ACCESS_WRITE_IND_T *ind)
{
    uint8_t attErr = 0;
    uint16_t attrid = ind->handle - g_tGattSvcHandle.ptSvcDef->startHdl;
    // printf("BleAppHandleGattServiceWrite attId = %d op = %x offset = %d\r\n", attrid, ind->flag, ind->offset);

    switch (attrid)
    {
        case GATT_SVC_IDX_SERVICE_CHANGE_CFG:
        {
            uint16_t val = *((uint16_t *)ind->pVal);

            LeGattChangeAttrVal(g_tGattSvcHandle.ptSvcDef, attrid, 2, &val);
        }
        break;

        default:
            attErr = LE_ATT_ERR_WRITE_NOT_PERMITTED;
        break;
    }
    
    LeGattAccessWriteRsp(ind->conn_hdl, ind->flag, ind->handle, attErr);
}

/*************************************************************************
* FUNCTION:
*   GATT_Svc_IndicateServiceChange
*
* DESCRIPTION:
*   indicate serivce change
*
* PARAMETERS
*   u16ConnHdl :    [IN] connection handle
*
* RETURNS
*   none
*
*************************************************************************/
void GATT_Svc_IndicateServiceChange(uint16_t u16ConnHdl)
{
    uint16_t len;
    uint16_t val;
    LE_ERR_STATE rc = LeGattGetAttrVal(g_tGattSvcHandle.ptSvcDef, GATT_SVC_IDX_SERVICE_CHANGE_CFG, &len, &val);

    if (rc) return;

    if (val == LE_GATT_CLIENT_CFG_INDICATION)
    {
        uint16_t handle[2];

        LeGattGetAttrVal(g_tGattSvcHandle.ptSvcDef, GATT_SVC_IDX_SERVICE_CHANGE_VAL, &len, handle);

        if (!handle[0] || !handle[1]) return;

        LeGattCharValIndicate(u16ConnHdl, LeGattGetAttrHandle(g_tGattSvcHandle.ptSvcDef, GATT_SVC_IDX_SERVICE_CHANGE_VAL), 4, (uint8_t *)handle);
    }
}

/*************************************************************************
* FUNCTION:
*   GATT_Svc_GattDispatchHandler
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
static T_OplErr GATT_Svc_GattDispatchHandler(MESSAGEID tId, MESSAGE tMsg)
{
    switch(tId)
    {
        case LE_GATT_MSG_ACCESS_READ_IND:
        {
            GATT_Svc_GattDispatchReadHandler((LE_GATT_MSG_ACCESS_READ_IND_T *)tMsg);
            break;
        }
        
        case LE_GATT_MSG_ACCESS_WRITE_IND:
        {
            GATT_Svc_GattDispatchWriteHandler((LE_GATT_MSG_ACCESS_WRITE_IND_T *)tMsg);
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
*   GATT_Svc_Init
*
* DESCRIPTION:
*   initiate gatt service
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void GATT_Svc_Init(void)
{
    g_tGattSvcHandle.patSvcDb = gGattSvcDb;
    g_tGattSvcHandle.u8SvcDbSize = GATT_SVC_IDX_TOTAL;
    g_tGattSvcHandle.ptSvcGattDispatchHandler = GATT_Svc_GattDispatchHandler;

    if(OPL_OK != Opl_Ble_Service_Assign(&g_tGattSvcHandle))
    {
        OPL_LOG_ERRO(GATT_SVC, "GATT Service assign fail");
    }
}
