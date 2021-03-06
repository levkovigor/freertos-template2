#include "boot_iobc.h"
#include <bootloaderConfig.h>
#include "../norflash/iobc_boot_sd.h"

#include <bootloader/utility/CRC.h>

#include <bsp_sam9g20/common/fram/FRAMApi.h>
#include <bsp_sam9g20/common/fram/CommonFRAM.h>
#include <bsp_sam9g20/common/SRAMApi.h>
#include <bsp_sam9g20/common/lowlevel.h>
#include <iobc/norflash/iobc_norflash.h>

#if USE_FREERTOS == 1
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#endif

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
int handle_hamming_code_check(SlotType slotType, size_t image_size);
int handle_hamming_code_result(int result);
int increment_sdc_loc_reboot_counter(BootSelect boot_select, uint16_t* curr_reboot_counter);
void go_to_jump_address(unsigned int jumpAddr, unsigned int matchType);
BootSelect determine_boot_select(bool* use_hamming);

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
#endif

    disable_pit_aic();
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

    if(boot_select == BOOT_NOR_FLASH) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        if(use_hamming) {
            TRACE_INFO("Booting from NOR-Flash, global hamming code checks enabled\n\r");
        }
        else {
            TRACE_INFO("Booting from NOR-Flash without hamming code check\n\r");
        }
#endif
        result = copy_norflash_binary_to_sdram(PRIMARY_IMAGE_RESERVED_SIZE, use_hamming);

        if(result != 0) {
            uint16_t curr_reboot_counter = 0;
            /* Increment local reboot counter */
            result = fram_increment_img_reboot_counter(FLASH_SLOT, &curr_reboot_counter);
#if BOOTLOADER_VERBOSE_LEVEL >= 1
            TRACE_WARNING("Copy operation or hamming code check on NOR-Flash image failed\n\r");
            TRACE_WARNING("Restarting, current reboot counter %d\n\r", curr_reboot_counter);
#endif
            /* Restart */
            restart();
        }
        /* Increment local reboot counter */
        result = fram_increment_img_reboot_counter(FLASH_SLOT, NULL);
    }
    else {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        if(use_hamming) {
            TRACE_INFO("Booting from SD card, global hamming code checks enabled\n\r");
        }
        else {
            TRACE_INFO("Booting from SD card without hamming code check\n\r");
        }
#endif
        result = copy_sdcard_binary_to_sdram(boot_select, use_hamming);

        if(result != 0) {
            uint16_t curr_reboot_counter = 0;
            result = increment_sdc_loc_reboot_counter(boot_select, &curr_reboot_counter);
#if BOOTLOADER_VERBOSE_LEVEL >= 1
            TRACE_WARNING("Copy operation or hamming code check on SD card image failed\n\r");
            TRACE_WARNING("Restarting, current reboot counter %d\n\r", curr_reboot_counter);
#endif
            /* Restart */
            restart();
        }
        increment_sdc_loc_reboot_counter(boot_select, NULL);
    }

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO("Copied successfully\n\r");
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */

    return result;
}

BootSelect determine_boot_select(bool* use_hamming) {
    BootloaderGroup bl_info_struct;
    BootSelect curr_boot_select = BOOT_NOR_FLASH;
    int result = fram_read_bootloader_block(&bl_info_struct);
    if (result != 0) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_ERROR("determine_boot_select: FRAM could not be read!\n\r");
#endif
        fram_faulty = true;
        *use_hamming = false;
        return BOOT_NOR_FLASH;
    }
    else {
        if(use_hamming != NULL) {
            if(bl_info_struct.global_hamming_flag == FRAM_TRUE) {
                *use_hamming = true;
            }
            else {
                *use_hamming = false;
            }
        }
    }

    if (bl_info_struct.software_update_available == FRAM_TRUE) {
        /* Slot 1 will be the update slot */
        if (bl_info_struct.software_update_in_volume_0 == FRAM_TRUE) {
            curr_boot_select = BOOT_SD_CARD_0_SLOT_1;
            if(bl_info_struct.sdc0_image_slot1_reboot_counter > 3) {
                /* Load flash image instead */
                curr_boot_select = BOOT_NOR_FLASH;
            }
            else {
                return curr_boot_select;
            }

        }
        else if(bl_info_struct.software_update_in_volume_1 == FRAM_TRUE) {
            curr_boot_select = BOOT_SD_CARD_1_SLOT_1;
            if(bl_info_struct.sdc1_image_slot1_reboot_counter > 3) {
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
    if(bl_info_struct.nor_flash_reboot_counter > 3) {
        if(bl_info_struct.preferred_sd_card == 0xff || bl_info_struct.preferred_sd_card == 1 ||
                bl_info_struct.preferred_sd_card == 0) {
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
        if(bl_info_struct.sdc0_image_slot0_reboot_counter > 3) {
            curr_boot_select = BOOT_SD_CARD_1_SLOT_0;
        }
        else {
            return curr_boot_select;
        }
    }
    else {
        if(bl_info_struct.sdc1_image_slot0_reboot_counter > 3) {
            curr_boot_select = BOOT_SD_CARD_0_SLOT_0;
        }
        else {
            return curr_boot_select;
        }
    }

    /* Check the other SD card. If this does not work, boot from NOR-Flash without ECC check */
    if(curr_boot_select == BOOT_SD_CARD_0_SLOT_0) {
        if(bl_info_struct.sdc0_image_slot0_reboot_counter > 3) {
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
        if(bl_info_struct.sdc1_image_slot0_reboot_counter > 3) {
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
    // Initialize Nor
    //-------------------------------------------------------------------------
    // => the configuration was already done in LowLevelInit()
    // Transfert data from Nor to External RAM
    //-------------------------------------------------------------------------

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO("Copying NOR-Flash binary to SDRAM..\n\r");
#endif

    /* This operation takes 100-200 milliseconds if the whole NOR-Flash is
    copied. But the watchdog is running in a separate task with the highest priority
    and we are using a pre-emptive scheduler so this should not be an issue. */
    memcpy((void*) SDRAM_DESTINATION, (const void*) BINARY_BASE_ADDRESS_READ, copy_size);

    int result = 0;
    /* Verify that the binary was copied properly. Ideally, we will also run a hamming
    code check here in the future.
    Check whether a hamming code check is necessary first. */
    if(use_hamming) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_INFO("Performing hamming code ECC check..\n\r");
#endif
        size_t image_size = 0;
        result = fram_read_binary_size(FLASH_SLOT, &image_size);
        if(result != 0) {
            /* Should really not happen, still jump to binary */
            return 0;
        }
        else if (image_size == 0 || image_size == 0xffffffff) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
            TRACE_WARNING("Flash image size %d invalid\n\r", image_size);
            /* Still jump to binary */
            return 0;
#endif
        }
        else {
            int check_result = handle_hamming_code_check(FLASH_SLOT, image_size);
            result = handle_hamming_code_result(check_result);
        }
    }
    return result;
}

int increment_sdc_loc_reboot_counter(BootSelect boot_select, uint16_t* curr_reboot_counter) {
    if(boot_select == BOOT_SD_CARD_0_SLOT_0) {
        return fram_increment_img_reboot_counter(SDC_0_SL_0, curr_reboot_counter);
    }
    else if(boot_select == BOOT_SD_CARD_0_SLOT_1) {
        return fram_increment_img_reboot_counter(SDC_0_SL_1, curr_reboot_counter);
    }
    else if(boot_select == BOOT_SD_CARD_1_SLOT_0) {
        return fram_increment_img_reboot_counter(SDC_1_SL_0, curr_reboot_counter);
    }
    else if(boot_select == BOOT_SD_CARD_1_SLOT_1) {
        return fram_increment_img_reboot_counter(SDC_1_SL_1, curr_reboot_counter);
    }
    return 0;
}


/**
 * Handle the hamming code check
 * @return
 *  - 0 on successfull hamming code check
 *  - Hamming_ERROR_SINGLEBIT(1) if a single bit was corrected
 *  - Hamming_ERROR_ECC(2) on ECC error
 *  - Hamming_ERROR_MULTIPLEBITS(3) on multibit error.
 *  - -1 for FRAM issues or other issues where we should still jump to the same binary
 */
int handle_hamming_code_check(SlotType slotType, size_t image_size) {
    /* Now we can perform the hamming code check here. */
    if(slotType == BOOTLOADER_0) {
        /* Invalid slot type */
        return -1;
    }

    size_t size_read = 0;
    size_t ham_size = 0;
    bool ham_flag = false;
    int result = fram_read_ham_size(slotType, &ham_size, &ham_flag);
    if(result != 0) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("Could not read hamming size and flag, error code %d!\n\r", result);
#endif
        return -1;
    }

    /* Hamming check disabled */
    if(!ham_flag) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_INFO("Hamming check disabled locally. Booting directly\n\r");
#endif
        return -1;
    }

    /* We will allocate enough memory in the SDRAM to store the hamming code, which is stored
    in the FRAM */
    uint8_t* hamming_code = malloc(IMAGES_HAMMING_RESERVED_SIZE);

    result = fram_read_ham_code(slotType, hamming_code,
            IMAGES_HAMMING_RESERVED_SIZE, 0, ham_size, &size_read);
    if(result != 0) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("Could not read hamming code, error code %d!\n\r", result);
#endif
        free(hamming_code);
        return -1;
    }

    /* The hamming code used here always checks blocks of 256 bytes but the image might
    not always have the fitting size to be a multiple of 256.
    This can be solved by padding the images with zeros for calculating the hamming code.
    It is assumed that the hamming code was generated in that way and the image itself
    was not padded. We need to pad the image with 0 in the SDRAM here so that the hamming
    code verification is valid */
    size_t fill_amount = 0;
    size_t mod_rest = image_size % 256;
    if(mod_rest != 0) {
        /* Determine the amount to fill with zeros where applicable */
        fill_amount = 256 - mod_rest;
    }

    if(fill_amount > 0) {
        memset((void*) SDRAM_DESTINATION + image_size, 0, fill_amount);
    }

    size_t size_to_check = image_size + fill_amount;
#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO("Verifying %d bytes with %d hamming code bytes\n\r", size_to_check, ham_size);
#endif
    uint8_t hamming_result = Hamming_Verify256x((unsigned char*) SDRAM_DESTINATION,
            size_to_check, hamming_code);
    if(hamming_result == Hamming_ERROR_SINGLEBIT) {
        result = Hamming_ERROR_SINGLEBIT;
    }
    else if(hamming_result == Hamming_ERROR_ECC) {
        result = Hamming_ERROR_ECC;
    }
    else if(hamming_result == Hamming_ERROR_MULTIPLEBITS) {
        result = Hamming_ERROR_MULTIPLEBITS;
    }
    else {
        result = 0;
    }
    free(hamming_code);
    return result;
}

int handle_hamming_code_result(int result) {
    if(result == 0) {
#if BOOTLOADER_VERBOSE_LEVEL >=1
        TRACE_INFO("ECC check: No errors detected\n\r");
#endif
        set_sram0_status_field(SRAM_OK);
        return result;
    }

    if(result == -1) {
        /* FRAM or other issues, we still jump to binary for now */
        set_sram0_status_field(SRAM_FRAM_ISSUES);
        return 0;
    }
    else if(result == Hamming_ERROR_SINGLEBIT) {
#if BOOTLOADER_VERBOSE_LEVEL >=1
        TRACE_INFO("ECC check: Single bit error corrected\n\r");
#endif
        /* We set a flag in SRAM to notify primary OBSW of bit flip */
        set_sram0_status_field(SRAM_HAMMING_ERROR_SINGLE_BIT);
        return 0;
    }
    else if(result == Hamming_ERROR_ECC) {
#if BOOTLOADER_VERBOSE_LEVEL >=1
        TRACE_INFO("ECC check: ECC error detected\n\r");
#endif
        /* We set a flag in SRAM to notify primary OBSW of bit flip in hamming code */
        set_sram0_status_field(SRAM_HAMMING_ERROR_ECC);
        return 0;
    }
    else if(result == Hamming_ERROR_MULTIPLEBITS) {
#if BOOTLOADER_VERBOSE_LEVEL >=1
        TRACE_INFO("ECC check: ECC multibit error detected\n\r");
#endif
        /* We set a flag in SRAM to notify primary OBSW of uncorrectable error
        in hamming code and try to load image from SD Card instead */
        set_sram0_status_field(SRAM_HAMMING_ERROR_MULTIBIT);
        return -1;
    }
    return 0;
}


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
