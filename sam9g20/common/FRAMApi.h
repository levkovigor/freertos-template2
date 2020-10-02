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
    /* Software information [4, 0-3] */
    uint8_t software_version;
    uint8_t software_subversion;
    uint8_t filler_sw_version[2];

    /* Reboot information [4, 4-7] */
    uint8_t filler_reboot_counter[1];
	uint16_t reboot_counter;
	bool bootloader_faulty;

	/* Second counter [4, 8 - 11] */
	uint32_t seconds_since_epoch;

	/* NOR-Flash binary information [4, 12 - 15] */
	size_t nor_flash_binary_size;
	size_t nor_flash_hamming_code_offset;
	uint8_t filler_nor_flash[3];
	uint8_t nor_flash_reboot_counter;

	/* SD-Card Binaries information */
	char sd_card_binary_folder[16];

	/* SD-card 1 binary slot 1 information */
//	char sdc1sl1_binary_file_name [16];
//	char sdc1sl1_hamming_code_file_name [16];
	uint8_t filler_sdc1sl1[3];
	uint8_t sdc1sl1_reboot_counter;

	/* SD-card 1 binary slot 2 information */
//	char sdc1sl2_file_name [16];
//	char sdc1sl2_hamming_code_file_name [16];
	uint8_t filler_sdc1sl2[3];
	uint8_t sdc1sl2_reboot_counter;

	/* SD-card 2 binary slot 1 information */
//	char sdc2sl1_binary_file_name [16];
//	char sdc2sl1_hamming_code_file_name [16];
	uint8_t filler_sdc2sl1[3];
	uint8_t sdc2sl1_reboot_counter;

	/* SD-card 2 binary slot 2 information */
//	char sdc2sl2_binary_file_name [16];
//	char sdc2sl2_hamming_code_file_name[16];
	uint8_t filler_sdc2sl2[3];
	uint8_t sdc2sl2_reboot_counter;

	/*
	 * Bootloader binary information. Bootloader itself could also be stored
	 * in FRAM. Hamming code will be stored in FRAM in any case.
	 */
	char bootloader_binary_file_name[16];
	char bootloader_hamming_code_file_name[16];

	/* Software update information */
    uint8_t filler_software_update[1];
    bool software_update_available;
	uint8_t software_update_in_slot1;
	uint8_t software_update_in_slot2;

	/* Task information */
	uint8_t filler_tasks[2];
    uint16_t number_of_active_tasks;
} FRAMCriticalData;

/* Software information offsets */
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

/* Reboot info offset */
static const uint32_t BOOTLOADER_FAULTY_ADDRESS =
		offsetof(FRAMCriticalData, bootloader_faulty);

/* NOR-Flash binary offsets */
static const uint32_t NOR_FLASH_BINARY_SIZE_ADDRESS =
		offsetof(FRAMCriticalData, nor_flash_binary_size);
static const uint32_t NOR_FLASH_HAMMING_CODE_OFFSET_ADDRESS =
		offsetof(FRAMCriticalData, nor_flash_hamming_code_offset);
static const uint32_t NOR_FLASH_REBOOT_COUNTER_ADDRESS =
        offsetof(FRAMCriticalData, nor_flash_reboot_counter);

/* SD-Card offsets */
static const uint32_t SDC_BINARY_FOLDER_ADDR =
        offsetof(FRAMCriticalData, sd_card_binary_folder);

//static const uint32_t SDC1SL1_FILE_NAME_ADDR =
//		offsetof(FRAMCriticalData, sdc1sl1_binary_file_name);
//static const uint32_t SDC1SL1_HAMM_CODE_NAME_ADDR =
//		offsetof(FRAMCriticalData, sdc1sl1_hamming_code_file_name);
static const uint32_t SDC1SL1_REBOOT_COUNTER_ADDR =
        offsetof(FRAMCriticalData, sdc1sl1_reboot_counter);

//static const uint32_t SDC1SL2_FILE_NAME_ADDR =
//		offsetof(FRAMCriticalData, sdc1sl2_file_name);
//static const uint32_t SDC1SL2_HAMM_CODE_NAME_ADDR =
//		offsetof(FRAMCriticalData, sdc1sl2_hamming_code_file_name);
static const uint32_t SDC1SL2_REBOOT_COUNTER_ADDR =
        offsetof(FRAMCriticalData, sdc1sl2_reboot_counter);

//static const uint32_t SDC2SL1_FILE_NAME_ADDR =
//		offsetof(FRAMCriticalData, sdc2sl1_binary_file_name);
//static const uint32_t SDC2SL1_HAMM_CODE_NAME_ADDR =
//		offsetof(FRAMCriticalData, sdc2sl1_hamming_code_file_name);
static const uint32_t SDC2SL1_REBOOT_COUNTER_ADDR =
        offsetof(FRAMCriticalData, sdc2sl1_reboot_counter);

//static const uint32_t SDC2SL2_FILE_NAME_ADDR  =
//		offsetof(FRAMCriticalData, sdc2sl2_binary_file_name);
//static const uint32_t SDC2SL2_HAMM_CODE_NAME_ADDR =
//		offsetof(FRAMCriticalData, sdc2sl2_hamming_code_file_name);
static const uint32_t SDC2SL2_REBOOT_COUNTER_ADDR =
        offsetof(FRAMCriticalData, sdc2sl2_reboot_counter);

static const uint32_t NUMBER_OF_ACTIVE_TASKS_ADDRESS =
        offsetof(FRAMCriticalData, number_of_active_tasks);

// 12 kB of the upper FRAM will be reserved for the NOR-Flash binary hamming
// code.
static const uint32_t NOR_FLASH_HAMMING_RESERVED_SIZE = 12000;

// 512 bytes of the upper FRAM will be reserved for the bootloader hamming
// code.
static const uint32_t BOOTLOADER_HAMMING_RESERVED_SIZE = 512;

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

int read_sdc_bin_folder_name(char* binary_folder);

/**
 * Please provide a buffer with sufficient size!
 * @param bin_name 		Provide buffer with 16 bytes
 * @param hamm_name 	Provide buffer with 16 bytes
 * @return
 */
int read_sdc1sl1_bin_names(char* bin_name, char* hamm_name);
/**
 * Make sure to provide a '\0' terminated name!
 * @param bin_name
 * @param hamm_name
 * @return
 */
int write_sdc1sl1_bin_names(const char* bin_name, size_t length_bin,
		const char* hamm_name, size_t lenght_hamm);

int increment_sdc1sl1_reboot_counter();
int read_sdc1sl1_reboot_counter(uint8_t* sdc1sl1_reboot_counter);
int reset_sdc1sl1_reboot_counter();

int set_bootloader_faulty(bool faulty);
int is_bootloader_faulty(bool* faulty);

#ifdef __cplusplus
}
#endif

#endif /* MISSION_MEMORY_FRAMAPI_H_ */
