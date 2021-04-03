#ifndef BSP_SAM9G20_COMMON_NORFLASHHOOK_C_
#define BSP_SAM9G20_COMMON_NORFLASHHOOK_C_

#ifndef NO_RTOS
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#endif

/* This implementation is required for the ISIS solution in the AT91 Nor-Flash library if
the AT91 library is not linked against the HAL library */
void NorFlash_Hook(void) {
#ifndef NO_RTOS
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        vTaskDelay(100);
    }
#else
#endif
}

#endif /* BSP_SAM9G20_COMMON_NORFLASHHOOK_C_ */
