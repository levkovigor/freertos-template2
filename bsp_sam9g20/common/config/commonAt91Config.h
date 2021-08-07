#ifndef SAM9G20_AT91_COMMON_COMMONAT91CONFIG_H_
#define SAM9G20_AT91_COMMON_COMMONAT91CONFIG_H_

#include "commonConfig.h"
#include <at91/memories/nandflash/NandCommon.h>

/**
 * The one-stage bootloader is loaded into the NAND-Flash at address 0x0 while the primary binary
 * is loaded at address 0x20000. It is loaded into the SRAM by the ROM-Boot and then executed
 * there.
 *
 *  - First-stage bootloader:   NAND-Flash      0x0
 *  - Primary image:            NAND-Flash      0x20000
 */
#define BOOTLOADER_ONE_STAGE        0
/**
 * The first-stage bootloader is identical to the bootloader for the one-stage bootloader.
 * The second-stage bootloader can be built by passing TYPE=2 into the build command.
 * It is loaded to the NAND-Flash at address 0x20000. It is loaded to the SDRAM
 * address 0x20100000 by the first-stage bootloader. The first 0x100000 bytes of the SDRAM
 * were reserved for the primary image, which is loaded by the second-stage bootloader.
 *
 *  - First-stage bootloader:   NAND-Flash      0x0
 *  - Second-stage bootloader:  NAND-Flash      0x20000
 *  - Primary image:            NAND-Flash      0x40000
 *
 */
#define BOOTLOADER_TWO_STAGE        1

#define BOOTLOADER_TYPE BOOTLOADER_TWO_STAGE

#ifdef __cplusplus
namespace config {
#endif

#if BOOTLOADER_TYPE == BOOTLOADER_TWO_STAGE
static const char* const BOOTLOADER_2_NAME =            "bl2.bin";
static const char* const BOOTLOADER_2_HAM =             "bl2_ham.bin";
#endif

#ifdef __cplusplus
}
#endif

#endif /* SAM9G20_AT91_COMMON_COMMONAT91CONFIG_H_ */
