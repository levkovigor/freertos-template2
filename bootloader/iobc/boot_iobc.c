#include "boot_iobc.h"

#include <core/watchdog.h>

#include <utility/trace.h>
#include <utility/CRC.h>
#include <hal/Drivers/LED.h>
#include <hal/Timing/RTT.h>

#include <string.h>
#include <bootloaderConfig.h>
#include <iobc/norflash/iobc_norflash.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

//static uint8_t read_buffer[256 * 11];
void go_to_jump_address(unsigned int jumpAddr, unsigned int matchType);

void perform_bootloader_core_operation() {
    int result = perform_iobc_copy_operation_to_sdram();
    if(result != 0) {
        // error
    }

#if DEBUG_IO_LIB == 1
    TRACE_INFO("Jumping to SDRAM application..\n\r");
#endif

    vTaskEndScheduler();
    jumpToSdramApplication();
}

int perform_iobc_copy_operation_to_sdram() {
    // determine which binary should be copied to SDRAM first.
    BootSelect boot_select = BOOT_NOR_FLASH;
    int result = 0;
    if(boot_select == BOOT_NOR_FLASH) {
        result = copy_norflash_binary_to_sdram(OBSW_MAX_SIZE);
        if(result != 0) {
        	// Attempt sd card boot.
        }
    }
    else {
        result = copy_sdcard_binary_to_sdram(boot_select);
        if(result != 0) {
            // try boot from other slot.
            //result = copy_norflash_binary_to_sdram(256);
        }
    }
    return result;
}

//------------------------------------------------------------------------------
/// Initialize NOR devices and transfer one or several modules from Nor to the
/// target memory (SRAM/SDRAM).
//------------------------------------------------------------------------------
int copy_norflash_binary_to_sdram(size_t binary_size)
{
    // Initialize Nor
    //-------------------------------------------------------------------------
    // => the configuration was already done in LowLevelInit()
    // Transfert data from Nor to External RAM
    //-------------------------------------------------------------------------

	// To allow watchdog task to kick if necessary.
	vTaskDelay(1);
	// This operation takes 100-200 milliseconds if the whole NOR-Flash is
	// copied.
	memcpy((void*) SDRAM_DESTINATION, (const void*) BINARY_BASE_ADDRESS_READ,
			binary_size);
	/* verify that the binary was copied properly. A hamming code check
	should have been performed previously. If this fails, we return with
	error and try SD card boot. */
	for(int idx = 0; idx < binary_size; idx++) {
		if(*(uint8_t*)(SDRAM_DESTINATION + idx) !=
				*(uint8_t*)(BINARY_BASE_ADDRESS_READ + idx)) {
			TRACE_ERROR("Byte SDRAM %d : %2x\n\r", idx,
					*(uint8_t*)(SDRAM_DESTINATION + idx));
			TRACE_ERROR("Byte NORFLASH %d : %2x\n\r", idx,
					*(uint8_t*)(BINARY_BASE_ADDRESS_READ + idx));
			return -1;
		}
	}
	TRACE_INFO("Copied successfully!\n\r");
    return 0;
}


void idle_loop() {
    uint32_t last_time = RTT_GetTime();
    LED_dark(led_2);
    for(;;) {
        uint32_t curr_time = RTT_GetTime();
        if(curr_time - last_time >= 1) {
#if DEBUG_IO_LIB == 1
            TRACE_INFO("Bootloader idle..\n\r");
#endif
            LED_toggle(led_2);
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
