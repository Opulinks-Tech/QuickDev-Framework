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
*  ble_mngr.c
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
#include "ble_msg.h"
#include "cmsis_os.h"
#include "gatt_svc.h"
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

#if 0
osSemaphoreId g_tBmUslctedCbRegSemId;
osSemaphoreId g_tBmSvcAssignRegSemId;
#endif

// definition for ble task
static BLE_APP_DATA_T g_tTheBle = {0};

static T_BmSvcHandle *g_patBmSvcHandle[BM_SVC_NUM_MAX] = {0};

static uint8_t g_u8AutoAdvFlag = false;

static uint8_t g_u8DiscByStopReqFlag = false;

static T_BmUslctedCbFp tBmUnslctedCb = NULL;

static BLE_ADV_SCAN_DATA_T g_tAdvData = {0};
static BLE_ADV_SCAN_DATA_T g_tScanRspData = {0};

// Sec 7: declaration of static function prototype

static void BM_GattMsgHandler(TASK task, MESSAGEID id, MESSAGE message);
static void BM_SmMsgHandler(TASK task, MESSAGEID id, MESSAGE message);
static void BM_CmMsgHandler(TASK task, MESSAGEID id, MESSAGE message);
static void BM_AppMsgHandler(TASK task, MESSAGEID id, MESSAGE message);
static void BM_TaskHandler(TASK task, MESSAGEID id, MESSAGE message);
static T_OplErr BM_TrigDispatchHandler(uint16_t u16Hdl, MESSAGEID id, MESSAGE message);
static T_OplErr BM_AdvertiseSwitch(bool blOnOff);
static void BM_StateChange(T_BmStateList tStateId);
static uint8_t BM_ServiceIsAssigned(uint8_t u8Index);

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
static void BM_GattMsgHandler(TASK task, MESSAGEID id, MESSAGE message)
{
    switch (id)
    {
        case LE_GATT_MSG_INIT_CFM:
        {
            BM_LOG_DEBG("MSG_INIT_CFM");

            // register service by assigned service
            uint8_t u8Cnt = 0;
            for(; u8Cnt < BM_SVC_NUM_MAX; u8Cnt ++)
            {
                // check assigned service
                if(true == BM_ServiceIsAssigned(u8Cnt))
                {
                    g_patBmSvcHandle[u8Cnt]->ptSvcDef = LeGattRegisterService(g_patBmSvcHandle[u8Cnt]->patSvcDb, g_patBmSvcHandle[u8Cnt]->u8SvcDbSize);
                    
                    if(g_patBmSvcHandle[u8Cnt]->ptSvcDef)
                    {
                        BM_LOG_DEBG("Register %d service ok", u8Cnt);
                    }
                    else
                    {
                        BM_LOG_DEBG("Register %d service fail", u8Cnt);
                    }
                }
            }
        }
        break;

        case LE_GATT_MSG_ACCESS_READ_IND:
        {
            BM_LOG_DEBG("MSG_ACCESS_READ_IND");

            LE_GATT_MSG_ACCESS_READ_IND_T *ind = (LE_GATT_MSG_ACCESS_READ_IND_T *)message;

            // trigger dispatch handler
            if(OPL_OK != BM_TrigDispatchHandler(ind->handle, id, message))
            {
                // if no any service dispatch handler been active, then send read response with not permitted
                LeGattAccessReadRsp(ind->conn_hdl, ind->handle, LE_ATT_ERR_READ_NOT_PERMITTED);
            }
        }
        break;

        case LE_GATT_MSG_ACCESS_WRITE_IND:
        {
            BM_LOG_DEBG("MSG_ACCESS_WRITE_IND");

            LE_GATT_MSG_ACCESS_WRITE_IND_T *ind = (LE_GATT_MSG_ACCESS_WRITE_IND_T *)message;

            // trigger dispatch handler
            if(OPL_OK != BM_TrigDispatchHandler(ind->handle, id, message))
            {
                // if no any service dispatch handler been active, then send read response with not permitted
                LeGattAccessReadRsp(ind->conn_hdl, ind->handle, LE_ATT_ERR_WRITE_NOT_PERMITTED);
            }
        }
        break;

        case LE_GATT_MSG_NOTIFY_IND:
        {
            BM_LOG_DEBG("MSG_NOTIFY_IND");

            LE_GATT_MSG_NOTIFY_IND_T *ind = (LE_GATT_MSG_NOTIFY_IND_T *)message;

            // trigger dispatch handler
            if(OPL_OK != BM_TrigDispatchHandler(ind->handle, id, message))
            {
                // if no any service dispatch handler been active, then send invalid handle
                LeGattAccessReadRsp(ind->conn_hdl, ind->handle, LE_ATT_ERR_INVALID_HANDLE);
            }
        }
        break;

        case LE_GATT_MSG_NOTIFY_CFM:
        {
            BM_LOG_DEBG("MSG_NOTIFY_CFM");

            LE_GATT_MSG_NOTIFY_CFM_T *ind = (LE_GATT_MSG_NOTIFY_CFM_T *)message;

            // trigger dispatch handler
            if(OPL_OK != BM_TrigDispatchHandler(ind->handle, id, message))
            {
                // if no any service dispatch handler been active, then send invalid handle
                LeGattAccessReadRsp(ind->conn_hdl, ind->handle, LE_ATT_ERR_INVALID_HANDLE);
            }
        }
        break;

        case LE_GATT_MSG_INDICATE_IND:
        {
            BM_LOG_DEBG("MSG_INDICATE_IND");

            LE_GATT_MSG_INDICATE_IND_T *ind = (LE_GATT_MSG_INDICATE_IND_T *)message;

            // trigger dispatch handler
            if(OPL_OK != BM_TrigDispatchHandler(ind->handle, id, message))
            {
                // if no any service dispatch handler been active, then send invalid handle
                LeGattAccessReadRsp(ind->conn_hdl, ind->handle, LE_ATT_ERR_INVALID_HANDLE);
            }
        }
        break;

        case LE_GATT_MSG_EXCHANGE_MTU_IND:
        {
            LE_GATT_MSG_EXCHANGE_MTU_IND_T *ind = (LE_GATT_MSG_EXCHANGE_MTU_IND_T *)message;
            BM_LOG_DEBG("MSG_EXCHANGE_MTU_IND client mtu=%d", ind->client_rx_mtu);
            LeGattExchangeMtuRsp(ind->conn_hdl, LE_ATT_MAX_MTU);
        }
        break;

        case LE_GATT_MSG_EXCHANGE_MTU_CFM:
        {
            LE_GATT_MSG_EXCHANGE_MTU_CFM_T *cfm = (LE_GATT_MSG_EXCHANGE_MTU_CFM_T *)message;
            BM_LOG_DEBG("MSG_EXCHANGE_MTU_CFM curr mtu=%d", cfm->current_rx_mtu);
            BM_EntityGet()->curr_mtu = cfm->current_rx_mtu;
        }
        break;

        case LE_GATT_MSG_CONFIRMATION_CFM:
        {
            LE_GATT_MSG_CONFIRMATION_CFM_T *cfm = (LE_GATT_MSG_CONFIRMATION_CFM_T *)message;
            
            // incase of comiler warning
            if(cfm){}
            
            BM_LOG_DEBG("MSG_CONFIRMATION_CFM curr handle=%d", cfm->handle);
        }
        break;

        case LE_GATT_MSG_OPERATION_TIMEOUT:
        {
            LE_GATT_MSG_OPERATION_TIMEOUT_T *ind = (LE_GATT_MSG_OPERATION_TIMEOUT_T *)message;
            
            // incase of comiler warning
            if(ind){}
            
            BM_LOG_DEBG("MSG_OPERATION_TIMEOUT op=%x", ind->att_op);
        }
        break;

        default:
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
static void BM_SmMsgHandler(TASK task, MESSAGEID id, MESSAGE message)
{
    switch (id)
    {
        case LE_SMP_MSG_PAIRING_ACTION_IND:
        {
            LE_SMP_MSG_PAIRING_ACTION_IND_T *ind = (LE_SMP_MSG_PAIRING_ACTION_IND_T *)message;
            BM_LOG_DEBG("MSG_PAIRING_ACTION_IND hdl=%x sc=%d action=%d", ind->conn_hdl, ind->sc, ind->action);
            
            LeSmpSecurityRsp(ind->conn_hdl, TRUE);
        }
        break;

        case LE_SMP_MSG_ENCRYPTION_CHANGE_IND:
        {
            LE_SMP_MSG_ENCRYPTION_CHANGE_IND_T *ind = (LE_SMP_MSG_ENCRYPTION_CHANGE_IND_T *)message;

            BM_LOG_DEBG("MSG_ENCRYPTION_CHANGE_IND enable=%d", ind->enable);
            g_tTheBle.encrypted = ind->enable;
        }
        break;
        
        case LE_SMP_MSG_ENCRYPTION_REFRESH_IND:
        {
            LE_SMP_MSG_ENCRYPTION_REFRESH_IND_T *ind = (LE_SMP_MSG_ENCRYPTION_REFRESH_IND_T *)message;
            
            // incase of compiler warning
            if(ind){}
            
            BM_LOG_DEBG("MSG_ENCRYPTION_REFRESH_IND status=%x", ind->status);
        }
        break;
        
        case LE_SMP_MSG_PAIRING_COMPLETE_IND:
        {
            LE_SMP_MSG_PAIRING_COMPLETE_IND_T *ind = (LE_SMP_MSG_PAIRING_COMPLETE_IND_T *)message;

            BM_LOG_DEBG("MSG_PAIRING_COMPLETE_IND status=%x", ind->status);

            if (ind->status == SYS_ERR_SUCCESS)
            {
                g_tTheBle.paired = TRUE;
            }
        }
        break;

        default:
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
static void BM_CmMsgHandler(TASK task, MESSAGEID id, MESSAGE message)
{
    switch (id)
    {
        case LE_CM_MSG_INIT_COMPLETE_CFM:
        {
            BM_LOG_DEBG("MSG_INIT_COMPLETE_CFM");
            LeGattInit(&g_tTheBle.task);
            LeSmpInit(&g_tTheBle.task);

            BM_AdvParamSet(LE_HCI_ADV_TYPE_ADV_IND, LE_HCI_OWN_ADDR_PUBLIC, 0, LE_HCI_ADV_FILT_NONE, g_tTheBle.min_itvl, g_tTheBle.max_itvl);

            LeSmpSetDefaultConfig(BLE_MNGR_SM_PARAM_IO_CAP, BLE_MNGR_SM_PARAM_MITM, BLE_MNGR_SM_PARAM_SC, BLE_MNGR_SM_PARAM_BOND);
        }
        break;
        
        case LE_CM_MSG_SET_ADVERTISING_DATA_CFM:
        {
            BM_LOG_DEBG("MSG_SET_ADVERTISING_DATA_CFM status=%x", ((LE_CM_MSG_SET_ADVERTISING_DATA_CFM_T *)message)->status);
        }
        break;

        case LE_CM_MSG_SET_SCAN_RSP_DATA_CFM:
        {
            BM_LOG_DEBG("MSG_SET_SCAN_RSP_DATA_CFM status=%x", ((LE_CM_MSG_SET_SCAN_RSP_DATA_CFM_T *)message)->status);

            // BM_AdvParamSet(LE_HCI_ADV_TYPE_ADV_NONCONN_IND, LE_HCI_OWN_ADDR_PUBLIC, 0, LE_HCI_ADV_FILT_NONE, g_tTheBle.min_itvl, g_tTheBle.max_itvl);
        }
        break;

        case LE_CM_MSG_SET_ADVERTISING_PARAMS_CFM:
        {
            BM_LOG_DEBG("MSG_SET_ADVERTISING_PARAMS_CFM status=%x", ((LE_CM_MSG_SET_ADVERTISING_PARAMS_CFM_T *)message)->status);

            if(BM_ST_INITING == g_tTheBle.u16CurrentState)
            {
                // state change to IDLE
                BM_StateChange(BM_ST_IDLE);

                // call unsolicited callback
                tBmUnslctedCb(USLCTED_CB_EVT_BLE_INIT, OPL_OK, NULL, 0);

                if(true == g_u8AutoAdvFlag)
                {
                    // turn on advertise while auto advertise is set
                    if(OPL_OK != BM_AdvertiseSwitch(true))
                    {
                        // call unsolicited callback
                        tBmUnslctedCb(USLCTED_CB_EVT_BLE_ENT_ADVERTISE, OPL_ERR_BLE_ENT_ADV_CMD_FAIL, NULL, 0);
                    }
                    else
                    {
                        // state change to WAIT_ADV
                        BM_StateChange(BM_ST_WAIT_ADV);
                    }
                }
            }
            else
            {
                BM_LOG_WARN("Unknown case %d %d", g_tTheBle.u16CurrentState, __LINE__);
            }
        }
        break;

        case LE_CM_MSG_ENTER_ADVERTISING_CFM:
        {
            LE_CM_MSG_ENTER_ADVERTISING_CFM_T *cfm = (LE_CM_MSG_ENTER_ADVERTISING_CFM_T *)message;
            BM_LOG_DEBG("MSG_ENTER_ADVERTISING_CFM status=%x", cfm->status);

            if (cfm->status == SYS_ERR_SUCCESS)
            {
                if(BM_ST_WAIT_ADV == g_tTheBle.u16CurrentState)
                {
                    // state change to ADVING
                    BM_StateChange(BM_ST_ADVING);

                    // call unsolicited callback
                    tBmUnslctedCb(USLCTED_CB_EVT_BLE_ENT_ADVERTISE, OPL_OK, NULL, 0);
                }
                else if(BM_ST_STOP_WAIT_ENT_ADV == g_tTheBle.u16CurrentState)
                {
                    // turn off the advertise
                    if(OPL_OK != BM_AdvertiseSwitch(false))
                    {
                        // state change to ADVING
                        BM_StateChange(BM_ST_ADVING);

                        // call unsolicited callback
                        tBmUnslctedCb(USLCTED_CB_EVT_BLE_STOP, OPL_ERR_BLE_EXI_ADV_CMD_FAIL, NULL, 0);
                    }
                    else
                    {
                        // state change to STOP_WAIT_EXI_ADV
                        BM_StateChange(BM_ST_STOP_WAIT_EXI_ADV);
                    }
                }
                else
                {
                    BM_LOG_WARN("Unknown case %d %d", g_tTheBle.u16CurrentState, __LINE__);
                }
            }
            else
            {
                BM_LOG_ERRO("Enter adv fail %d", cfm->status);
                
                // state change to IDLE
                BM_StateChange(BM_ST_IDLE);

                // call unsolicited callback
                tBmUnslctedCb(USLCTED_CB_EVT_BLE_ENT_ADVERTISE, OPL_ERR_BLE_ENT_ADV_FAIL, NULL, 0);
            }
        }
        break;

        case LE_CM_MSG_EXIT_ADVERTISING_CFM:
        {
            LE_CM_MSG_EXIT_ADVERTISING_CFM_T *cfm = (LE_CM_MSG_EXIT_ADVERTISING_CFM_T *)message;
            BM_LOG_DEBG("MSG_EXIT_ADVERTISING_CFM status=%x", cfm->status);

            if (cfm->status == SYS_ERR_SUCCESS)
            {
                if(BM_ST_STOP_WAIT_EXI_ADV == g_tTheBle.u16CurrentState)
                {
                    // clear the auto advertise flag
                    g_u8AutoAdvFlag = false;

                    // state change to IDLE
                    BM_StateChange(BM_ST_IDLE);

                    // call unsolicited callback
                    tBmUnslctedCb(USLCTED_CB_EVT_BLE_STOP, OPL_OK, NULL, 0);
                }
                else
                {
                    BM_LOG_WARN("Unknown case %d %d", g_tTheBle.u16CurrentState, __LINE__);
                }
            }
            else
            {
                BM_LOG_ERRO("Exit adv fail %d", cfm->status);

                // state change to ADVING
                BM_StateChange(BM_ST_ADVING);

                // call unsolicited callback
                tBmUnslctedCb(USLCTED_CB_EVT_BLE_STOP, OPL_ERR_BLE_EXI_ADV_FAIL, NULL, 0);
            }
        }
        break;
        
#if defined(OPL1000_A2) || defined(OPL1000_A3)
        case LE_CM_CONNECTION_COMPLETE_IND:
#elif defined(OPL2500_A0)
        case LE_CM_MSG_CONNECTION_COMPLETE_IND:
#endif
        {
#if defined(OPL1000_A2) || defined(OPL1000_A3)
            LE_CM_CONNECTION_COMPLETE_IND_T *ind = (LE_CM_CONNECTION_COMPLETE_IND_T *)message;
#elif defined(OPL2500_A0)
            LE_CM_MSG_CONNECTION_COMPLETE_IND_T *ind = (LE_CM_MSG_CONNECTION_COMPLETE_IND_T *)message;
#endif

            BM_LOG_DEBG("CONNECTION_COMPLETE_IND status=%x", ind->status);

            if (ind->status == SYS_ERR_SUCCESS) 
            {
                if(BM_ST_ADVING == g_tTheBle.u16CurrentState)
                {
                    g_tTheBle.conn_hdl = ind->conn_hdl;
                    g_tTheBle.bt_addr.type = ind->peer_addr_type;
                    MemCopy(g_tTheBle.bt_addr.addr, ind->peer_addr, 6);

                    g_tTheBle.max_itvl = ind->conn_interval;
                    g_tTheBle.latency = ind->conn_latency;
                    g_tTheBle.sv_tmo = ind->supervison_timeout;
                    
                    // indicate the service change to GATT profile
                    GATT_Svc_IndicateServiceChange(ind->conn_hdl);

                    // state change to CONNECTED
                    BM_StateChange(BM_ST_CONNECTED);

                    // call unsolicited callback
                    tBmUnslctedCb(USLCTED_CB_EVT_BLE_CONNECTED, OPL_OK, g_tTheBle.bt_addr.addr, 6);
                }
                else
                {
                    BM_LOG_WARN("Unknown case %d %d", g_tTheBle.u16CurrentState, __LINE__);
                }
            }
            else
            {
                // call unsolicited callback
                tBmUnslctedCb(USLCTED_CB_EVT_BLE_CONNECTED, OPL_ERR_BLE_CONNECT_FAIL, NULL, 0);

                // state change to IDLE
                BM_StateChange(BM_ST_IDLE);

                if(true == g_u8AutoAdvFlag)
                {
                    // turn on the advertise while auto advertise is set 
                    if(OPL_OK != BM_AdvertiseSwitch(true))
                    {
                        // call unsolicited callback
                        tBmUnslctedCb(USLCTED_CB_EVT_BLE_ENT_ADVERTISE, OPL_ERR_BLE_ENT_ADV_CMD_FAIL, NULL, 0);
                    }
                    else
                    {
                        // state change to WAIT_ADV
                        BM_StateChange(BM_ST_WAIT_ADV);
                    }
                }
            }
        }
        break;

#if defined(OPL1000_A2) || defined(OPL1000_A3)
        case LE_CM_MSG_SIGNAL_UPDATE_REQ:
#elif defined(OPL2500_A0)
        case LE_CM_MSG_SIG_CONN_UPDATE_REQ:
#endif
        {
#if defined(OPL1000_A2) || defined(OPL1000_A3)
            LE_CM_MSG_SIGNAL_UPDATE_REQ_T *req = (LE_CM_MSG_SIGNAL_UPDATE_REQ_T *)message;

            BM_LOG_DEBG("MSG_SIGNAL_UPDATE_REQ identifier=%d min=%x max=%x latency=%x timeout=%x", req->identifier, 
                                                                                                           req->interval_min, 
                                                                                                           req->interval_max, 
                                                                                                           req->slave_latency, 
                                                                                                           req->timeout_multiplier);

            LeGapConnUpdateResponse(req->conn_hdl, req->identifier, TRUE);
#elif defined(OPL2500_A0)
            LE_CM_MSG_SIG_CONN_UPDATE_REQ_T *req = (LE_CM_MSG_SIG_CONN_UPDATE_REQ_T *)message;

            BM_LOG_DEBG("MSG_SIGNAL_UPDATE_REQ identifier=%d min=%x max=%x latency=%x timeout=%x", req->identifier, 
                                                                                                req->interval_min, 
                                                                                                req->interval_max, 
                                                                                                req->slave_latency, 
                                                                                                req->timeout_multiplier);

            LeGapSigConnUpdateResponse(req->conn_hdl, req->identifier, TRUE);
#endif
        }
        break;

        case LE_CM_MSG_CONN_PARA_REQ:
        {
            LE_CM_MSG_CONN_PARA_REQ_T *req = (LE_CM_MSG_CONN_PARA_REQ_T *)message;
            BM_LOG_DEBG("MSG_CONN_PARA_REQ min=%x max=%x latency=%x timeout=%x", req->itv_min, req->itv_max, req->latency, req->sv_tmo);

#if defined(OPL1000_A2) || defined(OPL1000_A3)
            LeGapConnParaRequestRsp(req->conn_hdl, TRUE);
#elif defined(OPL2500_A0)
            LeGapConnParamResponse(req->conn_hdl, TRUE);
#endif
        }
        break;

        case LE_CM_MSG_CONN_UPDATE_COMPLETE_IND:
        {
            LE_CM_MSG_CONN_UPDATE_COMPLETE_IND_T *ind = (LE_CM_MSG_CONN_UPDATE_COMPLETE_IND_T *)message;
            BM_LOG_DEBG("MSG_CONN_UPDATE_COMPLETE_IND status=%x itv=%x latency=%x svt=%x", ind->status, ind->interval, ind->latency, ind->supervision_timeout);

            if (ind->status == SYS_ERR_SUCCESS)
            {
                g_tTheBle.max_itvl = ind->interval;
                g_tTheBle.latency = ind->latency;
                g_tTheBle.sv_tmo = ind->supervision_timeout;
            }
            else
            {
                LeGapDisconnectReq(ind->conn_hdl);
            }
        }
        break;

        case LE_CM_MSG_SET_DISCONNECT_CFM:
        {
            LE_CM_MSG_SET_DISCONNECT_CFM_T *cfm = (LE_CM_MSG_SET_DISCONNECT_CFM_T *)message;
            BM_LOG_DEBG("MSG_SET_DISCONNECT_CFM conn_hdl=%x status=%x", cfm->handle, cfm->status);

            if (cfm->status == SYS_ERR_SUCCESS)
            {
                // state change to STOP_WAIT_DISC
                BM_StateChange(BM_ST_STOP_WAIT_DISC);
            }
            else
            {
                // call unsolicited callback
                tBmUnslctedCb(USLCTED_CB_EVT_BLE_DISCONNECT, OPL_ERR_BLE_DISCONNECT_CMD_FAIL, NULL, 0);
            }
        }
        break;
        
        case LE_CM_MSG_DISCONNECT_COMPLETE_IND:
        {
            LE_CM_MSG_DISCONNECT_COMPLETE_IND_T *ind = (LE_CM_MSG_DISCONNECT_COMPLETE_IND_T *)message;

            BM_LOG_DEBG("MSG_DISCONNECT_COMPLETE_IND conn_hdl=%x status=%x reason=%x", ind->conn_hdl, ind->status, ind->reason);

            if (ind->status == SYS_ERR_SUCCESS) 
            {                
                if((BM_ST_CONNECTED == g_tTheBle.u16CurrentState) ||
                (BM_ST_STOP_WAIT_DISC == g_tTheBle.u16CurrentState))
                {
                    // state change to IDLE
                    BM_StateChange(BM_ST_IDLE);

                    // call unsolicited callback
                    if(g_u8DiscByStopReqFlag)
                    {
                        tBmUnslctedCb(USLCTED_CB_EVT_BLE_STOP, OPL_OK, NULL, 0);
                    }
                    else
                    {
                        tBmUnslctedCb(USLCTED_CB_EVT_BLE_DISCONNECT, OPL_OK, NULL, 0);
                    }

                    if(true == g_u8AutoAdvFlag)
                    {
                        // turn on the advertise while auto advertise is set
                        if(OPL_OK != BM_AdvertiseSwitch(true))
                        {
                            // call unsolicited callback
                            tBmUnslctedCb(USLCTED_CB_EVT_BLE_ENT_ADVERTISE, OPL_ERR_BLE_ENT_ADV_CMD_FAIL, NULL, 0);
                        }
                        else
                        {
                            // state change to WAIT_ADV
                            BM_StateChange(BM_ST_WAIT_ADV);
                        }
                    }
                }
                else
                {
                    BM_LOG_WARN("Unknown case %d %d", g_tTheBle.u16CurrentState, __LINE__);
                }
            }
            else
            {
                // TODO: Need to make sure the state
                // BM_StateChange(BM_ST_CONNECTED);
            }

            // Clear disconnect caused by stop request lag
            g_u8DiscByStopReqFlag = false;
        }
        break;

        default:
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
static void BM_AppMsgHandler(TASK task, MESSAGEID id, MESSAGE message)
{
    switch (id)
    {
        case BM_EVT_INIT_REQ:
        {
            if(BM_ST_NULL == g_tTheBle.u16CurrentState)
            {
                // set the auto advertise flag
                BLE_MESSAGE_T *ptMsg = (BLE_MESSAGE_T *)message;
                g_u8AutoAdvFlag = *((uint8_t *)ptMsg->u8Data);

                // initialize the connection manager
#if defined(OPL1000_A2) || defined(OPL1000_A3)
                LeCmInit(&g_tTheBle.task);
#elif defined(OPL2500_A0)
                LeHostInit(&g_tTheBle.task);
#endif

                // state change to INITING
                BM_StateChange(BM_ST_INITING);
            }
            else
            {
                BM_LOG_WARN("Unknown case %d %d", g_tTheBle.u16CurrentState, __LINE__);
            }
        }
        break;
        
        case BM_EVT_START_REQ:
        {
            if(BM_ST_IDLE == g_tTheBle.u16CurrentState)
            {
                // set the auto advertise flag
                BLE_MESSAGE_T *ptMsg = (BLE_MESSAGE_T *)message;
                g_u8AutoAdvFlag = *((uint8_t *)ptMsg->u8Data);

                // turn on the advertise
                if(OPL_OK != BM_AdvertiseSwitch(true))
                {
                    // call unsolicited callback
                    tBmUnslctedCb(USLCTED_CB_EVT_BLE_ENT_ADVERTISE, OPL_ERR_BLE_ENT_ADV_CMD_FAIL, NULL, 0);
                }
                else
                {
                    // state change to WAIT_ADV
                    BM_StateChange(BM_ST_WAIT_ADV);
                }
            }
            else
            {
                BM_LOG_WARN("Unknown case %d %d", g_tTheBle.u16CurrentState, __LINE__);

                // call unsolicited callback
                tBmUnslctedCb(USLCTED_CB_EVT_BLE_ENT_ADVERTISE, OPL_ERR_BLE_ENT_ADV_CMD_FAIL, NULL, 0);
            }
        }
        break;

        case BM_EVT_STOP_REQ:
        {
            if(BM_ST_WAIT_ADV == g_tTheBle.u16CurrentState)
            {
                // clear the auto advertise flag
                g_u8AutoAdvFlag = false;

                // state change to STOP_WAIT_ENT_ADV
                BM_StateChange(BM_ST_STOP_WAIT_ENT_ADV);
            }
            else if (BM_ST_ADVING == g_tTheBle.u16CurrentState)
            {
                // clear the auto advertise flag
                g_u8AutoAdvFlag = false;

                // turn off the advertise
                if(OPL_OK != BM_AdvertiseSwitch(false))
                {
                    // call unsolicited callback
                    tBmUnslctedCb(USLCTED_CB_EVT_BLE_STOP, OPL_ERR_BLE_EXI_ADV_CMD_FAIL, NULL, 0);
                }
                else
                {
                    // state change to STOP_WAIT_EXI_ADV
                    BM_StateChange(BM_ST_STOP_WAIT_EXI_ADV);
                }
            }
            else if(BM_ST_CONNECTED == g_tTheBle.u16CurrentState)
            {
                // clear the auto advertise flag
                g_u8AutoAdvFlag = false;

                // The discconnect is cause by stop request
                g_u8DiscByStopReqFlag = true;

                // terminate the connection
                LE_ERR_STATE rc = LeGapDisconnectReq(g_tTheBle.conn_hdl);
                
                if (rc != SYS_ERR_SUCCESS) 
                {
                    BM_LOG_ERRO("Disconnect Request fail (rc=%x)", rc);

                    // call unsolicited callback
                    tBmUnslctedCb(USLCTED_CB_EVT_BLE_STOP, OPL_ERR_BLE_DISCONNECT_CMD_FAIL, NULL, 0);

                    // Clear disconnect caused by stop request lag
                    g_u8DiscByStopReqFlag = false;
                }
            }
            else
            {
                BM_LOG_WARN("Unknown case %d %d", g_tTheBle.u16CurrentState, __LINE__);

                // call unsolicited callback
                tBmUnslctedCb(USLCTED_CB_EVT_BLE_STOP, OPL_ERR_BLE_EXI_ADV_CMD_FAIL, NULL, 0);
            }
        }
        break;

        default:
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
static void BM_TaskHandler(TASK task, MESSAGEID id, MESSAGE message)
{
    if ((id >= LE_GATT_MSG_BASE) && (id < LE_GATT_MSG_TOP))
    {
        BM_GattMsgHandler(task, id, message);
    }
    else if ((id >= LE_SMP_MSG_BASE) && (id < LE_SMP_MSG_TOP))
    {
        BM_SmMsgHandler(task, id, message);
    }
    else if ((id >= LE_CM_MSG_BASE) && (id < LE_CM_MSG_TOP))
    {
        BM_CmMsgHandler(task, id, message);
    }
    else if ((id >= BM_EVT_BASE) && (id < BM_EVT_TOTAL))
    {
        BM_AppMsgHandler(task, id, message);
    }
    else if ((id >= BM_USER_EVT_BASE))
    {
        uint8_t u8Cnt = 0;
        for(; u8Cnt < BM_SVC_NUM_MAX; u8Cnt ++)
        {
            if(true == BM_ServiceIsAssigned(u8Cnt))
            {
                if(NULL != g_patBmSvcHandle[u8Cnt]->ptSvcDataProcessHandler)
                {
                    if(g_patBmSvcHandle[u8Cnt]->u16SvcDataEvtBase <= id && id < g_patBmSvcHandle[u8Cnt]->u16SvcDataEvtTop)
                    {
                        g_patBmSvcHandle[u8Cnt]->ptSvcDataProcessHandler(id, message);
                        break;
                    }
                }
            }
        }

        if(BM_SVC_NUM_MAX == u8Cnt)
        {
            // BM_LOG_WARN("Can't find exactly data process handler base on event %d", id);
        }
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
static T_OplErr BM_TrigDispatchHandler(uint16_t u16Handle, MESSAGEID id, MESSAGE message)
{
    uint8_t u8Cnt = 0;

    for(; u8Cnt < BM_SVC_NUM_MAX; u8Cnt ++)
    {
        // check assigned service
        if(true == BM_ServiceIsAssigned(u8Cnt))
        {
            // check the service handle range
            if ((u16Handle >= g_patBmSvcHandle[u8Cnt]->ptSvcDef->startHdl) && (u16Handle <= g_patBmSvcHandle[u8Cnt]->ptSvcDef->endHdl))
            {
                // calling dispatch handler
                return g_patBmSvcHandle[u8Cnt]->ptSvcGattDispatchHandler(id, message);
            }
        }
    }

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
static T_OplErr BM_AdvertiseSwitch(bool blOnOff)
{
    LE_ERR_STATE rc = SYS_ERR_SUCCESS;

    if(true == blOnOff)
    {
        rc = LeGapAdvertisingEnable(TRUE);

        if(SYS_ERR_SUCCESS != rc)
        {
            BM_LOG_ERRO("Adv on fail %d", rc);

            return OPL_ERR_BLE_ENT_ADV_CMD_FAIL;
        }
    }
    else
    {
        rc = LeGapAdvertisingEnable(FALSE);

        if(SYS_ERR_SUCCESS != rc)
        {
            BM_LOG_ERRO("Adv off fail %d", rc);

            return OPL_ERR_BLE_EXI_ADV_CMD_FAIL;
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
static void BM_StateChange(T_BmStateList tStateId)
{
    g_tTheBle.u16HistoryState = g_tTheBle.u16CurrentState;
    g_tTheBle.u16CurrentState = tStateId;

    BM_LOG_INFO("State Change %d -> %d", g_tTheBle.u16HistoryState, g_tTheBle.u16CurrentState);
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
static uint8_t BM_ServiceIsAssigned(uint8_t u8Index)
{
    if(NULL != g_patBmSvcHandle[u8Index])
    {
        return true;
    }

    return false;
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
T_OplErr BM_AdvParamSet(uint8_t u8Type, uint8_t u8AddrType, LE_BT_ADDR_T *ptPeerAddr, uint8_t u8Filter, uint16_t u16AdvItvlMin, uint16_t u16AdvItvlMax)
{
    LE_ERR_STATE rc;
    LE_GAP_ADVERTISING_PARAM_T para;

    g_tTheBle.min_itvl = para.interval_min = u16AdvItvlMin;
    g_tTheBle.max_itvl = para.interval_max = u16AdvItvlMax;
    para.type = u8Type;
    para.own_addr_type = u8AddrType;

    if (ptPeerAddr)
    {
        para.peer_addr_type = ptPeerAddr->type;
        MemCopy(para.peer_addr, ptPeerAddr->addr, 6);
    }
    else
    {
        para.peer_addr_type = LE_HCI_ADV_PEER_ADDR_PUBLIC;
        MemSet(para.peer_addr, 0, 6);
    }

    para.channel_map = 0x7;
    para.filter_policy = u8Filter;

    rc = LeGapSetAdvParameter(&para);

    if (SYS_ERR_SUCCESS == rc)
    {
        return OPL_OK;
    }
    else
    {
        return OPL_ERR;
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
T_OplErr BM_AdvIntervalSet(uint16_t u16AdvItvlMin, uint16_t u16AdvItvlMax)
{
    return BM_AdvParamSet(LE_HCI_ADV_TYPE_ADV_IND, LE_HCI_OWN_ADDR_PUBLIC, 0, LE_HCI_ADV_FILT_NONE, u16AdvItvlMin, u16AdvItvlMax);
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
T_OplErr BM_AdvDataSet(uint8_t *pau8AdvData, uint8_t u8AdvDataLen)
{
    LE_ERR_STATE rc;

    g_tAdvData.u8Len = u8AdvDataLen;
    MemCopy(g_tAdvData.au8Buf, pau8AdvData, u8AdvDataLen);

    rc = LeGapSetAdvData(g_tAdvData.u8Len, g_tAdvData.au8Buf);

    if(SYS_ERR_SUCCESS == rc)
    {
        return OPL_OK;
    }
    else
    {
        return OPL_ERR;
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
T_OplErr BM_ScanRspDataSet(uint8_t *pau8ScanRspData, uint8_t u8ScanRspDataLen)
{
    LE_ERR_STATE rc;

    g_tScanRspData.u8Len = u8ScanRspDataLen;
    MemCopy(g_tScanRspData.au8Buf, pau8ScanRspData, u8ScanRspDataLen);

#if defined(OPL1000_A2) || defined(OPL1000_A3)
    rc = LeSetScanRspData(g_tScanRspData.u8Len, g_tScanRspData.au8Buf);
#elif defined(OPL2500_A0)
    rc = LeGapSetScanRspData(g_tScanRspData.u8Len, g_tScanRspData.au8Buf);
#endif

    if(SYS_ERR_SUCCESS == rc)
    {
        return OPL_OK;
    }
    else
    {
        return OPL_ERR;
    }
}

/*************************************************************************
* FUNCTION:
*   BM_ServiceAssign
*
* DESCRIPTION:
*   Run this function before calling BM_Init
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
T_OplErr BM_ServiceAssign(T_BmSvcHandle *tBmSvcHandle)
{
    uint8_t u8Count = 0;

    if(g_tTheBle.u16CurrentState != BM_ST_NULL)
    {
        BM_LOG_WARN("BM already init");
        
        return OPL_ERR_BLE_TASK_REINIT;
    }

    if((NULL == tBmSvcHandle->patSvcDb) && (NULL == tBmSvcHandle->ptSvcGattDispatchHandler))
    {
        BM_LOG_WARN("Param invalid");

        return OPL_ERR_PARAM_INVALID;
    }

    if(NULL != tBmSvcHandle->ptSvcDataProcessHandler)
    {
        if((BM_USER_EVT_BASE > tBmSvcHandle->u16SvcDataEvtBase) ||
           (tBmSvcHandle->u16SvcDataEvtBase >= tBmSvcHandle->u16SvcDataEvtTop))
        {
            BM_LOG_WARN("Event range invalid");

            return OPL_ERR_PARAM_INVALID;   
        }
    }

    while (u8Count != BM_SVC_NUM_MAX)
    {
#if 0
        osSemaphoreWait(g_tBmSvcAssignRegSemId, osWaitForever);
#endif

        if (false == BM_ServiceIsAssigned(u8Count))
        {
            g_patBmSvcHandle[u8Count] = tBmSvcHandle;

#if 0
            osSemaphoreRelease(g_tBmSvcAssignRegSemId);
#endif

            return OPL_OK;
        }

#if 0
        osSemaphoreRelease(g_tBmSvcAssignRegSemId);
#endif

        u8Count ++;
    }

    return OPL_ERR_BLE_SVC_ASSIGN_INVALID;
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
T_OplErr BM_UslctedCbReg(T_BmUslctedCbFp fpUslctedCb)
{
#if 0
    osSemaphoreWait(g_tBmUslctedCbRegSemId, osWaitForever);
#endif

    if (NULL == tBmUnslctedCb)
    {
        tBmUnslctedCb = fpUslctedCb;

#if 0
        osSemaphoreRelease(g_tBmUslctedCbRegSemId);
#endif

        return OPL_OK;
    }

#if 0
    osSemaphoreRelease(g_tBmUslctedCbRegSemId);
#endif

    return OPL_ERR_USLCTED_CB_REG_INVALID;
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
T_OplErr BM_SendMessage(uint16_t u16Evt, void *vData, uint32_t u32DataLen, uint32_t u32DelayMs)
{
    if (((BM_EVT_BASE <= u16Evt) && (u16Evt < BM_EVT_TOTAL)) || (BM_USER_EVT_BASE <= u16Evt))
    {
        void *p = 0;

        if (0 < u32DataLen)
        {
            MESSAGE_DATA_BULID(BLE_MESSAGE, u32DataLen);

            if(NULL == msg)
            {
                BM_LOG_ERRO("malloc fail");

                return OPL_ERR_ALLOC_MEMORY_FAIL;
            }

            msg->u32DataLen = u32DataLen;
            msg->u8Data = MESSAGE_OFFSET(BLE_MESSAGE);
            MemCopy(msg->u8Data, vData, u32DataLen);
            p = msg;
        }

        if(u32DelayMs)
        {
            LeSendMessageAfter(&g_tTheBle.task, u16Evt, p, u32DelayMs);
        }
        else
        {
            LeSendMessage(&g_tTheBle.task, u16Evt, p);
        }

        return OPL_OK;
    }

    return OPL_ERR_FSM_EVT_INVALID;
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
BLE_APP_DATA_T *BM_EntityGet(void)
{
    return &g_tTheBle;
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
void BM_TaskInit(void)
{
#if 0
    osSemaphoreDef_t tSemaphoreDef;
    tSemaphoreDef.dummy = 0;
#endif

    // state initialize
    g_tTheBle.u16CurrentState = BM_ST_NULL;
    g_tTheBle.u16HistoryState = BM_ST_NULL;

    g_tTheBle.curr_mtu = 23;    
    g_tTheBle.min_itvl = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    g_tTheBle.max_itvl = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    g_tTheBle.latency  = DEFAULT_DESIRED_SLAVE_LATENCY;
    g_tTheBle.sv_tmo   = DEFAULT_DESIRED_SUPERVERSION_TIMEOUT;

#if 0
    // create semaphore
    g_tBmUslctedCbRegSemId = osSemaphoreCreate(&tSemaphoreDef, 1);

    if (g_tBmUslctedCbRegSemId == NULL)
    {
        BM_LOG_ERRO("Create uslcted semaphore fail");
    }

    // create semaphore
    g_tBmSvcAssignRegSemId = osSemaphoreCreate(&tSemaphoreDef, 1);

    if (g_tBmSvcAssignRegSemId == NULL)
    {
        BM_LOG_ERRO("Create svc semaphore fail");
    }
#endif
    
    LeHostCreateTask(&g_tTheBle.task, BM_TaskHandler);

    BM_LOG_DEBG("Create task ok");
}

#endif /* BM_ENABLED */
