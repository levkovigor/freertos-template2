#ifndef SAM9G20_COMMON_CRITICALDATABLOCK_H_
#define SAM9G20_COMMON_CRITICALDATABLOCK_H_

#include <stdint.h>
#include <stddef.h>

/**
 * Big blocks at the end of FRAM. This address was retrieved FRAM_getMaxAddress.
 * Actually the iOBC datasheet states that the FRAM has 256kB, but the functions returns
 * almost double the size... We still hardcode half of the returned value.
 */
static const uint32_t FRAM_END_ADDR = 0x3ffff;

/**
 * This struct will gather all critical data stored on (virutalized) FRAM.
 */
typedef struct __attribute__((__packed__))  _CriticalDataBlock {
    /* Software information */
    uint8_t software_version;
    uint8_t software_subversion;
    uint8_t software_subsubversion;
    uint8_t filler_sw_version;

    /* Reboot information */
    uint32_t reboot_counter;

    /* Second counter */
    uint32_t seconds_since_epoch;

    /* This flag determines whether hamming codes will be used to check the binary.
    It is recommended to clear the flag when updating an image and setting the flag after
    the hamming code for this image has been uploaded as well. Otherwise, a new image might
    be checked with an invalid old hamming code. */
    uint32_t use_hamming_flag;

    /* NOR-Flash binary information */
    uint32_t nor_flash_binary_size;
    /* NOR-Flash binary hamming code size */
    size_t nor_flash_hamming_code_size;
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
    size_t sdc0_image_slot0_hamming_size;

    /* Reboot counters SD Card Card 0 slot 1 */
    uint32_t sdc0_image_slot1_reboot_counter;
    /* Hamming code flag, explanation see above */
    uint32_t sdc0_image_slot1_use_hamming_flag;
    /* Hamming code size for SD Card 0 slot 1 */
    size_t sdc0_image_slot1_hamming_size;

    /* Reboot counters SD Card 1 slot 0 */
    uint32_t sdc1_image_slot0_reboot_counter;
    /* Hamming code flag, explanation see above */
    uint32_t sdc1_image_slot0_use_hamming_flag;
    /* Hamming code size for SD Card 1 slot 0 */
    size_t sdc1_image_slot0_hamming_size;

    /* Reboot counters SD Card Card 1 slot 1 */
    uint32_t sdc1_image_slot1_reboot_counter;
    /* Hamming code flag, explanation see above */
    uint32_t sdc1_image_slot1_use_hamming_flag;
    /* Hamming code size for SD Card 1 slot 1*/
    size_t sdc1_image_slot1_hamming_size;

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
} CriticalDataBlock;

static const uint8_t CRITICAL_BLOCK_START_ADDR = 0x0;

/* Software information offsets */
static const uint8_t SOFTWARE_VERSION_ADDR =
        offsetof(CriticalDataBlock, software_version);
static const uint8_t SOFTWARE_SUBVERSION_ADDR =
        offsetof(CriticalDataBlock, software_subversion);
static const uint8_t SOFTWARE_SUBSUBVERSION_ADDR =
        offsetof(CriticalDataBlock, software_subsubversion);

/* Reboot info offset */
static const uint32_t REBOOT_COUNTER_ADDR =
        offsetof(CriticalDataBlock, reboot_counter);

static const uint32_t BOOTLOADER_HAMMING_SIZE_ADDR =
        offsetof(CriticalDataBlock, bootloader_hamming_code_size);
static const uint32_t BOOTLOADER_FAULTY_ADDRESS =
        offsetof(CriticalDataBlock, bootloader_faulty);

static const uint32_t SEC_SINCE_EPOCH_ADDR =
        offsetof(CriticalDataBlock, seconds_since_epoch);
static const uint32_t SOFTWARE_UPDATE_BOOL_ADDR =
        offsetof(CriticalDataBlock, software_update_available);

static const uint32_t HAMMING_CHECK_FLAG_ADDR =
        offsetof(CriticalDataBlock, use_hamming_flag);

/* NOR-Flash binary offsets */
static const uint32_t NOR_FLASH_BINARY_SIZE_ADDR =
        offsetof(CriticalDataBlock, nor_flash_binary_size);
static const uint32_t NOR_FLASH_HAMMING_CODE_SIZE_ADDR =
        offsetof(CriticalDataBlock, nor_flash_hamming_code_size);
static const uint32_t NOR_FLASH_REBOOT_COUNTER_ADDRESS =
        offsetof(CriticalDataBlock, nor_flash_reboot_counter);


static const uint32_t PREFERED_SD_CARD_ADDR = offsetof(CriticalDataBlock, preferedSdCard);

static const uint32_t SDC0_SL0_REBOOT_COUNTER_ADDR =
        offsetof(CriticalDataBlock, sdc0_image_slot0_reboot_counter);
static const uint32_t SDC0_SL0_HAMMING_SIZ_ADDR =
        offsetof(CriticalDataBlock, sdc0_image_slot0_hamming_size);

static const uint32_t SDC0_SL1_REBOOT_COUNTER_ADDR =
        offsetof(CriticalDataBlock, sdc0_image_slot1_reboot_counter);
static const uint32_t SDC0_SL1_HAMMING_SIZ_ADDR =
        offsetof(CriticalDataBlock, sdc0_image_slot1_hamming_size);

static const uint32_t SDC1_SL0_REBOOT_COUNTER_ADDR =
        offsetof(CriticalDataBlock, sdc1_image_slot0_reboot_counter);
static const uint32_t SDC1_SL0_HAMMING_SIZ_ADDR =
        offsetof(CriticalDataBlock, sdc1_image_slot0_hamming_size);

static const uint32_t SDC1_SL1_REBOOT_COUNTER_ADDR =
        offsetof(CriticalDataBlock, sdc1_image_slot1_reboot_counter);
static const uint32_t SDC1_SL1_HAMMING_SIZ_ADDR =
        offsetof(CriticalDataBlock, sdc1_image_slot1_hamming_size);

static const uint32_t NUMBER_OF_ACTIVE_TASKS_ADDRESS =
        offsetof(CriticalDataBlock, number_of_active_tasks);


#endif /* SAM9G20_COMMON_CRITICALDATABLOCK_H_ */
