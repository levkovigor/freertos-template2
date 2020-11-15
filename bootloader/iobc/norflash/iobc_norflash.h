#ifndef BOOTLOADER_IOBC_NORFLASH_IOBC_NORFLASH_H_
#define BOOTLOADER_IOBC_NORFLASH_IOBC_NORFLASH_H_

#include <hal/Storage/NORflash.h>

#include <stdint.h>

/* Address definitions */

// Please note that read addresses incorporate the offset needed for raw access
// with functions like memcpy. The NOR-Flash API functions take addresses
// without that offset!
static const uint32_t NOR_FLASH_BASE_ADDRESS_WRITE = 0x0;
static const uint32_t NOR_FLASH_END_ADDRESS_WRITE = BOARD_NORFLASH_SIZE;
static const uint32_t NOR_FLASH_BASE_ADDRESS_READ = NOR_FLASH_BASE_ADDRESS;
static const uint32_t NOR_FLASH_END_ADDRESS_READ = NOR_FLASH_BASE_ADDRESS +
        BOARD_NORFLASH_SIZE;

// 5 small sectors will be reserved for the bootloader. This should be sufficient
// if the bootloader is smaller than 40960 bytes (0xA000).
static const uint32_t BOOTLOADER_RESERVED_SIZE = 5 * NORFLASH_SMALL_SECTOR_SIZE;
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



#endif /* BOOTLOADER_IOBC_NORFLASH_IOBC_NORFLASH_H_ */
