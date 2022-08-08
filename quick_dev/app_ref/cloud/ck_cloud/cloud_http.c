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
*  cloud_http.c
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

#include "cloud_config.h"
#include "cloud_http.h"

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

#if CLOUD_HTTP_POST_ENHANCEMENT
static uint8_t g_u8ClientFlag = 0;
#endif

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

#if CLOUD_HTTP_POST_ENHANCEMENT
static HTTPCLIENT_RESULT httpclient_common_ex(httpclient_t *client, char *url, int method, httpclient_data_t *client_data)
{
    HTTPCLIENT_RESULT ret = HTTPCLIENT_OK;
#ifdef HTTPCLIENT_TIME_DEBUG
    int start_time, end_time;
#endif

    if (g_u8ClientFlag == 0)
    {
        ret = httpclient_connect(client, url);
        if (ret == HTTPCLIENT_OK)
        {
            g_u8ClientFlag = 1;
        }
        else
        {
        }
    }

    if (ret == HTTPCLIENT_OK)
    {
        ret = httpclient_send_request(client, url, method, client_data);

        if (ret == HTTPCLIENT_OK)
        {
#ifdef HTTPCLIENT_TIME_DEBUG
            start_time = sys_now();
#endif
            ret = httpclient_recv_response(client, client_data);
#ifdef HTTPCLIENT_TIME_DEBUG
            end_time = sys_now();
            printf("recv_response time =%d\r\n", end_time - start_time);
#endif
        }
    }

    if (ret != HTTPCLIENT_OK)
    {
        httpclient_close(client);
        g_u8ClientFlag = 0;
    }

    return ret;
}
#endif

HTTPCLIENT_RESULT Cloud_Http_Post(httpclient_t *client, char *url, httpclient_data_t *client_data)
{
#if CLOUD_HTTP_POST_ENHANCEMENT
    return httpclient_common_ex(client, url, HTTPCLIENT_POST, client_data);
#else
    return httpclient_post(client, url, client_data);
#endif
}

HTTPCLIENT_RESULT Cloud_Http_Get(httpclient_t *client, char *url, httpclient_data_t *client_data)
{
#if CLOUD_HTTP_POST_ENHANCEMENT
    return httpclient_common_ex(client, url, HTTPCLIENT_GET, client_data);
#else
    return httpclient_get(client, url, client_data);
#endif
}

#if CLOUD_HTTP_POST_ENHANCEMENT
HTTPCLIENT_RESULT Cloud_Http_Close(httpclient_t *client)
{
    if (g_u8ClientFlag == 1)
    {
        httpclient_close(client);
        g_u8ClientFlag = 0;
    }

    return HTTPCLIENT_OK;
}
#endif
