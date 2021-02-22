#include "at91_boot_from_sd.h"
#include <bootloaderConfig.h>

#include <at91/utility/trace.h>

#include <privlib/hcc/include/hcc/api_hcc_mem.h>
#include <sam9g20/common/SDCardApi.h>

int copy_sdc_image_to_sdram() {
    VolumeId current_volume = SD_CARD_0;
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
        TRACE_WARNING("Target SW repository \"%s\" does not exist.\n\r", SW_REPOSITORY);
#endif
        /* not good, should not happen. */
        close_filesystem(true, true, current_volume);
        return -1;
    }

    F_FILE* file = f_open(SW_SLOT_1_NAME, "r");
    result = f_getlasterror();
    if (result != F_NO_ERROR) {
#if BOOTLOADER_VERBOSE_LEVEL >= 1
        TRACE_WARNING("f_open of file \"%s\" failed with code %d.\n\r",
                SW_SLOT_1_NAME, result);
#endif
        /* opening file failed! */
        close_filesystem(true, true, current_volume);
        return -1;
    }

    size_t filelength = f_filelength(SW_SLOT_1_NAME);

#if BOOTLOADER_VERBOSE_LEVEL >= 1
    TRACE_INFO("Copying image \"%s\" from SD-Card %u to SDRAM\n\r", SW_SLOT_1_NAME,
            (unsigned int) current_volume);
#endif

    if(f_read((void*) SDRAM_DESTINATION, 1, filelength, file) != filelength) {
        /* Not all bytes copied! */
        f_close(file);
        close_filesystem(true, true, current_volume);
        return -1;
    }

    f_close(file);
    return close_filesystem(true, true, current_volume);
}
