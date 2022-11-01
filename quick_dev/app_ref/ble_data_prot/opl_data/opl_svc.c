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
*  opl_svc.c
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
#include "opl_svc.h"
#include "opl_data_prot.h"
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (OPL_DATA_ENABLED == 1)

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

static uint16_t g_OplSvcUuid                      = OPL_SVC_SERVICE_UUID;

static uint16_t g_OplSvcTxCharUuid                = OPL_SVC_DATA_IN_UUID;
static uint8_t  g_OplSvcTxCharVal[]               = CHAR_DECL_UUID16_ATTR_VAL(LE_GATT_CHAR_PROP_WR_NO_RESP | LE_GATT_CHAR_PROP_WR, OPL_SVC_DATA_IN_UUID);
static uint8_t  g_OplSvcTxVal[LE_ATT_MAX_MTU]     = {0};

static uint16_t g_OplSvcRxCharUuid                = OPL_SVC_DATA_OUT_UUID;
static uint8_t  g_OplSvcRxCharVal[]               = CHAR_DECL_UUID16_ATTR_VAL(LE_GATT_CHAR_PROP_NTF, OPL_SVC_DATA_OUT_UUID);
static uint8_t  g_OplSvcRxVal[LE_ATT_MAX_MTU]     = {0};
static uint16_t g_OplSvcRxClientCfg               = 1;

// OPL service definition
static LE_GATT_ATTR_T g_OplSvcDb[OPL_SVC_IDX_TOTAL] =
{
    // OPL service declaration
    [OPL_SVC_IDX_SVC]                   = PRIMARY_SERVICE_DECL_UUID16(&g_OplSvcUuid),
    // OPL service tx characteristic 
    [OPL_SVC_IDX_DATA_IN_CHAR]          = CHARACTERISTIC_DECL_UUID16(g_OplSvcTxCharVal),
    [OPL_SVC_IDX_DATA_IN_VAL]           = CHARACTERISTIC_UUID16(&g_OplSvcTxCharUuid, LE_GATT_PERMIT_AUTHOR_WRITE, sizeof(g_OplSvcTxVal), 0, g_OplSvcTxVal),
    // OPL service rx characteristic
    [OPL_SVC_IDX_DATA_OUT_CHAR]         = CHARACTERISTIC_DECL_UUID16(g_OplSvcRxCharVal),
    [OPL_SVC_IDX_DATA_OUT_VAL]          = CHARACTERISTIC_UUID16(&g_OplSvcRxCharUuid, 0, sizeof(g_OplSvcRxVal), 0, g_OplSvcRxVal),
    [OPL_SVC_IDX_DATA_OUT_CFG]          = CHAR_CLIENT_CONFIG_DESCRIPTOR(LE_GATT_PERMIT_AUTHOR_READ | LE_GATT_PERMIT_AUTHOR_WRITE, &g_OplSvcRxClientCfg)
};

static T_BmSvcHandle g_tOplSvcHandle = {0};

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
static void OPL_Svc_GattDispatchReadHandler(LE_GATT_MSG_ACCESS_READ_IND_T *ind)
{
    // process the read access activity in each characteristic
    uint8_t u8AttErr = 0;
    uint16_t u16AttrId = ind->handle - g_tOplSvcHandle.ptSvcDef->startHdl;

    switch(u16AttrId)
    {
        case OPL_SVC_IDX_DATA_OUT_CFG:
        {
            uint16_t u16Enable;
            uint16_t u16Len = 0;

            LeGattGetAttrVal(g_tOplSvcHandle.ptSvcDef, OPL_SVC_IDX_DATA_OUT_CFG, &u16Len, &u16Enable);

            OPL_LOG_INFO(OPL, "Svc data out config val=%d", u16Enable);

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
static void OPL_Svc_GattDispatchWriteHandler(LE_GATT_MSG_ACCESS_WRITE_IND_T *ind)
{
    // process the write access activity in each characteristic
    uint8_t u8AttErr = 0;
    uint16_t u16AttrId = ind->handle - g_tOplSvcHandle.ptSvcDef->startHdl;

    switch(u16AttrId)
    {
        case OPL_SVC_IDX_DATA_IN_VAL:
        {
            APP_SendMessage(APP_EVT_BLE_DATA_IND, ind->pVal, ind->len);

            break;
        }

        case OPL_SVC_IDX_DATA_OUT_CFG:
        {
            uint16_t u16Enable = *((uint16_t *)ind->pVal);

            if ((ind->len == 2) && (u16Enable <= 1))
            {
                LeGattChangeAttrVal(g_tOplSvcHandle.ptSvcDef, OPL_SVC_IDX_DATA_OUT_CFG, sizeof(u16Enable), &u16Enable);
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
static T_OplErr OPL_Svc_DataCopyToBuf(uint16_t len, uint8_t *data)
{
    BLE_DATA_OUT_STORE_T *pstBleDataOut = &(Opl_Ble_EntityGet()->store);
    uint16_t u16CopyPos = 0;
    uint16_t u16CopySize = 0;
    uint16_t u16RemainCopyLen = len;
    uint8_t u8DivideCurrentCnt = 1; // start 1
    uint8_t u8DivideTotalCnt = 1;   // start 1

#if 0
    uint8_t u8aPrintBuf[256] = {0};
    uint8_t u8PrintPos = 0;

    u8PrintPos = 0;
    u8PrintPos = sprintf((char *)&u8aPrintBuf[u8PrintPos] , "Copy buffer : ");
    for (int j=0 ; j<len ; j++)
    {
        u8PrintPos += sprintf((char *)&u8aPrintBuf[u8PrintPos] , "%02X ", data[j]);
    }
    printf("%s\r\n" , u8aPrintBuf);
#endif

    // TODO: check ble status
    // if (BLE_STATE_CONNECTED != APP_BleCurrentState()) return OPL_ERR;
	// if (g_u8BleFsmState != BLEWIFI_BLE_STATE_CONNECTED) return BLEWIFI_BLE_RET_FAIL;

    //printf("[ASH]len = %u\r\n" , len);

    if(len == 0 || data == NULL)
    {
        return OPL_OK;
    }
    else
    {
        if((len % LE_GATT_DATA_OUT_BUF_BLOCK_SIZE) == 0)
        {
            u8DivideTotalCnt = (len / LE_GATT_DATA_OUT_BUF_BLOCK_SIZE);
        }
        else
        {
            u8DivideTotalCnt = (len / LE_GATT_DATA_OUT_BUF_BLOCK_SIZE) + 1;
        }
        //printf("[ASH]u8DivideTotalIdx = %u\r\n" , u8DivideTotalIdx);
        if(u8DivideTotalCnt >= LE_GATT_DATA_OUT_BUF_CNT)
        {
            OPL_LOG_ERRO(OPL, "Ble buffer not enough");
            return OPL_ERR;
        }

        if(pstBleDataOut->u8ReadIdx > pstBleDataOut->u8WriteIdx)
        {
            if((pstBleDataOut->u8WriteIdx + u8DivideTotalCnt) >= pstBleDataOut->u8ReadIdx)
            {
                OPL_LOG_ERRO(OPL, "Ble buffer not enough");
                return OPL_ERR;
            }
        }
        else if(pstBleDataOut->u8ReadIdx < pstBleDataOut->u8WriteIdx)
        {
            if(pstBleDataOut->u8WriteIdx + u8DivideTotalCnt >= LE_GATT_DATA_OUT_BUF_CNT &&
               ((pstBleDataOut->u8WriteIdx + u8DivideTotalCnt) % LE_GATT_DATA_OUT_BUF_CNT) >= pstBleDataOut->u8ReadIdx)
            {
                OPL_LOG_ERRO(OPL, "Ble buffer not enough");
                return OPL_ERR;
            }
        }

        while(u16RemainCopyLen != 0)
        {
            pstBleDataOut->u8WriteIdx = (pstBleDataOut->u8WriteIdx+1) % LE_GATT_DATA_OUT_BUF_CNT;
            //printf("[ASH]pstBleDataOut->u8WriteIdx = %u\r\n" , pstBleDataOut->u8WriteIdx);
            if(u16RemainCopyLen <= LE_GATT_DATA_OUT_BUF_BLOCK_SIZE)
            {
                u16CopySize = u16RemainCopyLen;
            }
            else
            {
                u16CopySize = LE_GATT_DATA_OUT_BUF_BLOCK_SIZE;
            }
            //printf("[ASH]u16CopySize = %u\r\n" , u16CopySize);
            //printf("[ASH]u16CopyPos = %u\r\n" , u16CopyPos);
            //printf("[ASH]pstBleDataOut->u8WriteIdx * LE_GATT_DATA_OUT_BUF_BLOCK_SIZE = %u\r\n" , pstBleDataOut->u8WriteIdx * LE_GATT_DATA_OUT_BUF_BLOCK_SIZE);

            memcpy(&pstBleDataOut->buf[(pstBleDataOut->u8WriteIdx * LE_GATT_DATA_OUT_BUF_BLOCK_SIZE)] , &data[u16CopyPos] , u16CopySize);

            pstBleDataOut->u8CurrentPackageCnt[pstBleDataOut->u8WriteIdx] = u8DivideCurrentCnt;
            //printf("[ASH]pstBleDataOut->u8CurrentPackageCnt[pstBleDataOut->u8WriteIdx] = %u\r\n" , pstBleDataOut->u8CurrentPackageCnt[pstBleDataOut->u8WriteIdx]);
            pstBleDataOut->u8TotalPackageCnt[pstBleDataOut->u8WriteIdx] = u8DivideTotalCnt;
            //printf("[ASH]pstBleDataOut->u8TotalPackageCnt[pstBleDataOut->u8WriteIdx] = %u\r\n" , pstBleDataOut->u8TotalPackageCnt[pstBleDataOut->u8WriteIdx]);
            pstBleDataOut->u8BufDataLen[pstBleDataOut->u8WriteIdx] = u16CopySize;
            //printf("[ASH]pstBleDataOut->u8BufDataLen[pstBleDataOut->u8WriteIdx] = %u\r\n" , pstBleDataOut->u8BufDataLen[pstBleDataOut->u8WriteIdx]);
            u16CopyPos += u16CopySize;
            //printf("[ASH]u16CopyPos = %u\r\n" , u16CopyPos);
            u16RemainCopyLen = u16RemainCopyLen - u16CopySize;
            //printf("[ASH]u16RemainCopyLen = %u\r\n" , u16RemainCopyLen);
            u8DivideCurrentCnt ++;
            //printf("[ASH]u8DivideCurrentCnt = %u\r\n" , u8DivideCurrentCnt);
        }

        // BleWifi_Ble_SendAppMsgToBle(BW_APP_MSG_SEND_TO_PEER, 0, NULL);
        Opl_Ble_Send_Message(OPL_SVC_EVT_SEND_TO_PEER, NULL, 0, 0);

        return OPL_OK;
    }
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
static T_OplErr OPL_Svc_DataSendToPeer(void)
{
    uint8_t u8SendingBuf[LE_GATT_DATA_SENDING_BUF_MAX_SIZE] = {0};
    BLE_DATA_OUT_STORE_T *pstBleDataOut = &(Opl_Ble_EntityGet()->store);
    uint16_t u16CopyPos = 0;
    uint16_t u16CopySize = 0;
    uint16_t u16RemainCopySize = 0;
    uint16_t u16SendingDataSize = 0;
    LE_ERR_STATE status;

    uint16_t u16NotiAttrId = g_tOplSvcHandle.ptSvcDef->startHdl + OPL_SVC_IDX_DATA_OUT_VAL;

    u16RemainCopySize = (Opl_Ble_EntityGet()->curr_mtu) - 3; // max mtu size

    do
    {
        if(pstBleDataOut->u8ReadIdx == pstBleDataOut->u8WriteIdx) // queue is empty
        {
            return OPL_OK;
        }
        pstBleDataOut->u8ReadIdx = (pstBleDataOut->u8ReadIdx+1) % LE_GATT_DATA_OUT_BUF_CNT;
        //printf("[ASH]pstBleDataOut->u8ReadIdx = %u\r\n" , pstBleDataOut->u8ReadIdx);
        u16CopySize = pstBleDataOut->u8BufDataLen[pstBleDataOut->u8ReadIdx];
        //printf("[ASH]u16CopySize = %u\r\n" , u16CopySize);
        memcpy(&u8SendingBuf[u16CopyPos] , &pstBleDataOut->buf[(pstBleDataOut->u8ReadIdx * LE_GATT_DATA_OUT_BUF_BLOCK_SIZE)] , u16CopySize);
        u16CopyPos += u16CopySize;
        //printf("[ASH]u16CopyPos = %u\r\n" , u16CopyPos);
        u16SendingDataSize += u16CopySize;
        //printf("[ASH]u16SendingDataSize = %u\r\n" , u16SendingDataSize);
        u16RemainCopySize -= u16CopySize;
        //printf("[ASH]u16RemainCopySize = %u\r\n" , u16RemainCopySize);
    }while((pstBleDataOut->u8TotalPackageCnt[pstBleDataOut->u8ReadIdx] != pstBleDataOut->u8CurrentPackageCnt[pstBleDataOut->u8ReadIdx]) && (u16RemainCopySize >= LE_GATT_DATA_OUT_BUF_BLOCK_SIZE));

#if 0
    uint8_t u8PrintPos = 0;
    uint8_t u8aPrintBuf[256] = {0};

    u8PrintPos = 0;
    u8PrintPos = sprintf((char *)&u8aPrintBuf[u8PrintPos] , "u8aSendBuf : ");
    for (int j=0 ; j<u16SendingDataSize ; j++)
    {
        u8PrintPos += sprintf((char *)&u8aPrintBuf[u8PrintPos] , "%02X ", u8SendingBuf[j]);
    }
    printf("%s\r\n" , u8aPrintBuf);
#endif

    status = LeGattCharValNotify(Opl_Ble_EntityGet()->conn_hdl, u16NotiAttrId, u16SendingDataSize, &u8SendingBuf[0]);
    // status = LeGattCharValNotify(gTheBle.conn_hdl, pstBleDataOut->send_hdl, u16SendingDataSize, &u8SendingBuf[0]);

    if (status != SYS_ERR_SUCCESS)
    {
        OPL_LOG_ERRO(OPL, "Ble sending data fail");
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
static T_OplErr OPL_Svc_GattDispatchHandler(MESSAGEID tId, MESSAGE tMsg)
{
    switch(tId)
    {
        case LE_GATT_MSG_ACCESS_READ_IND:
        {
            OPL_Svc_GattDispatchReadHandler((LE_GATT_MSG_ACCESS_READ_IND_T *)tMsg);
            break;
        }
        
        case LE_GATT_MSG_ACCESS_WRITE_IND:
        {
            OPL_Svc_GattDispatchWriteHandler((LE_GATT_MSG_ACCESS_WRITE_IND_T *)tMsg);
            break;
        }

        case LE_GATT_MSG_NOTIFY_CFM:
        {					
            Opl_Ble_Send_Message(OPL_SVC_EVT_SEND_TO_PEER_CFM, NULL, 0, 0);
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
static void OPL_Svc_MsgDataProcessHandler(uint16_t u16Evt, MESSAGE tMsg)
{
    switch(u16Evt)
    {
        case OPL_SVC_EVT_SEND_DATA:
        {
            BLE_MESSAGE_T *msg = (BLE_MESSAGE_T *)tMsg;

            OPL_Svc_DataCopyToBuf(msg->u32DataLen, msg->u8Data);
            break;
        }

        case OPL_SVC_EVT_SEND_TO_PEER:
        {
            OPL_Svc_DataSendToPeer();
            break;
        }

        case OPL_SVC_EVT_SEND_TO_PEER_CFM:
        {
            Opl_Ble_Send_Message(OPL_SVC_EVT_SEND_TO_PEER, NULL, 0, 0);
            break;
        }

        default:
            OPL_LOG_WARN(OPL, "Unrecognize event index %d", u16Evt);
            break;
    }
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
void OPL_Svc_Init(void)
{
    g_tOplSvcHandle.patSvcDb = g_OplSvcDb;
    g_tOplSvcHandle.u8SvcDbSize = OPL_SVC_IDX_TOTAL;
    g_tOplSvcHandle.ptSvcGattDispatchHandler = OPL_Svc_GattDispatchHandler;

    g_tOplSvcHandle.u16SvcDataEvtBase = OPL_SVC_EVT_BASE;
    g_tOplSvcHandle.u16SvcDataEvtTop = OPL_SVC_EVT_TOTAL;
    g_tOplSvcHandle.ptSvcDataProcessHandler = OPL_Svc_MsgDataProcessHandler;

    if(OPL_OK != Opl_Ble_Service_Assign(&g_tOplSvcHandle))
    {
        OPL_LOG_ERRO(OPL, "OPL Service assign fail");
    }
}

#endif /* OPL_DATA_ENABLED */
