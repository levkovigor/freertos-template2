#ifndef BOOTLOADER_MAIN_H_
#define BOOTLOADER_MAIN_H_

#include <stdint.h>

#define SDRAM_DESTINATION 0x20000000

extern void jump_to_sdram_application(void);

typedef enum {
	BOOT_SD_CARD_0_UPDATE,
	BOOT_SD_CARD_1_UPDATE,
	BOOT_NOR_FLASH
} BootSelect;

#endif /* BOOTLOADER_MAIN_H_ */
