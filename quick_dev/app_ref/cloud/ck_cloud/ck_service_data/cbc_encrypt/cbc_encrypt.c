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
*  cloud_data.c
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

#include "cbc_encrypt.h"

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

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions

/*************************************************************************
* FUNCTION:
*   Cloud_DataConstruct
*
* DESCRIPTION:
*   constructing the post data
*
* PARAMETERS
*   pInData :       [IN] input data
*   u32InDataLen :  [IN] input data lens
*   pOutData :      [OUT] output data
*   u32OutDataLen : [OUT] output data lens
*
* RETURNS
*   none
*
*************************************************************************/
int _CK_DataWifiCbcEncrypt(void *src , int len , unsigned char *iv , const unsigned char *key , void *out)
{
    int len1 = len & 0xfffffff0;
    int len2 = len1 + 16;
    int pad = len2 - len;
    uint32_t u32Keybits = 128;
    uint16_t i = 0;
    uint16_t u16BlockNum = 0;
    int ret = 0;
    void * pTempSrcPos = src;
    void * pTempOutPos = out;

    if((pTempSrcPos == NULL) || (pTempOutPos == NULL))
    {
        return -1;
    }
    mbedtls_aes_context aes_ctx = {0};

    mbedtls_aes_init(&aes_ctx);
    mbedtls_aes_setkey_enc(&aes_ctx , key , u32Keybits);

    if (len1) //execute encrypt for n-1 block
    {
        u16BlockNum = len >> 4 ;
        for (i = 0; i < u16BlockNum ; ++i)
        {
            ret = mbedtls_aes_crypt_cbc(&aes_ctx , MBEDTLS_AES_ENCRYPT , AES_BLOCK_SIZE, iv , (unsigned char *)pTempSrcPos , (unsigned char *)pTempOutPos);
            pTempSrcPos = ((char*)pTempSrcPos)+16;
            pTempOutPos = ((char*)pTempOutPos)+16;
        }
    }
    if (pad) //padding & execute encrypt for last block
    {
        char buf[16];
        memcpy((char *)buf, (char *)src + len1, len - len1);
        memset((char *)buf + len - len1, pad, pad);
        ret = mbedtls_aes_crypt_cbc(&aes_ctx , MBEDTLS_AES_ENCRYPT , AES_BLOCK_SIZE, iv , (unsigned char *)buf , (unsigned char *)out + len1);
    }
    mbedtls_aes_free(&aes_ctx);

    if(ret != 0)
        return -1;
    else
        return 0;
}

/*************************************************************************
* FUNCTION:
*   Cloud_DataParser
*
* DESCRIPTION:
*   parsing received data and activate to application
*
* PARAMETERS
*   pInData :       [IN] received data
*   u32InDataLen :  [IN] received data lens
*
* RETURNS
*   none
*
*************************************************************************/
int _CK_DataWifiCbcDecrypt(void *src, int len , unsigned char *iv , const unsigned char *key, void *out)
{
    mbedtls_aes_context aes_ctx = {0};
    int n = len >> 4;
    char *out_c = NULL;
    int offset = 0;
    int ret = 0;
    uint32_t u32Keybits = 128;
    uint16_t u16BlockNum = 0;
    char pad = 0;
    void * pTempSrcPos = src;
    void * pTempOutPos = out;
    uint16_t i = 0;

    if((pTempSrcPos == NULL) || (pTempOutPos == NULL))
    {
        return -1;
    }

    mbedtls_aes_init(&aes_ctx);
    mbedtls_aes_setkey_dec(&aes_ctx , key , u32Keybits);

    //decrypt n-1 block
    u16BlockNum = n - 1;
    if (n > 1)
    {
        for (i = 0; i < u16BlockNum ; ++i)
        {
            ret = mbedtls_aes_crypt_cbc(&aes_ctx , MBEDTLS_AES_DECRYPT , AES_BLOCK_SIZE, iv , (unsigned char *)pTempSrcPos , (unsigned char *)pTempOutPos);
            pTempSrcPos = ((char*)pTempSrcPos)+16;
            pTempOutPos = ((char*)pTempOutPos)+16;
        }

    }

    out_c = (char *)out;
    offset = n > 0 ? ((n - 1) << 4) : 0;
    out_c[offset] = 0;

    //decrypt last block
    ret = mbedtls_aes_crypt_cbc(&aes_ctx , MBEDTLS_AES_DECRYPT , AES_BLOCK_SIZE, iv , (unsigned char *)src + offset , (unsigned char *)out_c + offset);

    //paddind data set 0
    pad = out_c[len - 1];
    out_c[len - pad] = 0;

    mbedtls_aes_free(&aes_ctx);

    if(ret != 0)
        return -1;
    else
        return 0;
}
