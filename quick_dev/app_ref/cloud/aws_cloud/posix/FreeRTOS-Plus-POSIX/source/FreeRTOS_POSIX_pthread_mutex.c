/*
 * Amazon FreeRTOS+POSIX V1.0.1
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/**
 * @file FreeRTOS_POSIX_pthread_mutex.c
 * @brief Implementation of mutex functions in pthread.h
 */

/* C standard library includes. */
#include <stddef.h>
#include <string.h>

/* FreeRTOS+POSIX includes. */
#include "FreeRTOS_POSIX.h"
#include "FreeRTOS_POSIX/errno.h"
#include "FreeRTOS_POSIX/pthread.h"
#include "FreeRTOS_POSIX/utils.h"

#define pxMutexHolder					pcTail
#define uxQueueType						pcHead
#define queueQUEUE_IS_MUTEX				NULL

/*
 * Definition of the queue used by the scheduler.
 * Items are queued by copy, not reference.  See the following link for the
 * rationale: http://www.freertos.org/Embedded-RTOS-Queues.html
 */
typedef struct QueueDefinition
{
	int8_t *pcHead;					/*< Points to the beginning of the queue storage area. */
	int8_t *pcTail;					/*< Points to the byte at the end of the queue storage area.  Once more byte is allocated than necessary to store the queue items, this is used as a marker. */
	int8_t *pcWriteTo;				/*< Points to the free next place in the storage area. */

	union							/* Use of a union is an exception to the coding standard to ensure two mutually exclusive structure members don't appear simultaneously (wasting RAM). */
	{
		int8_t *pcReadFrom;			/*< Points to the last place that a queued item was read from when the structure is used as a queue. */
		UBaseType_t uxRecursiveCallCount;/*< Maintains a count of the number of times a recursive mutex has been recursively 'taken' when the structure is used as a mutex. */
	} u;

	List_t xTasksWaitingToSend;		/*< List of tasks that are blocked waiting to post onto this queue.  Stored in priority order. */
	List_t xTasksWaitingToReceive;	/*< List of tasks that are blocked waiting to read from this queue.  Stored in priority order. */

	volatile UBaseType_t uxMessagesWaiting;/*< The number of items currently in the queue. */
	UBaseType_t uxLength;			/*< The length of the queue defined as the number of items it will hold, not the number of bytes. */
	UBaseType_t uxItemSize;			/*< The size of each items that the queue will hold. */

	volatile int8_t cRxLock;		/*< Stores the number of items received from the queue (removed from the queue) while the queue was locked.  Set to queueUNLOCKED when the queue is not locked. */
	volatile int8_t cTxLock;		/*< Stores the number of items transmitted to the queue (added to the queue) while the queue was locked.  Set to queueUNLOCKED when the queue is not locked. */

	#if( ( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
		uint8_t ucStaticallyAllocated;	/*< Set to pdTRUE if the memory used by the queue was statically allocated to ensure no attempt is made to free the memory. */
	#endif

	#if ( configUSE_QUEUE_SETS == 1 )
		struct QueueDefinition *pxQueueSetContainer;
	#endif

	#if ( configUSE_TRACE_FACILITY == 1 )
		UBaseType_t uxQueueNumber;
		uint8_t ucQueueType;
	#endif

} xQUEUE;

/* The old xQUEUE name is maintained above then typedefed to the new Queue_t
name below to enable the use of older kernel aware debuggers. */
typedef xQUEUE Queue_t;


void* xQueueGetMutexHolder( QueueHandle_t xSemaphore )
{
    void *pxReturn;

    /* This function is called by xSemaphoreGetMutexHolder(), and should not
    be called directly.  Note:  This is a good way of determining if the
    calling task is the mutex holder, but not a good way of determining the
    identity of the mutex holder, as the holder may change between the
    following critical section exiting and the function returning. */
    taskENTER_CRITICAL();
    {
        if( ( ( Queue_t * ) xSemaphore )->uxQueueType == queueQUEUE_IS_MUTEX )
        {
            pxReturn = ( void * ) ( ( Queue_t * ) xSemaphore )->pxMutexHolder;
        }
        else
        {
            pxReturn = NULL;
        }
    }
    taskEXIT_CRITICAL();

    return pxReturn;
} /*lint !e818 xSemaphore cannot be a pointer to const because it is a typedef. */




/**
 * @brief Initialize a PTHREAD_MUTEX_INITIALIZER mutex.
 *
 * PTHREAD_MUTEX_INITIALIZER sets a flag for a mutex to be initialized later.
 * This function performs the initialization.
 * @param[in] pxMutex The mutex to initialize.
 *
 * @return nothing
 */
static void prvInitializeStaticMutex( pthread_mutex_internal_t * pxMutex );

/**
 * @brief Default pthread_mutexattr_t.
 */
static const pthread_mutexattr_internal_t xDefaultMutexAttributes =
{
    .iType = PTHREAD_MUTEX_DEFAULT,
};

/*-----------------------------------------------------------*/

static void prvInitializeStaticMutex( pthread_mutex_internal_t * pxMutex )
{
    /* Check if the mutex needs to be initialized. */
    if( pxMutex->xIsInitialized == pdFALSE )
    {
        /* Mutex initialization must be in a critical section to prevent two threads
         * from initializing it at the same time. */
        taskENTER_CRITICAL();

        /* Check again that the mutex is still uninitialized, i.e. it wasn't
         * initialized while this function was waiting to enter the critical
         * section. */
        if( pxMutex->xIsInitialized == pdFALSE )
        {
            /* Set the mutex as the default type. */
            pxMutex->xAttr.iType = PTHREAD_MUTEX_DEFAULT;

            /* Call the correct FreeRTOS mutex initialization function based on
             * the mutex type. */
            #if PTHREAD_MUTEX_DEFAULT == PTHREAD_MUTEX_RECURSIVE
                ( void ) xSemaphoreCreateRecursiveMutexStatic( &pxMutex->xMutex );
            #else
                ( void ) xSemaphoreCreateMutexStatic( &pxMutex->xMutex );
            #endif

            pxMutex->xIsInitialized = pdTRUE;
        }

        /* Exit the critical section. */
        taskEXIT_CRITICAL();
    }
}

/*-----------------------------------------------------------*/

int pthread_mutex_destroy( pthread_mutex_t * mutex )
{
    pthread_mutex_internal_t * pxMutex = ( pthread_mutex_internal_t * ) ( *mutex );

    /* Free resources in use by the mutex. */
    if( pxMutex->xTaskOwner == NULL )
    {
        vSemaphoreDelete( ( SemaphoreHandle_t ) &pxMutex->xMutex );
        vPortFree( pxMutex );
    }

    return 0;
}

/*-----------------------------------------------------------*/

int pthread_mutex_init( pthread_mutex_t * mutex,
                        const pthread_mutexattr_t * attr )
{
    int iStatus = 0;
    pthread_mutex_internal_t * pxMutex = NULL;

    /* Allocate memory for new mutex object. */
    pxMutex = ( pthread_mutex_internal_t * ) pvPortMalloc( sizeof( pthread_mutex_internal_t ) );

    if( pxMutex == NULL )
    {
        /* No memory. */
        iStatus = ENOMEM;
    }

    if( iStatus == 0 )
    {
        /* Clear the newly-allocated mutex. */
        ( void ) memset( pxMutex, 0x00, sizeof( pthread_mutex_internal_t ) );

        /* No attributes given, use default attributes. */
        if( attr == NULL )
        {
            pxMutex->xAttr = xDefaultMutexAttributes;
        }
        /* Otherwise, use provided attributes. */
        else
        {
            pxMutex->xAttr = *( ( pthread_mutexattr_internal_t * ) ( *attr ) );
        }

        /* Call the correct FreeRTOS mutex creation function based on mutex type. */
        if( pxMutex->xAttr.iType == PTHREAD_MUTEX_RECURSIVE )
        {
            /* Recursive mutex. */
            ( void ) xSemaphoreCreateRecursiveMutexStatic( &pxMutex->xMutex );
        }
        else
        {
            /* All other mutex types. */
            ( void ) xSemaphoreCreateMutexStatic( &pxMutex->xMutex );
        }

        /* Ensure that the FreeRTOS mutex was successfully created. */
        if( ( SemaphoreHandle_t ) &pxMutex->xMutex == NULL )
        {
            /* Failed to create mutex. Set error EAGAIN and free mutex object. */
            iStatus = EAGAIN;
            vPortFree( pxMutex );
        }
        else
        {
            /* Mutex successfully created. */
            pxMutex->xIsInitialized = pdTRUE;
            *mutex = ( pthread_mutex_t ) pxMutex;
        }
    }

    return iStatus;
}

/*-----------------------------------------------------------*/

int pthread_mutex_lock( pthread_mutex_t * mutex )
{
    return pthread_mutex_timedlock( mutex, NULL );
}

/*-----------------------------------------------------------*/

int pthread_mutex_timedlock( pthread_mutex_t * mutex,
                             const struct timespec * abstime )
{
    int iStatus = 0;
    pthread_mutex_internal_t * pxMutex = ( pthread_mutex_internal_t * ) ( *mutex );
    TickType_t xDelay = portMAX_DELAY;
    BaseType_t xFreeRTOSMutexTakeStatus = pdFALSE;

    /* If mutex in uninitialized, perform initialization. */
    prvInitializeStaticMutex( pxMutex );

    /* At this point, the mutex should be initialized. */
    configASSERT( pxMutex->xIsInitialized == pdTRUE );

    /* Convert abstime to a delay in TickType_t if provided. */
    if( abstime != NULL )
    {
        iStatus = UTILS_AbsoluteTimespecToTicks( abstime, &xDelay );

        /* If abstime was in the past, still attempt to lock the mutex without
         * blocking, per POSIX spec. */
        if( iStatus == ETIMEDOUT )
        {
            xDelay = 0;
            iStatus = 0;
        }
    }

    /* Check if trying to lock a currently owned mutex. */
    if( ( iStatus == 0 ) &&
        ( pxMutex->xAttr.iType == PTHREAD_MUTEX_ERRORCHECK ) &&  /* Only PTHREAD_MUTEX_ERRORCHECK type detects deadlock. */
        ( pxMutex->xTaskOwner == xTaskGetCurrentTaskHandle() ) ) /* Check if locking a currently owned mutex. */
    {
        iStatus = EDEADLK;
    }

    if( iStatus == 0 )
    {
        /* Call the correct FreeRTOS mutex take function based on mutex type. */
        if( pxMutex->xAttr.iType == PTHREAD_MUTEX_RECURSIVE )
        {
            xFreeRTOSMutexTakeStatus = xSemaphoreTakeRecursive( ( SemaphoreHandle_t ) &pxMutex->xMutex, xDelay );
        }
        else
        {
            xFreeRTOSMutexTakeStatus = xSemaphoreTake( ( SemaphoreHandle_t ) &pxMutex->xMutex, xDelay );
        }

        /* If the mutex was successfully taken, set its owner. */
        if( xFreeRTOSMutexTakeStatus == pdPASS )
        {
            pxMutex->xTaskOwner = xTaskGetCurrentTaskHandle();
        }
        /* Otherwise, the mutex take timed out. */
        else
        {
            iStatus = ETIMEDOUT;
        }
    }

    return iStatus;
}

/*-----------------------------------------------------------*/

int pthread_mutex_trylock( pthread_mutex_t * mutex )
{
    int iStatus = 0;
    struct timespec xTimeout =
    {
        .tv_sec  = 0,
        .tv_nsec = 0
    };

    /* Attempt to lock with no timeout. */
    iStatus = pthread_mutex_timedlock( mutex, &xTimeout );

    /* POSIX specifies that this function should return EBUSY instead of
     * ETIMEDOUT for attempting to lock a locked mutex. */
    if( iStatus == ETIMEDOUT )
    {
        iStatus = EBUSY;
    }

    return iStatus;
}

/*-----------------------------------------------------------*/

int pthread_mutex_unlock( pthread_mutex_t * mutex )
{
    int iStatus = 0;
    pthread_mutex_internal_t * pxMutex = ( pthread_mutex_internal_t * ) ( *mutex );

    /* If mutex in uninitialized, perform initialization. */
    prvInitializeStaticMutex( pxMutex );

    /* Check if trying to unlock an unowned mutex. */
    if( ( ( pxMutex->xAttr.iType == PTHREAD_MUTEX_ERRORCHECK ) ||
          ( pxMutex->xAttr.iType == PTHREAD_MUTEX_RECURSIVE ) ) &&
        ( pxMutex->xTaskOwner != xTaskGetCurrentTaskHandle() ) )
    {
        iStatus = EPERM;
    }

    if( iStatus == 0 )
    {
        /* Call the correct FreeRTOS mutex unlock function based on mutex type. */
        if( pxMutex->xAttr.iType == PTHREAD_MUTEX_RECURSIVE )
        {
            ( void ) xSemaphoreGiveRecursive( ( SemaphoreHandle_t ) &pxMutex->xMutex );
        }
        else
        {
            ( void ) xSemaphoreGive( ( SemaphoreHandle_t ) &pxMutex->xMutex );
        }

        /* Update the owner of the mutex. A recursive mutex may still have an
         * owner, so it should be updated with xSemaphoreGetMutexHolder. */
        pxMutex->xTaskOwner = xSemaphoreGetMutexHolder( ( SemaphoreHandle_t ) &pxMutex->xMutex );
    }

    return iStatus;
}

/*-----------------------------------------------------------*/

int pthread_mutexattr_destroy( pthread_mutexattr_t * attr )
{
    /* Free mutex attributes object. */
    vPortFree( *attr );

    return 0;
}

/*-----------------------------------------------------------*/

int pthread_mutexattr_gettype( const pthread_mutexattr_t * attr,
                               int * type )
{
    pthread_mutexattr_internal_t * pxAttr = ( pthread_mutexattr_internal_t * ) ( *attr );

    *type = pxAttr->iType;

    return 0;
}

/*-----------------------------------------------------------*/

int pthread_mutexattr_init( pthread_mutexattr_t * attr )
{
    int iStatus = 0;

    /* Allocate memory for new mutex attributes object. */
    *attr = pvPortMalloc( sizeof( pthread_mutexattr_internal_t ) );

    if( attr == NULL )
    {
        /* No memory. */
        iStatus = ENOMEM;
    }

    /* Set the mutex attributes to default values. */
    if( iStatus == 0 )
    {
        *( ( pthread_mutexattr_internal_t * ) ( *attr ) ) = xDefaultMutexAttributes;
    }

    return iStatus;
}

/*-----------------------------------------------------------*/

int pthread_mutexattr_settype( pthread_mutexattr_t * attr,
                               int type )
{
    int iStatus = 0;
    pthread_mutexattr_internal_t * pxAttr = ( pthread_mutexattr_internal_t * ) ( *attr );

    switch( type )
    {
        case PTHREAD_MUTEX_NORMAL:
        case PTHREAD_MUTEX_RECURSIVE:
        case PTHREAD_MUTEX_ERRORCHECK:
            pxAttr->iType = type;
            break;

        default:
            iStatus = EINVAL;
            break;
    }

    return iStatus;
}

/*-----------------------------------------------------------*/
