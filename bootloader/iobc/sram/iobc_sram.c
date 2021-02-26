#include <config/bootloaderConfig.h>
#include <core/watchdog.h>

#include <hal/Drivers/LED.h>
#include <hal/Timing/RTT.h>

#include <board.h>
#include <AT91SAM9G20.h>
#include <board_memories.h>
#include <cp15/cp15.h>
#include "../boot_iobc.h"

#if BOOTLOADER_VERBOSE_LEVEL >= 1
#include <utility/trace.h>
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */

int iobc_sram() {
	// Initiate ISIS watchdog
    initiate_external_watchdog();

    //-------------------------------------------------------------------------
    // Configure traces
    //-------------------------------------------------------------------------
#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */

    //-------------------------------------------------------------------------
    // Enable I-Cache
    //-------------------------------------------------------------------------
    CP15_Enable_I_Cache();

    feed_watchdog_if_necessary();

    LED_start();
    LED_glow(led_2);
    LED_glow(led_3);
    LED_glow(led_4);

    //-------------------------------------------------------------------------
    // Configure SDRAM
    //-------------------------------------------------------------------------
    BOARD_ConfigureSdram(BOARD_SDRAM_BUSWIDTH);

    feed_watchdog_if_necessary();

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO_WP("-- SOURCE Bootloader (SRAM) --\n\r");
    TRACE_INFO_WP("-- %s --\n\r", BOARD_NAME_PRINT);
    TRACE_INFO_WP("-- Software version v%d.%d --\n\r", BL_VERSION,
            BL_SUBVERSION);
    TRACE_INFO_WP("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
    TRACE_INFO("Running initialization task..\n\r");
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */

    feed_watchdog_if_necessary();

    /* Configure RTT for second time base. */
    RTT_start();

    /* AT91 Bootloader */
    perform_bootloader_core_operation();
    return 0;
}



