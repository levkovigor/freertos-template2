#ifndef BOOTLOADER_CONFIG_BOOTLOADERCONFIG_H_
#define BOOTLOADER_CONFIG_BOOTLOADERCONFIG_H_

#define BL_VERSION      1
#define BL_SUBVERSION   0

//! Can be used to disable printouts to reduce code size (does not make much of
//! a difference, most of the AT91 lib uses IO so it is difficult to remove it)
#define DEBUG_IO_LIB            1

#define DEBUG_VERBOSE           0
//! 1 MB minus reserved size of bootloader.
#ifdef AT91SAM9G20_EK
#define OBSW_BINARY_MAX_SIZE 1100000 // 1.007.616 bytes
#else
#define OBSW_BINARY_MAX_SIZE 0x100000 - 0xA000 // 1.007.616 bytes
#endif

#ifdef AT91SAM9G20_EK
//! This should translate to the second block of the NAND flash.
#define NAND_FLASH_OFFSET 0x20000
#endif

//! If the bootloader is flashed with SAM-BA, certain operations like writing
//! CRC of binary sizes and checks performed with them are not possible anymore
//! This flag should be enabled if the software is flashed with SAM-BA.
#define SAM_BA_BOOT 1

#endif /* BOOTLOADER_CONFIG_BOOTLOADERCONFIG_H_ */
