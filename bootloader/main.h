#ifndef BOOTLOADER_MAIN_H_
#define BOOTLOADER_MAIN_H_
#include <stdint.h>
#include <hal/Storage/NORflash.h>

/* Address definitions */
static const uint32_t NOR_FLASH_BASE_ADDRESS_WRITE = 0x0;
static const uint32_t NOR_FLASH_END_ADDRESS_WRITE = BOARD_NORFLASH_SIZE;
static const uint32_t NOR_FLASH_BASE_ADDRESS_READ = NOR_FLASH_BASE_ADDRESS;
static const uint32_t NOR_FLASH_END_ADDRESS_READ = NOR_FLASH_BASE_ADDRESS +
        BOARD_NORFLASH_SIZE;

// Assuming that bootloader binary is smaller than 24.576 bytes!
// Extend if necessary and don't forget to adapt CRC address.
static const uint32_t BOOTLOADER_SIZE = 0x6000;
static const uint32_t BOOTLOADER_BASE_ADDRESS_WRITE = NOR_FLASH_BASE_ADDRESS_WRITE;
static const uint32_t BOOTLOADER_END_ADDRESS_WRITE = BOOTLOADER_SIZE;
static const uint32_t BOOTLOADER_CRC16_ADDRESS_WRITE = BOOTLOADER_SIZE - 2;

static const uint32_t BOOTLOADER_END_ADDRESS_READ = NOR_FLASH_BASE_ADDRESS +
        BOOTLOADER_END_ADDRESS_WRITE;

// Note: This is the end addresses for read access!
static const uint32_t BINARY_BASE_ADDRESS_READ = BOOTLOADER_END_ADDRESS_READ;
static const uint32_t BINARY_END_ADDRESS_READ = NOR_FLASH_END_ADDRESS_READ;





#endif /* BOOTLOADER_MAIN_H_ */
