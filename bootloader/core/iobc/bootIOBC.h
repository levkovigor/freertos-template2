#ifndef BOOTLOADER_CORE_BOOTIOBC_H_
#define BOOTLOADER_CORE_BOOTIOBC_H_

#include <stddef.h>
#include <stdint.h>
#include <main.h>

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

// Note: This is the end addresses for read access for the OBSW binary.
// 5 small sectors will be reserved for the bootloader.
static const uint32_t BINARY_BASE_ADDRESS_READ = NORFLASH_SA5_ADDRESS;
static const uint32_t BINARY_END_ADDRESS_READ = NOR_FLASH_END_ADDRESS_READ;


int copy_norflash_binary_to_sdram(size_t binary_size);
int copy_sdcard_binary_to_sdram(BootSelect boot_select);

#endif /* BOOTLOADER_CORE_BOOTIOBC_H_ */
