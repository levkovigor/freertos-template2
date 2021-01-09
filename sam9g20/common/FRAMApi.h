#ifndef MISSION_MEMORY_FRAMAPI_H_
#define MISSION_MEMORY_FRAMAPI_H_

#include <sam9g20/common/SDCardApi.h>
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
    uint8_t software_subsubversion;
    uint8_t filler_sw_version;

    /* Reboot information [4, 4-7] */
	uint32_t reboot_counter;

	bool bootloader_faulty;
	size_t bootloader_hamming_code_size;

	/* Second counter [4, 8 - 11] */
	uint32_t seconds_since_epoch;

	/* NOR-Flash binary information [4, 12 - 15] */
	size_t nor_flash_binary_size;
	size_t nor_flash_hamming_code_size;
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
	uint8_t software_update_in_slot_0;
	uint8_t software_update_in_slot_1;

	/* Task information */
	uint8_t filler_tasks[2];
    uint16_t number_of_active_tasks;
} FRAMCriticalData;

#ifdef __cplusplus
extern "C" {
#endif

int write_software_version(uint8_t sw_version, uint8_t sw_subversion,
        uint8_t sw_subsubversion);
int read_software_version(uint8_t* sw_version, uint8_t* sw_subversion,
        uint8_t* sw_subsubversion);

/**
 * Helper function to increment the reboot counter.
 * @param verify_write  If this is set to true, the write operation will be
 * verified. This should be disabled when incrementing for exceptions.
 * @param set_reboot_flag
 * If the reboot was intended (e.g. commanded), this flag should be set.
 * The flag is used to be able to track restarts from exceptions.
 * @return
 */
int increment_reboot_counter(uint32_t* new_reboot_counter);
int read_reboot_counter(uint32_t* reboot_counter);

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

int write_bootloader_hamming_code(const uint8_t* code, size_t size);
int read_bootloader_hamming_code(uint8_t* code, size_t* size);

int set_to_load_softwareupdate(bool slot0);
int get_software_to_be_updated(bool* yes, bool* slot0);

#ifdef __cplusplus
}
#endif

#endif /* MISSION_MEMORY_FRAMAPI_H_ */
