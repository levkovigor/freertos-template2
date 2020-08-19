#include "watchdog.h"
#include "timer.h"
/**
 * Note: This is a special include of a header which was newly created to avoid
 * including FreeRTOS.
 * It only include WDT_start() and WDT_forceKick() are needed.
 */
#include <hal/Timing/WatchDogTimerNoOS.h>

uint32_t watchdog_last_fed_ms_count = 0;

void initiate_external_watchdog() {
    WDT_start();
    WDT_forceKick();
    // Now, we need to ensure the watchdog is kicked every 10 ms
    watchdog_last_fed_ms_count = u32_ms_counter;
}

void feed_watchdog_if_necessary(void) {
    if(u32_ms_counter - watchdog_last_fed_ms_count > 10) {
        WDT_forceKick();
    }
}


