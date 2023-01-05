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
*  ble_mngr_api.h
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

#include "ble.h"
#include "ble_msg.h"
#include "ble_hci_if.h"
#include "ble_cm_if.h"
#include "ble_smp_if.h"
#include "ble_gap_if.h"
#include "ble_gatt_if.h"
#include "ble_util.h"
#include "sys_common_api.h"
#include "opl_err.h"
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __BLE_MNGR_API_H__
#define __BLE_MNGR_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (BM_ENABLED == 1)

// <o> BM_DEF_DESIRED_MIN_ADV_INTERVAL - minimum advertisment interval (units of 0.625ms)
#ifndef BM_DEF_DESIRED_MIN_ADV_INTERVAL
#define BM_DEF_DESIRED_MIN_ADV_INTERVAL             (100)
#endif

// <o> BM_DEF_DESIRED_MAX_ADV_INTERVAL - maximum advertisment interval (units of 0.625ms)
#ifndef BM_DEF_DESIRED_MAX_ADV_INTERVAL
#define BM_DEF_DESIRED_MAX_ADV_INTERVAL             (200)
#endif

// <o> BM_DEF_DESIRED_SLAVE_LATENCY - slave latency
#ifndef BM_DEF_DESIRED_SLAVE_LATENCY
#define BM_DEF_DESIRED_SLAVE_LATENCY                (0)
#endif

// <o> BM_DEF_DESIRED_SUPERVERSION_TIMEOUT - superversion timeout time
#ifndef BM_DEF_DESIRED_SUPERVERSION_TIMEOUT
#define BM_DEF_DESIRED_SUPERVERSION_TIMEOUT         (500)
#endif


// Minimum advertisment interval (units of 0.625ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL           BM_DEF_DESIRED_MIN_ADV_INTERVAL
// Maximum advertisment interval (units of 0.625ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL           BM_DEF_DESIRED_MAX_ADV_INTERVAL
// Maximum advertisment interval (units of 0.625ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY               BM_DEF_DESIRED_SLAVE_LATENCY
// Maximum advertisment interval (units of 0.625ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SUPERVERSION_TIMEOUT        BM_DEF_DESIRED_SUPERVERSION_TIMEOUT

#define LE_GATT_DATA_OUT_BUF_CNT                    50
#define LE_GATT_DATA_OUT_BUF_BLOCK_SIZE             20
#define LE_GATT_DATA_OUT_BUF_SIZE				    (LE_GATT_DATA_OUT_BUF_CNT * LE_GATT_DATA_OUT_BUF_BLOCK_SIZE)
#define LE_GATT_DATA_SENDING_BUF_MAX_SIZE           256

#define BLE_ADV_SCAN_BUF_SIZE                       31

#define BM_USER_EVT_BASE                            0x1700

#define BLE_MNGR_SM_PARAM_IO_CAP                    LE_SM_IO_CAP_NO_IO
#define BLE_MNGR_SM_PARAM_MITM                      false
#define BLE_MNGR_SM_PARAM_SC                        false
#define BLE_MNGR_SM_PARAM_BOND                      true

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

// BLE manager unsolicited callback typedef

typedef enum E_BmUnslctedEvtType
{
    USLCTED_CB_EVT_BLE_INIT             = 0,
    USLCTED_CB_EVT_BLE_ENT_ADVERTISE,
    USLCTED_CB_EVT_BLE_EXI_ADVERTISE,
    USLCTED_CB_EVT_BLE_CONNECTED,
    USLCTED_CB_EVT_BLE_DISCONNECT,
} T_BmReqEventList;

typedef struct
{
	uint8_t         u8Len;
	uint8_t         au8Buf[BLE_ADV_SCAN_BUF_SIZE];
} BLE_ADV_SCAN_DATA_T;

typedef struct
{
	uint32_t	    u32DataLen;
	uint8_t		    *u8Data;
} BLE_MESSAGE_T;

typedef struct
{
	uint16_t        send_hdl;
    uint8_t         u8WriteIdx;
    uint8_t         u8ReadIdx;
    uint8_t         u8CurrentPackageCnt[LE_GATT_DATA_OUT_BUF_CNT];
    uint8_t         u8TotalPackageCnt[LE_GATT_DATA_OUT_BUF_CNT];
    uint8_t         u8BufDataLen[LE_GATT_DATA_OUT_BUF_CNT];
	uint8_t         buf[LE_GATT_DATA_OUT_BUF_SIZE];
} BLE_DATA_OUT_STORE_T;

typedef struct
{
    TASKPACK                task;
    LE_BT_ADDR_T            bt_addr;
    BLE_DATA_OUT_STORE_T    store;
    uint16_t                u16CurrentState;
    uint16_t                u16HistoryState;
    uint16_t                conn_hdl;
    uint16_t                curr_mtu;
    uint16_t                min_itvl;
    uint16_t                max_itvl;
    uint16_t                latency;
    uint16_t                sv_tmo;
    bool                    encrypted;
    bool                    paired;
} BLE_APP_DATA_T;

typedef void (* T_BmUslctedCbFp)(uint16_t u16EvtType, T_OplErr tEvtRst, uint8_t *pu8Data, uint32_t u32DataLen);

typedef void (* T_BmSvcDataProcessHandlerFp)(uint16_t u16Evt, MESSAGE tMsg);

typedef T_OplErr (* T_BmSvcGattDispatchHandlerFp)(MESSAGEID tId, MESSAGE tMsg);

typedef struct S_BmSvcHandle
{
    LE_GATT_SERVICE_T             *ptSvcDef;
    LE_GATT_ATTR_T                *patSvcDb;
    uint8_t                       u8SvcDbSize;
    uint8_t                       u8Reserved;
    uint16_t                      u16SvcDataEvtBase;
    uint16_t                      u16SvcDataEvtTop;
    T_BmSvcDataProcessHandlerFp   ptSvcDataProcessHandler;
    T_BmSvcGattDispatchHandlerFp  ptSvcGattDispatchHandler;
} T_BmSvcHandle;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

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
T_OplErr Opl_Ble_Init_Req(uint8_t u8AutoAdvEn);

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
T_OplErr Opl_Ble_Start_Req(uint8_t u8AutoAdvEn);

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
T_OplErr Opl_Ble_Stop_Req(void);

/*************************************************************************
* FUNCTION:
*   Opl_Ble_Uslctd_CB_Reg
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
T_OplErr Opl_Ble_Uslctd_CB_Reg(T_BmUslctedCbFp fpUslctdCb);

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
T_OplErr Opl_Ble_Send_Message(uint16_t u16Event, uint8_t *u8Data, uint32_t u32DataLen, uint32_t u32DelayMs);

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
T_OplErr Opl_Ble_Service_Assign(T_BmSvcHandle *ptBmSvcHandle);

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
T_OplErr Opl_Ble_Advertise_Data_Set(uint8_t *pau8AdvData, uint8_t u8AdvDataLen);

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
T_OplErr Opl_Ble_ScanRsp_Data_Set(uint8_t *pau8ScanRspData, uint8_t u8ScanRspDataLen);

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
BLE_APP_DATA_T *Opl_Ble_EntityGet(void);

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
T_OplErr Opl_Ble_MacAddr_Write(uint8_t *pau8Data);

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
T_OplErr Opl_Ble_MacAddr_Read(uint8_t *pau8Data);

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

#endif /* BM_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* __BLE_MNGR_API_H__ */
