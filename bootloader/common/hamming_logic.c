#include <bootloaderConfig.h>

#include <at91/utility/hamming.h>
#include <at91/utility/trace.h>
#include <bsp_sam9g20/common/fram/CommonFRAM.h>
#include <bsp_sam9g20/common/SRAMApi.h>
#include <bsp_sam9g20/common/At91SpiDriver.h>
#include <bootloader/core/timer.h>

#ifdef ISIS_OBC_G20
#include <bsp_sam9g20/common/fram/FRAMApiNoOs.h>
#endif

#include <stddef.h>
#include <stdint.h>
#include <string.h>

uint8_t hamming_code_buf[IMAGES_HAMMING_RESERVED_SIZE];

int read_hamming_code(SlotType slotType, size_t ham_code_size);

/**
 * Handle the hamming code check
 * @return
 *  - 0 on successfull hamming code check
 *  - Hamming_ERROR_SINGLEBIT(1) if a single bit was corrected
 *  - Hamming_ERROR_ECC(2) on ECC error
 *  - Hamming_ERROR_MULTIPLEBITS(3) on multibit error.
 *  - -1 for FRAM issues or other issues where we should still jump to the same binary
 */
int handle_hamming_code_check(SlotType slotType, size_t image_size, size_t ham_code_size) {
    /* Now we can perform the hamming code check here. */
    if(slotType == BOOTLOADER_0) {
        /* Invalid slot type */
        return -1;
    }
    int result = 0;

    result = read_hamming_code(slotType, ham_code_size);
    if(result != 0) {
        return result;
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
    TRACE_INFO("Verifying %d bytes with %d hamming code bytes\n\r", size_to_check, ham_code_size);
#endif
    uint8_t hamming_result = Hamming_Verify256x((unsigned char*) SDRAM_DESTINATION,
            size_to_check, hamming_code_buf);
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
    return result;
}

int read_hamming_code(SlotType slotType, size_t ham_code_size) {
    int result = 0;
#ifdef ISIS_OBC_G20
    /* Assuming that the Hamming code was already read for the NO OS case */
#if USE_FREERTOS == 1
    size_t size_read = 0;
    result = fram_read_ham_code(slotType, hamming_code_buf,
            IMAGES_HAMMING_RESERVED_SIZE, 0, ham_code_size, &size_read);
#else

#if USE_FRAM_NON_INTERRUPT_DRV == 0
    /* Transfer is initiated earlier and should have finished by now */
    At91TransferStates state = wait_on_transfer(9999, NULL);
    if(state != SPI_SUCCESS) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("Hamming code read operation failed with code %d!\n\r", (int) state);
#endif
        return -1;
    }
    result = 0;
#else

    /* Measurement: Took around 38 ms. So it makes sense to split up the operation in two pieces
    if the watchdog is not kicked by the PIT/AIC */
#if BOOTLOADER_TIME_MEASUREMENT == 1
    uint32_t start_fram_read = get_ms_counter();
#endif
    result = fram_no_os_blocking_read_ham_code(slotType, hamming_code_buf,
            IMAGES_HAMMING_RESERVED_SIZE, 0, ham_code_size);
#if BOOTLOADER_TIME_MEASUREMENT == 1
    uint32_t end_fram_read = get_ms_counter();
    TRACE_INFO("FRAM hamming code read operation took %d ms\n\r",
            (int) (end_fram_read - start_fram_read));
#endif

#endif /* USE_FRAM_NON_INTERRUPT_DRV == 1 */

#endif /* USE_FREERTOS == 0 */

#elif defined(AT91SAM9G20_EK)

#endif

    if(result != 0) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("Could not read hamming code, error code %d!\n\r", result);
#endif
        return -1;
    }

#if BOOTLOADER_HAM_CODE_DEBUGGING == 1
    TRACE_INFO("Hamming code size: %d\n\r", ham_code_size);
    TRACE_INFO("First four bytes of hamming code %02x, %02x, %02x, %02x\n\r", hamming_code_buf[0],
            hamming_code_buf[1], hamming_code_buf[2], hamming_code_buf[3]);
    uint32_t last_idx = ham_code_size - 1;
    TRACE_INFO("Last four bytes of hamming code %02x, %02x, %02x, %02x\n\r",
            hamming_code_buf[last_idx - 3], hamming_code_buf[last_idx - 2],
            hamming_code_buf[last_idx - 1], hamming_code_buf[last_idx]);
#endif
    return 0;
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
