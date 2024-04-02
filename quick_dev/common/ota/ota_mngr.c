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
*  ota_mngr.c
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

#include "cmsis_os.h"
#include "hal_system.h"
#include "ota_mngr.h"
#include "qd_config.h"
#include "qd_module.h"
#if defined(AWS_MODULE)
#include "transfer.h"
#include "app_main.h"
#endif

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (OTA_ENABLED == 1)

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

static const T_FsmStateExctblEvent tOtaIdleExctblEvent[] =
{
    {OTA_EVT_PREPARE_REQ,       OTA_PrepareRequestHandler},
    {OTA_EVT_REBOOT_REQ,        OTA_RebootRequestHandler},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateExctblEvent tOtaProcessExctblEvent[] =
{
    {OTA_EVT_DATA_IN_REQ,       OTA_DataInRequestHandler},
    {OTA_EVT_FINISH_REQ,        OTA_FinishRequestHandler},
    {OTA_EVT_GIVEUP_REQ,        OTA_GiveUpRequestHandler},
    {OTA_EVT_PROC_TIMEOUT,      OTA_GiveUpRequestHandler},
    {FSM_EV_NULL_EVENT,         FSM_AT_NULL_ACTION},
};

static const T_FsmStateTable tOtaStateTbl[] =
{
    {tOtaIdleExctblEvent},
    {tOtaProcessExctblEvent},
};

osTimerId g_tOtaProcTimer;

static T_FsmDef g_tOtaFsmDef;
static T_OtaProcTimeoutIndCbFp g_tOtaProcTimeoutIndCb = NULL;

static uint16_t g_u16SeqId = 0;

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

// OTA apis
/*************************************************************************
* FUNCTION:
*   OTA_InProgress
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
bool OTA_InProgress(void)
{
    if(g_tOtaFsmDef.ptFsmStateInfo.tCurrentState == OTA_ST_PROC)
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
T_OplErr OTA_CurrentVersionGet(uint16_t *u16PrjId, uint16_t *u16ChipId, uint16_t *u16FwId)
{
    if(MW_OTA_OK != MwOta_VersionGet(u16PrjId, u16ChipId, u16FwId))
    {
        return OPL_OTA_FAIL;
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
T_OplErr OTA_UpgradeBegin(uint16_t *pu16SeqId, T_MwOtaFlashHeader *ptOtaHdr, T_OtaProcTimeoutIndCbFp tOtaProcTimeoutIndCb)
{
    T_OplErr tEvtRst = OPL_ERR;

    if(NULL == pu16SeqId)
    {
        return OPL_ERR_PARAM_INVALID; // invalid parameter
    }

    tEvtRst = FSM_Run(&g_tOtaFsmDef, OTA_EVT_PREPARE_REQ, (uint8_t *)ptOtaHdr, sizeof(T_MwOtaFlashHeader), NULL);

    if(OPL_OK == tEvtRst)
    {
        // generate seq id
        *pu16SeqId = g_u16SeqId = rand() % 1000;

        // assign ota timeout callback
        g_tOtaProcTimeoutIndCb = tOtaProcTimeoutIndCb;

        OTA_LOG_INFO("OTA start (seq id %d)", g_u16SeqId);   
#if defined(AWS_MODULE)     
        APP_SendMessage(APP_EVT_OTA_START_IND, NULL, 0);
#endif
        return OPL_OK;
    }
    else
    {
        OTA_LOG_ERRO("OTA fail");
#if defined(AWS_MODULE)       
        APP_SendMessage(APP_EVT_OTA_FAIL_IND, NULL, 0);
#endif
        return OPL_OTA_FAIL;
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
T_OplErr OTA_WriteData(uint16_t u16SeqId, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_OplErr tEvtRst = OPL_ERR;

    if(g_u16SeqId != u16SeqId)
    {
        return OPL_OTA_SEQ_ERROR; // invalid seq id
    }

    tEvtRst = FSM_Run(&g_tOtaFsmDef, OTA_EVT_DATA_IN_REQ, pu8Data, u32DataLen, NULL);

    return tEvtRst;
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
T_OplErr OTA_UpgradeFinish(uint16_t u16SeqId)
{
    T_OplErr tEvtRst = OPL_ERR;

    if(g_u16SeqId != u16SeqId)
    {
        return OPL_OTA_SEQ_ERROR; // invalid seq id
    }

    tEvtRst = FSM_Run(&g_tOtaFsmDef, OTA_EVT_FINISH_REQ, NULL, 0, NULL);

    if(OPL_OK == tEvtRst)
    {
        //OTA_LOG_INFO("OTA finish");
        tracer_drct_printf("\n\nOTA finish\n\n");
    }
    else
    {
        //OTA_LOG_ERRO("OTA fail");
#if defined(AWS_MODULE)         
        APP_SendMessage(APP_EVT_OTA_FAIL_IND, NULL, 0);
#endif
        tracer_drct_printf("\n\nOTA fail\n\n");
    }

    return tEvtRst;
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
T_OplErr OTA_UpgradeGiveUp(uint16_t u16SeqId)
{
    T_OplErr tEvtRst = OPL_ERR;

    if(g_u16SeqId != u16SeqId)
    {
        return OPL_OTA_SEQ_ERROR; // invalid seq id
    }

    tEvtRst = FSM_Run(&g_tOtaFsmDef, OTA_EVT_GIVEUP_REQ, NULL, 0, NULL);

    if(OPL_OK == tEvtRst)
    {
        OTA_LOG_INFO("OTA give up");
    }
    else
    {
        OTA_LOG_ERRO("OTA fail");
#if defined(AWS_MODULE) 
        APP_SendMessage(APP_EVT_OTA_FAIL_IND, NULL, 0);
#endif
    }

    return tEvtRst;
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
T_OplErr OTA_TriggerReboot(uint16_t u16Delay)
{
    T_OplErr tEvtRst = OPL_ERR;

    tEvtRst = FSM_Run(&g_tOtaFsmDef, OTA_EVT_REBOOT_REQ, (uint8_t *)&u16Delay, sizeof(uint16_t), NULL);

    // system will reboot here if success process, below program won't process

    return tEvtRst;
}

// callback
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
static void OTA_ProcTimerTimeoutHandler(void const *argu)
{
    FSM_Run(&g_tOtaFsmDef, OTA_EVT_PROC_TIMEOUT, NULL, 0, NULL);

    g_tOtaProcTimeoutIndCb();
    
    g_tOtaProcTimeoutIndCb = NULL;
}

/// FSM handler
static T_OplErr OTA_PrepareRequestHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_MwOtaFlashHeader *tOtaHdr = (T_MwOtaFlashHeader *)pu8Data;

    if(MW_OTA_OK != MwOta_Prepare(tOtaHdr->uwProjectId, tOtaHdr->uwChipId, tOtaHdr->uwFirmwareId, tOtaHdr->ulImageSize, tOtaHdr->ulImageSum))
    {
        OTA_LOG_ERRO("Prepare fail");

        if(MW_OTA_OK != MwOta_DataGiveUp())
        {
            OTA_LOG_ERRO("Give up fail");
        }

        return OPL_OTA_FAIL;
    }

    osTimerStop(g_tOtaProcTimer);
    osTimerStart(g_tOtaProcTimer, OTA_PROCESS_PERIOD_TIMEOUT);

    FSM_StateChange(&g_tOtaFsmDef, OTA_ST_PROC);

    return OPL_OK;
}

static T_OplErr OTA_DataInRequestHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    if(MW_OTA_OK != MwOta_DataIn(pu8Data, u32DataLen))
    {
        if(MW_OTA_OK != MwOta_DataGiveUp())
        {
            OTA_LOG_ERRO("Give up fail");
        }

        osTimerStop(g_tOtaProcTimer);

        g_u16SeqId = 0;
        g_tOtaProcTimeoutIndCb = NULL;

        FSM_StateChange(&g_tOtaFsmDef, OTA_ST_IDLE);

        return OPL_OTA_FAIL;
    }

    return OPL_OK;
}

static T_OplErr OTA_FinishRequestHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_OplErr tEvtRst = OPL_OK;

    if(MW_OTA_OK != MwOta_DataFinish())
    {
        OTA_LOG_ERRO("Finish fail");

        tEvtRst = OPL_OTA_FAIL;
    }
    else
    {
        OTA_LOG_DEBG("Finish ok");
    }

    osTimerStop(g_tOtaProcTimer);

    g_u16SeqId = 0;
    g_tOtaProcTimeoutIndCb = NULL;

    FSM_StateChange(&g_tOtaFsmDef, OTA_ST_IDLE);

    return tEvtRst;
}

static T_OplErr OTA_GiveUpRequestHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    T_OplErr tEvtRst = OPL_OK;

    if(MW_OTA_OK != MwOta_DataGiveUp())
    {
        OTA_LOG_ERRO("Give up fail");

        tEvtRst = OPL_OTA_FAIL;
    }

    osTimerStop(g_tOtaProcTimer);

    g_u16SeqId = 0;

    // if event is from timer, callback pointer will clear in timer callback
    if(OTA_EVT_PROC_TIMEOUT != tFsmEvent)
    {
        g_tOtaProcTimeoutIndCb = NULL;
    }

    FSM_StateChange(&g_tOtaFsmDef, OTA_ST_IDLE);

    return tEvtRst;
}

/*************************************************************************
* FUNCTION:
*   AppInit
*
* DESCRIPTION:
*   the initial of application
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
static T_OplErr OTA_RebootRequestHandler(T_FsmState tFsmState, T_FsmEvent tFsmEvent, uint8_t *pu8Data, uint32_t u32DataLen)
{
    uint16_t *u16Delay = (uint16_t *)pu8Data;
    osDelay(*u16Delay);
    Hal_Sys_SwResetAll();
    
    // system will reboot here, below code won't process
	
	return OPL_OK;
}

/*************************************************************************
* FUNCTION:
*   AppInit
*
* DESCRIPTION:
*   the initial of application
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
T_FsmDef *OTA_FsmDefGet(void)
{
    return &g_tOtaFsmDef;
}

/*************************************************************************
* FUNCTION:
*   AppInit
*
* DESCRIPTION:
*   the initial of application
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
void OTA_Init(void)
{
    // name of WI-FI agent
    uint8_t u8aOtaName[3] = "OTA";

    // create ota process timer
    osTimerDef_t tTimerDef;

    tTimerDef.ptimer = OTA_ProcTimerTimeoutHandler;
    g_tOtaProcTimer = osTimerCreate(&tTimerDef, osTimerOnce, NULL);
    if(g_tOtaProcTimer == NULL)
    {
        OTA_LOG_ERRO("Create ota proc timer fail");
    }

    // initialize the WI-FI agent state machine
    FSM_Init(&g_tOtaFsmDef, u8aOtaName, tOtaStateTbl, OTA_ST_IDLE);
}

#endif /* OTA_ENABLED */
