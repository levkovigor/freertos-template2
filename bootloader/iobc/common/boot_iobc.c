#include "boot_iobc.h"
#include <bootloaderConfig.h>

#include <bsp_sam9g20/common/lowlevel.h>

#if USE_FREERTOS == 1

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <bsp_sam9g20/common/fram/FRAMApi.h>
#include <bsp_sam9g20/common/fram/CommonFRAM.h>

#else

#endif /* USE_FREERTOS == 0 */

#include <at91/utility/trace.h>
#include <at91/peripherals/aic/aic.h>
#include <at91/peripherals/pit/pit.h>

#include <hal/Drivers/LED.h>
#include <hal/Timing/RTT.h>

#include <string.h>
#include <stdlib.h>

/* Forward declarations */
int perform_iobc_copy_operation_to_sdram();
void go_to_jump_address(unsigned int jumpAddr, unsigned int matchType);

/**
 * This is the core function of the bootloader which handles the copy operation,
 * performing ECC functionalities where applicable and jumping to the application.
 */
void perform_bootloader_core_operation() {
    int result = perform_iobc_copy_operation_to_sdram();
    if(result != 0) {
        /* This really should not happen. We still assume we can jump to SDRAM because there is
        not much we can do anyway */
    }

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO("Jumping to SDRAM application..\n\r");
#endif

#if USE_FREERTOS == 1
    vTaskEndScheduler();
#else

#if USE_FRAM_NON_INTERRUPT_DRV == 0
    /* Considered a configuration error, the last transfers increment the reboot counter
    and block until the transfer is completed (state should be IDLE then) */
    if(spi_transfer_state != IDLE) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("perform_bootloader_core_operation: SPI transfer "
                "might still be ongoing!\n\r");
#endif
    }
#endif

#endif /* USE_FREERTOS == 0 */

#if USE_FRAM_NON_INTERRUPT_DRV == 0
    fram_stop_no_os();
#endif
    disable_pit_aic();
    jump_to_sdram_application(0x22000000 - 1024, SDRAM_DESTINATION);
}

/**
 * Used internally to jump to SDRAM.
 * @param jumpAddr
 * @param matchType
 */
void go_to_jump_address(unsigned int jumpAddr, unsigned int matchType)
{
    typedef void (*fctType) (volatile unsigned int, volatile unsigned int);
    void (*pFct) (volatile unsigned int r0_val, volatile unsigned int r1_val);

    pFct = (fctType) jumpAddr;
    pFct(0/*dummy value in r0*/, matchType/*matchType in r1*/);

    while(1);//never reach
}

void idle_loop() {
    uint32_t last_time = RTT_GetTime();
    LED_dark(led_2);
    for(;;) {
        uint32_t curr_time = RTT_GetTime();
        if(curr_time - last_time >= 1) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
            TRACE_INFO("Bootloader idle..\n\r");
#endif
            LED_toggle(led_2);
            last_time = curr_time;
        }
    }
}

void print_bl_info() {
    TRACE_INFO_WP("\n\rStarting FreeRTOS task scheduler.\n\r");
    TRACE_INFO_WP("-- SOURCE Bootloader --\n\r");
    TRACE_INFO_WP("-- %s --\n\r", BOARD_NAME_PRINT);
    TRACE_INFO_WP("-- Software version v%d.%d --\n\r", BL_VERSION,
            BL_SUBVERSION);
    TRACE_INFO_WP("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
#if USE_FREERTOS == 1
    TRACE_INFO("Running initialization task..\n\r");
#endif
}
