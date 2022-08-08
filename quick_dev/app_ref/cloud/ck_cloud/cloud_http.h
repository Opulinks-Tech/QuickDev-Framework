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
*  cloud_http.h
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

#include <stdint.h>
#include <stdbool.h>
#include "cloud_config.h"
#include "httpclient.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#ifndef __CLOUD_HTTP_H__
#define __CLOUD_HTTP_H__

#ifdef __cplusplus
extern "C" {
#endif

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

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

HTTPCLIENT_RESULT Cloud_Http_Post(httpclient_t *client, char *url, httpclient_data_t *client_data);
HTTPCLIENT_RESULT Cloud_Http_Get(httpclient_t *client, char *url, httpclient_data_t *client_data);
#if CLOUD_HTTP_POST_ENHANCEMENT
HTTPCLIENT_RESULT Cloud_Http_Close(httpclient_t *client);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CLOUD_HTTP_H__ */

