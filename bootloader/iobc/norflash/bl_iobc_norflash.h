#ifndef BOOTLOADER_IOBC_NORFLASH_BL_IOBC_NORFLASH_H_
#define BOOTLOADER_IOBC_NORFLASH_BL_IOBC_NORFLASH_H_

#include <bsp_sam9g20/common/At91SpiDriver.h>
#include <bsp_sam9g20/common/fram/CommonFRAM.h>
#include <stdbool.h>

extern volatile At91TransferStates spi_transfer_state;
extern BootloaderGroup bl_fram_block;
extern bool fram_faulty;

/**
 * Wait on transfers. Resets the global state flag for the next transfer internally.
 * Block cycles can be specified to avoid deadlock.
 * @return Transfer state
 */
extern At91TransferStates wait_on_transfer(uint32_t block_cycles);

#endif /* BOOTLOADER_IOBC_NORFLASH_BL_IOBC_NORFLASH_H_ */
