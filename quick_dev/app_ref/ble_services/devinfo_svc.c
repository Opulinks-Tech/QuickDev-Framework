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
*  devinfo_svc.c
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
#include "devinfo_svc.h"
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

// This is used for DevInfo service
static uint16_t g_DevInfoSvcUuid                            = ATT_SVC_DEVICE_INFO;

static uint16_t g_DevInfoSvcFwRevUuid                       = ATT_CHAR_FW_REV;
static uint8_t  g_DevInfoSvcFwRevCharVal[]                  = CHAR_DECL_UUID16_ATTR_VAL(LE_GATT_CHAR_PROP_RD, ATT_CHAR_FW_REV);
static uint8_t  g_DevInfoSvcFwRevVal[FW_REV_VAL_MAX_LEN]    = {0};

static uint16_t g_DevInfoSvcModelNbUuid                     = ATT_CHAR_MODEL_NB;
static uint8_t  g_DevInfoSvcModelNbCharVal[]                = CHAR_DECL_UUID16_ATTR_VAL(LE_GATT_CHAR_PROP_RD, ATT_CHAR_MODEL_NB);
static uint8_t  g_DevInfoSvcModelNbVal[MODEL_NB_VAL_MAX_LEN]= {0};

static LE_GATT_ATTR_T g_DevInfoSvcDb[DEV_INFO_SVC_IDX_TOTAL] =
{
    // DevInfo Service Declaration
    [DEV_INFO_SVC_IDX_SVC]                  = PRIMARY_SERVICE_DECL_UUID16(&g_DevInfoSvcUuid),
    // DevInfo Fw Rev Characteristic
    [DEV_INFO_SVC_IDX_FW_REV_CHAR]          = CHARACTERISTIC_DECL_UUID16(g_DevInfoSvcFwRevCharVal),
    [DEV_INFO_SVC_IDX_FW_REV_VAL]           = CHARACTERISTIC_UUID16(&g_DevInfoSvcFwRevUuid, LE_GATT_PERMIT_AUTHOR_READ, 0, sizeof(g_DevInfoSvcFwRevVal), g_DevInfoSvcFwRevVal),
    // DevInfo Model Nb Characteristic
    [DEV_INFO_SVC_IDX_MODEL_NB_CHAR]        = CHARACTERISTIC_DECL_UUID16(g_DevInfoSvcModelNbCharVal),
    [DEV_INFO_SVC_IDX_MODEL_NB_VAL]         = CHARACTERISTIC_UUID16(&g_DevInfoSvcModelNbUuid, LE_GATT_PERMIT_AUTHOR_READ, 0, sizeof(g_DevInfoSvcModelNbVal), g_DevInfoSvcModelNbVal),
};

static T_BmSvcHandle g_tDevInfoSvcHandle = {0};

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

/*************************************************************************
* FUNCTION:
*   DevInfo_Svc_GattDispatchReadHandler
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
static void DevInfo_Svc_GattDispatchReadHandler(LE_GATT_MSG_ACCESS_READ_IND_T *ind)
{
    uint8_t attErr = 0;
    uint16_t attrid = ind->handle - g_tDevInfoSvcHandle.ptSvcDef->startHdl;
        
    switch (attrid)
    {
        case DEV_INFO_SVC_IDX_FW_REV_VAL:
        case DEV_INFO_SVC_IDX_MODEL_NB_VAL:
            break;
        
        default:
            attErr = LE_ATT_ERR_READ_NOT_PERMITTED;
            break;
    }
    
    LeGattAccessReadRsp(ind->conn_hdl, ind->handle, attErr);
}

/*************************************************************************
* FUNCTION:
*   DevInfo_Svc_GattDispatchHandler
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
static T_OplErr DevInfo_Svc_GattDispatchHandler(MESSAGEID tId, MESSAGE tMsg)
{
    switch(tId)
    {
        case LE_GATT_MSG_ACCESS_READ_IND:
        {
            DevInfo_Svc_GattDispatchReadHandler((LE_GATT_MSG_ACCESS_READ_IND_T *)tMsg);
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
*   DevInfo_Svc_FirmwareRevision_Set
*
* DESCRIPTION:
*   setting firmware revision
*
* PARAMETERS
*   pu8Data :       [IN] data
*   u32DataLen :    [IN] data lens
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr DevInfo_Svc_FirmwareRevision_Set(uint8_t *pu8Data, uint32_t u32DataLen)
{
    if(sizeof(g_DevInfoSvcFwRevVal) < u32DataLen && NULL == pu8Data)
    {
        return OPL_ERR_PARAM_INVALID;
    }

    memset(&g_DevInfoSvcFwRevVal, 0, sizeof(g_DevInfoSvcFwRevVal));
    memcpy(&g_DevInfoSvcFwRevVal, pu8Data, u32DataLen);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION:
*   DevInfo_Svc_ModelNumber_Set
*
* DESCRIPTION:
*   setting model number
*
* PARAMETERS
*   pu8Data :       [IN] data to post
*   u32DataLen :    [IN] post data lens
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr DevInfo_Svc_ModelNumber_Set(uint8_t *pu8Data, uint32_t u32DataLen)
{
    if(sizeof(g_DevInfoSvcModelNbVal) < u32DataLen && NULL == pu8Data)
    {
        return OPL_ERR_PARAM_INVALID;
    }

    memset(&g_DevInfoSvcModelNbVal, 0, sizeof(g_DevInfoSvcModelNbVal));
    memcpy(&g_DevInfoSvcModelNbVal, pu8Data, u32DataLen);

    return OPL_OK;
}

/*************************************************************************
* FUNCTION:
*   DevInfo_Svc_Init
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
void DevInfo_Svc_Init(void)
{
    g_tDevInfoSvcHandle.patSvcDb = g_DevInfoSvcDb;
    g_tDevInfoSvcHandle.u8SvcDbSize = DEV_INFO_SVC_IDX_TOTAL;
    g_tDevInfoSvcHandle.ptSvcGattDispatchHandler = DevInfo_Svc_GattDispatchHandler;

    if(OPL_OK != Opl_Ble_Service_Assign(&g_tDevInfoSvcHandle))
    {
        OPL_LOG_ERRO(DEVINFO_SVC, "GAP Service assign fail");
    }
}
