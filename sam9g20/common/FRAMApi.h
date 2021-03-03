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

#include <sam9g20/common/CommonFRAM.h>
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
 * Helper function to increment the global reboot counter.
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
 * Shall be used to enable hamming code checks globally for the bootloader or the scrubbing engine.
 * @return
 */
int fram_set_ham_check_flag();
/**
 * Shall be used to disable hamming code checks globally, e.g. if software was updated but hamming
 * code has not been updated yet.
 * @return
 */
int fram_clear_ham_check_flag();
int fram_get_ham_check_flag();

/**
 * Only valid for slot type FLASH and BOOTLOADER_0 for now.
 * @param slotType
 * @param binary_size
 * @return
 */
int fram_write_binary_size(SlotType slotType, size_t binary_size);
int fram_read_binary_size(SlotType slotType, size_t* binary_size);

/*
 * Functions used to enable image specific haming flags. Should be cleared
 * when the flash image is updated and set again when  the corresponding hamming code
 * has been uploaded. The flag will also be used to determine if a scrubbing operation
 * can be performed.
 */
int fram_set_ham_flag(SlotType slotType);
int fram_clear_ham_flag(SlotType slotType);
int fram_get_ham_flag(SlotType slotType, bool* flag_set);

/*
 * Functions to manipulate local reboot counter belonging to an image type.
 * These are used to switch binary types in case booting multiple times with one type
 * did not work. They should be reset via telecommand after the image is deemed stable.
 */
int fram_increment_img_reboot_counter(SlotType slotType, uint32_t* new_reboot_counter);
int fram_read_img_reboot_counter(SlotType slotType, uint32_t* reboot_counter);
int fram_reset_img_reboot_counter(SlotType slotType);

int fram_write_ham_code(SlotType slotType, uint8_t* buffer, size_t current_offset,
        size_t size_to_write);
/**
 * Shall be used by the to read the hamming codes. Its recommended to supply
 * the reserved size (0xA000 for images, 0x600 for bootloader) as max_buffer.
 * @param slotType
 * @param buffer            Hamming code will be written to this location.
 * @param max_buffer        Maximum allowed size to store into buffer
 * @param current_offset    Offset of start address to read from.
 * @param size_to_read      Specify bytes to read from the specified offset. Set to 0 to determine
 *                          size to read from designated hamming code size field.
 * @param size_read         Return actual size read (size of the hamming code)
 * @return
 */
int fram_read_ham_code(SlotType slotType, uint8_t* buffer, const size_t max_buffer,
        size_t current_offset, size_t size_to_read, size_t* size_read);

int fram_write_ham_size(SlotType slotType, size_t ham_size);
/**
 * Hamming flag can be supplied optionally to determine whether the respective hamming flag is
 * set
 */
int fram_read_ham_size(SlotType slotType, size_t* ham_size, bool* ham_flag_set);

/*
 * Collections of functions used to update the binaries, size of binary fields, the hamming codes
 * of the binaries and their respective size fields. The hamming codes can be used to perform
 * ECC on the images.
 */

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
