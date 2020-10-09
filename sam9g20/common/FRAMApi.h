#ifndef MISSION_MEMORY_FRAMAPI_H_
#define MISSION_MEMORY_FRAMAPI_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <sam9g20/memory/SDCardApi.h>

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
	uint16_t reboot_counter;
	uint16_t reboot_flag;

	bool bootloader_faulty;

	/* Second counter [4, 8 - 11] */
	uint32_t seconds_since_epoch;

	/* NOR-Flash binary information [4, 12 - 15] */
	size_t nor_flash_binary_size;
	size_t nor_flash_hamming_code_offset;
	uint8_t filler_nor_flash[3];
	uint8_t nor_flash_reboot_counter;

	/* SD-Card */
	// This value will be used on reboot to determine which SD card is the
	// default SD card on reboot.
	// 0: None, 1: SD Card 0, 2: SD Card 1
	VolumeId preferedSdCard;
	uint32_t sdc1sl1_reboot_counter;
	uint32_t sdc1sl2_reboot_counter;
	uint32_t sdc2sl1_reboot_counter;
	uint32_t sdc2sl2_reboot_counter;

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
/* Reboot info offset */
static const uint32_t REBOOT_COUNTER_ADDR =
		offsetof(FRAMCriticalData, reboot_counter);
static const uint32_t REBOOT_FLAG_ADDR =
        offsetof(FRAMCriticalData, reboot_flag);
static const uint32_t BOOTLOADER_FAULTY_ADDRESS =
        offsetof(FRAMCriticalData, bootloader_faulty);

static const uint32_t SEC_SINCE_EPOCH_ADDR =
		offsetof(FRAMCriticalData, seconds_since_epoch);
static const uint32_t SOFTWARE_UPDATE_BOOL_ADDR =
        offsetof(FRAMCriticalData, software_update_available);



/* NOR-Flash binary offsets */
static const uint32_t NOR_FLASH_BINARY_SIZE_ADDRESS =
		offsetof(FRAMCriticalData, nor_flash_binary_size);
static const uint32_t NOR_FLASH_HAMMING_CODE_OFFSET_ADDRESS =
		offsetof(FRAMCriticalData, nor_flash_hamming_code_offset);
static const uint32_t NOR_FLASH_REBOOT_COUNTER_ADDRESS =
        offsetof(FRAMCriticalData, nor_flash_reboot_counter);


static const uint32_t PREFERED_SD_CARD_ADDR =
		offsetof(FRAMCriticalData, preferedSdCard);
static const uint32_t SDC1SL1_REBOOT_COUNTER_ADDR =
        offsetof(FRAMCriticalData, sdc1sl1_reboot_counter);
static const uint32_t SDC1SL2_REBOOT_COUNTER_ADDR =
        offsetof(FRAMCriticalData, sdc1sl2_reboot_counter);
static const uint32_t SDC2SL1_REBOOT_COUNTER_ADDR =
        offsetof(FRAMCriticalData, sdc2sl1_reboot_counter);
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

/**
 * Helper function to increment the reboot counter.
 * @param verify_write  If this is set to true, the write operation will be
 * verified. This should be disabled when incrementing for exceptions.
 * @param set_reboot_flag
 * If the reboot was intended (e.g. commanded), this flag should be set.
 * The flag is used to be able to track restarts from exceptions.
 * @return
 */
int increment_reboot_counter(bool verify_write, bool set_reboot_flag);

/**
 * This helper function will be called on reboot to verify whether the
 * last restart was commanded (which should ideally always be the case)
 * or whether an exception or something else occured where the restart was
 * not intentional
 * @return
 */
int verify_reboot_flag(bool* bootcounter_incremented);
int read_reboot_counter(uint16_t* reboot_counter);
/**
 * Should only be used during development!
 * @return
 */
int reset_reboot_counter();

int update_seconds_since_epoch(uint32_t secondsSinceEpoch);
int read_seconds_since_epoch(uint32_t* secondsSinceEpoch);

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

int set_prefered_sd_card(VolumeId volumeId);
int get_prefered_sd_card(VolumeId* volumeId);

#ifdef __cplusplus
}
#endif

#endif /* MISSION_MEMORY_FRAMAPI_H_ */
