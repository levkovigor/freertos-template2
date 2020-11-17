#include "bootIOBC.h"
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
        size_t sizeToCopy = 0;
        memcpy(&sizeToCopy, (const void *) (NOR_FLASH_BASE_ADDRESS_READ +
                NORFLASH_SA5_ADDRESS + 0x14), 4);
        size_t maxObswSize = NORFLASH_SIZE - BOOTLOADER_RESERVED_SIZE;
        if(sizeToCopy > maxObswSize) {
        	sizeToCopy = maxObswSize;
#if DEBUG_IO_LIB == 1
        	TRACE_INFO("Detected binary size from sixth ARM vector at address "
        			"0x%8x invalid, setting to %d!\n\r",
					(unsigned int) NOR_FLASH_BASE_ADDRESS_READ +
					NORFLASH_SA5_ADDRESS + 0x14, sizeToCopy);
#endif
        }
#if DEBUG_IO_LIB == 1
        else {
        TRACE_INFO("Detected binary size from sixth ARM vector at address "
        		"0x%8x: %u\n\r",
        		(unsigned int) NOR_FLASH_BASE_ADDRESS_READ +
				NORFLASH_SA5_ADDRESS + 0x14,
				(unsigned int) sizeToCopy);
        }
#endif
        result = copy_norflash_binary_to_sdram(sizeToCopy);
    }
    else {
        result = copy_sdcard_binary_to_sdram(boot_select);
        if(result != 0) {
            // fatal failure. boot from NOR-Flash
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

    // Need to test how long copying that much data takes, we might still
    // have to feed the watchdog..
    // For now, we copy in buckets instead of one go.
	memcpy((void*) SDRAM_DESTINATION, (const void*) BINARY_BASE_ADDRESS_READ,
			binary_size);
	for(int idx = 0; idx < binary_size; idx++) {
		if(*(uint8_t*)(SDRAM_DESTINATION + idx) !=
				*(uint8_t*)(BINARY_BASE_ADDRESS_READ + idx)) {
			TRACE_ERROR("Byte SDRAM %d : %2x\n\r", idx,
					*(uint8_t*)(SDRAM_DESTINATION + idx));
			TRACE_ERROR("Byte NORFLASH %d : %2x\n\r", idx,
					*(uint8_t*)(BINARY_BASE_ADDRESS_READ + idx));
		}
	}
	TRACE_INFO("Copied successfully!\n\r");
//    uint8_t binary_divisor = 5;
//    TRACE_INFO("Copying NOR-Flash binary (%d bytes) from NOR 0x%lx "
//            "to SDRAM 0x%09x\n\r", binary_size, BINARY_BASE_ADDRESS_READ,
//                (int) BOARD_SDRAM_BASE_ADDRESS);
//    // It is assumed that a sanity check of the binary size was already
//    // performed. The binary size is extracted from FRAM.
//    uint32_t bucket_size = binary_size / binary_divisor;
//    size_t last_packet_bytes = binary_size % bucket_size;
//#if ENHANCED_DEBUG_OUTPUT == 1
//    TRACE_INFO("Copying in %d buckets of %d times %d plus %d bytes",
//            binary_divisor, binary_divisor, (int) bucket_size,
//            (int) last_packet_bytes);
//#endif
//
//    size_t offset = 0;
//    for(uint16_t packet_idx = 0; packet_idx < bucket_size; packet_idx++) {
//        offset = packet_idx * bucket_size;
//        memcpy((void*)BOARD_SDRAM_BASE_ADDRESS + offset,
//                (const void *)BINARY_BASE_ADDRESS_READ + offset,
//                binary_size);
//        uint16_t sdram_crc = crc16ccitt_default_start_crc(
//                (void*)BOARD_SDRAM_BASE_ADDRESS + offset,
//                bucket_size);
//        uint16_t nor_flash_crc =crc16ccitt_default_start_crc(
//                (const void *)BINARY_BASE_ADDRESS_READ + offset,
//                bucket_size);
//        if(sdram_crc != nor_flash_crc) {
//            // Should not happen! Try copying again and if that does not work
//            // maybe reboot?
//        }
//        feed_watchdog_if_necessary();
//    }
//
//    if(last_packet_bytes > 0) {
//        offset = bucket_size * binary_divisor;
//        memcpy((void*)BOARD_SDRAM_BASE_ADDRESS + offset,
//                (const void *)BINARY_BASE_ADDRESS_READ + offset,
//                last_packet_bytes);
//        uint16_t sdram_crc = crc16ccitt_default_start_crc(
//                (void*)BOARD_SDRAM_BASE_ADDRESS + offset,
//                last_packet_bytes);
//        uint16_t nor_flash_crc =crc16ccitt_default_start_crc(
//                (const void *)BINARY_BASE_ADDRESS_READ + offset,
//                last_packet_bytes);
//        if(sdram_crc != nor_flash_crc) {
//            // Should not happen! Try copying again and if that does not work
//            // maybe reboot?
//        }
//    }
//    feed_watchdog_if_necessary();
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
