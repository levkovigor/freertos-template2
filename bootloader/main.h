#ifndef BOOTLOADER_MAIN_H_
#define BOOTLOADER_MAIN_H_
#include <stdint.h>

#define SDRAM_DESTINATION 0x20000000

extern void jumpToSdramApplication(void);

typedef enum {
	BOOT_SD_CARD_0,
	BOOT_SD_CARD_0_SLOT2,
	BOOT_SD_CARD_1,
	BOOT_SD_CARD_1_SLOT2,
	BOOT_NOR_FLASH
} BootSelect;

#endif /* BOOTLOADER_MAIN_H_ */
