#ifndef BOOTLOADER_CONFIG_BOOTLOADERCONFIG_H_
#define BOOTLOADER_CONFIG_BOOTLOADERCONFIG_H_

#ifdef AT91SAM9G20_EK
// This should translate to the second block of the NAND flash.
#define NAND_FLASH_OFFSET 0x20000
#endif

// Can be used to disable printouts to reduce code size (does not make much of
// a difference, most of the AT91 lib uses IO so it iss difficult to remove it)
#define DEBUG_IO_LIB 1

// Specify whether the current binary is the debug binary or the mission
// binary.
#define COPY_MISSION_BINARY 1

// Binary size in bytes. The binary will propably be aligned to a fitting
// higher size and have the hamming code at its end.

// Adapt theses sizes as the binary grows.
#define DEBUG_BINARY_SIZE 0x100000 // 1.048.576 bytes
#define MISSION_BINARY_SIZE 0xA0000 // 655.360 bytes

#if COPY_MISSION_BINARY == 1
#define BINARY_SIZE DEBUG_BINARY_SIZE
#else
#define BINARY_SIZE MISSION_BINARY_SIZE
#endif

#endif /* BOOTLOADER_CONFIG_BOOTLOADERCONFIG_H_ */