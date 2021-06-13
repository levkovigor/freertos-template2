#ifndef BOOTLOADER_CORE_TIMER_H_
#define BOOTLOADER_CORE_TIMER_H_

#include <stdint.h>
#include <bootloaderConfig.h>

/* MS timer counter (needed for watchdog) */
extern volatile uint32_t u32_ms_counter;
static const uint32_t TICK_RATE_HZ = 1000;
static const uint32_t port1SECOND_IN_uS = 1000000.0;
static const uint32_t port1MHz_IN_Hz = 1000000ul;

/* Setup interrupt which incremens the MS counter accordingly */
void setup_timer_interrupt(void);

uint32_t get_ms_counter();

#endif /* BOOTLOADER_CORE_TIMER_H_ */
