/*
 * ExitHandler.c
 *
 *  Created on: Jun 22, 2011
 *      Author: Akhil
 */

#include <AT91SAM9G20.h>
#include <board.h>
#include <at91/commons.h>
#include <at91/peripherals/cp15/cp15.h>
#include <at91/utility/exithandler.h>
#include <at91/utility/trace.h>

#include <OBSWConfig.h>

#define RSTC_KEY_PASSWORD       (0xA5 << 24)

void gracefulReset() __attribute__ ((long_call, section (".sramfunc")));

/*!
 * @brief Disables the Instruction and Data caches of the ARM926EJ-S core.
 */
static void disableCaches() {
	CP15_DisableMMU();
	CP15_Disable_D_Cache();
	CP15_Disable_I_Cache();
}

void restart() {
    // Disable this for mission, this also allocated when using newlib
#if OBSW_VERBOSE_LEVEL >= 1
	TRACE_ERROR_WP("\n\r____RESTARTING____\n\r");
#endif
	disableCaches();
	gracefulReset();
}

void restartPrefetchAbort() {
	//TRACE_WARNING_WP("\n\r\t\t ____PREFETCH ABORT! RESTARTING____\n\r");
	disableCaches();
	gracefulReset();
}

void restartDataAbort() {
	//TRACE_WARNING_WP("\n\r\t\t ____DATA ABORT! RESTARTING____\n\r");
	disableCaches();
	gracefulReset();
}

void gracefulReset() {
	TRACE_ERROR_WP("\n\rPerforming Graceful Reset\n\r");
	// Disable SDRAM else it may cause interference on the EBI
	AT91C_BASE_SDRAMC->SDRAMC_TR = 1;
	AT91C_BASE_SDRAMC->SDRAMC_LPR = AT91C_SDRAMC_LPCB_POWER_DOWN;
	// Reset the peripherals and the processor
	AT91C_BASE_RSTC->RSTC_RCR = AT91C_RSTC_PROCRST | AT91C_RSTC_PERRST | RSTC_KEY_PASSWORD;
}
