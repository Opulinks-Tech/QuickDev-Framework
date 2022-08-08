/******************************************************************************
*  Copyright 2017 - 2018, Opulinks Technology Ltd.
*  ---------------------------------------------------------------------------
*  Statement:
*  ----------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Opulinks Technology Ltd. (C) 2020
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmsis_os.h"

#include "opulinks.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "boot_sequence.h"
#include "ps_task.h"
#include "hal_system.h"
#include "sys_os_config.h"
#include "hal_os_patch.h"

typedef int (*T_inHandlerModeFp)(void);
extern RET_DATA T_inHandlerModeFp inHandlerMode;

/************************  Message Queue Management Functions  *****************/

#if (defined (osFeature_MessageQ)  &&  (osFeature_MessageQ != 0)) /* Use Message Queues */

/**
* @brief Put a Message to the front of a Queue.
* @param  queue_id  message queue ID obtained with \ref osMessageCreate.
* @param  info      message information.
* @param  millisec  timeout value or 0 in case of no time-out.
* @retval status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osMessagePut shall be consistent in every CMSIS-RTOS.
*/
osStatus osMessagePutFront (osMessageQId queue_id, uint32_t info, uint32_t millisec)
{
    portBASE_TYPE taskWoken = pdFALSE;
    TickType_t ticks;

    ticks = millisec / portTICK_PERIOD_MS;
    if (ticks == 0)
    {
        ticks = 1;
    }

    if (inHandlerMode())
    {
        if (xQueueSendToFrontFromISR(queue_id, &info, &taskWoken) != pdTRUE)
        {
            return osErrorOS;
        }
        portEND_SWITCHING_ISR(taskWoken);
    }
    else
    {
        if (xQueueSendToFront(queue_id, &info, ticks) != pdTRUE)
        {
            return osErrorOS;
        }
    }

    return osOK;
}

/**
* @brief Get a Message or Wait for a Message from a Queue.
* @param  queue_id  message queue ID obtained with \ref osMessageCreate.
* @param  millisec  timeout value or 0 in case of no time-out.
* @retval event information that includes status code.
* @note   MUST REMAIN UNCHANGED: \b osMessageGet shall be consistent in every CMSIS-RTOS.
*/
osEvent osMessagePeek (osMessageQId queue_id, uint32_t millisec)
{
    TickType_t ticks;
    osEvent event;

    event.def.message_id = queue_id;

    if (queue_id == NULL)
    {
        event.status = osErrorParameter;
        return event;
    }

    ticks = 0;
    if (millisec == osWaitForever)
    {
        ticks = portMAX_DELAY;
    }
    else if (millisec != 0)
    {
        ticks = millisec / portTICK_PERIOD_MS;
        if (ticks == 0)
        {
            ticks = 1;
        }
    }
    if (inHandlerMode())
    {
        if (xQueuePeekFromISR(queue_id, &event.value.v) == pdTRUE)
        {
            /* We have mail */
            event.status = osEventMessage;
        }
        else
        {
            event.status = osOK;
        }
    }
    else
    {
        if (xQueuePeek(queue_id, &event.value.v, ticks) == pdTRUE)
        {
            /* We have mail */
            event.status = osEventMessage;
        }
        else
        {
            event.status = (ticks == 0) ? osOK : osEventTimeout;
        }
    }

    return event;
}

#endif     /* Use Message Queues */

/************************  SW patch ********************************************/
