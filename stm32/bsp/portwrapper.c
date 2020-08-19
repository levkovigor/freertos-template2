#include <stm32/bsp/portwrapper.h>
#include "FreeRTOS.h"
#include "projdefs.h"

void requestContextSwitchFromISR() {
     portYIELD_FROM_ISR(pdTRUE);
 }
