/**
 * @brief   This common API can be used by both C++ and C code
 *          to write data to FRAM.
 * @details
 * This header encapsulates the data stored on the FRAM and provides
 * funtions to read and write/overwrite them conveniently.
 * The functions use the HAL library provided by ISIS to write on the FRAM.
 * The FRAM needs to be started first before using any functions here! (see FRAM.h file)
 * @author R. Mueller
 */
#ifndef MISSION_MEMORY_FRAMAPI_H_
#define MISSION_MEMORY_FRAMAPI_H_

/**
         ____________________________________________
        |                                            |
        |               CRITICAL BLOCK               |
        |____________________________________________|
        |                                            |
        |                                            |
        |____________________________________________|
        |                                            |
        |                                            |
        |____________________________________________|
        |                                            |
        |              BOOTLOADER_HAMMING            |
        |____________________________________________|

 */

#ifdef __cplusplus
extern "C" {
#endif

#include <sam9g20/common/SDCardApi.h>
#include <sam9g20/common/config/commonConfig.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * Read the whole critical block. Size of buffer has to be provided in max_size.
 * It is recommended to set max_size to sizeof(CriticalDataBlock)
 * @param buffer
 * @param max_size
 * @return
 */
int fram_read_critical_block(uint8_t* buffer, const size_t max_size);
int fram_write_software_version(uint8_t sw_version, uint8_t sw_subversion,
        uint8_t sw_subsubversion);
int fram_read_software_version(uint8_t* sw_version, uint8_t* sw_subversion,
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
int fram_increment_reboot_counter(uint32_t* new_reboot_counter);
int fram_read_reboot_counter(uint32_t* reboot_counter);
/**
 * Should only be used during development! Part of the Execute Before Flight sequence
 * @return
 */
int fram_reset_reboot_counter();

int fram_update_seconds_since_epoch(uint32_t secondsSinceEpoch);
int fram_read_seconds_since_epoch(uint32_t* secondsSinceEpoch);

/**
 * Shall be used to enable hamming code checks for the bootloader or the scrubbing engine.
 * @return
 */
int fram_set_ham_check_flag();
/**
 * Shall be used to disable hamming code checks, e.g. if software was updated but hamming
 * code has not been updated yet.
 * @return
 */
int fram_clear_ham_check_flag();
int fram_get_ham_check_flag();

/*
 * Collections of functions used to update the binaries, size of binary fields, the hamming codes
 * of the binaries and their respective size fields. The hamming codes can be used to perform
 * ECC on the images.
 */
int fram_write_flash_binary_size(size_t binary_size);
int fram_read_flash_binary_size(size_t* binary_size);
/* Reboot counter flash */
int increment_flash_reboot_counter();
int read_flash_reboot_counter(uint8_t* nor_flash_reboot_counter);
int reset_flash_reboot_counter();
/* Hamming utilities */
int fram_write_flash_ham_size(size_t ham_size);
int fram_read_flash_ham_size(size_t* ham_size, bool* hamming_flag_set);
/**
 * Functions used to enable hamming flag checks for the NOR-Flash. Should be cleared
 * when the NOR-Flash image is updated and set again when  the corresponding hamming code
 * has been uploaded.
 * @return
 */
int fram_set_flash_ham_flag();
int fram_clear_flash_ham_flag();
int fram_get_flash_ham_flag(bool* flag_set);
/**
 * Shall be used by the bootloader to read the hamming code. Its recommended to supply
 * max_buffer = 0x2A00 (maximum possible size of hamming code)
 * @param buffer
 * @param max_buffer
 * @param size_read Return actual size read (size of the hamming code)
 * @return
 */
int fram_read_flash_ham_code(uint8_t* buffer, const size_t max_buffer, size_t* size_read);
int fram_write_flash_ham_code(uint8_t* hamming_code, size_t current_offset,
        size_t size_to_write);

int fram_write_bootloader_ham_size(size_t ham_size);
int fram_read_bootloader_ham_size(size_t* ham_size);

int fram_write_bootloader_ham_code(uint8_t* ham_code, size_t size_to_write);
int fram_read_bootloader_ham_code(uint8_t* ham_code, size_t* size);

int fram_write_sdc_0_sl_0_ham_size(size_t ham_size);
int fram_read_sdc_0_sl_0_ham_size(size_t* ham_size);
int fram_write_sdc_0_sl_0_ham_code(uint8_t* hamming_code, size_t current_offset,
        size_t size_to_write);

int fram_write_sdc_0_sl_1_ham_size(size_t ham_size);
int fram_read_sdc_0_sl_1_ham_size(size_t* ham_size);
int fram_write_sdc_0_sl_1_ham_code(uint8_t* hamming_code, size_t current_offset,
        size_t size_to_write);

int fram_write_sdc_1_sl_0_ham_size(size_t ham_size);
int fram_read_sdc_1_sl_0_ham_size(size_t* ham_size);
int fram_write_sdc_1_sl_0_ham_code(uint8_t* hamming_code, size_t current_offset,
        size_t size_to_write);

int fram_write_sdc_1_sl_1_ham_size(size_t ham_size);
int fram_read_sdc_1_sl_1_ham_size(size_t* ham_size);
int fram_write_sdc_1_sl_1_ham_code(uint8_t* hamming_code, size_t current_offset,
        size_t size_to_write);

int increment_sdc0_slot0_reboot_counter();
int read_sdc0_slot0_reboot_counter(uint8_t* sdc1sl1_reboot_counter);
int reset_sdc0_slot0_reboot_counter();

int set_sdc_hamming_flag(VolumeId volume, SdSlots slot);
int get_sdc_hamming_flag(bool* flag_set, VolumeId volume, SdSlots slot);
int clear_sdc_hamming_flag(VolumeId volume, SdSlots slot);

int set_bootloader_faulty(bool faulty);
int is_bootloader_faulty(bool* faulty);

int set_preferred_sd_card(VolumeId volumeId);
int get_preferred_sd_card(VolumeId* volumeId);

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
