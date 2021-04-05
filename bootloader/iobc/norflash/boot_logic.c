#include "main_norflash.h"
#include <bootloaderConfig.h>

#include <bsp_sam9g20/common/fram/FRAMApiNoOs.h>
#include <at91/utility/trace.h>
#include <at91/utility/exithandler.h>


#include <stdint.h>

extern int increment_sdc_loc_reboot_counter(BootSelect boot_select, uint16_t* curr_reboot_counter);

BootSelect determine_boot_select(bool* use_hamming) {
    BootSelect curr_boot_select = BOOT_NOR_FLASH;
    /* Wait for the last transfer to finish */
#if USE_FREERTOS == 0 && USE_FRAM_NON_INTERRUPT_DRV == 0
    At91TransferStates current_transfer_state = wait_on_transfer(100000, NULL);
    if(current_transfer_state != SPI_SUCCESS) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("determine_boot_select: Reading BL block failed with code %d\n\r",
                (int) current_transfer_state);
#endif
        fram_faulty = true;
    }
#endif /* USE_FREERTOS == 0 && USE_FRAM_NON_INTERRUPT_DRV == 0 */

    if (fram_faulty) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_ERROR("determine_boot_select: FRAM error!\n\r");
#endif
        *use_hamming = false;
        return BOOT_NOR_FLASH;
    }

    if(use_hamming != NULL) {
        if(bl_fram_block.global_hamming_flag == FRAM_TRUE) {
            *use_hamming = true;
        }
        else {
            *use_hamming = false;
        }
    }

    if (bl_fram_block.software_update_available == FRAM_TRUE) {
        /* Slot 1 will be the update slot */
        if (bl_fram_block.software_update_in_volume_0 == FRAM_TRUE) {
            curr_boot_select = BOOT_SD_CARD_0_SLOT_1;
            if(bl_fram_block.sdc0_image_slot1_reboot_counter > 3) {
                /* Load flash image instead */
                curr_boot_select = BOOT_NOR_FLASH;
            }
            else {
                return curr_boot_select;
            }

        }
        else if(bl_fram_block.software_update_in_volume_1 == FRAM_TRUE) {
            curr_boot_select = BOOT_SD_CARD_1_SLOT_1;
            if(bl_fram_block.sdc1_image_slot1_reboot_counter > 3) {
                /* Load flash image instead */
                curr_boot_select = BOOT_NOR_FLASH;
            }
            else {
                return curr_boot_select;
            }
        }
    }

    /* If we reach this point, no SW update is to be loaded or the update boot counters are too high
    and we check the NOR-Flash image instead */
    if(bl_fram_block.nor_flash_reboot_counter > 3) {
        if(bl_fram_block.preferred_sd_card == 0xff || bl_fram_block.preferred_sd_card == 1 ||
                bl_fram_block.preferred_sd_card == 0) {
            curr_boot_select = BOOT_SD_CARD_0_SLOT_0;
        }
        else {
            curr_boot_select = BOOT_SD_CARD_1_SLOT_0;
        }
    }
    else {
        return curr_boot_select;
    }

    /* Check the preferred SD card */
    if(curr_boot_select == BOOT_SD_CARD_0_SLOT_0) {
        if(bl_fram_block.sdc0_image_slot0_reboot_counter > 3) {
            curr_boot_select = BOOT_SD_CARD_1_SLOT_0;
        }
        else {
            return curr_boot_select;
        }
    }
    else {
        if(bl_fram_block.sdc1_image_slot0_reboot_counter > 3) {
            curr_boot_select = BOOT_SD_CARD_0_SLOT_0;
        }
        else {
            return curr_boot_select;
        }
    }

    /* Check the other SD card. If this does not work, boot from NOR-Flash without ECC check */
    if(curr_boot_select == BOOT_SD_CARD_0_SLOT_0) {
        if(bl_fram_block.sdc0_image_slot0_reboot_counter > 3) {
            if(use_hamming != NULL) {
                *use_hamming = false;
            }
            curr_boot_select = BOOT_NOR_FLASH;
        }
        else {
            return curr_boot_select;
        }
    }
    else {
        if(bl_fram_block.sdc1_image_slot0_reboot_counter > 3) {
            if(use_hamming != NULL) {
                *use_hamming = false;
            }
            curr_boot_select = BOOT_NOR_FLASH;
        }
        else {
            return curr_boot_select;
        }
    }
    return curr_boot_select;
}

void handle_problematic_norflash_copy_result() {
    uint16_t curr_reboot_counter = 0;
    int result = 0;
    /* Increment local reboot counter */
#if USE_FREERTOS == 1
    result = fram_increment_img_reboot_counter(FLASH_SLOT, &curr_reboot_counter);
#else

#if USE_FRAM_NON_INTERRUPT_DRV == 0
    result = fram_no_os_read_img_reboot_counter(FLASH_SLOT, &curr_reboot_counter);
    if(result != 0) {
        At91TransferStates state = wait_on_transfer(500, NULL);
        if(state == SPI_SUCCESS) {
            curr_reboot_counter++;
            result = fram_no_os_increment_img_reboot_counter(FLASH_SLOT, curr_reboot_counter);
            state = wait_on_transfer(500, NULL);
            if(state != SPI_SUCCESS) {
                result = state;
            }
        }
        else {
            result = state;
        }
    }
#else

    result = increment_reboot_counter_no_os(FLASH_SLOT, &curr_reboot_counter);

#endif /* USE_FRAM_NON_INTERRUPT_DRV == 1 */

#endif /* USE_FREERTOS == 0 */

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_WARNING("Copy operation or hamming code check on NOR-Flash image failed\n\r");
    TRACE_WARNING("Restarting, current reboot counter %d\n\r", curr_reboot_counter);
    if(result != 0) {
        TRACE_WARNING("Issues setting new reboot counter, error code %d\n\r", result);
    }
#endif

    /* Restart */
    restart();
}

void handle_problematic_sdc_copy_result(BootSelect boot_select) {
    uint16_t curr_reboot_counter = 0;
    int result = increment_sdc_loc_reboot_counter(boot_select, &curr_reboot_counter);
#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_WARNING("Copy operation or hamming code check on SD card image failed\n\r");
    TRACE_WARNING("Restarting, current reboot counter %d\n\r", curr_reboot_counter);
    if(result != 0) {
        TRACE_WARNING("Issues setting new reboot counter, error code %d\n\r", result);
    }
#endif
    /* Restart */
    restart();
}
