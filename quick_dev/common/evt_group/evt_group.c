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
*  evt_group.c
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

#include "evt_group.h"
#include "log.h"

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
T_OplErr EG_Create(EventGroupHandle_t *tEventGroup)
{
    if(tEventGroup == NULL)
    {
        return OPL_ERR;
    }

    *tEventGroup = xEventGroupCreate();

    if(*tEventGroup == NULL)
    {
        OPL_LOG_ERRO(EG, "Create event group fail");
        return OPL_ERR;
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
void EG_StatusSet(EventGroupHandle_t tEventGroup , uint32_t dwEventBit , uint8_t status)
{
// ISR mode is not supported.
#if 0
    BaseType_t xHigherPriorityTaskWoken, xResult;

    // check if it is ISR mode or not
    if (0 != __get_IPSR())
    {
        if (true == status)
        {
            // xHigherPriorityTaskWoken must be initialised to pdFALSE.
    		xHigherPriorityTaskWoken = pdFALSE;

            // Set bit in xEventGroup.
            xResult = xEventGroupSetBitsFromISR(tEventGroup , dwEventBit, &xHigherPriorityTaskWoken);
            if( xResult == pdPASS )
    		{
    			// If xHigherPriorityTaskWoken is now set to pdTRUE then a context
    			// switch should be requested.  The macro used is port specific and
    			// will be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() -
    			// refer to the documentation page for the port being used.
    			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    		}
        }
        else
            xEventGroupClearBitsFromISR(tEventGroup , dwEventBit);
    }
    // Taske mode
    else
#endif
    {
        if (true == status)
        {
            xEventGroupSetBits(tEventGroup, dwEventBit);
        }
        else
        {
            xEventGroupClearBits(tEventGroup, dwEventBit);
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
uint8_t EG_StatusGet(EventGroupHandle_t tEventGroup , uint32_t dwEventBit)
{
    EventBits_t tRetBit;

    tRetBit = xEventGroupGetBits(tEventGroup);

    if (dwEventBit == (dwEventBit & tRetBit))
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
T_OplErr EG_StatusWait(EventGroupHandle_t tEventGroup , uint32_t dwEventBit , uint32_t millisec)
{
    EventBits_t tRetBit;

    tRetBit = xEventGroupWaitBits(tEventGroup,
                                  dwEventBit,
                                  pdFALSE,
                                  pdTRUE,
                                  millisec);

    if (dwEventBit == (dwEventBit & tRetBit))
    {
        return OPL_OK;
    }

    return OPL_ERR;
}
