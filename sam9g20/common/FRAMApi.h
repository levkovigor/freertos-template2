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
    /* Software information */
    uint8_t software_version;
    uint8_t software_subversion;
    uint8_t software_subsubversion;
    uint8_t filler_sw_version;

    /* Reboot information */
    uint32_t reboot_counter;

    /* Second counter */
    uint32_t seconds_since_epoch;

    /* NOR-Flash binary information */
    uint32_t nor_flash_binary_size;
    uint32_t nor_flash_hamming_code_size;
    /* This flag determines whether hamming codes will be used to check the binary.
    It is recommended to clear the flag when updating an image and setting the flag after
    the hamming code for this image has been uploaded as well. Otherwise, a new image might
    be checked with an invalid old hamming code. */
    uint32_t nor_flash_use_hamming_flag;
    uint32_t nor_flash_reboot_counter;

    /* SD-Card */

    /* These value will be used on reboot to determine which SD card is the
	default SD card on reboot. 0: None, 1: SD Card 0, 2: SD Card 1 */
    uint32_t preferedSdCard;

    /* Reboot counters SD Card 0 slot 0 */
    uint32_t sdc0_image_slot0_reboot_counter;
    /* Hamming code flag, explanation see above (nor_flash_use_hamming_flag) */
    uint32_t sdc0_image_slot0_use_hamming_flag;
    /* Hamming code size for SD Card 0 slot 0 */
    uint32_t sdc0_image_slot0_hamming_size;

    /* Reboot counters SD Card Card 0 slot 1 */
    uint32_t sdc0_image_slot1_reboot_counter;
    /* Hamming code flag, explanation see above */
    uint32_t sdc0_image_slot1_use_hamming_flag;
    /* Hamming code size for SD Card 0 slot 1 */
    uint32_t sdc0_image_slot1_hamming_size;

    /* Reboot counters SD Card 1 slot 0 */
    uint32_t sdc1_image_slot0_reboot_counter;
    /* Hamming code flag, explanation see above */
    uint32_t sdc1_image_slot0_use_hamming_flag;
    /* Hamming code size for SD Card 1 slot 0 */
    uint32_t sdc1_image_slot0_hamming_size;

    /* Reboot counters SD Card Card 1 slot 1 */
    uint32_t sdc1_image_slot1_reboot_counter;
    /* Hamming code flag, explanation see above */
    uint32_t sdc1_image_slot1_use_hamming_flag;
    /* Hamming code size for SD Card 1 slot 1*/
    uint32_t sdc1_image_slot1_hamming_size;

    /*
     * Bootloader binary information. Bootloader itself could also be stored
     * in FRAM. Hamming code will be stored in FRAM in any case.
     */
    uint32_t bootloader_faulty;
    uint32_t bootloader_hamming_code_size;

    /* Software update information */
    uint8_t filler_software_update[1];
    uint8_t software_update_available;
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


int increment_sdc0_slot0_reboot_counter();
int read_sdc0_slot0_reboot_counter(uint8_t* sdc1sl1_reboot_counter);
int reset_sdc0_slot0_reboot_counter();

int set_bootloader_faulty(bool faulty);
int is_bootloader_faulty(bool* faulty);

int set_prefered_sd_card(VolumeId volumeId);
int get_prefered_sd_card(VolumeId* volumeId);

int write_bootloader_hamming_code(const uint8_t* code, size_t size);
int read_bootloader_hamming_code(uint8_t* code, size_t* size);

int set_to_load_softwareupdate(bool enable, VolumeId volume);

/**
 * Check whether the software should be loaded from the SD-Card or the NOR-Flash.
 * @param enable Will be set to true if software update should be loaded.
 * @param volume If enable is set to true, the target volume ID will be set as well.
 * @return
 * 0 on success, -1 and -2 on FRAM failures, -3 on invalid input.
 */
int get_to_load_softwareupdate(bool* enable, VolumeId* volume);

#ifdef __cplusplus
}
#endif

#endif /* MISSION_MEMORY_FRAMAPI_H_ */
