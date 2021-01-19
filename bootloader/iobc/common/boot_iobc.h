/**
 * @brief   Bootloader core module.
 * @
 */
#ifndef BOOTLOADER_CORE_BOOTIOBC_H_
#define BOOTLOADER_CORE_BOOTIOBC_H_

#include <stddef.h>
#include <stdint.h>
#include <main.h>

void perform_bootloader_core_operation();
int copy_norflash_binary_to_sdram(size_t binary_size);

void idle_loop();

#endif /* BOOTLOADER_CORE_BOOTIOBC_H_ */
