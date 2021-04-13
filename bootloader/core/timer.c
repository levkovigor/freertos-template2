#include <at91/utility/trace.h>
#include "timer.h"
#include <board.h>
#include <AT91SAM9G20.h>
#if defined(ISIS_OBC_G20) && BOOTLOADER_KICK_WATCHDOG_IN_PIT_IRQ == 1
#include <hal/Timing/WatchdogTimerNoOs.h>
#endif
#include <peripherals/pit/pit.h>
#include <peripherals/aic/aic.h>

volatile uint32_t u32_ms_counter = 0;

#if defined(ISIS_OBC_G20) && BOOTLOADER_KICK_WATCHDOG_IN_PIT_IRQ == 1
static const uint8_t TICK_SYSTEM_PRIORITY = AT91C_AIC_PRIOR_LOWEST + 5;
#else
static const uint8_t TICK_SYSTEM_PRIORITY = AT91C_AIC_PRIOR_LOWEST;
#endif

// Private functions
void ms_tick_isr(void);

/*
 * Setup the PIT to generate the tick interrupts at the required frequency.
 */
void setup_timer_interrupt(void)
{
    // each 1 ms the pit throws an interrupt
    const unsigned long ulPeriodIn_uS = ( 1.0 /
            ( double ) TICK_RATE_HZ ) * port1SECOND_IN_uS;

    /* Setup the PIT for the required frequency. */
    PIT_Init( ulPeriodIn_uS, BOARD_MCK / port1MHz_IN_Hz );

    /* Setup the PIT interrupt. */
    AIC_DisableIT( AT91C_ID_SYS );
    AIC_ConfigureIT( AT91C_ID_SYS, TICK_SYSTEM_PRIORITY, ms_tick_isr);
    AIC_EnableIT( AT91C_ID_SYS );
    PIT_EnableIT();
    PIT_Enable();
}

void ms_tick_isr(void) __attribute__((section(".sramfunc")));
void ms_tick_isr(void)
{
    volatile unsigned long ulDummy;
    u32_ms_counter++;

#if defined(ISIS_OBC_G20) && BOOTLOADER_KICK_WATCHDOG_IN_PIT_IRQ == 1
    /* Kick watchdog every 10 ms */
    if(u32_ms_counter % 10 == 0) {
        WDT_forceKick();
    }
#endif

    /* Clear the PIT interrupt. */
    ulDummy = AT91C_BASE_PITC->PITC_PIVR;
    /* To remove compiler warning. */
    ( void ) ulDummy;
    /* The AIC is cleared in the asm wrapper, outside of this function. */
}

uint32_t get_ms_counter() {
    return u32_ms_counter;
}
