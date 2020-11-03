#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include <assert.h>

#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
// Definition to be compatible to ISIS library
BaseType_t xTaskGenericCreate(  TaskFunction_t pxTaskCode,
        const char * const pcName,      /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
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



