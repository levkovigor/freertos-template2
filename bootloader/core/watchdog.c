#include "watchdog.h"
#include "timer.h"
#include <stdio.h>

/**
 * Note: This is a special include of a header which was newly created to avoid
 * including FreeRTOS.
 * Only include WDT_start() and WDT_forceKick() are needed.
 */
#include <hal/Timing/WatchDogTimerNoOS.h>

uint32_t watchdog_last_fed_ms_count = 0;

void initiate_external_watchdog() {
    WDT_start();
    WDT_forceKick();
    // Now, we need to ensure the watchdog is fed often enough but not too often
    watchdog_last_fed_ms_count = u32_ms_counter;
}

void feed_watchdog_if_necessary(void) {
    if(u32_ms_counter - watchdog_last_fed_ms_count >= WATCHDOG_FEED_PERIOD_MS) {
        WDT_forceKick();
        watchdog_last_fed_ms_count = u32_ms_counter;
    }
}


