#ifndef BSP_SAM9G20_COMMON_FRAM_FRAMAPINOOS_H_
#define BSP_SAM9G20_COMMON_FRAM_FRAMAPINOOS_H_

/**
 * For documentation see FRAMApi.h with functions of similar name
 * @author  R. Mueller
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

int fram_no_os_read_critical_block(uint8_t* buffer, const size_t max_size);
int fram_no_os_read_bootloader_block(BootloaderGroup* bl_info);
int fram_no_os_read_bootloader_block_raw(uint8_t* buff, size_t max_size);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SAM9G20_COMMON_FRAM_FRAMAPINOOS_H_ */
