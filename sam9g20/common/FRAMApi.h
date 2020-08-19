#ifndef MISSION_MEMORY_FRAMAPI_H_
#define MISSION_MEMORY_FRAMAPI_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief	This common API can be used by both C++ and C code
 * 			to write data to FRAM.
 * @details
 * This header encapsulates the data stored on the FRAM and provides
 * funtions to read and write/overwrite them conveniently.
 * The functions use the HAL library provided by ISIS to write on the FRAM.
 * The FRAM needs to be started first! (see FRAM.h file)
 * @author R. Mueller
 */

/**
 * This struct will gather all critical data stored on FRAM.
 */
typedef struct __attribute__((__packed__))  _FRAMCriticalData {
	uint16_t reboot_counter;

	size_t nor_flash_binary_size;
	size_t nor_flash_hamming_code_offset;

	char sd_card1_slot1_binary_file_name [32];
	char sd_card1_slot1_hamming_code_file_name [32];
	char sd_card1_slot2_file_name [32];
	char sd_card1_slot2_hamming_code_file_name [32];

	char sd_card2_slot1_binary_file_name [32];
	char sd_card2_slot1_hamming_code_file_name [32];
	char sd_card2_slot2_binary_file_name [32];
	char sd_card2_slot2_hamming_code_file_name[32];

	bool bootloader_faulty;
} FRAMCriticalData;

static const uint32_t REBOOT_COUNTER_ADDRESS =
		offsetof(FRAMCriticalData, reboot_counter);
static const uint32_t NOR_FLASH_BINARY_SIZE_ADDRESS =
		offsetof(FRAMCriticalData, nor_flash_binary_size);
static const uint32_t NOR_FLASH_HAMMING_CODE_OFFSET_ADDRESS =
		offsetof(FRAMCriticalData, nor_flash_hamming_code_offset);

static const uint32_t sdCard1Slot1BinaryFileNameAddress =
		offsetof(FRAMCriticalData, sd_card1_slot1_binary_file_name);
static const uint32_t sdCard1Slot1HammingCodeFileNameAddress =
		offsetof(FRAMCriticalData, sd_card1_slot1_hamming_code_file_name);
static const uint32_t sdCard1Slot2FileNameAddress =
		offsetof(FRAMCriticalData, sd_card1_slot2_file_name);
static const uint32_t sdCard1Slot2HammingCodeFileNameAddress =
		offsetof(FRAMCriticalData, sd_card1_slot2_hamming_code_file_name);

static const uint32_t sdCard2Slot1BinaryFileNameAddress =
		offsetof(FRAMCriticalData, sd_card2_slot1_binary_file_name);
static const uint32_t sdCard2Slot1HammingCodeFileName =
		offsetof(FRAMCriticalData, sd_card2_slot1_hamming_code_file_name);
static const uint32_t sdCard2Slot2FileNameAddress =
		offsetof(FRAMCriticalData, sd_card2_slot2_binary_file_name);
static const uint32_t sdCard2Slot2HammingCodeFileNameAddress =
		offsetof(FRAMCriticalData, sd_card2_slot2_hamming_code_file_name);

static const uint32_t BOOTLOADER_FAULTY_ADDRESS =
		offsetof(FRAMCriticalData, bootloader_faulty);

int increment_reboot_counter();
int read_reboot_counter(uint16_t* reboot_counter);

int write_nor_flash_binary_info(size_t binary_size, size_t hamming_code_offset);
int read_nor_flash_binary_info(size_t* binary_size, size_t* hamming_code_offset);

int set_bootloader_faulty(bool faulty);
int is_bootloader_faulty(bool* faulty);

#endif /* MISSION_MEMORY_FRAMAPI_H_ */
