#ifndef BOOTLOADER_CORE_BOOTIOBC_H_
#define BOOTLOADER_CORE_BOOTIOBC_H_

#include <stddef.h>
#include <stdint.h>
#include <main.h>

void perform_bootloader_core_operation();
int perform_iobc_copy_operation_to_sdram();
void idle_loop();

extern void jumpToSdramApplication(void);

int copy_norflash_binary_to_sdram(size_t binary_size);
int copy_sdcard_binary_to_sdram(BootSelect boot_select);

#endif /* BOOTLOADER_CORE_BOOTIOBC_H_ */
