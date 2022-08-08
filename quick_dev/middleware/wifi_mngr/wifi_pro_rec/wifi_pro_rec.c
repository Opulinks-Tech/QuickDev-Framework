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
*  file_temp.c
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

#include "mw_fim.h"
#include "opl_err.h"
#include "qd_config.h"
#include "qd_module.h"
#include "wifi_api.h"
#include "wifi_mngr.h"
#include "wifi_pro_rec.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if (WM_ENABLED == 1)

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

extern const T_PrApProfile g_tMwFimDefaultGp11ApProfile;
extern const T_PrApProfile g_tMwFimDefaultGp11FixedApProfile;

// Sec 5: declaration of global function prototype

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

osSemaphoreId g_tProfileProtectSemaphoreId;

T_PrApProfile g_tPrAPInfo[PR_AP_PROFILE_MAX_NUM];

// Sec 7: declaration of static function prototype

void _PrIdxFind(uint32_t *pu32Latest, uint32_t *pu32Next);

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
void _PrIdxFind(uint32_t *pu32Latest, uint32_t *pu32Next)
{
    uint32_t i;
    uint32_t u32NextIdx = 0;

    *pu32Latest = PR_AP_OFFSET;
    *pu32Next = PR_AP_OFFSET;

    for (i = PR_AP_OFFSET; i < PR_AP_PROFILE_MAX_NUM; i++)
    {
        if (g_tPrAPInfo[i].u8Used)
        {
            u32NextIdx = (i + 1) % PR_AP_PROFILE_MAX_NUM;

            if (0 == u32NextIdx)
            {
                u32NextIdx = PR_AP_OFFSET;
            }

            // Found
            if ( ((g_tPrAPInfo[i].u8SeqNo+1) % PR_SEQ_MAX_NUM) != g_tPrAPInfo[u32NextIdx].u8SeqNo )
            {
                *pu32Latest = i;
                *pu32Next = u32NextIdx;

                break;
            }
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
void WM_PrInit(void)
{
    int i;

    // create semaphore
    osSemaphoreDef_t tSemaphoreDef;

    tSemaphoreDef.dummy = 0;
    g_tProfileProtectSemaphoreId = osSemaphoreCreate(&tSemaphoreDef, 1);

    if (g_tProfileProtectSemaphoreId == NULL)
    {
        WM_LOG_ERRO("Create PR semaphore fail");
        return;
    }

    memset(g_tPrAPInfo, 0, sizeof(g_tPrAPInfo));

#ifdef WM_PR_FIX_AP_ENABLED
    for(i = 0; i < WM_PR_FIX_AP_NUM; i++)
    { 
        if ( MW_FIM_OK != MwFim_FileRead( MW_FIM_IDX_GP11_PROJECT_AP_PROFILE, 
                                          i, 
                                          MW_FIM_GP11_AP_PROFILE_SIZE, 
                                          (uint8_t*)&(g_tPrAPInfo[i])
                                        )
           )

        {
            WM_LOG_ERRO("Fix AP read fail");
            memcpy( &(g_tPrAPInfo[i]), &g_tMwFimDefaultGp11FixedApProfile, MW_FIM_GP11_AP_PROFILE_SIZE);
        }

        if (g_tPrAPInfo[i].u8Used) // Should be true
        {
            WM_LOG_DEBG("Fix AP Info[%d]=%s", i, g_tPrAPInfo[i].u8Ssid);
        }
    }
#endif

    for (i = PR_AP_OFFSET; i < PR_AP_PROFILE_MAX_NUM; i++)
    {
        if ( MW_FIM_OK != MwFim_FileRead( MW_FIM_IDX_GP11_PROJECT_AP_PROFILE, 
                                          i, 
                                          MW_FIM_GP11_AP_PROFILE_SIZE, 
                                          (uint8_t*)&(g_tPrAPInfo[i])
                                        )
           )

        {
            WM_LOG_ERRO("AP read fail");
            memcpy( &(g_tPrAPInfo[i]), &g_tMwFimDefaultGp11ApProfile, MW_FIM_GP11_AP_PROFILE_SIZE);
        }

        if (g_tPrAPInfo[i].u8Used)
        {
            WM_LOG_DEBG("AP Info[%d]=%s, s[%d], u[%d]", i,
                                                    g_tPrAPInfo[i].u8Ssid,
                                                    g_tPrAPInfo[i].u8SeqNo,
                                                    g_tPrAPInfo[i].u8Used);
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
int WM_PrInsert(T_PrApProfile tNewProfile)
{
    int i;
    uint32_t u32Lasted = PR_AP_OFFSET;
    uint32_t u32Next = PR_AP_OFFSET;

    // protect the data
    osSemaphoreWait(g_tProfileProtectSemaphoreId, osWaitForever);

#ifdef WM_PR_FIX_AP_ENABLED
    // Check if same SSID as fixd AP profile, do nothing and return
    for (i = 0; i < WM_PR_FIX_AP_NUM; i++)
    {
        if (g_tPrAPInfo[i].u8Used)  // Should be true
        {
            if (0 == strcmp((const char*)tNewProfile.u8Ssid, (const char*)g_tPrAPInfo[i].u8Ssid))
            {
                WM_LOG_INFO("SSID is the same as fixed ssid, refuse to insert!");

                osSemaphoreRelease(g_tProfileProtectSemaphoreId);

                return 1;
            }
        }
    }
#endif

    // Checki if same SSID as exist AP profile, overwrite the exist profile
    for (i = PR_AP_OFFSET; i < PR_AP_PROFILE_MAX_NUM; i++)
    {
        if (g_tPrAPInfo[i].u8Used)
        {
            if (0 == strcmp((const char*)g_tPrAPInfo[i].u8Ssid, (const char*)tNewProfile.u8Ssid))
            {
                if (0 != strcmp((const char*)g_tPrAPInfo[i].u8Pwd, (const char*)tNewProfile.u8Pwd))
                {
                    // Overwrite
                    memcpy((void*)&g_tPrAPInfo[i].u8Pwd, (void*)&tNewProfile.u8Pwd, sizeof(tNewProfile.u8Pwd));

                    WM_LOG_INFO("Update PWD, SSID(Index=%d):%s", i, tNewProfile.u8Ssid);

                    if (MW_FIM_OK != MwFim_FileWrite(MW_FIM_IDX_GP11_PROJECT_AP_PROFILE, i, MW_FIM_GP11_AP_PROFILE_SIZE, (uint8_t*)&(g_tPrAPInfo[i])))
                    {
                        WM_LOG_ERRO("Update AP write fail");

                        osSemaphoreRelease(g_tProfileProtectSemaphoreId);
                       
                        return 2;
                    }
                }

                osSemaphoreRelease(g_tProfileProtectSemaphoreId);
              
                return 0;
           }
        }
    }

    _PrIdxFind(&u32Lasted, &u32Next);

    tNewProfile.u8SeqNo = (g_tPrAPInfo[u32Lasted].u8SeqNo+1) % PR_SEQ_MAX_NUM;
    tNewProfile.u8Used = 1;

    // Replace next AP profile
    memset((void*)&g_tPrAPInfo[u32Next], 0, sizeof(T_PrApProfile));
    memcpy((void*)&g_tPrAPInfo[u32Next], (void*)&tNewProfile, sizeof(T_PrApProfile));

    WM_LOG_INFO("Insert SSID(Index=%d):%s, s[%d]", u32Next,
                                                g_tPrAPInfo[u32Next].u8Ssid,
                                                g_tPrAPInfo[u32Next].u8SeqNo);
    
    if (MW_FIM_OK != MwFim_FileWrite(MW_FIM_IDX_GP11_PROJECT_AP_PROFILE, u32Next, MW_FIM_GP11_AP_PROFILE_SIZE, (uint8_t*)&(g_tPrAPInfo[u32Next])))
    {
        WM_LOG_ERRO("AP write fail");

        osSemaphoreRelease(g_tProfileProtectSemaphoreId);

        return 2;
    }

    osSemaphoreRelease(g_tProfileProtectSemaphoreId);

    return 0;
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
void WM_PrClear(void)
{    
    int i;

    // protect the data
    osSemaphoreWait(g_tProfileProtectSemaphoreId, osWaitForever);

    for (i = PR_AP_OFFSET; i < PR_AP_PROFILE_MAX_NUM; i++)
    {
        memset((void *)&g_tPrAPInfo[i], 0, MW_FIM_GP11_AP_PROFILE_SIZE);
        if (MW_FIM_OK != MwFim_FileWrite(MW_FIM_IDX_GP11_PROJECT_AP_PROFILE, i, MW_FIM_GP11_AP_PROFILE_SIZE, (uint8_t*)&(g_tPrAPInfo[i])))
        {
            WM_LOG_ERRO("Clear AP write fail");
        }
    }

    // clear auto_connect list in SDK layer
    wifi_auto_connect_reset();

    osSemaphoreRelease(g_tProfileProtectSemaphoreId);
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
T_PrApProfilePtr WM_PrGet(void)
{
    return g_tPrAPInfo;
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
uint32_t WM_PrProfileCount(void)
{
    int i;
    uint32_t u32ProCount = 0;

    for (i = 0; i < PR_AP_PROFILE_MAX_NUM; i++)
    {
        if (g_tPrAPInfo[i].u8Used)
        {
            u32ProCount++;
        }
    }

    return u32ProCount;
}

#endif /* WM_ENABLED */
