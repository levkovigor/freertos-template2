#include "portwrapper.h"
#include "FreeRTOS.h"
#include "projdefs.h"

void vRequestContextSwitchFromISR() {
     portYIELD_FROM_ISR(pdTRUE);
 }
