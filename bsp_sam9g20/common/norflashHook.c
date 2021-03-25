#ifndef BSP_SAM9G20_COMMON_NORFLASHHOOK_C_
#define BSP_SAM9G20_COMMON_NORFLASHHOOK_C_

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/* This implementation is required for the ISIS solution in the AT91 Nor-Flash library if
the AT91 library is not linked against the HAL library */
void NorFlash_Hook(void) {
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        vTaskDelay(100);
    }
    else {

    }
}

#endif /* BSP_SAM9G20_COMMON_NORFLASHHOOK_C_ */
