#ifndef SAM9G20_COMMON_CRITICALDATABLOCK_H_
#define SAM9G20_COMMON_CRITICALDATABLOCK_H_

#include <stdint.h>
#include <stddef.h>

static const uint32_t FRAM_TRUE = 1;
static const uint32_t FRAM_FALSE = 0;
static const uint32_t FRAM_UNINITIALIZED = 0xff;

/**
 * Big blocks at the end of FRAM. This address was retrieved FRAM_getMaxAddress.
 * Actually the iOBC datasheet states that the FRAM has 256kB, but the function provided by
 * ISIS to determine the highest address returns almost double the size... We still hardcode
 * half of the returned value.
 */
static const uint32_t FRAM_END_ADDR = 0x3ffff;

//! Calculated required size: 0x20000 (bootloader) * 3 / 256
//! (because 3 parity bits are generated per 256 byte block)
static const size_t BOOTLOADER_HAMMING_RESERVED_SIZE = 0x600;

//! Calculated required size for images: 0x100000 (NOR-Flash) - 0x20000 (bootloader) * 3 / 256
static const uint32_t IMAGES_HAMMING_RESERVED_SIZE = 0x2A00;

static const uint32_t BOOTLOADER_HAMMING_ADDR = FRAM_END_ADDR - BOOTLOADER_HAMMING_RESERVED_SIZE;

static const uint32_t NOR_FLASH_HAMMING_ADDR = BOOTLOADER_HAMMING_ADDR -
        IMAGES_HAMMING_RESERVED_SIZE;
static const uint32_t SDC0_SLOT0_HAMMING_ADDR = NOR_FLASH_HAMMING_ADDR -
        IMAGES_HAMMING_RESERVED_SIZE;
static const uint32_t SDC0_SLOT1_HAMMING_ADDR =  SDC0_SLOT0_HAMMING_ADDR -
        IMAGES_HAMMING_RESERVED_SIZE;
static const uint32_t SDC1_SLOT0_HAMMING_ADDR = SDC0_SLOT1_HAMMING_ADDR -
        IMAGES_HAMMING_RESERVED_SIZE;
static const uint32_t SDC1_SLOT1_HAMMING_ADDR =  SDC1_SLOT0_HAMMING_ADDR -
        IMAGES_HAMMING_RESERVED_SIZE;

typedef enum {
    FLASH_SLOT,
    SDC_0_SL_0,
    SDC_0_SL_1,
    SDC_1_SL_0,
    SDC_1_SL_1,
    BOOTLOADER_0,
    BOOTLOADER_1
} SlotType;

/* Can't forward declare this in a simple way. This is the struct used by the bootloader
to read all required information at once */
typedef struct __attribute__((__packed__)) _BootloaderGroup {
    /* These value will be used on reboot to determine which SD card is the
    default SD card on reboot. 0 or 0xff: None (use SD-Card 0), 1: SD Card 0, 2: SD Card 1 */
    uint16_t preferred_sd_card;
    /* This flag determines whether hamming codes will be used to check the binary. */
    uint16_t global_hamming_flag;

    /* Hamming code flag for individual binaries.  It is recommended to clear the flags when
    updating an image and setting the flag after the hamming code for this image has been uploaded
    as well. Otherwise, a new image might be checked with an invalid old hamming code. */
    uint16_t nor_flash_hamming_flag;
    uint16_t sdc0_image_slot0_hamming_flag;
    uint16_t sdc0_image_slot1_hamming_flag;
    uint16_t sdc1_image_slot0_hamming_flag;
    uint16_t sdc1_image_slot1_hamming_flag;
    uint16_t filler_hamming_flags[1];

    /* Reboot counters for individual binaries */
    uint16_t nor_flash_reboot_counter;
    uint16_t sdc0_image_slot0_reboot_counter;
    uint16_t sdc0_image_slot1_reboot_counter;
    uint16_t sdc1_image_slot0_reboot_counter;
    uint16_t sdc1_image_slot1_reboot_counter;
    uint16_t filler_reboot_counter[1];

    /* Software update information */
    uint8_t software_update_available;
    uint8_t software_update_in_slot_0;
    uint8_t software_update_in_slot_1;
    uint8_t filler_software_update[1];
} BootloaderGroup;

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

    /* The following block will be read at once by the bootloader */
    BootloaderGroup bl_group;

    /* NOR-Flash binary information */
    size_t nor_flash_binary_size;
    /* NOR-Flash binary hamming code size */
    size_t nor_flash_hamming_code_size;
    /* Hamming code size for SD Card 0 slot 0 */
    size_t sdc0_image_slot0_hamming_size;
    size_t sdc0_image_slot1_hamming_size;
    size_t sdc1_image_slot0_hamming_size;
    size_t sdc1_image_slot1_hamming_size;

    /*
     * Bootloader binary information. Bootloader itself could also be stored
     * in FRAM. Hamming code will be stored in FRAM in any case.
     */
    uint32_t bootloader_faulty;
    size_t bootloader_size;
    size_t bootloader_hamming_code_size;

    /* Task information */
    uint16_t number_of_active_tasks;
    uint8_t filler_tasks[2];
} CriticalDataBlock;

static const uint32_t CRITICAL_BLOCK_START_ADDR = 0x0;

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

static const uint32_t SEC_SINCE_EPOCH_ADDR =
        offsetof(CriticalDataBlock, seconds_since_epoch);

/* Bootloader block addresses */
static const uint32_t BL_GROUP_ADDR = offsetof(CriticalDataBlock, bl_group);

static const uint32_t HAMMING_CHECK_FLAG_ADDR =
        offsetof(CriticalDataBlock, bl_group.global_hamming_flag);
static const uint32_t PREFERRED_SD_CARD_ADDR = offsetof(CriticalDataBlock,
        bl_group.preferred_sd_card);

static const uint32_t NOR_FLASH_HAMMING_FLAG_ADDR =
        offsetof(CriticalDataBlock, bl_group.nor_flash_hamming_flag);
static const uint32_t NOR_FLASH_REBOOT_COUNTER_ADDRESS =
        offsetof(CriticalDataBlock, bl_group.nor_flash_reboot_counter);

static const uint32_t SDC0_SL0_HAMMING_FLAG_ADDR =
        offsetof(CriticalDataBlock, bl_group.sdc0_image_slot0_hamming_flag);
static const uint32_t SDC0_SL0_REBOOT_COUNTER_ADDR =
        offsetof(CriticalDataBlock, bl_group.sdc0_image_slot0_reboot_counter);
static const uint32_t SDC0_SL1_HAMMING_FLAG_ADDR =
        offsetof(CriticalDataBlock, bl_group.sdc0_image_slot1_hamming_flag);
static const uint32_t SDC0_SL1_REBOOT_COUNTER_ADDR =
        offsetof(CriticalDataBlock, bl_group.sdc0_image_slot1_reboot_counter);

static const uint32_t SDC1_SL0_HAMMING_FLAG_ADDR =
        offsetof(CriticalDataBlock, bl_group.sdc1_image_slot0_hamming_flag);
static const uint32_t SDC1_SL0_REBOOT_COUNTER_ADDR =
        offsetof(CriticalDataBlock, bl_group.sdc1_image_slot0_reboot_counter);
static const uint32_t SDC1_SL1_HAMMING_FLAG_ADDR =
        offsetof(CriticalDataBlock, bl_group.sdc1_image_slot1_hamming_flag);
static const uint32_t SDC1_SL1_REBOOT_COUNTER_ADDR =
        offsetof(CriticalDataBlock, bl_group.sdc1_image_slot1_reboot_counter);

static const uint32_t SOFTWARE_UPDATE_BOOL_ADDR =
        offsetof(CriticalDataBlock, bl_group.software_update_available);

/* Size addresses which are not part of the bootloader block. */
static const uint32_t NOR_FLASH_BINARY_SIZE_ADDR =
        offsetof(CriticalDataBlock, nor_flash_binary_size);
static const uint32_t NOR_FLASH_HAMMING_CODE_SIZE_ADDR =
        offsetof(CriticalDataBlock, nor_flash_hamming_code_size);
static const uint32_t SDC0_SL0_HAMMING_SIZE_ADDR =
        offsetof(CriticalDataBlock, sdc0_image_slot0_hamming_size);
static const uint32_t SDC0_SL1_HAMMING_SIZE_ADDR =
        offsetof(CriticalDataBlock, sdc0_image_slot1_hamming_size);
static const uint32_t SDC1_SL0_HAMMING_SIZE_ADDR =
        offsetof(CriticalDataBlock, sdc1_image_slot0_hamming_size);
static const uint32_t SDC1_SL1_HAMMING_SIZE_ADDR =
        offsetof(CriticalDataBlock, sdc1_image_slot1_hamming_size);

/* Bootloader addresses */
static const uint32_t BOOTLOADER_SIZE_ADDR = offsetof(CriticalDataBlock, bootloader_size);
static const uint32_t BOOTLOADER_HAMMING_SIZE_ADDR =
        offsetof(CriticalDataBlock, bootloader_hamming_code_size);
static const uint32_t BOOTLOADER_FAULTY_ADDRESS =
        offsetof(CriticalDataBlock, bootloader_faulty);

static const uint32_t NUMBER_OF_ACTIVE_TASKS_ADDRESS =
        offsetof(CriticalDataBlock, number_of_active_tasks);

#endif /* SAM9G20_COMMON_CRITICALDATABLOCK_H_ */
