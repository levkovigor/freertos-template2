#include "portwrapper.h"

#include <fsfwconfig/OBSWConfig.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <FreeRTOSConfig.h>

#include <AT91SAM9G20.h>
#include <at91/peripherals/tc/tc.h>

// If this is changed, change both!
// The ISIS PWM driver uses TC0, TC1 and TC2 to provide 6 PWM signals!
AT91S_TC* tcStatsPeripheral = AT91C_BASE_TC5;
TcPeripherals tcPeripheralSelect = TcPeripherals::TC5;

volatile uint16_t timerOverflowCounter = 0;

void vRequestContextSwitchFromISR() {
     portYIELD_FROM_ISR(); // @suppress("Function cannot be resolved")
}

void vConfigureTimerForRunTimeStats() {
	// Tick interrupt has 1000Hz, timer frequency which is 10-100 times larger
	// recommended. Starting at lower end with 10kHz for now.
	uint32_t timerFrequency = 10000;
	TCTimerHandler::configureOverflowInterrupt(tcPeripheralSelect,
			timerFrequency, timerOverflowISR,
			TCTimerHandler::LOWEST_ISR_PRIORITY + 1, nullptr);
}

uint32_t vGetCurrentTimerCounterValue() {
	// uint32_t counter implemented by using the timer counter overflow ISR.
	return (timerOverflowCounter << 16 | tcStatsPeripheral->TC_CV);
}

void timerOverflowISR(isr_args_t args) {
	timerOverflowCounter++;
}

/*
 * These are implementation of newlib hook functions to make
 * malloc safe in a FreeRTOS environment.
 */
extern "C" void __malloc_lock (struct _reent *reent) {
#if OBSW_MONITOR_ALLOCATION == 1
	if(config::softwareInitializationComplete) {
		TRACE_WARNING("Software initialization complete but "
				"memory is allocated!\n\r");
	}
#endif
	vTaskSuspendAll();
}

extern "C" void __malloc_unlock (struct _reent *reent) {
	xTaskResumeAll();
}
