#include "iobc_boot_sd.h"
#include "bl_iobc_norflash.h"
#include "../common/boot_iobc.h"

#include <bootloaderConfig.h>
#include <fatfs_config.h>

#include <bsp_sam9g20/common/CommonFRAM.h>

#include <hcc/api_hcc_mem.h>
#include <at91/memories/sdmmc/MEDSdcard.h>
#include <at91/utility/trace.h>

#if USE_TINY_FS == 0
#include <bsp_sam9g20/common/SDCardApi.h>
#else
#include <tinyfatfs/tff.h>
#include <peripherals/pio/pio.h>

#define MAX_LUNS        1
Media medias[MAX_LUNS];
#endif /* USE_TINY_FS != 1 */

#include <string.h>

/**
 * Will be implemented in a common file because it is also used by NOR-Flash copy algorithm.
 * @param slotType
 * @return
 */
extern int handle_hamming_code_check(SlotType slotType, size_t image_size);
int handle_hamming_code_result(int result);

#if USE_TINY_FS == 0
int copy_with_hcc_lib(BootSelect boot_select, size_t* image_size);
#else
int copy_with_tinyfatfs_lib(BootSelect boot_select);
#endif


int copy_sdcard_binary_to_sdram(BootSelect boot_select, bool use_hamming) {
    if(boot_select == BOOT_NOR_FLASH) {
        return -1;
    }
#if USE_TINY_FS == 0
    size_t image_size = 0;
    int result = copy_with_hcc_lib(boot_select, &image_size);
    if(result != 0) {
        /* Copy operation failed, we can not jump */
        return result;
    }

    if(!use_hamming) {
        return result;
    }

    if(boot_select == BOOT_SD_CARD_0_SLOT_0) {
        result = handle_hamming_code_check(SDC_0_SL_0, image_size);
    }
    else if(boot_select == BOOT_SD_CARD_0_SLOT_1) {
        result = handle_hamming_code_check(SDC_0_SL_1, image_size);
    }
    else if(boot_select == BOOT_SD_CARD_1_SLOT_0) {
        result = handle_hamming_code_check(SDC_1_SL_0, image_size);
    }
    else if(boot_select == BOOT_SD_CARD_1_SLOT_1) {
        result = handle_hamming_code_check(SDC_1_SL_1, image_size);
    }

    return handle_hamming_code_result(result);
#else
    return copy_with_tinyfatfs_lib(boot_select);
#endif
}


#if USE_TINY_FS == 0
int copy_with_hcc_lib(BootSelect boot_select, size_t* image_size) {
    VolumeId current_volume = SD_CARD_0;
    if (boot_select == BOOT_SD_CARD_1_SLOT_1 || boot_select == BOOT_SD_CARD_1_SLOT_0) {
        current_volume = SD_CARD_1;
    }

    const char* slot_name = NULL;
    if(boot_select == BOOT_SD_CARD_0_SLOT_1 || boot_select == BOOT_SD_CARD_0_SLOT_1) {
        slot_name = SW_SLOT_1_NAME;
    }
    else {
        slot_name = SW_SLOT_0_NAME;
    }

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO("Setting up HCC filesystem\n\r");
#endif /* BOOTLOADER_VERBOSE_LEVEL >= 1 */

    int result = open_filesystem();
    if(result != F_NO_ERROR) {
        /* not good, should not happen. */
        hcc_mem_delete();
        return -1;
    }

    result = select_sd_card(current_volume, true);
    if(result != F_NO_ERROR) {
        /* not good, should not happen. */
        close_filesystem(true, true, current_volume);
        return -1;
    }

    result = change_directory(SW_REPOSITORY, true);
    if(result != F_NO_ERROR) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("Target SW repository \"%s\" does not exist\n\r", SW_REPOSITORY);
#endif
        /* not good, should not happen. */
        close_filesystem(true, true, current_volume);
        return -1;
    }

    F_FILE* file = f_open(slot_name, "r");
    result = f_getlasterror();
    if (result != F_NO_ERROR) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("f_open of file \"%s\" failed with code %d\n\r", slot_name, result);
#endif
        /* opening file failed! */
        close_filesystem(true, true, current_volume);
        return -1;
    }

    ssize_t filelength = f_filelength(slot_name);
    if(filelength <= 0) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_ERROR("Copying image \"%s\" from SD-Card %u: File does not exist \n\r",
                SW_SLOT_1_NAME, (unsigned int) current_volume);
#endif
        /* File does not exist or has an error */
        close_filesystem(true, true, current_volume);
        return -1;
    }
    if(image_size != NULL) {
        *image_size = filelength;
    }

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO("Copying image \"%s\" from SD card %u to SDRAM\n\r", slot_name,
            (unsigned int) current_volume);
#endif

    if(f_read((void*) SDRAM_DESTINATION, 1, filelength, file) != filelength) {
        /* Not all bytes copied! */
        f_close(file);
        close_filesystem(true, true, current_volume);
        return -1;
    }

    f_close(file);
    close_filesystem(true, true, current_volume);
    return 0;
}

#else

// Unfortunately, the tiny FATFS library is not working yet. There are issues reading
// files on the iOBC, possibly related to hardware related steps which are normally performed
// by the HCC and could not be reverse engineered from the information ISIS has given us.
int copy_with_tinyfatfs_lib(BootSelect boot_select) {
    FATFS fs;
    FIL fileObject;

    const int ID_DRV = DRV_MMC;
    FRESULT res = 0;
    Pin sd_select_pin[1] = {PIN_SDSEL};
    PIO_Configure(sd_select_pin, PIO_LISTSIZE(sd_select_pin));

    if (boot_select == BOOT_SD_CARD_1_UPDATE) {
        PIO_Set(sd_select_pin);
    }
    else {
        PIO_Clear(sd_select_pin);
    }

    Pin npWrPinsThatDoMagic[2] = {PIN_NPWR_SD0, PIN_NPWR_SD1};
    PIO_Configure(npWrPinsThatDoMagic, PIO_LISTSIZE(npWrPinsThatDoMagic));
    PIO_Clear(npWrPinsThatDoMagic);
    PIO_Clear(npWrPinsThatDoMagic + 1);

    Pin pinsMci1Off[2] = {PINS_MCI1_OFF};
    PIO_Configure(pinsMci1Off, PIO_LISTSIZE(pinsMci1Off));

    MEDSdcard_Initialize(&medias[ID_DRV], 0);
    memset(&fs, 0, sizeof(FATFS));  // Clear file system object
    res = f_mount(0, &fs);
    if( res != FR_OK ) {
        printf("f_mount pb: 0x%X\n\r", res);
        return 0;
    }

    char file_name [strlen(SW_REPOSITORY) + strlen(SW_UPDATE_FILE_NAME) + 2];
    snprintf(file_name, sizeof (file_name) + 1, "/%s%s", SW_REPOSITORY, SW_UPDATE_FILE_NAME);

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO("Copying image \"%s\" from SD-Card %u to SDRAM\n\r", file_name,
            (unsigned int) boot_select);
#endif

    res = f_open(&fileObject, file_name, FA_OPEN_EXISTING|FA_READ);
    if( res != FR_OK ) {
        TRACE_ERROR("f_open read pb: 0x%X\n\r", res);
        return 0;
    }
    size_t bytes_read;
    res = f_read(&fileObject, (void*)(SDRAM_DESTINATION), OBSW_MAX_SIZE, &bytes_read);
    if(res != FR_OK) {
        TRACE_ERROR("f_read pb: 0x%X\n\r", res);
        return 0;
    }

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO("Copied %lu bytes from to SDRAM successfully.\n\r", (unsigned long) bytes_read);
#endif

    res = f_close(&fileObject);
    if( res != FR_OK ) {
        TRACE_ERROR("f_close pb: 0x%X\n\r", res);
        return 0;
    }
    return 0;
}

#endif /* USE_TINY_FS != 1 */

