#ifndef BOOTLOADER_CONFIG_BOOTLOADERCONFIG_H_
#define BOOTLOADER_CONFIG_BOOTLOADERCONFIG_H_

#include <stddef.h>
#include <stdint.h>

#ifdef ISIS_OBC_G20
#include <commonIOBCConfig.h>
#else
#include <commonAt91Config.h>
#endif

#define BL_VERSION      2
#define BL_SUBVERSION   2

//! Can be used if SAM-BA is used to flash the bootloader, which is not able to write
//! the CRC expected written at the end of the NOR-Flash memory.
#define BOOTLOADER_CRC_CHECK_ENABLED        1

//! Use PIT/AIC to kick watchdog periodically. If this is problematic, it can be disabled but
//! then watchdog needs to be kicked manually, which requires measurement for operations
//! like copying large images or the hamming code check
#define BOOTLOADER_KICK_WATCHDOG_IN_PIT_IRQ 1

//! Minimalistic bootloader which copies NOR to SDRAM and jumps there.
#define USE_SIMPLE_BOOTLOADER               0

//! This is related to the boot logic, which will increment a counter for each boot
//! to avoid reboot loops if it is not reset
#define BOOTLOADER_MAX_REBOOT_COUNTER       5

//! Can be used to enable or disable diagnostic printouts in the bootloader.
//! Set to 0 for mission code, 1 recommended for development
#define BOOTLOADER_VERBOSE_LEVEL            1

//! For debugging purposes, can be used to measure elapsed times fpr the whole bootloader
//! operation
#define BOOTLOADER_TIME_MEASUREMENT         0

//! Can be used to display additional information about hamming code check
#define BOOTLOADER_HAM_CODE_DEBUGGING       0

//! Use the SPI driver which does not make use of interrupts. Using AIC can be problematic.
#define USE_FRAM_NON_INTERRUPT_DRV          1

//! Use FreeRTOS in bootloaders. Transfer of control can be problematic, crashes when first task is
//! started, not recommended!
#define USE_FREERTOS                        0

//! Use tiny FS instead of HCC FS. Problematic on iOBC
#define USE_TINY_FS                         0

//! If the bootloader is flashed with SAM-BA, certain operations like writing
//! CRC of binary sizes and checks performed with them are not possible anymore
//! This flag should be enabled if the software is flashed with SAM-BA.
#define SAM_BA_BOOT                         0

static const uint32_t SDRAM_DESTINATION = 0x20000000;

#ifdef AT91SAM9G20_EK

#if BOOTLOADER_TYPE == BOOTLOADER_ONE_STAGE

//! This should translate to the third block of the NAND flash.
static const size_t PRIMARY_IMAGE_NAND_OFFSET = 0x20000;
static const size_t PRIMARY_IMAGE_SDRAM_OFFSET = 0x0;
static const size_t PRIMARY_IMAGE_RESERVED_SIZE = 0x100000; /* 1.007.616 bytes */

static const size_t SECOND_STAGE_BL_JUMP_ADDR = SDRAM_DESTINATION;

//! This should translate to the second block of the NAND flash.
static const size_t SECOND_STAGE_BL_NAND_OFFSET = PRIMARY_IMAGE_NAND_OFFSET;
static const size_t SECOND_STAGE_BL_RESERVED_SIZE = PRIMARY_IMAGE_RESERVED_SIZE;
static const size_t SECOND_STAGE_SDRAM_OFFSET = PRIMARY_IMAGE_SDRAM_OFFSET;

#else /* Two-stage bootloader */

static const size_t FIRST_STAGE_BL_NAND_OFFSET = 0x0;

//! This should translate to the second block of the NAND flash.
static const size_t SECOND_STAGE_BL_NAND_OFFSET = 0x20000;
//! First 1 MB of the SDRAM are reserved for the primary image.
static const size_t SECOND_STAGE_SDRAM_OFFSET = 0x100000;
static const size_t SECOND_STAGE_BL_RESERVED_SIZE = 0x20000;

//! This should translate to the third block of the NAND flash.
static const size_t PRIMARY_IMAGE_NAND_OFFSET = 0x40000;
static const size_t PRIMARY_IMAGE_SDRAM_OFFSET = 0x0;
static const size_t PRIMARY_IMAGE_RESERVED_SIZE = 0x100000; /* 1.007.616 bytes */

static const size_t SECOND_STAGE_BL_JUMP_ADDR = SDRAM_DESTINATION + SECOND_STAGE_SDRAM_OFFSET;
#endif /* BOOTLOADER_TYPE == BOOTLOADER_ONE_STAGE */

#else

#endif /* !defined(AT91SAM9G20_EK) */

typedef enum {
    BOOT_SD_CARD_0_SLOT_0,
    BOOT_SD_CARD_0_SLOT_1,
    BOOT_SD_CARD_1_SLOT_0,
    BOOT_SD_CARD_1_SLOT_1,
    BOOT_NOR_FLASH
} BootSelect;

#endif /* BOOTLOADER_CONFIG_BOOTLOADERCONFIG_H_ */

