#ifndef SAM9G20_COMMON_FRAMNOOS_H_
#define SAM9G20_COMMON_FRAMNOOS_H_

#include "../At91SpiDriver.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This module allows use of the CY15B104QI FRAM device on the iOBC without FreeRTOS. No block
 * protection implemented for now.
 * @return -1 for invalid input, -2 is configuration of the status register failed.
 */

int fram_start_no_os(at91_user_callback_t callback, void* callback_args);

int fram_read_no_os(uint8_t* rec_buf, uint32_t address, size_t len);
int fram_write_no_os(uint8_t* send_buf, uint32_t address, size_t len);

void fram_assign_callback(at91_user_callback_t callback, void* args);

/**
 * This will deinitialize the SPI driver. Be careful if SPI bus is used by other devices!
 * @return
 */
int fram_stop_no_os();

#ifdef __cplusplus
}
#endif

#endif /* SAM9G20_COMMON_FRAMNOOS_H_ */
