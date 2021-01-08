#include "watchdog.h"

#include <FreeRTOSConfig.h>
#include <freertos/task.h>

#include <hal/Drivers/LED.h>
#include <hal/Timing/WatchDogTimerNoOS.h>

struct WatchdogArgs {
	uint32_t kickInterval;
	bool toggleLed1;
};

struct WatchdogArgs watchdogArgs;

/* Custom watchdog task which has maximum priority
to make use of task preemption */
BaseType_t startCustomIsisWatchdogTask(uint32_t watchdogKickIntervalMs,
		bool toggleLed1) {
	watchdogArgs.kickInterval = watchdogKickIntervalMs;
	watchdogArgs.toggleLed1 = toggleLed1;
	return xTaskCreate(customWatchdogKickTask, "WDT_TASK",
			256, NULL, configMAX_PRIORITIES - 1, NULL);
}

void customWatchdogKickTask(void* args) {
	if(watchdogArgs.toggleLed1) {
	    LED_start();
	}
	while(1) {
		WDT_forceKick();
		if(watchdogArgs.toggleLed1) {
			LED_toggle(led_1);
		}
		vTaskDelay(watchdogArgs.kickInterval);
	}
}
