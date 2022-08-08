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
*  gat_svc.c
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
#include "gap_svc.h"
#include "log.h"
#include "qd_config.h"
#include "qd_module.h"

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

// This is used for GAP service
static uint16_t gGapSvcUuid = ATT_SVC_GENERIC_ACCESS;

static uint16_t gGapDeviceNameUuid          = ATT_CHAR_DEVICE_NAME;
static uint8_t  gGapDeviceNameCharVal[]     = CHAR_DECL_UUID16_ATTR_VAL(LE_GATT_CHAR_PROP_RD | LE_GATT_CHAR_PROP_WR, ATT_CHAR_DEVICE_NAME);
static uint8_t  gGapDeviceNameVal[31]       = BLE_GAP_PF_DEVICE_NAME;

static uint16_t gGapAppearanceUuid          = ATT_CHAR_APPEARANCE;
static uint8_t  gGapAppearanceCharVal[]     = CHAR_DECL_UUID16_ATTR_VAL(LE_GATT_CHAR_PROP_RD, ATT_CHAR_APPEARANCE);
static uint8_t  gGapAppearanceVal[2]        = {0, 0};

static uint16_t gGapConnParamUuid           = ATT_CHAR_PERIPH_PREF_CON_PARAM;
static uint8_t  gGapConnParamCharVal[]      = CHAR_DECL_UUID16_ATTR_VAL(LE_GATT_CHAR_PROP_RD, ATT_CHAR_PERIPH_PREF_CON_PARAM);
static uint16_t gGapConnParamVal[4]         = {DEFAULT_DESIRED_MIN_CONN_INTERVAL, DEFAULT_DESIRED_MAX_CONN_INTERVAL, DEFAULT_DESIRED_SLAVE_LATENCY, DEFAULT_DESIRED_SUPERVERSION_TIMEOUT};

static LE_GATT_ATTR_T gGapSvcDb[GAP_SVC_IDX_TOTAL] =
{
    // GAP Service Declaration
    [GAP_SVC_IDX_SVC]                  = PRIMARY_SERVICE_DECL_UUID16(&gGapSvcUuid),
    // GAP Device Name Characteristic
    [GAP_SVC_IDX_DEVICE_NAME_CHAR]     = CHARACTERISTIC_DECL_UUID16(gGapDeviceNameCharVal),
    [GAP_SVC_IDX_DEVICE_NAME_VAL]      = CHARACTERISTIC_UUID16(&gGapDeviceNameUuid, LE_GATT_PERMIT_AUTHOR_READ | LE_GATT_PERMIT_AUTHOR_WRITE, sizeof(gGapDeviceNameVal), sizeof(BLE_GAP_PF_DEVICE_NAME) - 1, gGapDeviceNameVal),
    // GAP Appearance Characteristic
    [GAP_SVC_IDX_APPEARANCE_CHAR]      = CHARACTERISTIC_DECL_UUID16(gGapAppearanceCharVal),
    [GAP_SVC_IDX_APPEARANCE_VAL]       = CHARACTERISTIC_UUID16(&gGapAppearanceUuid, LE_GATT_PERMIT_READ, 0, 2, gGapAppearanceVal),
    // GAP Connection Parameter Characteristic Declaration
    [GAP_SVC_IDX_CONN_PARAM_CHAR]      = CHARACTERISTIC_DECL_UUID16(gGapConnParamCharVal),
    [GAP_SVC_IDX_CONN_PARAM_VAL]       = CHARACTERISTIC_UUID16(&gGapConnParamUuid, LE_GATT_PERMIT_READ, 0, 8, gGapConnParamVal)
};

static T_BmSvcHandle g_tGapSvcHandle = {0};

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

/*************************************************************************
* FUNCTION:
*   GAP_Svc_GattDispatchReadHandler
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
static void GAP_Svc_GattDispatchReadHandler(LE_GATT_MSG_ACCESS_READ_IND_T *ind)
{
    uint8_t attErr = 0;
    uint16_t attrid = ind->handle - g_tGapSvcHandle.ptSvcDef->startHdl;
        
    switch (attrid)
    {
        case GAP_SVC_IDX_DEVICE_NAME_VAL:
        {
            // OPL_LOG_DEBG(GAP_SVC, "GAP_SVC_IDX_DEVICE_NAME_VAL offset=%d", ind->offset);
        }
        break;
        
        default:
            attErr = LE_ATT_ERR_READ_NOT_PERMITTED;
        break;
    }
    
    LeGattAccessReadRsp(ind->conn_hdl, ind->handle, attErr);
}

/*************************************************************************
* FUNCTION:
*   GAP_Svc_GattDispatchWriteHandler
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
static void GAP_Svc_GattDispatchWriteHandler(LE_GATT_MSG_ACCESS_WRITE_IND_T *ind)
{
    uint8_t attErr = 0;
    uint16_t attrid = ind->handle - g_tGapSvcHandle.ptSvcDef->startHdl;

    switch (attrid)
    {
        case GAP_SVC_IDX_DEVICE_NAME_VAL:
        {
            if (ind->offset > 31)
            {
                attErr = LE_ATT_ERR_INVALID_OFFSET;
            }
            else if ((ind->offset + ind->len) > 31)
            {
                attErr = LE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            else
            {
                // It could change or modify the attribute value, because the length of device name is variable length. (max length is not equal zero.)
                LeGattChangeAttrVal(g_tGapSvcHandle.ptSvcDef, attrid, ind->len, ind->pVal);
            }
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
*   GAP_Svc_GattDispatchHandler
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
static T_OplErr GAP_Svc_GattDispatchHandler(MESSAGEID tId, MESSAGE tMsg)
{
    switch(tId)
    {
        case LE_GATT_MSG_ACCESS_READ_IND:
        {
            GAP_Svc_GattDispatchReadHandler((LE_GATT_MSG_ACCESS_READ_IND_T *)tMsg);
            break;
        }
        
        case LE_GATT_MSG_ACCESS_WRITE_IND:
        {
            GAP_Svc_GattDispatchWriteHandler((LE_GATT_MSG_ACCESS_WRITE_IND_T *)tMsg);
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
*   GAP_Svc_Init
*
* DESCRIPTION:
*   initiate gap service
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void GAP_Svc_Init(void)
{
    g_tGapSvcHandle.patSvcDb = gGapSvcDb;
    g_tGapSvcHandle.u8SvcDbSize = GAP_SVC_IDX_TOTAL;
    g_tGapSvcHandle.ptSvcGattDispatchHandler = GAP_Svc_GattDispatchHandler;

    if(OPL_OK != Opl_Ble_Service_Assign(&g_tGapSvcHandle))
    {
        OPL_LOG_ERRO(GAP_SVC, "GAP Service assign fail");
    }
}
