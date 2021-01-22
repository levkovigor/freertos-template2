#include "VirtualFRAMApi.h"
#include "SDCardApi.h"
#include "CriticalDataBlock.h"

#include <stdlib.h>
#include <hcc/api_fat.h>
#include <hcc/api_hcc_mem.h>


//bool handle_filesystem_opening = false;
//VolumeId volume_for_filesystem = SD_CARD_0;

int open_fram_file(F_FILE** file, size_t seek_pos, const char* const access_type);
int close_fram_file(F_FILE* file);

int FRAM_start() {
    return create_generic_fram_file();
}

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
        if(result == F_NO_ERROR) {
            return close_fram_file(file);
        }
        return result;
    }


    file = f_open(VIRT_FRAM_NAME, "w");
    result = f_getlasterror();
    if(result != F_NO_ERROR) {
        /* Could not create file */
        return result;
    }

    result = f_close(file);
    if(result != F_NO_ERROR) {
        return result;
    }

    /* We do it like this so we don't have to allocate a buffer just to create an empty file
    with the correct length */
    file = f_truncate(VIRT_FRAM_NAME, sizeof(CriticalDataBlock));
    if(file == NULL) {
        /* Could not truncate file should not happen! */
        return -1;
    }

    return close_fram_file(file);
}

int delete_generic_fram_file() {
    int result = change_directory(VIRT_FRAM_PATH , true);
    if(result != 0) {
        return result;
    }

    return f_delete(VIRT_FRAM_NAME);
}

int set_hamming_check_flag() {
    F_FILE* file = NULL;
    int result = open_fram_file(&file, HAMMING_CHECK_FLAG_ADDR, "r+");
    if(result != 0) {
        return result;
    }

    uint8_t value = 1;
    result = f_write((void *) &value, 1, 1, file);
    /* Result is bytes written */
    if(result != 1) {
        return -1;
    }

    return close_fram_file(file);
}

int set_to_load_softwareupdate(bool enable, VolumeId volume) {
    return 0;
}

int write_software_version(uint8_t software_version, uint8_t software_subversion,
        uint8_t sw_subsubversion) {
    F_FILE* file = NULL;
    int result = open_fram_file(&file, SOFTWARE_VERSION_ADDR, "r+");
    if(result != 0) {
        return result;
    }

    uint8_t write_buffer[3] = {software_version, software_subversion, sw_subsubversion};
    result = f_write((void *) write_buffer, 1, sizeof(write_buffer), file);
    /* Result is bytes written */
    if(result != sizeof(write_buffer)) {
        close_fram_file(file);
        return -1;
    }
    return close_fram_file(file);
}

int read_software_version(uint8_t *software_version, uint8_t* software_subversion,
        uint8_t* sw_subsubversion) {
    if(!software_version || !software_subversion || !sw_subsubversion) {
        return -3;
    }

    F_FILE* file = NULL;
    int result = open_fram_file(&file, SOFTWARE_VERSION_ADDR, "r");
    if(result != 0) {
        return result;
    }

    uint8_t read_buffer[3];
    result = f_read(read_buffer, 1, sizeof(read_buffer), file);
    if(result != sizeof(read_buffer)) {
        close_fram_file(file);
        return -1;
    }

    *software_version = read_buffer[0];
    *software_subversion = read_buffer[1];
    *sw_subsubversion = read_buffer[2];
    return close_fram_file(file);
}

int open_fram_file(F_FILE** file, size_t seek_pos, const char* const access_type) {
    int result = 0;
    if(file == NULL) {
        return -1;
    }

    result = change_directory(VIRT_FRAM_PATH , true);
    if(result != 0) {
        return result;
    }
    /* Check whether file exists and open for reading and writing */
    *file = f_open(VIRT_FRAM_NAME, access_type);
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

int close_fram_file(F_FILE* file) {
    int result = change_directory("/" , true);
    if(result != 0) {
        return result;
    }

    return f_close(file);
}


