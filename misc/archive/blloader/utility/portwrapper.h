#ifndef SAM9G20_UTILITY_PORTWRAPPER_H_
#define SAM9G20_UTILITY_PORTWRAPPER_H_

#include <sam9g20/utility/TCTimerHandler.h>
#include <cstdint>

extern uint16_t timerOverflowCounter;
extern AT91S_TC* tcStatsPeripheral;

void vRequestContextSwitchFromISR();

/**
 * Called by FreeRTOS.
 * Configures the AT91 TC peripheral for the FreeRTOS run time stats.
 */
extern "C" void vConfigureTimerForRunTimeStats();

/**
 * Called by FreeRTOS.
 * Get the current timer counter. Please note that a uint32_t counter was
 * implemented with a uint16_t timer by using an overflow interrupt.
 */
extern "C" uint32_t vGetCurrentTimerCounterValue();

/**
 * This ISR is called when the TC peripheral counter overflows.
 */
void timerOverflowISR(isr_args_t isrArgs);

#endif /* SAM9G20_UTILITY_PORTWRAPPER_H_ */
