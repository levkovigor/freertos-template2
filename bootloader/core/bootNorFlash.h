#ifndef BOOTLOADER_BOOTNORFLASH_H_
#define BOOTLOADER_BOOTNORFLASH_H_
#include <main.h>
#include <stddef.h>

int copy_norflash_binary_to_sdram(char* binary_name, size_t binary_size);

#endif /* BOOTLOADER_BOOTNORFLASH_H_ */
