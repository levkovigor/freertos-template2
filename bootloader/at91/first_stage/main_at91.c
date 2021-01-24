#include "../common/at91_boot_from_nand.h"
#include <bootloaderConfig.h>
#include <core/timer.h>

#include <board.h>
#include <AT91SAM9G20.h>
#include <led_ek.h>
#include <board_memories.h>
#include <peripherals/dbgu/dbgu.h>
#include <peripherals/pio/pio.h>
#include <peripherals/aic/aic.h>
#include <peripherals/pit/pit.h>
#include <cp15/cp15.h>

#include <hal/Timing/RTT.h>

#if BOOTLOADER_VERBOSE_LEVEL >= 1
#include <utility/trace.h>
#endif

#include <stdbool.h>
#include <string.h>

void disablePitAic();
extern void jump_to_sdram_application(uint32_t stack_ptr, uint32_t jump_address);

int perform_bootloader_core_operation();


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
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);

    //-------------------------------------------------------------------------
    // Enable I-Cache
    //-------------------------------------------------------------------------
    CP15_Enable_I_Cache();

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO_WP("-- SOURCE Bootloader (First Stage SRAM) --\n\r");
    TRACE_INFO_WP("-- %s --\n\r", BOARD_NAME_PRINT);
    TRACE_INFO_WP("-- Software version v%d.%d --\n\r", BL_VERSION, BL_SUBVERSION);
    TRACE_INFO_WP("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
#endif

    //-------------------------------------------------------------------------
    // Initiate periodic MS interrupt
    //-------------------------------------------------------------------------
    // Issues if this is enabled..
    //setup_timer_interrupt();

    //-------------------------------------------------------------------------
    // Configure SDRAM
    //-------------------------------------------------------------------------
    //BOARD_ConfigureSdram(BOARD_SDRAM_BUSWIDTH);

    //-------------------------------------------------------------------------
    // Configure LEDs and set both of them.
    //-------------------------------------------------------------------------
    LED_Configure(1);
    LED_Configure(0);
    LED_Set(0);
    LED_Set(1);

    //-------------------------------------------------------------------------
    // AT91SAM9G20-EK Bootloader
    //-------------------------------------------------------------------------
    // Configure RTT for second time base. Not required for now.
    // RTT_start();

    //-------------------------------------------------------------------------
    // AT91 Bootloader
    //-------------------------------------------------------------------------
    perform_bootloader_core_operation();
    return 0;
}


int perform_bootloader_core_operation() {
    LED_Clear(0);
    LED_Clear(1);

    copy_nandflash_image_to_sdram(SECOND_STAGE_BL_NAND_OFFSET, SECOND_STAGE_BL_RESERVED_SIZE,
            SECOND_STAGE_SDRAM_OFFSET, true);

    LED_Set(0);

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO("Jumping to SDRAM application address 0x%08x!\n\r", SECOND_STAGE_BL_JUMP_ADDR);
#endif

    CP15_Disable_I_Cache();

    /* Not required, PIT not used for now */
    // disablePitAic();

    jump_to_sdram_application(0x304000, SECOND_STAGE_BL_JUMP_ADDR);

    /* Should never be reached */
    return 0;
}

void disablePitAic() {
    AIC_DisableIT( AT91C_ID_SYS );
    PIT_DisableIT();
    PIT_Disable();
}


