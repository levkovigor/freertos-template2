#include "boot_iobc.h"
#include <bootloaderConfig.h>
#include "../norflash/bl_iobc_norflash.h"
#include "../norflash/iobc_boot_sd.h"

#include <bootloader/utility/CRC.h>


#include <bsp_sam9g20/common/SRAMApi.h>
#include <bsp_sam9g20/common/lowlevel.h>

#if USE_FREERTOS == 1

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <bsp_sam9g20/common/fram/FRAMApi.h>
#include <bsp_sam9g20/common/fram/CommonFRAM.h>

#else

#include <bsp_sam9g20/common/fram/FRAMNoOs.h>
#include <bsp_sam9g20/common/fram/FRAMApiNoOs.h>
#include <bsp_sam9g20/common/At91SpiDriver.h>
#include <hal/Timing/WatchDogTimerNoOS.h>

#endif /* USE_FREERTOS == 0 */

#include <at91/utility/trace.h>
#include <at91/utility/hamming.h>
#include <at91/utility/exithandler.h>
#include <at91/peripherals/aic/aic.h>
#include <at91/peripherals/pit/pit.h>

#include <hal/Drivers/LED.h>
#include <hal/Timing/RTT.h>

#include <string.h>
#include <stdlib.h>

/* Forward declarations */
int perform_iobc_copy_operation_to_sdram();
int handle_hamming_code_check(SlotType slotType, size_t image_size, size_t ham_code_size);
int handle_hamming_code_result(int result);
int increment_sdc_loc_reboot_counter(BootSelect boot_select, uint16_t* curr_reboot_counter);
void go_to_jump_address(unsigned int jumpAddr, unsigned int matchType);
BootSelect determine_boot_select(bool* use_hamming);
bool local_hamming_flag_check(BootSelect boot_select);
void handle_problematic_norflash_copy_result();
void handle_problematic_sdc_copy_result(BootSelect boot_select);

#if USE_FREERTOS == 0
int increment_reboot_counter_no_os(SlotType slot_type, uint16_t* new_reboot_coumter);
#endif

/**
 * This is the core function of the bootloader which handles the copy operation,
 * performing ECC functionalities where applicable and jumping to the application.
 */
void perform_bootloader_core_operation() {
    int result = perform_iobc_copy_operation_to_sdram();
    if(result != 0) {
        /* This really should not happen. We still assume we can jump to SDRAM because there is
        not much we can do anyway */
    }

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO("Jumping to SDRAM application..\n\r");
#endif

#if USE_FREERTOS == 1
    vTaskEndScheduler();
#else

    /* Considered a configuration error, the last transfers increment the reboot counter
    and block until the transfer is completed (state should be IDLE then) */
    if(spi_transfer_state != IDLE) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("perform_bootloader_core_operation: SPI transfer "
                "might still be ongoing!\n\r");
#endif
    }

#endif /* USE_FREERTOS == 0 */

    disable_pit_aic();
    fram_stop_no_os();
    jump_to_sdram_application(0x22000000 - 1024, SDRAM_DESTINATION);
}

int perform_iobc_copy_operation_to_sdram() {
    /* The following bootloader sequence was designed by Jakob Meier and implemented by
    Robin Mueller. A graph can be found in the OBSW DDJF document.

    Determine which binary should be copied to SDRAM first.
    First, we check whether a software update needs to be loaded by checking a FRAM flag.
    The SW update volume (SD card 0 or 1) is specified in the FRAM as well.
    After that, we also check the local reboot counter of either the software update
    (SD card slot 1) or the primary flash image.

    If the reboot counter for the SW update is larger than 3, we go to the flash image.
    If the flash image reboot counter is larger than 3, we try the preferred SD card image.
    If the default SD card image reboot counter is larger than 3, we switch the SD card and
    try to boot the image from the other SD card. If the fault counter is larger than 3 here too
    we boot from NOR-Flash without ECC.

    Hamming code checks can be disabled individually for image types, which will lead to a
    boot of that image without a hamming code check.
    If a hamming code checks fails with ECC error or multibit errors, we increment the reboot
    counter in the FRAM and restart the OBC immediately.
     */
    BootSelect boot_select = BOOT_NOR_FLASH;
    int result = 0;
    bool use_hamming = false;

    /* If there are issues with the FRAM, we just boot from flash */
    if(!fram_faulty) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_INFO("Determining boot select..\n\r");
#endif
        boot_select = determine_boot_select(&use_hamming);
    }

    if(use_hamming) {
        use_hamming = local_hamming_flag_check(boot_select);
    }

    if(boot_select == BOOT_NOR_FLASH) {

#if BOOTLOADER_VERBOSE_LEVEL >= 1
        if(use_hamming) {
            TRACE_INFO("Booting from NOR-Flash with hamming code check\n\r");
        }
        else {
            TRACE_INFO("Booting from NOR-Flash without hamming code check\n\r");
        }
#endif
        result = copy_norflash_binary_to_sdram(PRIMARY_IMAGE_RESERVED_SIZE, use_hamming);

        if(result != 0) {
            handle_problematic_norflash_copy_result();
        }
        /* Increment local reboot counter */
#if USE_FREERTOS == 1
        result = fram_increment_img_reboot_counter(FLASH_SLOT, NULL);
#else
        result = increment_reboot_counter_no_os(FLASH_SLOT, NULL);
#endif
    }
    else {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        if(use_hamming) {
            TRACE_INFO("Booting from SD card with hamming code check\n\r");
        }
        else {
            TRACE_INFO("Booting from SD card without hamming code check\n\r");
        }
#endif
        result = copy_sdcard_binary_to_sdram(boot_select, use_hamming);

        if(result != 0) {
            handle_problematic_sdc_copy_result(boot_select);
        }
        result = increment_sdc_loc_reboot_counter(boot_select, NULL);
    }

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    if(result == 0) {
        TRACE_INFO("Copied successfully\n\r");
    }
    else {
        TRACE_INFO("Copied with issues..\n\r");
    }
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */

    return result;
}

void handle_problematic_norflash_copy_result() {
    uint16_t curr_reboot_counter = 0;
    int result = 0;
    /* Increment local reboot counter */
#if USE_FREERTOS == 1
    result = fram_increment_img_reboot_counter(FLASH_SLOT, &curr_reboot_counter);
#else
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
#endif
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

BootSelect determine_boot_select(bool* use_hamming) {
    BootSelect curr_boot_select = BOOT_NOR_FLASH;
    /* Wait for the last transfer to finish */
#if USE_FREERTOS == 0
    At91TransferStates current_transfer_state = wait_on_transfer(9999, NULL);
    if(current_transfer_state != SPI_SUCCESS) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("determine_boot_select: Reading BL block failed with code %d\n\r",
                (int) current_transfer_state);
#endif
        fram_faulty = true;
    }
#endif

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

/**
 * Handles the copy operation from NOR-Flash to SDRAM
 * @param copy_size
 * @return
 *  - 0 on success (jump to SDRAM)
 *  - -1 on failure (do not jump to SDRAM, increment reboot counter and restart)
 */
int copy_norflash_binary_to_sdram(size_t copy_size, bool use_hamming)
{
    int result = 0;
    // Initialize Nor
    //-------------------------------------------------------------------------
    // => the configuration was already done in LowLevelInit()
    // Transfert data from Nor to External RAM
    //-------------------------------------------------------------------------

    /* For the OSless case, we try to read the hamming code in parallel to the memcpy operation */
#if USE_FREERTOS == 0
    /* This should not happen, we blocked on completion previously */
    if(spi_transfer_state != IDLE) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("FRAM BL block might have failed\n\r");
#endif
        use_hamming = false;
    }

    size_t ham_size = bl_fram_block.nor_flash_hamming_code_size;
    if(ham_size == 0x00 || ham_size == 0xff) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("Hamming code size might be invalid\n\r");
#endif
        use_hamming = false;
    }

    if(use_hamming) {
        /* Start DMA transfer in background to be run in parallel to the memcpy operation */
        result = fram_no_os_read_ham_code(FLASH_SLOT, hamming_code_buf,
                sizeof(hamming_code_buf), 0, ham_size);
        if(result != 0) {
            use_hamming = false;
        }
    }
#endif /* USE_FREERTOS == 0 */

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO("Copying NOR-Flash binary to SDRAM..\n\r");
#endif

#if USE_FREERTOS == 1
    /* This operation takes 100-200 milliseconds if the whole NOR-Flash is
    copied. But the watchdog is running in a separate task with the highest priority
    and we are using a pre-emptive scheduler so this should not be an issue. */
    memcpy((void*) SDRAM_DESTINATION, (const void*) BINARY_BASE_ADDRESS_READ, copy_size);
#else
    /* Now we need to split up the copy operation to kick the watchdog. Watchdog window
    is 1ms to 50ms */
    {
        uint8_t bucket_num = 10;
        size_t bucket_size = copy_size / bucket_num;
        size_t bucket_rest = copy_size % bucket_num;
        size_t offset = 0;
        for(uint8_t idx = 0; idx < bucket_num; idx++) {
            offset = idx * bucket_size;
            memcpy((void*) SDRAM_DESTINATION + offset,
                    (const void*) BINARY_BASE_ADDRESS_READ + offset, bucket_size);
            WDT_forceKick();
        }
        offset = bucket_size * bucket_num;
        memcpy((void*) SDRAM_DESTINATION + offset,
                (const void*) BINARY_BASE_ADDRESS_READ + offset, bucket_rest);
    }
#endif /* USE_FREERTOS == 0 */

    /* Now we can perform the hamming code check on the image in the SDRAM */
    if(use_hamming) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_INFO("Performing hamming code ECC check..\n\r");
#endif
        size_t image_size = bl_fram_block.nor_flash_binary_size;
        size_t ham_code_size = bl_fram_block.nor_flash_hamming_code_size;
        if (image_size == 0 || image_size == 0xffffffff) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
            TRACE_WARNING("Flash image size %d invalid\n\r", image_size);
            /* Still jump to binary */
            return 0;
#endif
        }
        else if(ham_code_size == 0 || ham_code_size == 0xffffffff) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
            TRACE_WARNING("Flash hamming code size %d invalid\n\r", image_size);
            /* Still jump to binary */
            return 0;
#endif
        }
        else {
            int check_result = handle_hamming_code_check(FLASH_SLOT, image_size, ham_code_size);
            result = handle_hamming_code_result(check_result);
        }
    }
    return result;
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

void fram_callback(At91SpiBuses bus, At91TransferStates state, void* args) {
    volatile At91TransferStates* transfer_state = (At91TransferStates*) args;
    if(transfer_state != NULL) {
        *transfer_state = state;
    }
}

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

#if USE_FREERTOS == 0
int increment_reboot_counter_no_os(SlotType slot_type, uint16_t* new_reboot_counter) {
    uint16_t new_reboot_counter_loc = 0;
    At91TransferStates state;
    int result = fram_no_os_read_img_reboot_counter(slot_type, &new_reboot_counter_loc);
    if(result == 0) {
        state = wait_on_transfer(1000, NULL);
        if(state != SPI_SUCCESS) {
            result = (int) state;
        }
        else {
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
        }
    }
    return result;
}
#endif

/**
 * Used internally to jump to SDRAM.
 * @param jumpAddr
 * @param matchType
 */
void go_to_jump_address(unsigned int jumpAddr, unsigned int matchType)
{
    typedef void (*fctType) (volatile unsigned int, volatile unsigned int);
    void (*pFct) (volatile unsigned int r0_val, volatile unsigned int r1_val);

    pFct = (fctType) jumpAddr;
    pFct(0/*dummy value in r0*/, matchType/*matchType in r1*/);

    while(1);//never reach
}

void idle_loop() {
    uint32_t last_time = RTT_GetTime();
    LED_dark(led_2);
    for(;;) {
        uint32_t curr_time = RTT_GetTime();
        if(curr_time - last_time >= 1) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
            TRACE_INFO("Bootloader idle..\n\r");
#endif
            LED_toggle(led_2);
            last_time = curr_time;
        }
    }
}
