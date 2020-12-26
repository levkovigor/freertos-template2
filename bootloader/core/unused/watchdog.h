#ifndef BLLOADER_CORE_WATCHDOG_H_
#define BLLOADER_CORE_WATCHDOG_H_

#include <stdint.h>

// Kick period. Recommended value by ISIS is 5ms (200Hz) to 30ms (25Hz)
static const uint8_t WATCHDOG_FEED_PERIOD_MS = 10;
extern uint32_t watchdog_last_fed_ms_count;

/**
 * Initiate the watchdog of the iOBC
 */
void initiate_external_watchdog();

/**
 * Helper function which checks whether the watchdog has to be kicked
 */
void feed_watchdog_if_necessary();


#endif /* BLLOADER_CORE_WATCHDOG_H_ */
