#ifndef BOOTLOADER_CORE_BOOTAT91_H_
#define BOOTLOADER_CORE_BOOTAT91_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

void NandInit();
int copy_nandflash_image_to_sdram(const uint32_t source_offset, const size_t source_size,
        const size_t target_offset, bool configureNand);
int BOOT_NAND_CopyBin(const uint32_t binary_offset, size_t binary_size, size_t target_offset);
void go_to_jump_address(unsigned int jumpAddr, unsigned int matchType);
void idle_loop();

#endif /* BOOTLOADER_CORE_BOOTAT91_H_ */
