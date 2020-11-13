#include "main.h"
#include "config/bootloaderConfig.h"

#include <board.h>
#include <AT91SAM9G20.h>
#include <led_ek.h>
#include <board_memories.h>
#include <peripherals/dbgu/dbgu.h>
#include <peripherals/pio/pio.h>
#include <peripherals/aic/aic.h>
#include <peripherals/pio/pio.h>
#include <cp15/cp15.h>

#include <hal/Timing/RTT.h>

#include <core/timer.h>

// The AT91SAM9G20-EK does not have a pre-installed NOR-Flash. Therefore,
// we only include the NorFlash boot header for iOBC projects.
#include <at91/bootAt91.h>

#if DEBUG_IO_LIB == 1
#include <utility/trace.h>
#endif

#include <stdbool.h>
#include <string.h>

int perform_bootloader_core_operation();
void go_to_jump_address(unsigned int jumpAddr, unsigned int matchType);
void idle_loop();

/**
 * @brief   Bootloader which will copy the primary software to SDRAM and
 *          execute it.
 * @details
 * This is the implementation for the AT91SAM9G20-EK  and its NAND-Flash.
 * Please note that the compiled binary needs to be smaller than 16kB to fit
 * into SRAM. When written with SAM-BA, use ther special option "Send Boot File"
 * to ensure the sixth ARM vector is set to the binary size.
 * @author  R. Mueller
 */
int at91_main()
{
    //-------------------------------------------------------------------------
    // Configure traces
    //-------------------------------------------------------------------------
#if DEBUG_IO_LIB == 1
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
#endif
    //-------------------------------------------------------------------------
    // Enable I-Cache
    //-------------------------------------------------------------------------
    CP15_Enable_I_Cache();

#if DEBUG_IO_LIB == 1
    TRACE_INFO_WP("-- SOURCE Bootloader --\n\r");
    TRACE_INFO_WP("-- %s --\n\r", BOARD_NAME);
    TRACE_INFO_WP("-- Software version v%d.%d --\n\r", SW_VERSION, SW_SUBVERSION);
    TRACE_INFO_WP("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
    TRACE_INFO("Running initialization task..\n\r");
#endif

    //-------------------------------------------------------------------------
    // Initiate periodic MS interrupt
    //-------------------------------------------------------------------------
#ifndef ISIS_OBC_G20
    setup_timer_interrupt();
#endif

    //-------------------------------------------------------------------------
    // Configure SDRAM
    //-------------------------------------------------------------------------
    BOARD_ConfigureSdram(BOARD_SDRAM_BUSWIDTH);

    LED_Set(1);

    //-------------------------------------------------------------------------
    // AT91SAM9G20-EK Bootloader
    //-------------------------------------------------------------------------
    // Configure RTT for second time base.
    RTT_start();

    //-------------------------------------------------------------------------
    // AT91 Bootloader
    //-------------------------------------------------------------------------
    perform_bootloader_core_operation();

    // to see its alive, will not be reached later.
    idle_loop();
    return 0;
}


void idle_loop() {
    uint32_t last_time = RTT_GetTime();
    for(;;) {
        uint32_t curr_time = RTT_GetTime();
        if(curr_time - last_time >= 1) {
#if DEBUG_IO_LIB == 1
            TRACE_INFO("Bootloader idle..\n\r");
#endif
            last_time = curr_time;
        }
    }
}

void go_to_jump_address(unsigned int jumpAddr, unsigned int matchType)
{
    typedef void (*fctType) (volatile unsigned int, volatile unsigned int);
    void (*pFct) (volatile unsigned int r0_val, volatile unsigned int r1_val);

    pFct = (fctType) jumpAddr;
    pFct(0/*dummy value in r0*/, matchType/*matchType in r1*/);

    while(1);//never reach
}

int perform_bootloader_core_operation() {
    LED_Set(1);
    // Clear SDRAM completely.
    memset((void*) SDRAM_DESTINATION, 0 , 0x100000);
    int result = copy_nandflash_binary_to_sdram(false);
    if(result != 0) {
        return result;
    }
    LED_Clear(1);
    go_to_jump_address(SDRAM_DESTINATION, 0);
    return 0;
}


