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
*  sha_256.c
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

#include "sha_256.h"
#if defined(OPL1000_A2) || defined(OPL1000_A3)
#include "scrt_patch.h"
#elif defined(OPL2500_A0)
#include "scrt.h"
#endif

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
*   SHA256_Calucate
*
* DESCRIPTION:
*   calculate the value of SHA256 
*
* PARAMETERS
*   SHA256_Output : [OUT] an empty array to store the value of SHA256
*   len :           [IN] input the size value of array
*   uwSHA256_Buff : [IN] sha256 buffer
*
* RETURNS
*   int :           [IN] true -> setting complete
*                        false -> error
*
*************************************************************************/
int SHA256_Calucate(uint8_t SHA256_Output[], int len, unsigned char uwSHA256_Buff[])
{       
    /*
        int nl_scrt_sha
        (
            uint8_t u8Type,         => SCRT_TYPE_SHA_256
            uint8_t u8Step,         => 0 (Data send once, so this value set 0.)
            uint32_t u32TotalLen,   => input data length
            uint8_t *u8aData,       => start address of input data
            uint32_t u32DataLen,    => same as u32TotalLen
            uint8_t u8HasInterMac,  => 0
            uint8_t *u8aMac         => output buffer: caller needs to prepare a 32-bytes buffer to save SHA-256 output
        )
    */

    if(!nl_scrt_sha(SCRT_TYPE_SHA_256, 0, len, uwSHA256_Buff, len, 0, SHA256_Output))
    {
        tracer_cli(LOG_HIGH_LEVEL, "nl_scrt_sha_1_step fail\n");
        return false;
    }
    
    return true;
}

/*************************************************************************
* FUNCTION:
*   SHA256_ValueForOta
*
* DESCRIPTION:
*   convert date to unix timestamp
*
* PARAMETERS
*   date :          [IN] pointer to structure represeting the date and time
*
* RETURNS
*   time_t :        [OUT] unix timestamp
*
*************************************************************************/
int SHA256_ValueForOta(time_t TimeStamp, uint8_t ubaSHA256CalcStrBuf[])
{
    int len = 0;
    unsigned char uwSHA256_Buff[SHA256_BUF_SIZE] = {0};

    /* Combine https Post key */
    memset(uwSHA256_Buff, 0, SHA256_BUF_SIZE);
    len = sprintf((char *)uwSHA256_Buff, SHA256_FOR_OTA_FORMAT, g_tHttpPostContent.ubaDeviceId, TimeStamp, g_tHttpPostContent.ubaApiKey);

    SHA256_Calucate(ubaSHA256CalcStrBuf, len, uwSHA256_Buff);

    return 0;
}
