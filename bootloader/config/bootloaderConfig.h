#ifndef BOOTLOADER_CONFIG_BOOTLOADERCONFIG_H_
#define BOOTLOADER_CONFIG_BOOTLOADERCONFIG_H_

#include <stddef.h>
#include <stdint.h>

#ifdef ISIS_OBC_G20
#include <commonIOBCConfig.h>
#endif

#define BL_VERSION      1
#define BL_SUBVERSION   2

//! Can be used to enable or disable diagnostic printouts in the bootloader.
#define BOOTLOADER_VERBOSE_LEVEL    1

#define BOOTLOADER_ONE_STAGE        0

//! Use tiny FS instead of HCC FS.
#define USE_TINY_FS 			0

static const uint32_t SDRAM_DESTINATION = 0x20000000;

#ifdef AT91SAM9G20_EK

#if BOOTLOADER_ONE_STAGE == 1

//! This should translate to the third block of the NAND flash.
static const size_t PRIMARY_IMAGE_NAND_OFFSET = 0x20000;
static const size_t PRIMARY_IMAGE_SDRAM_OFFSET = 0x0;
static const size_t PRIMARY_IMAGE_RESERVED_SIZE = 0x100000; /* 1.007.616 bytes */

static const size_t SECOND_STAGE_BL_JUMP_ADDR = SDRAM_DESTINATION;

//! This should translate to the second block of the NAND flash.
static const size_t SECOND_STAGE_BL_NAND_OFFSET = PRIMARY_IMAGE_NAND_OFFSET;
static const size_t SECOND_STAGE_BL_RESERVED_SIZE = PRIMARY_IMAGE_RESERVED_SIZE;
static const size_t SECOND_STAGE_SDRAM_OFFSET = PRIMARY_IMAGE_SDRAM_OFFSET;

#else

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
#endif /* BOOTLOADER_ONE_STAGE == 1 */

#else

typedef enum {
    BOOT_SD_CARD_0_UPDATE,
    BOOT_SD_CARD_1_UPDATE,
    BOOT_NOR_FLASH
} BootSelect;

#endif

//! If the bootloader is flashed with SAM-BA, certain operations like writing
//! CRC of binary sizes and checks performed with them are not possible anymore
//! This flag should be enabled if the software is flashed with SAM-BA.
#define SAM_BA_BOOT         0

#ifdef AT91SAM9G20_EK
static const char* const SW_REPOSITORY =                 "BIN/AT91/OBSW";
#else
static const char* const SW_REPOSITORY =                 "BIN/IOBC/OBSW";
#endif

static const char* const SW_UPDATE_FILE_NAME =           "obsw_up.bin";


#endif /* BOOTLOADER_CONFIG_BOOTLOADERCONFIG_H_ */

