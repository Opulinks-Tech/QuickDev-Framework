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
*  fsm_kernel.c
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

#include "fsm_kernel.h"
#include "cmsis_os.h"
#include "log.h"
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (FSM_ENABLED == 1)

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

static const T_FsmStateExctblEvent *FSM_Scan_Exctbl_Event(const T_FsmStateExctblEvent *tEventExctblEvent, T_FsmEvent tEventId);

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

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
static const T_FsmStateExctblEvent *FSM_Scan_Exctbl_Event(const T_FsmStateExctblEvent *tEventExctblEvent, T_FsmEvent tEventId)
{
    const T_FsmStateExctblEvent *tEventExctblEvent_t = NULL;

    // scan down the state list
    while(tEventExctblEvent->tEventId != FSM_EV_NULL_EVENT)
    {

        if(tEventExctblEvent->tEventId == tEventId)
        {
            // Find the matched event transition
            tEventExctblEvent_t = tEventExctblEvent;
            break;
        }

        tEventExctblEvent++;
    }

    return tEventExctblEvent_t;
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
void FSM_Init(T_FsmDef *tFsmDef, uint8_t *pu8Name, const T_FsmStateTable *tStateTable, T_FsmState tInitState)
{
    // initalize information
    tFsmDef->ptFsmStateTable = tStateTable;
    tFsmDef->ptFsmStateInfo.tHistoryState = tFsmDef->ptFsmStateInfo.tCurrentState = tInitState;

    uint8_t u8FsmNameLen = 0;

    if(sizeof(tFsmDef->u8aFsmName) < strlen((char *)pu8Name))
    {
        u8FsmNameLen = sizeof(tFsmDef->u8aFsmName);
    }
    else
    {
        u8FsmNameLen = strlen((char *)pu8Name);
    }

    memcpy(tFsmDef->u8aFsmName, pu8Name, u8FsmNameLen); 

    FSM_LOG_DEBG("{%s} state machine init ok", tFsmDef->u8aFsmName);
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
void FSM_IndCbHdlInit(T_FsmDef *tFsmDef, FsmIndCbSet fpIndCbSet, FsmIndCbProc fpIndCbProc)
{
    tFsmDef->ptFsmIndCbSet = fpIndCbSet;
    tFsmDef->ptFsmIndCbProc = fpIndCbProc;

    FSM_LOG_DEBG("{%s} ind cb init ok", tFsmDef->u8aFsmName);
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
void FSM_StateChange(T_FsmDef *tFsmDef, T_FsmState tStateId)
{
    tFsmDef->ptFsmStateInfo.tHistoryState = tFsmDef->ptFsmStateInfo.tCurrentState;
    tFsmDef->ptFsmStateInfo.tCurrentState = tStateId;

    FSM_LOG_DEBG("{%s} state change %d -> %d", tFsmDef->u8aFsmName,
                                               tFsmDef->ptFsmStateInfo.tHistoryState,
                                               tFsmDef->ptFsmStateInfo.tCurrentState);
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
T_OplErr FSM_Test(T_FsmDef *tFsmDef, T_FsmEvent tEventId)
{
    const T_FsmStateExctblEvent *tEventTableEnter;
    T_FsmStateTable tStateTableEnter;
    T_FsmState tCurrentState;

    tCurrentState = tFsmDef->ptFsmStateInfo.tCurrentState;

    tStateTableEnter = tFsmDef->ptFsmStateTable[tCurrentState];
    tEventTableEnter = tStateTableEnter.ptFsmStateExctblEvent;

    // scan down the event table searching for the executable event to be action
    tEventTableEnter = FSM_Scan_Exctbl_Event(tEventTableEnter, tEventId);

    // if no event is matched, return error and do nothing
    if(tEventTableEnter == NULL)
    {
        FSM_LOG_WARN("{%s} event %X not found in table", tFsmDef->u8aFsmName, tEventId);
        return OPL_ERR_FSM_EVT_INVALID;
    }

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
T_OplErr FSM_Run(T_FsmDef *tFsmDef, T_FsmEvent tEventId, uint8_t *u8Data, uint32_t u32DataLen, FsmIndicateCbFunc fpIndCb)
{
    const T_FsmStateExctblEvent *tEventTableEnter;
    T_FsmStateTable tStateTableEnter;
    T_FsmState tCurrentState; 

    tCurrentState = tFsmDef->ptFsmStateInfo.tCurrentState;

    tStateTableEnter = tFsmDef->ptFsmStateTable[tCurrentState];
    tEventTableEnter = tStateTableEnter.ptFsmStateExctblEvent;

    // scan down the event table searching for the executable event to be calling
    tEventTableEnter = FSM_Scan_Exctbl_Event(tEventTableEnter, tEventId);

    // if no event is matched, return error and do nothing
    if(tEventTableEnter == NULL)
    {
        FSM_LOG_WARN("{%s} event %X not found in table", tFsmDef->u8aFsmName, tEventId);
        return OPL_ERR_FSM_EVT_INVALID;
    }

    if(tEventTableEnter->fpFsmActionFunc != FSM_AT_NULL_ACTION)
    {
        // set indicate callback
        if(NULL != tFsmDef->ptFsmIndCbSet)
        {
            if(OPL_ERR_IND_CB_REG_INVALID == tFsmDef->ptFsmIndCbSet(tEventId, fpIndCb))
            {
                FSM_LOG_WARN("{%s} ind cb assign invalid", tFsmDef->u8aFsmName);
                return OPL_ERR_IND_CB_REG_INVALID;
            }
        }
 
        // process the event handler
        return (*tEventTableEnter->fpFsmActionFunc)(tCurrentState, tEventId, u8Data, u32DataLen);
    }

    return OPL_OK;
}

#endif /* FSM_ENABLED */
