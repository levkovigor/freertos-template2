#ifndef SAM9G20_AT91_COMMON_COMMONIOBCCONFIG_H_
#define SAM9G20_AT91_COMMON_COMMONIOBCCONFIG_H_

#include <hal/Storage/NORflash.h>
#include <stdint.h>
#include <stddef.h>

/**
 * Common configuration constants for both bootloader and main OBSW will be located here.
 */

//! Use 8 small sections of the NOR-Flash for the bootloader.
#define IOBC_SMALL_BOOTLOADER_65KB 0
//! Use 8 small sections and one large section of the NOR-Flash for the bootloader.
#define IOBC_LARGE_BOOTLOADER_128KB 1

#define IOBC_BOOTLOADER_SIZE IOBC_LARGE_BOOTLOADER_128KB

/* Address definitions */
static const uint8_t NORFLASH_SMALL_SECTORS_NUMBER = 8;
static const uint8_t NORFLASH_LARGE_SECTORS_NUMBER = 15;
static const uint8_t NORFLASH_SECTORS_NUMBER = 23;

// Please note that read addresses incorporate the offset needed for raw access
// with functions like memcpy. The NOR-Flash API functions take addresses
// without that offset!
static const uint32_t NOR_FLASH_BASE_ADDRESS_WRITE = 0x0;
static const uint32_t NOR_FLASH_END_ADDRESS_WRITE = BOARD_NORFLASH_SIZE;
static const uint32_t NOR_FLASH_BASE_ADDRESS_READ = NOR_FLASH_BASE_ADDRESS;
static const uint32_t NOR_FLASH_END_ADDRESS_READ = NOR_FLASH_BASE_ADDRESS +
        BOARD_NORFLASH_SIZE;

// All 8 small sectors of the NOR-Flash will be reserved for the bootloader.
// This should be sufficient if the bootloader is smaller than 65536 bytes (0x10000).
#if IOBC_BOOTLOADER_SIZE == IOBC_SMALL_BOOTLOADER_65KB


static const uint8_t RESERVED_BL_SMALL_SECTORS = NORFLASH_SMALL_SECTORS_NUMBER;
static const uint8_t RESERVED_BL_LARGE_SECTORS = 0;
static const uint8_t RESERVED_BL_SECTORS = RESERVED_BL_SMALL_SECTORS +  RESERVED_BL_LARGE_SECTORS;


static const uint8_t RESERVED_OBSW_SMALL_SECTORS =
        NORFLASH_SMALL_SECTORS_NUMBER - RESERVED_BL_SMALL_SECTORS;
static const uint8_t RESERVED_OBSW_SECTORS = NORFLASH_SECTORS_NUMBER - RESERVED_BL_SECTORS;
static const uint32_t BOOTLOADER_RESERVED_SIZE = NORFLASH_SMALL_SECTORS_NUMBER *
        NORFLASH_SMALL_SECTOR_SIZE;

#else

static const uint8_t RESERVED_BL_SMALL_SECTORS = NORFLASH_SMALL_SECTORS_NUMBER;
static const uint8_t RESERVED_BL_LARGE_SECTORS = 1;
static const uint8_t RESERVED_BL_SECTORS = RESERVED_BL_SMALL_SECTORS + RESERVED_BL_LARGE_SECTORS;

static const uint8_t RESERVED_OBSW_SMALL_SECTORS = 0;

static const uint8_t RESERVED_OBSW_SECTORS = NORFLASH_SECTORS_NUMBER - RESERVED_BL_SECTORS;
static const uint32_t BOOTLOADER_RESERVED_SIZE =
        (RESERVED_BL_SMALL_SECTORS * NORFLASH_SMALL_SECTOR_SIZE) +
        (RESERVED_BL_LARGE_SECTORS * NORFLASH_LARGE_SECTOR_SIZE);

#endif /* IOBC_BOOTLOADER_SIZE == IOBC_SMALL_BOOTLOADER_65KB */

static const uint32_t BOOTLOADER_BASE_ADDRESS_WRITE = NOR_FLASH_BASE_ADDRESS_WRITE;
static const uint32_t BOOTLOADER_END_ADDRESS_WRITE = BOOTLOADER_RESERVED_SIZE;
static const uint32_t BOOTLOADER_CRC16_ADDRESS_WRITE = BOOTLOADER_RESERVED_SIZE - 2;

static const uint32_t BOOTLOADER_BASE_ADDRESS_READ = NOR_FLASH_BASE_ADDRESS;
static const uint32_t BOOTLOADER_END_ADDRESS_READ = NOR_FLASH_BASE_ADDRESS +
        BOOTLOADER_END_ADDRESS_WRITE;

// This is the end addresses for read access for the OBSW binary.
static const uint32_t BINARY_BASE_ADDRESS_WRITE = BOOTLOADER_RESERVED_SIZE;
static const uint32_t BINARY_BASE_ADDRESS_READ = NOR_FLASH_BASE_ADDRESS +
        BOOTLOADER_RESERVED_SIZE;

static const uint32_t NORFLASH_BL_SIZE_START = BOOTLOADER_RESERVED_SIZE - 6;
static const uint32_t NORFLASH_BL_CRC16_START = BOOTLOADER_RESERVED_SIZE - 2;

static const uint32_t NORFLASH_BL_SIZE_START_READ = BOOTLOADER_BASE_ADDRESS_READ +
        NORFLASH_BL_SIZE_START;
static const uint32_t NORFLASH_BL_CRC16_START_READ = BOOTLOADER_BASE_ADDRESS_READ +
        NORFLASH_BL_CRC16_START;

static const uint32_t NORFLASH_BASE_ADDRESS_READ = NOR_FLASH_BASE_ADDRESS;
static const size_t IOBC_NORFLASH_SIZE = 0x100000;

static const size_t PRIMARY_IMAGE_RESERVED_SIZE = IOBC_NORFLASH_SIZE - BOOTLOADER_RESERVED_SIZE;

#endif /* SAM9G20_AT91_COMMON_COMMONIOBCCONFIG_H_ */
