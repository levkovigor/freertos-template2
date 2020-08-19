#ifndef BOOTLOADER_CORE_TIMER_H_
#define BOOTLOADER_CORE_TIMER_H_

#include <stdint.h>

/* MS timer counter (needed for watchdog) */
extern volatile uint32_t u32_ms_counter;

/* Setup interrupt which incremens the MS counter accordingly */
void setup_timer_interrupt(void);

#endif /* BOOTLOADER_CORE_TIMER_H_ */
