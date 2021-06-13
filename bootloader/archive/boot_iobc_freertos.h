/**
 * @brief   Bootloader core module.
 * @
 */
#ifndef BOOTLOADER_CORE_BOOTIOBC_H_
#define BOOTLOADER_CORE_BOOTIOBC_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

void perform_bootloader_core_operation();
int copy_norflash_binary_to_sdram(size_t binary_size, bool use_hamming);
extern void jump_to_sdram_application(uint32_t stack_ptr, uint32_t jump_address);

void idle_loop();

#endif /* BOOTLOADER_CORE_BOOTIOBC_H_ */
