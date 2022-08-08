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
*  fsm_kernel.h
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

#include "opl_err.h"
#include "qd_config.h"
#include "qd_module.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __FSM_KERNEL_H__
#define __FSM_KERNEL_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (FSM_ENABLED == 1)

#if (FSM_LOG_ENABLED == 1)
#define FSM_LOG_DEBG(...)                                OPL_LOG_DEBG(FSM, __VA_ARGS__)
#define FSM_LOG_INFO(...)                                OPL_LOG_INFO(FSM, __VA_ARGS__)
#define FSM_LOG_WARN(...)                                OPL_LOG_WARN(FSM, __VA_ARGS__)
#define FSM_LOG_ERRO(...)                                OPL_LOG_ERRO(FSM, __VA_ARGS__)
#else
#define FSM_LOG_DEBG(...)
#define FSM_LOG_INFO(...)
#define FSM_LOG_WARN(...)
#define FSM_LOG_ERRO(...)
#endif

#define GET_NAME(x) #x

#define FSM_AT_NULL_ACTION              (FsmActionFunc)NULL
#define FSM_EV_NULL_EVENT               (T_FsmEvent)0xFFFF

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef uint16_t T_FsmState;
typedef uint32_t T_FsmEvent;

typedef void    (* FsmIndicateCbFunc)(void);
typedef T_OplErr(* FsmActionFunc)    (T_FsmState, T_FsmEvent, uint8_t *, uint32_t);
typedef uint8_t (* FsmIndCbSet)      (T_FsmEvent, FsmIndicateCbFunc);
typedef void    (* FsmIndCbProc)     (T_FsmEvent, T_OplErr);

typedef struct S_FsmStateInfo
{
    T_FsmState tCurrentState;
    T_FsmState tHistoryState;
} T_FsmStateInfo;

typedef struct S_FsmStateExctblEvent
{
    T_FsmEvent tEventId;
    FsmActionFunc fpFsmActionFunc;
} T_FsmStateExctblEvent;

typedef struct S_FsmStateTable
{
    const T_FsmStateExctblEvent *ptFsmStateExctblEvent;
} T_FsmStateTable;

typedef struct S_FsmDef
{
    const T_FsmStateTable *ptFsmStateTable;
    T_FsmStateInfo ptFsmStateInfo;
    FsmIndCbSet ptFsmIndCbSet;
    FsmIndCbProc ptFsmIndCbProc;
    uint8_t u8aFsmName[10];
} T_FsmDef;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

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
void FSM_Init(T_FsmDef *tFsmDef, uint8_t *pu8Name, const T_FsmStateTable *tStateTable, T_FsmState tInitState);

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
void FSM_IndCbHdlInit(T_FsmDef *tFsmDef, FsmIndCbSet fpIndCbSet, FsmIndCbProc fpIndCbProc);

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
void FSM_StateChange(T_FsmDef *tFsmDef, T_FsmState tStateId);

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
T_OplErr FSM_Test(T_FsmDef *tFsmDef, T_FsmEvent tEventId);

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
T_OplErr FSM_Run(T_FsmDef *tFsmDef, T_FsmEvent tEventId, uint8_t *u8Data, uint32_t u32DataLen, FsmIndicateCbFunc fpIndCb);

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

#endif /* FSM_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* __FSM_KERNEL_H__ */
