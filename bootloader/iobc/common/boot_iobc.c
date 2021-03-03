#include "boot_iobc.h"
#include <bootloaderConfig.h>
#include "../norflash/bl_iobc_norflash.h"
#include "../norflash/iobc_boot_sd.h"

#include <sam9g20/common/FRAMApi.h>
#include <sam9g20/common/SRAMApi.h>
#include <sam9g20/common/CommonFRAM.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <utility/trace.h>
#include <utility/CRC.h>
#include <utility/hamming.h>
#include <hal/Drivers/LED.h>
#include <hal/Timing/RTT.h>

#include <string.h>
#include <stdlib.h>

int perform_iobc_copy_operation_to_sdram();
int handle_hamming_code_check(SlotType slotType);
void go_to_jump_address(unsigned int jumpAddr, unsigned int matchType);

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

    vTaskEndScheduler();
    jump_to_sdram_application(0x22000000 - 1024, SDRAM_DESTINATION);
}

int perform_iobc_copy_operation_to_sdram() {
    /* Determine which binary should be copied to SDRAM first. */
    BootSelect boot_select = BOOT_NOR_FLASH;
    bool load_sw_update = false;
    VolumeId volume = SD_CARD_0;
    int result = 0;
    if(!fram_faulty) {
        result = get_to_load_softwareupdate(&load_sw_update, &volume);
        if (result != 0) {
            TRACE_ERROR("FRAM could not be read!\n\r");
        }
    }

    if (load_sw_update) {
        if (volume == SD_CARD_0) {
            boot_select = BOOT_SD_CARD_0_UPDATE;
        }
        else {
            boot_select = BOOT_SD_CARD_1_UPDATE;
        }
    }

    if(boot_select == BOOT_NOR_FLASH) {
        result = copy_norflash_binary_to_sdram(PRIMARY_IMAGE_RESERVED_SIZE);

        if(result != 0) {
            result = copy_sdcard_binary_to_sdram(BOOT_SD_CARD_0_UPDATE);
            if(result != 0) {
                result = copy_sdcard_binary_to_sdram(BOOT_SD_CARD_1_UPDATE);
            }
        }
    }
    else {
        result = copy_sdcard_binary_to_sdram(boot_select);

        if(result != 0) {
            result = copy_norflash_binary_to_sdram(PRIMARY_IMAGE_RESERVED_SIZE);
        }
    }

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    if(result == 0) {
        TRACE_INFO("Copied successfully!\n\r");
    }
    else {
        TRACE_ERROR("Copy failed with code %d!\n\r", result);
    }
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */

    return result;
}

/**
 * Handles the copy operation from NOR-Flash to SDRAM
 * @param copy_size
 * @return
 *  - 0 on success (jump to SDRAM)
 *  - -1 on failure (do not jump to SDRAM and try to load image from SD Card instead)
 */
int copy_norflash_binary_to_sdram(size_t copy_size)
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

    /* Verify that the binary was copied properly. Ideally, we will also run a hamming
    code check here in the future.
    Check whether a hamming code check is necessary first. */
    bool hamming_flag = false;
    int result = fram_get_ham_check_flag(&hamming_flag);
    if(result != 0) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("Could not read FRAM for hamming flag, error code %d!\n\r", result);
#endif
        /* We still jump to the binary for now but we set a flag in SRAM to notify
        OBSW of FRAM issues */
        set_sram0_status_field(SRAM_FRAM_ISSUES);
        result = 0;
    }
    else if(hamming_flag) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_INFO("Performing hamming code ECC check..\n\r");
#endif
        result = handle_hamming_code_check(FLASH_SLOT);
        if(result != 0) {
            if(result == -1) {
                /* FRAM or other issues, we still jump to binary for now */
                set_sram0_status_field(SRAM_FRAM_ISSUES);
                return 0;
            }
            else if(result == Hamming_ERROR_SINGLEBIT) {
                /* We set a flag in SRAM to notify primary OBSW of bit flip */
                set_sram0_status_field(SRAM_HAMMING_ERROR_SINGLE_BIT);
                return 0;
            }
            else if(result == Hamming_ERROR_ECC) {
                /* We set a flag in SRAM to notify primary OBSW of bit flip in hamming code */
                set_sram0_status_field(SRAM_HAMMING_ERROR_ECC);
                return 0;
            }
            else if(result == Hamming_ERROR_MULTIPLEBITS) {
                /* We set a flag in SRAM to notify primary OBSW of uncorrectable error
                in hamming code and try to load image from SD Card instead */
                set_sram0_status_field(SRAM_HAMMING_ERROR_MULTIBIT);
                return -1;
            }
        }

    }
    return result;
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
int handle_hamming_code_check(SlotType slotType) {
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
        TRACE_WARNING("Could not read hamming size and  flag, error code %d!\n\r", result);
#endif
        return -1;
    }

    /* Hamming check disabled */
    if(!ham_flag) {
        return -1;
    }

    /* We will allocate enough memory in the SDRAM to store the hamming code, which is stored
    in the FRAM */
    uint8_t* hamming_code = malloc(IMAGES_HAMMING_RESERVED_SIZE);

    result = fram_read_ham_code(slotType, hamming_code,
            IMAGES_HAMMING_RESERVED_SIZE,0, ham_size, &size_read);
    if(result != 0) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("Could not read hamming code, error code %d!\n\r", result);
#endif
        free(hamming_code);
        return -1;
    }

    size_t image_size;
    result = fram_read_binary_size(FLASH_SLOT, &image_size);
    if(result != 0) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("Could not read image size, error code %d!\n\r", result);
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
