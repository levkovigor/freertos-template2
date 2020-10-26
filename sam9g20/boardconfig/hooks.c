/*
 * hooks.c
 *
 *  Created on: 16 jan. 2015
 *      Author: pbot
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <at91/utility/exithandler.h>
#include <assert.h>
#include <stdio.h>

extern void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName) __attribute__ ((weak));
extern void vApplicationIdleHook( void ) __attribute__ ((weak));

/*
 * This hook is provided by FreeRTOS as a way for the upper layer (user code) to
 * handle a stack overflow without introducing an dependency in the FreeRTOS
 * code.
 */
void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName)
{
	printf("\n\r STACK OVERFLOW DETECTED!! \n\r");
	printf(" Culprit task %p name: %s \n\r", pxTask, pcTaskName);
	printf(" !!Restarting Now!! \n\r");
	restart();
}

/*
 * This hook is provided by FreeRTOS as a way for the upper layer (user code) to
 * determine what happens to the iOBC when idle without introducing an
 * dependency in the FreeRTOS code. This is an ideal place to kick the watchdog.
 * It is imperative that the hook function does not call any API functions that
 * might cause the idle task to block (vTaskDelay(), or a queue or semaphore
 * function with a block time, for example).
 */
void vApplicationIdleHook( void )
{
	return;
}

#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
// Definition to be compatible to ISIS library
BaseType_t xTaskGenericCreate(	TaskFunction_t pxTaskCode,
		const char * const pcName,		/*lint !e971 Unqualified char types are allowed for strings and single characters only. */
		const configSTACK_DEPTH_TYPE usStackDepth,
		void * const pvParameters,
		UBaseType_t uxPriority,
		TaskHandle_t * const pxCreatedTask,
		portSTACK_TYPE *puxStackBuffer,
		const void * const xRegions) {
	//printf("ISIS DEBUGGING:  xTaskGenericCreate called\n\r");
	assert(puxStackBuffer == NULL);
	assert(xRegions == NULL);
	return xTaskCreate(pxTaskCode, pcName, usStackDepth, pvParameters, uxPriority, pxCreatedTask);
}
#endif

BaseType_t xQueueGenericReceive(QueueHandle_t xQueue, void * const pvBuffer,
		TickType_t xTicksToWait, portBASE_TYPE xJustPeek) {
	//printf("ISIS DEBUGGING:  xQueueGenericReceive called\n\r");
	assert(xJustPeek == pdFALSE);
	return xQueueReceive(xQueue, pvBuffer, xTicksToWait);
}
