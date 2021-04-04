#ifndef BSP_SAM9G20_COMMON_FRAM_FRAMAPINOOS_H_
#define BSP_SAM9G20_COMMON_FRAM_FRAMAPINOOS_H_

/**
 * For documentation see FRAMApi.h with functions of similar name
 * @author  R. Mueller
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "CommonFRAM.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

int fram_no_os_read_critical_block(uint8_t* buffer, const size_t max_size);
int fram_no_os_read_bootloader_block(BootloaderGroup* bl_info);
int fram_no_os_read_bootloader_block_raw(uint8_t* buff, size_t max_size);

int fram_no_os_increment_img_reboot_counter(SlotType slotType, uint16_t* new_reboot_counter);
int fram_no_os_read_img_reboot_counter(SlotType slotType, uint16_t* reboot_counter);

int fram_no_os_read_binary_size(SlotType slotType, size_t *binary_size);

int fram_no_os_read_ham_size(SlotType slotType, size_t *ham_size);
int fram_no_os_read_ham_code(SlotType slotType, uint8_t* buffer, const size_t max_buffer,
        size_t current_offset, size_t size_to_read, size_t* size_read);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SAM9G20_COMMON_FRAM_FRAMAPINOOS_H_ */
