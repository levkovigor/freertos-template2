/*
 * hooks.c
 *
 *  Created on: 16 jan. 2015
 *      Author: pbot
 */

#if defined(freeRTOS)

#include <freertos/include/freertos/FreeRTOS.h>
#include <freertos/include/freertos/task.h>
#include <at91/utility/exithandler.h>

#include <stdio.h>

#if configSUPPORT_STATIC_ALLOCATION == 1
#include <stdlib.h>
/* If the buffers to be provided to the Idle task are declared inside this
    function then they must be declared static – otherwise they will be allocated on
    the stack and so not exists after this function exits. */
static StaticTask_t xIdleTaskTCB;
#endif

extern void vApplicationStackOverflowHook( xTaskHandle pxTask, char *pcTaskName) __attribute__ ((weak));
extern void vApplicationIdleHook( void ) __attribute__ ((weak));

/*
 * This hook is provided by FreeRTOS as a way for the upper layer (user code) to
 * handle a stack overflow without introducing an dependency in the FreeRTOS
 * code.
 */
void vApplicationStackOverflowHook( xTaskHandle pxTask, char *pcTaskName)
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

#if configSUPPORT_STATIC_ALLOCATION == 1
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
        StackType_t **ppxIdleTaskStackBuffer,
        uint32_t *pulIdleTaskStackSize ) {
    StackType_t * uxIdleTaskStack = malloc(configMINIMAL_STACK_SIZE * sizeof(StackType_t));

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task’s
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task’s stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
#endif

#endif /* defined(FREERTOS) */

