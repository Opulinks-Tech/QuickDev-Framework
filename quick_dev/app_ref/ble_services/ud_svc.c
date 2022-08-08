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
*  ud_pf.c
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
#include "ble_mngr_api.h"
#include "ble_gatt_if.h"
#include "ble_uuid.h"
#include "log.h"
#include "ud_svc.h"

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

static uint16_t g_UdSvcUuid                      = UD_SVC_SERVICE_UUID;

static uint16_t g_UdSvcTxCharUuid                = UD_SVC_TX_CHAR_UUID;
static uint8_t  g_UdSvcTxCharVal[]               = CHAR_DECL_UUID16_ATTR_VAL(LE_GATT_CHAR_PROP_WR, UD_SVC_TX_CHAR_UUID);
static uint8_t  g_UdSvcTxVal[LE_ATT_MAX_MTU]     = {0};

static uint16_t g_UdSvcRxCharUuid                = UD_SVC_RX_CHAR_UUID;
static uint8_t  g_UdSvcRxCharVal[]               = CHAR_DECL_UUID16_ATTR_VAL(LE_GATT_CHAR_PROP_NTF, UD_SVC_RX_CHAR_UUID);
static uint8_t  g_UdSvcRxVal[LE_ATT_MAX_MTU]     = {0};
static uint16_t g_UdSvcRxClientCfg               = 1;

// UD service definition
static LE_GATT_ATTR_T g_UdSvcDb[UD_SVC_IDX_TOTAL] =
{
    // UD service declaration
    [UD_SVC_IDX_SVC]                   = PRIMARY_SERVICE_DECL_UUID16(&g_UdSvcUuid),
    // UD service tx characteristic 
    [UD_SVC_IDX_TX_CHAR]               = CHARACTERISTIC_DECL_UUID16(g_UdSvcTxCharVal),
    [UD_SVC_IDX_TX_VAL]                = CHARACTERISTIC_UUID16(&g_UdSvcTxCharUuid, LE_GATT_PERMIT_AUTHOR_WRITE, sizeof(g_UdSvcTxVal), 0, g_UdSvcTxVal),
    // UD service rx characteristic
    [UD_SVC_IDX_RX_CHAR]               = CHARACTERISTIC_DECL_UUID16(g_UdSvcRxCharVal),
    [UD_SVC_IDX_RX_VAL]                = CHARACTERISTIC_UUID16(&g_UdSvcRxCharUuid, LE_GATT_PERMIT_AUTHOR_READ, sizeof(g_UdSvcRxVal), sizeof(g_UdSvcRxVal), g_UdSvcRxVal),
    [UD_SVC_IDX_RX_CFG]                = CHAR_CLIENT_CONFIG_DESCRIPTOR(LE_GATT_PERMIT_AUTHOR_READ | LE_GATT_PERMIT_AUTHOR_WRITE, &g_UdSvcRxClientCfg)
};

static T_BmSvcHandle g_tUdSvcHandle = {0};

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

/*************************************************************************
* FUNCTION:
*   UD_Svc_GattDispatchReadHandler
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
static void UD_Svc_GattDispatchReadHandler(LE_GATT_MSG_ACCESS_READ_IND_T *ind)
{
    // process the read access activity in each characteristic
    uint8_t u8AttErr = 0;
    uint16_t u16AttrId = ind->handle - g_tUdSvcHandle.ptSvcDef->startHdl;

    // printf("UD Svc attId = %d op = %x offset = %d\r\n", u16AttrId, ind->flag, ind->offset);

    switch(u16AttrId)
    {
        case UD_SVC_IDX_RX_VAL:
        case UD_SVC_IDX_RX_CFG:
        {
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
*   UD_Svc_GattDispatchWriteHandler
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
static void UD_Svc_GattDispatchWriteHandler(LE_GATT_MSG_ACCESS_WRITE_IND_T *ind)
{
    // process the write access activity in each characteristic
    uint8_t u8AttErr = 0;
    uint16_t u16AttrId = ind->handle - g_tUdSvcHandle.ptSvcDef->startHdl;

    // printf("UD Svc attId = %d op = %x offset = %d\r\n", u16AttrId, ind->flag, ind->offset);

    switch(u16AttrId)
    {
        case UD_SVC_IDX_TX_VAL:
        {
            // send message to app
            APP_SendMessage(APP_EVT_BLE_DATA_IND, ind->pVal, ind->len);

            break;
        }

        case UD_SVC_IDX_RX_CFG:
        {
            uint16_t u16Enable = *((uint16_t *)ind->pVal);

            if ((ind->len == 2) && (u16Enable <= 1))
            {
                LeGattChangeAttrVal(g_tUdSvcHandle.ptSvcDef, UD_SVC_IDX_RX_CFG, sizeof(u16Enable), &u16Enable);
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
*   UD_Svc_GattDispatchHandler
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
static T_OplErr UD_Svc_GattDispatchHandler(MESSAGEID tId, MESSAGE tMsg)
{
    switch(tId)
    {
        case LE_GATT_MSG_ACCESS_READ_IND:
        {
            UD_Svc_GattDispatchReadHandler((LE_GATT_MSG_ACCESS_READ_IND_T *)tMsg);
            break;
        }
        
        case LE_GATT_MSG_ACCESS_WRITE_IND:
        {
            UD_Svc_GattDispatchWriteHandler((LE_GATT_MSG_ACCESS_WRITE_IND_T *)tMsg);
            break;
        }

        case LE_GATT_MSG_NOTIFY_CFM:
        {
            OPL_LOG_INFO(UDS, "Notify Confirm");
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
*   UD_Svc_RxDataOutNotify
*
* DESCRIPTION:
*   notify data to host
*
* PARAMETERS
*   pu8Data :       [IN] data to post
*   u32DataLen :    [IN] post data lens
*
* RETURNS
*   T_OplErr :      see in opl_err.h
*
*************************************************************************/
T_OplErr UD_Svc_RxDataOutNotify(uint8_t *pu8Data, uint32_t u32DataLen)
{
    LE_ERR_STATE status;

    uint16_t u16NotiAttrId = g_tUdSvcHandle.ptSvcDef->startHdl + UD_SVC_IDX_RX_VAL;

    // update rx data to attribute value
    LeGattChangeAttrVal(g_tUdSvcHandle.ptSvcDef, (uint16_t)UD_SVC_IDX_RX_VAL, u32DataLen, pu8Data);

    // notify rx data to host
    status = LeGattCharValNotify(Opl_Ble_EntityGet()->conn_hdl, u16NotiAttrId, u32DataLen, pu8Data);

    if (status != SYS_ERR_SUCCESS)
    {
        OPL_LOG_ERRO(BAS, "Ble sending data fail");
        return OPL_ERR;
    }

    return OPL_OK;
}

// /*************************************************************************
// * FUNCTION:
// *   UD_Svc_MsgDataProcessHandler
// *
// * DESCRIPTION:
// *   message processer to handle received event from ble manager 
// *
// * PARAMETERS
// *   u16Evt :     [IN] event ID
// *   tMsg :       [IN] data message
// *
// * RETURNS
// *   none
// *
// *************************************************************************/
// static void UD_Svc_MsgDataProcessHandler(uint16_t u16Evt, MESSAGE tMsg)
// {
//     printf("UD SVC Data Process\r\n");
//     switch(u16Evt)
//     {
//         case UD_SVC_EVT_XXXXX:
//         {
//             BLE_MESSAGE_T *ptMsg = (BLE_MESSAGE_T *)tMsg;

//             uint8_t test = *((uint8_t *)ptMsg->u8Data);
//             printf("XXXXX %d\r\n", test);

//             break;
//         }
//         case UD_SVC_EVT_OOOOO:
//         {
//             BLE_MESSAGE_T *ptMsg = (BLE_MESSAGE_T *)tMsg;

//             uint8_t test = *((uint8_t *)ptMsg->u8Data);
//             printf("OOOOO %d\r\n", test);

//             break;
//         }
        
//         default:
//             printf("Unrecognize event index %d\r\n", u16Evt);
//             break;
//     }
// }

/*************************************************************************
* FUNCTION:
*   UD_Svc_Init
*
* DESCRIPTION:
*   initiate user define service
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void UD_Svc_Init(void)
{
    g_tUdSvcHandle.patSvcDb = g_UdSvcDb;
    g_tUdSvcHandle.u8SvcDbSize = UD_SVC_IDX_TOTAL;
    g_tUdSvcHandle.ptSvcGattDispatchHandler = UD_Svc_GattDispatchHandler;

    // g_tUdSvcHandle.u16SvcDataEvtBase = UD_SVC_EVT_BASE;
    // g_tUdSvcHandle.u16SvcDataEvtTop = UD_SVC_EVT_TOTAL;
    // g_tUdSvcHandle.ptSvcDataProcessHandler = UD_Svc_MsgDataProcessHandler;

    if(OPL_OK != Opl_Ble_Service_Assign(&g_tUdSvcHandle))
    {
        OPL_LOG_ERRO(BAS, "UD Service assign fail\r\n");
    }
}
