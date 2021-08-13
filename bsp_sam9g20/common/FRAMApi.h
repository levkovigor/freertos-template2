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

//! Calculated required size: 0x20000 (bootloader) * 3 / 256
//! (because 3 parity bits are generated per 256 byte block)
static const size_t BOOTLOADER_HAMMING_RESERVED_SIZE = 0x600;

//! Calculated required size for images: 0x100000 (NOR-Flash) - 0x20000 (bootloader) * 3 / 256
static const uint32_t NOR_FLASH_HAMMING_RESERVED_SIZE = 0x2A00;

/**
 * Read the whole critical block. Size of buffer has to be provided in max_size.
 * It is recommended to set max_size to sizeof(CriticalDataBlock)
 * @param buffer
 * @param max_size
 * @return
 */
int read_critical_block(uint8_t* buffer, const size_t max_size);

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

/**
 * Shall be used to disable hamming code checks, e.g. if software was updated but hamming
 * code has not been updated yet.
 * @return
 */
int clear_hamming_check_flag();
/**
 * Shall be used to enable hamming code checks for the bootloader or the scrubbing engine.
 * @return
 */
int set_hamming_check_flag();
int get_hamming_check_flag();

int write_nor_flash_binary_size(size_t binary_size);
int read_nor_flash_binary_size(size_t* binary_size);

/**
 * Shall be used once when updating the NOR-Flash hamming code.
 * @param hamming_size
 * @param set_hamming_flag
 * @return
 */
int write_nor_flash_hamming_size(size_t hamming_size, bool set_hamming_flag);;
/**
 * Shall be used to update the hamming code for the NOR-Flash binary.
 * Can be performed in multiple steps by supplying the current offset and a pointer
 * to the data to be written.
 * @param current_offset
 * @param size_to_write
 * @return
 */
int write_nor_flash_hamming_code(uint8_t* hamming_code, size_t current_offset,
        size_t size_to_write);
/**
 * Functions used to enable hamming flag checks for the NOR-Flash. Should be cleared
 * when the NOR-Flash image is updated and set again when  the corresponding hamming code
 * has been uploaded.
 * @return
 */
int set_flash_hamming_flag();
int clear_flash_hamming_flag();
int get_flash_hamming_flag();
/**
 * Can be used to determine the size of the hamming code.
 * @param hamming_size
 * @param set_hamming_flag
 * @return
 */
int read_nor_flash_hamming_size(size_t* hamming_size, bool* set_hamming_flag);
/**
 * Shall be used by the bootloader to read the hamming code. Its recommended to supply
 * max_buffer = 0x2A00 (maximum possible size of hamming code)
 * @param buffer
 * @param max_buffer
 * @param size_read Return actual size read (size of the hamming code)
 * @return
 */
int read_nor_flash_hamming_code(uint8_t* buffer, const size_t max_buffer, size_t* size_read);


int increment_nor_flash_reboot_counter();
int read_nor_flash_reboot_counter(uint8_t* nor_flash_reboot_counter);
int reset_nor_flash_reboot_counter();


int increment_sdc0_slot0_reboot_counter();
int read_sdc0_slot0_reboot_counter(uint8_t* sdc1sl1_reboot_counter);
int reset_sdc0_slot0_reboot_counter();

int set_sdc_hamming_flag(VolumeId volume, SdSlots slot);
int get_sdc_hamming_flag(bool* flag_set, VolumeId volume, SdSlots slot);
int clear_sdc_hamming_flag(VolumeId volume, SdSlots slot);

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
