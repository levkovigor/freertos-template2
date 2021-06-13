#include "main_norflash.h"
#include <bootloaderConfig.h>

#include <at91/utility/trace.h>
#include <bsp_sam9g20/common/fram/FRAMApiNoOs.h>

#include <stdbool.h>

#if USE_FREERTOS == 0
int increment_reboot_counter_no_os(SlotType slot_type, uint16_t* new_reboot_counter);
#endif

bool local_hamming_flag_check(BootSelect boot_select) {
    bool use_hamming = false;
    if(boot_select == BOOT_NOR_FLASH) {
        if(bl_fram_block.nor_flash_hamming_flag == FRAM_TRUE) {
            use_hamming = true;
        }
    }
    else if(boot_select == BOOT_SD_CARD_0_SLOT_0) {
        if(bl_fram_block.sdc0_image_slot0_hamming_flag == FRAM_TRUE) {
            use_hamming = true;
        }
    }
    else if(boot_select == BOOT_SD_CARD_0_SLOT_1) {
        if(bl_fram_block.sdc0_image_slot1_hamming_flag == FRAM_TRUE) {
            use_hamming = true;
        }
    }
    else if(boot_select == BOOT_SD_CARD_1_SLOT_0) {
        if(bl_fram_block.sdc1_image_slot0_hamming_flag == FRAM_TRUE) {
            use_hamming = true;
        }
    }
    else if(boot_select == BOOT_SD_CARD_1_SLOT_1) {
        if(bl_fram_block.sdc1_image_slot1_hamming_flag == FRAM_TRUE) {
            use_hamming = true;
        }
    }
    return use_hamming;
}

int increment_sdc_loc_reboot_counter(BootSelect boot_select, uint16_t* curr_reboot_counter) {
    int result = 0;
    SlotType slot_type = SDC_0_SL_0;
    if(boot_select == BOOT_SD_CARD_0_SLOT_0) {
        slot_type = SDC_0_SL_0;
    }
    else if(boot_select == BOOT_SD_CARD_0_SLOT_1) {
        slot_type = SDC_0_SL_1;
    }
    else if(boot_select == BOOT_SD_CARD_1_SLOT_0) {
        slot_type = SDC_1_SL_0;
    }
    else if(boot_select == BOOT_SD_CARD_1_SLOT_1) {
        slot_type = SDC_1_SL_1;
    }

#if USE_FREERTOS == 1
    result = fram_increment_img_reboot_counter(slot_type, curr_reboot_counter);
#else
    result = increment_reboot_counter_no_os(slot_type, curr_reboot_counter);
#endif /* USE_FREERTOS == 0 */
    return result;
}

#if USE_FREERTOS == 0
int increment_reboot_counter_no_os(SlotType slot_type, uint16_t* new_reboot_counter) {
    uint16_t new_reboot_counter_loc = 0;
    int result = 0;

    if(slot_type == FLASH_SLOT) {
        new_reboot_counter_loc = bl_fram_block.nor_flash_reboot_counter;
    }
    else if(slot_type == SDC_0_SL_0) {
        new_reboot_counter_loc = bl_fram_block.sdc0_image_slot0_reboot_counter;
    }
    else if(slot_type == SDC_0_SL_1) {
        new_reboot_counter_loc = bl_fram_block.sdc0_image_slot1_reboot_counter;
    }
    else if(slot_type == SDC_1_SL_0) {
        new_reboot_counter_loc = bl_fram_block.sdc1_image_slot0_reboot_counter;
    }
    else if(slot_type == SDC_1_SL_1) {
        new_reboot_counter_loc = bl_fram_block.sdc1_image_slot1_reboot_counter;
    }

    if(new_reboot_counter_loc == 0xff) {
        new_reboot_counter_loc = 0;
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("Reboot counter found might be unset. Setting to zero\n\r");
#endif
    }
#if USE_FRAM_NON_INTERRUPT_DRV == 0
    At91TransferStates state;
    new_reboot_counter_loc++;
    result = fram_no_os_increment_img_reboot_counter(slot_type, new_reboot_counter_loc);
    if(result != 0) {
        state = wait_on_transfer(1000, NULL);
        if(state != SPI_SUCCESS) {
            result = (int) state;
        }
        else {
            if(new_reboot_counter != NULL) {
                *new_reboot_counter = new_reboot_counter_loc;
            }
        }
    }

    return result;
#else

    new_reboot_counter_loc++;
    result = fram_no_os_blocking_increment_img_reboot_counter(slot_type,
            new_reboot_counter_loc);
    if(new_reboot_counter != NULL) {
        *new_reboot_counter = new_reboot_counter_loc;
    }

    return result;

#endif /* USE_FRAM_NON_INTERRUPT_DRV == 1 */
}
#endif

void fram_callback(At91SpiBuses bus, At91TransferStates state, void* args) {
    volatile At91TransferStates* transfer_state = (At91TransferStates*) args;
    if(transfer_state != NULL) {
        *transfer_state = state;
    }
}
