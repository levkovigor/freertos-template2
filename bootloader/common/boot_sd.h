#ifndef BOOTLOADER_IOBC_NORFLASH_IOBC_BOOT_SD_H_
#define BOOTLOADER_IOBC_NORFLASH_IOBC_BOOT_SD_H_

#include <bootloaderConfig.h>
#include <stdbool.h>

int copy_sdcard_binary_to_sdram(BootSelect boot_select, bool use_hamming);


#endif /* BOOTLOADER_IOBC_NORFLASH_IOBC_BOOT_SD_H_ */
