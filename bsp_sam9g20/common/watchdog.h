#ifndef SAM9G20_COMMON_WATCHDOG_H_
#define SAM9G20_COMMON_WATCHDOG_H_

#include <stdbool.h>
#include <stdint.h>

#include <freertos/FreeRTOS.h>

BaseType_t startCustomIsisWatchdogTask(uint32_t watchdogKickIntervalMs,
		bool toggleLed1);
void customWatchdogKickTask(void* args);

#endif /* SAM9G20_COMMON_WATCHDOG_H_ */
