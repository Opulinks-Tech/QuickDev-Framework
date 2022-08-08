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
*  log.h
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

#include "msg.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __LOG_H__
#define __LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define OPL_LOG_DEBG(module, ...)                                           \
        {                                                                   \
            tracer_log(LOG_LOW_LEVEL, "\x1B[0m[D][%s]", #module);           \
            tracer_log(LOG_LOW_LEVEL, __VA_ARGS__);                         \
            tracer_log(LOG_LOW_LEVEL, "\r\n");                              \
        }

#define OPL_LOG_INFO(module, ...)                                           \
        {                                                                   \
            tracer_log(LOG_MED_LEVEL, "\x1B[0m[I][%s]", #module);           \
            tracer_log(LOG_MED_LEVEL, __VA_ARGS__);                         \
            tracer_log(LOG_MED_LEVEL, "\r\n");                              \
        }

#define OPL_LOG_WARN(module, ...)                                           \
        {                                                                   \
            tracer_log(LOG_MED_LEVEL, "\x1B[33m[W][%s]", #module);          \
            tracer_log(LOG_MED_LEVEL, __VA_ARGS__);                         \
            tracer_log(LOG_MED_LEVEL, "\x1B[0m\r\n");                       \
        }

#define OPL_LOG_ERRO(module, ...)                                           \
        {                                                                   \
            tracer_log(LOG_HIGH_LEVEL, "\x1B[31m[E][%s]", #module);         \
            tracer_log(LOG_HIGH_LEVEL, __VA_ARGS__);                        \
            tracer_log(LOG_HIGH_LEVEL, "\x1B[0m\r\n");                      \
        }

#define OPL_HEX_DUMP_DEBG(module, buf, len)                                 \
        {                                                                   \
            tracer_log(LOG_LOW_LEVEL, "\x1B[0m[I][%s]", #module);           \
            LOG_HexDump(LOG_LOW_LEVEL, buf, len);                           \
            tracer_log(LOG_LOW_LEVEL, "\r\n");                              \
        }

#define OPL_HEX_DUMP_INFO(module, buf, len)                                 \
        {                                                                   \
            tracer_log(LOG_MED_LEVEL, "\x1B[0m[I][%s]", #module);           \
            LOG_HexDump(LOG_MED_LEVEL, buf, len);                           \
            tracer_log(LOG_MED_LEVEL, "\r\n");                              \
        }

#define OPL_HEX_DUMP_WARN(module, buf, len)                                 \
        {                                                                   \
            tracer_log(LOG_MED_LEVEL, "\x1B[33m[W][%s]", #module);          \
            LOG_HexDump(LOG_MED_LEVEL, buf, len);                           \
            tracer_log(LOG_MED_LEVEL, "\x1B[0m\r\n");                       \
        }

#define OPL_HEX_DUMP_ERRO(module, buf, len)                                 \
        {                                                                   \
            tracer_log(LOG_HIGH_LEVEL, "\x1B[31m[E][%s]", #module);         \
            LOG_HexDump(LOG_HIGH_LEVEL, buf, len);                          \
            tracer_log(LOG_HIGH_LEVEL, "\x1B[0m\r\n");                      \
        }

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

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
void LOG_HexDump(uint8_t u8LogLvl, uint8_t *pau8Buf, uint32_t u32Len);

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

#ifdef __cplusplus
}
#endif

#endif /* __LOG_H__ */
