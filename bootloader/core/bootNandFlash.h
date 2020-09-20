#ifndef BOOTLOADER_CORE_BOOTNANDFLASH_H_
#define BOOTLOADER_CORE_BOOTNANDFLASH_H_

#include <memories/nandflash/SkipBlockNandFlash.h>
#include <stddef.h>
#include <stdint.h>

#define SDRAM_DESTINATION 0x20000000

int copy_nandflash_binary_to_sdram();
void NandInit();
int BOOT_NAND_CopyBin(const uint32_t binary_offset, size_t binary_size);

#endif /* BOOTLOADER_CORE_BOOTNANDFLASH_H_ */
