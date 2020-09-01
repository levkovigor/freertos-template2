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
    uint8_t software_version;
    uint8_t software_subversion;

	uint16_t reboot_counter;
	uint64_t seconds_since_epoch;
	//! To copy software update from the SD card into the SDRAM at restart.
	bool software_update_available;

	size_t nor_flash_binary_size;
	size_t nor_flash_hamming_code_offset;
	uint8_t nor_flash_reboot_counter;

	char sd_card1_slot1_binary_file_name [32];
	char sd_card1_slot1_hamming_code_file_name [32];
	uint8_t sdc1sl1_reboot_counter;

	char sd_card1_slot2_file_name [32];
	char sd_card1_slot2_hamming_code_file_name [32];
	uint8_t sc_card1_slot2_reboot_counter;

	char sd_card2_slot1_binary_file_name [32];
	char sd_card2_slot1_hamming_code_file_name [32];
	uint8_t sc_card2_slot1_reboot_counter;

	char sd_card2_slot2_binary_file_name [32];
	char sd_card2_slot2_hamming_code_file_name[32];
	uint8_t sc_card2_slot2_reboot_counter;

	bool bootloader_faulty;

    uint16_t number_of_active_tasks;
} FRAMCriticalData;

static const uint8_t SOFTWARE_VERSION_ADDR =
        offsetof(FRAMCriticalData, software_version);
static const uint8_t SOFTWARE_SUBVERSION_ADDR =
        offsetof(FRAMCriticalData, software_subversion);
static const uint32_t REBOOT_COUNTER_ADDR =
		offsetof(FRAMCriticalData, reboot_counter);
static const uint32_t SEC_SINCE_EPOCH_ADDR =
		offsetof(FRAMCriticalData, seconds_since_epoch);
static const uint32_t SOFTWARE_UPDATE_BOOL_ADDR =
        offsetof(FRAMCriticalData, software_update_available);

static const uint32_t NOR_FLASH_BINARY_SIZE_ADDRESS =
		offsetof(FRAMCriticalData, nor_flash_binary_size);
static const uint32_t NOR_FLASH_HAMMING_CODE_OFFSET_ADDRESS =
		offsetof(FRAMCriticalData, nor_flash_hamming_code_offset);
static const uint32_t NOR_FLASH_REBOOT_COUNTER_ADDRESS =
        offsetof(FRAMCriticalData, nor_flash_reboot_counter);

static const uint32_t SDC1SL1_FILE_NAME_ADDR =
		offsetof(FRAMCriticalData, sd_card1_slot1_binary_file_name);
static const uint32_t SDC1SL1_HAMM_CODE_NAME_ADDR =
		offsetof(FRAMCriticalData, sd_card1_slot1_hamming_code_file_name);
static const uint32_t SDC1SL1_REBOOT_COUNTER_ADDR =
        offsetof(FRAMCriticalData, sdc1sl1_reboot_counter);

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
static const uint32_t NUMBER_OF_ACTIVE_TASKS_ADDRESS =
        offsetof(FRAMCriticalData, number_of_active_tasks);

#ifdef __cplusplus
extern "C" {
#endif

int write_software_version(uint8_t software_version, uint8_t software_subversion);
int read_software_version(uint8_t* software_version, uint8_t* software_subversion);

int increment_reboot_counter();
int read_reboot_counter(uint16_t* reboot_counter);

int update_seconds_since_epoch(uint64_t secondsSinceEpoch);
int read_seconds_since_epoch(uint64_t* secondsSinceEpoch);

int write_nor_flash_binary_info(size_t binary_size, size_t hamming_code_offset);
int read_nor_flash_binary_info(size_t* binary_size, size_t* hamming_code_offset);
int increment_nor_flash_reboot_counter();
int read_nor_flash_reboot_counter(uint8_t* nor_flash_reboot_counter);
int reset_nor_flash_reboot_counter();

int increment_sdc1sl1_reboot_counter();
int read_sdc1sl1_reboot_counter(uint8_t* sdc1sl1_reboot_counter);
int reset_sdc1sl1_reboot_counter();

int set_bootloader_faulty(bool faulty);
int is_bootloader_faulty(bool* faulty);

#ifdef __cplusplus
}
#endif

#endif /* MISSION_MEMORY_FRAMAPI_H_ */
