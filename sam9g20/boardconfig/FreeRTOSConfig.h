/*
    FreeRTOS Kernel V10.3.1

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that has become a de facto standard.             *
     *                                                                       *
     *    Help yourself get started quickly and support the FreeRTOS         *
     *    project by purchasing a FreeRTOS tutorial book, reference          *
     *    manual, or both from: http://www.FreeRTOS.org/Documentation        *
     *                                                                       *
     *    Thank you!                                                         *
     *                                                                       *
    ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    >>! NOTE: The modification to the GPL is included to allow you to distribute
    >>! a combined work that includes FreeRTOS without being obliged to provide
    >>! the source code for proprietary components outside of the FreeRTOS
    >>! kernel.

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available from the following
    link: http://www.freertos.org/a00114.html

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?"                                     *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org - Documentation, books, training, latest versions,
    license and Real Time Engineers Ltd. contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High
    Integrity Systems to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

#ifndef SAM9G20_FREERTOS_CONFIG_H
#define SAM9G20_FREERTOS_CONFIG_H

#include <at91/utility/trace.h>
#include <at91/utility/assert.h>
#include <board.h>

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 * Also good: http://www.lirmm.fr/~bosio/HMEE209/06%20-%20freertos.pdf
 *----------------------------------------------------------*/

/**
 * Specifies the scheduler to run in pre-emptive mode when set to 1
 * and to run in cooperative mode when set to 0.
 *
 * ISIS default configuration is to set this to 0, but
 * we don't know yet whether settings this to 1 has
 * an influence on ISIS drivers internal functions (no source code given)
 */
#define configUSE_PREEMPTION			1
/**
 * This parameter is only used if task pre-emption is on.
 * It specifies the scheduler to share processing time between
 * tasks of equal priority
 */
#define configUSE_TIME_SLICING			1
// ISIS uses FreeRTOS V7.5.3
#define configENABLE_BACKWARD_COMPATIBILITY 1
#define configCPU_CLOCK_HZ				( ( unsigned long ) BOARD_MCK )
// Do not change this value
#define configTICK_RATE_HZ				( ( portTickType ) 1000 )
#define configMAX_PRIORITIES			10
#define configMINIMAL_STACK_SIZE		( ( unsigned short ) 300 )
#define configMAX_TASK_NAME_LEN			24
#define configUSE_16_BIT_TICKS			0
#define configIDLE_SHOULD_YIELD			0
#define configUSE_MUTEXES				1

/* Memory allocation related definitions. */
// added for heap_4 memory management
#define configSUPPORT_DYNAMIC_ALLOCATION    1
// maximum of heap freertos can use to allocate memory for tasks
// we have plenty of SDRAM, so feel free to increase this if the remaining
// heap is getting low.
#define configTOTAL_HEAP_SIZE			( ( size_t ) ( 400000 ) )

/* Hook function related definitions. */
#define configUSE_IDLE_HOOK				1
#define configUSE_TICK_HOOK				0
#define configCHECK_FOR_STACK_OVERFLOW	1

#define configUSE_RECURSIVE_MUTEXES		1
#define configSTACK_DEPTH_TYPE  		portSTACK_TYPE
// Defines the maximum number of queues and semaphores that can be registered
// Only those queues and semaphores that should be viewed using a
// RTOS kernel aware debugger need be registered
#define configQUEUE_REGISTRY_SIZE		32
#define configUSE_COUNTING_SEMAPHORES	1

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 		0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Define configASSERT() to call vAssertCalled() if the assertion fails.  The assertion
has failed if the value of the parameter passed into configASSERT() equals zero. */
#define configASSERT( x )  SANITY_CHECK( ( x ) )
//#define configASSERT( x ) if ((x) == 0) {taskDISABLE_INTERRUPTS(); for( ;; );}

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet			1
#define INCLUDE_uxTaskPriorityGet			1
#define INCLUDE_vTaskDelete					1
#define INCLUDE_vTaskCleanUpResources		0
#define INCLUDE_vTaskSuspend				1
#define INCLUDE_vTaskDelayUntil				1
#define INCLUDE_vTaskDelay					1
#define INCLUDE_xTaskResumeFromISR			1
#define INCLUDE_xTaskGetCurrentTaskHandle	1
#define INCLUDE_eTaskGetState               1
#define INCLUDE_xTaskGetSchedulerState		1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_xTaskGetIdleTaskHandle		1

/* Run time stats. Can be turned off for mission code. */
#define configGENERATE_RUN_TIME_STATS   		0
#define configUSE_STATS_FORMATTING_FUNCTIONS 	0
#define configUSE_TRACE_FACILITY				0
// Defined in portwrapper.cpp inside the utility folder.
extern void vConfigureTimerForRunTimeStats();
extern uint32_t vGetCurrentTimerCounterValue();
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() vConfigureTimerForRunTimeStats()
#define portGET_RUN_TIME_COUNTER_VALUE() vGetCurrentTimerCounterValue()

#endif /* SAM9G20_FREERTOS_CONFIG_H */
