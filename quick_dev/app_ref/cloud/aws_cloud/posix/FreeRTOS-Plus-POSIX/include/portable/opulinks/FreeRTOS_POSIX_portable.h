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
 * @file FreeRTOS_POSIX_portable.h
 * @brief Port-specific configuration of FreeRTOS+POSIX.
 */

#ifndef _FREERTOS_POSIX_PORTABLE_H_
#define _FREERTOS_POSIX_PORTABLE_H_



/* Opulinks already defines the following types. */
#define posixconfigENABLE_SSIZE_T         1
//#define posixconfigENABLE_TIMESPEC      0
//#define posixconfigENABLE_ITIMERSPEC    0
//#define posixconfigENABLE_CLOCKID_T     0
#define posixconfigENABLE_TM            1
//#define posixconfigENABLE_TIME_T        0
//#define posixconfigENABLE_TIMER_T       0
/* This port uses the defaults in FreeRTOS_POSIX_portable_default.h, so this
 * file is empty. */

/* Use the FreeRTOS+POSIX time.h header instead of the ARMCC time.h. Disable
 * time.h by defining its double inclusion guard. */
#define __time_h
#include <time.h>

//for pvPortMalloc(), vPortFree
#include "cmsis_os.h"
//#include "sys/types.h"
#include "lwip/errno.h" // for errno


#endif /* _FREERTOS_POSIX_PORTABLE_H_ */
