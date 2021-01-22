#include "VirtualFRAMApi.h"
#include "SDCardApi.h"
#include "CriticalDataBlock.h"

#include <stdlib.h>
#include <hcc/api_fat.h>

int open_fram_file(F_FILE** file, size_t seek_pos);

int create_generic_fram_file() {
    int result = change_directory(VIRT_FRAM_PATH , true);
    if(result != 0) {
        return result;
    }

    F_FILE* file = NULL;
    /* Check whether file exists */
    file = f_open(VIRT_FRAM_NAME, "r");
    result = f_getlasterror();
    if(result != F_ERR_NOTFOUND) {
        /* File already exists or is locked */
        return result;
    }

    file = f_open(VIRT_FRAM_NAME, "w");
    result = f_getlasterror();
    if(result != F_NO_ERROR) {
        /* Could not create file */
        return result;
    }

    file = f_truncate(VIRT_FRAM_NAME, sizeof(CriticalDataBlock));
    if(file == NULL) {
        /* Could not truncate file should not happen! */
        return -1;
    }
    return 0;
}

int set_hamming_check_flag() {
    F_FILE* file = NULL;
    int result = open_fram_file(&file, HAMMING_CHECK_FLAG_ADDR);
    if(result != 0) {
        return result;
    }

    uint8_t valueToWrite = 1;
    result = f_write((void *) &valueToWrite, 1, 1, file);
    /* Result is bytes written */
    if(result != 1) {
        return -1;
    }
    return 0;
}

int open_fram_file(F_FILE** file, size_t seek_pos) {
    if(file == NULL) {
        return -1;
    }

    int result = change_directory(VIRT_FRAM_PATH , true);
    if(result != 0) {
        return result;
    }
    /* Check whether file exists and open for reading and writing */
    *file = f_open(VIRT_FRAM_NAME, "r+");
    result = f_getlasterror();
    if(result != F_NO_ERROR || *file == NULL) {
        /* File not found, locked or file pointer invalid */
        return result;
    }

    if(seek_pos > 0) {
        result = f_seek(*file, seek_pos, F_SEEK_SET);
        if(result != F_NO_ERROR) {
            return result;
        }
    }

    return 0;
}

int set_to_load_softwareupdate(bool enable, VolumeId volume) {
    return 0;
}


