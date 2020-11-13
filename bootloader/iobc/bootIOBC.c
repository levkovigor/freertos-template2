#include "bootIOBC.h"
#include "../watchdog.h"

#include <utility/trace.h>
#include <utility/CRC.h>
#include <sam9g20/memory/SDCardApi.h>
#include <sam9g20/common/FRAMApi.h>

#include <string.h>
#include <bootloaderConfig.h>

//static uint8_t read_buffer[256 * 11];

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

	binary_size = 0x100000 - 5 * 8192;
    // Need to test how long copying that much data takes, we might still
    // have to feed the watchdog..
    // For now, we copy in buckets instead of one go.
	memcpy((void*) SDRAM_DESTINATION, (const void*) BINARY_BASE_ADDRESS_READ,
			binary_size);
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

int copy_sdcard_binary_to_sdram(BootSelect boot_select) {
	int result = open_filesystem(SD_CARD_0);
	VolumeId current_filesystem = SD_CARD_0;
	if(result != 0) {
		// should not happen..
		result = open_filesystem(SD_CARD_1);
		if(result != 0) {
			// not good at all. boot from NOR-Flash instead
			return -1;
		}
		current_filesystem = SD_CARD_1;
		if ((boot_select == BOOT_SD_CARD_0) ||
				(boot_select == BOOT_SD_CARD_0_SLOT2)) {
			// just take the first binary for now
			boot_select = BOOT_SD_CARD_1;
		}
	}

	switch(boot_select) {
	case(BOOT_SD_CARD_0): {
		// get repostiory
		char bin_folder_name[16];
		// hardcoded
		//int result = read_sdc_bin_folder_name(bin_folder_name);
		if(result != 0) {
			// not good
		}
		result = change_directory(bin_folder_name, true);
		if(result != F_NO_ERROR) {
			// not good
		}
		char binary_name[16];
		char hamming_name[16];
		// hardcoded
		//result = read_sdc1sl1_bin_names(binary_name, hamming_name);
		if(result != 0) {

		}
		// read in buckets.. multiple of 256. maybe 10 * 256 bytes?
		F_FILE* file = f_open(binary_name, "r");
		if (f_getlasterror() != F_NO_ERROR) {
			// opening file failed!
			return -1;
		}
		// get total file size.
		long filesize = f_filelength(binary_name);
		size_t current_idx = 0;
		size_t read_bucket_size = 10 * 256;
		while(true) {
			// we are close to the end of the file and don't have to read
			// the full bucket size
			if(filesize - current_idx < read_bucket_size) {
				read_bucket_size = filesize - current_idx;
			}
			// copy to SDRAM directly
			size_t bytes_read = f_read(
					(void *) (SDRAM_DESTINATION + current_idx),
					sizeof(uint8_t),
					read_bucket_size,
					file);
			if(bytes_read == -1) {
				// this should definitely not happen
				// we need to ensure we will not be locked in a permanent loop.
			}
			else if(bytes_read < 10 * 256) {
				// should not happen!
			}

			// now we could perform the hamming check on the RAM code directly
			// If the last bucket is smaller than 256, we pad with 0
			// and assume the hamming code calculation was performed with
			// padding too.
			current_idx += bytes_read;
			if(current_idx >= filesize) {
				break;
			}

		}


		break;
	}
	case(BOOT_SD_CARD_0_SLOT2): {
		break;
	}
	case(BOOT_SD_CARD_1): {
		break;
	}
	case(BOOT_SD_CARD_1_SLOT2): {
		break;
	}
	case(BOOT_NOR_FLASH): {
		return -1;
	}
	}
	close_filesystem(current_filesystem);
	return 0;
}
