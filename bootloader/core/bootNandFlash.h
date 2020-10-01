#ifndef BOOTLOADER_CORE_BOOTNANDFLASH_H_
#define BOOTLOADER_CORE_BOOTNANDFLASH_H_

#include <memories/nandflash/SkipBlockNandFlash.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

int copy_nandflash_binary_to_sdram(bool enable_full_printout);
void NandInit();
int BOOT_NAND_CopyBin(const uint32_t binary_offset, size_t binary_size);

#endif /* BOOTLOADER_CORE_BOOTNANDFLASH_H_ */
