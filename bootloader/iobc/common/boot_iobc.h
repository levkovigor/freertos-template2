#ifndef BOOTLOADER_CORE_BOOTIOBC_H_
#define BOOTLOADER_CORE_BOOTIOBC_H_

#include <stddef.h>
#include <stdint.h>
#include <main.h>

void perform_bootloader_core_operation();
int perform_iobc_copy_operation_to_sdram();
void idle_loop();

int copy_norflash_binary_to_sdram(size_t binary_size);


#endif /* BOOTLOADER_CORE_BOOTIOBC_H_ */
