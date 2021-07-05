#include "VirtualFRAMApi.h"
#include "CommonFRAM.h"

#include <stdlib.h>
#include <hcc/api_fat.h>
#include <hcc/api_hcc_mem.h>


//bool handle_filesystem_opening = false;
//VolumeId volume_for_filesystem = SD_CARD_0;

/* Private functions */
int open_fram_file(F_FILE** file, size_t seek_pos, const char* const access_type);
int close_fram_file(F_FILE* file);

/* Implementation */

int FRAM_start() {
    return create_generic_fram_file();
}

int create_generic_fram_file() {
    int result = change_directory(VIRT_FRAM_PATH , true);
    if(result != 0) {
        return result;
    }

    F_FILE* file = NULL;
    // Check whether file exists
    file = f_open(VIRT_FRAM_NAME, "r");
    result = f_getlasterror();
    if(result != F_ERR_NOTFOUND) {
        // File already exists or is locked
        if(result == F_NO_ERROR) {
            // Check correct size as well and extend file if necessary
            size_t filesize = f_filelength(VIRT_FRAM_NAME);
            if(filesize < FRAM_END_ADDR + 1) {
                file = f_truncate(VIRT_FRAM_NAME, FRAM_END_ADDR + 1);
            }
            return close_fram_file(file);
        }
        return result;
    }


    file = f_open(VIRT_FRAM_NAME, "w");
    result = f_getlasterror();
    if(result != F_NO_ERROR) {
        // Could not create file
        return result;
    }

    result = f_close(file);
    if(result != F_NO_ERROR) {
        return result;
    }

    /* We do it like this so we don't have to allocate a buffer just to create an empty file
    with the correct length */
    file = f_truncate(VIRT_FRAM_NAME, FRAM_END_ADDR + 1);
    if(file == NULL) {
        // Could not truncate file should not happen!
        return -1;
    }

    return close_fram_file(file);
}


int fram_read_critical_block(uint8_t* buffer, const size_t max_size) {
    size_t size_to_read = sizeof(CriticalDataBlock);
    if(max_size < size_to_read) {
        return -3;
    }

    F_FILE* file = NULL;
    int result = open_fram_file(&file, CRITICAL_BLOCK_START_ADDR, "r");
    if(result != 0) {
        return result;
    }

    size_t readSize = f_read(buffer, 1, size_to_read, file);
    if(readSize != size_to_read) {
        close_fram_file(file);
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

int fram_set_ham_check_flag() {
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

int fram_set_to_load_softwareupdate(bool enable, VolumeId volume) {
    return 0;
}

int fram_write_software_version(uint8_t software_version, uint8_t software_subversion,
        uint8_t sw_subsubversion) {
    F_FILE* file = NULL;
    int result = open_fram_file(&file, FRAM_SOFTWARE_VERSION_ADDR, "r+");
    if(result != 0) {
        return result;
    }

    uint8_t write_buffer[3] = {software_version, software_subversion, sw_subsubversion};
    result = f_write((void *) write_buffer, sizeof(uint8_t), sizeof(write_buffer), file);
    /* Result is bytes written */
    if(result != sizeof(write_buffer)) {
        close_fram_file(file);
        return -1;
    }
    return close_fram_file(file);
}

int fram_read_software_version(uint8_t *software_version, uint8_t* software_subversion,
        uint8_t* sw_subsubversion) {
    if(!software_version || !software_subversion || !sw_subsubversion) {
        return -3;
    }

    F_FILE* file = NULL;
    int result = open_fram_file(&file, FRAM_SOFTWARE_VERSION_ADDR, "r");
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

int fram_zero_out_default_zero_fields() {
    return 0;
}

int fram_clear_ham_check_flag() {
    return 0;
}

int fram_get_ham_check_flag() {
    return 0;
}

int fram_set_img_ham_flag(SlotType slotType) {
    return 0;
}

int fram_clear_img_ham_flag(SlotType slotType) {
    return 0;
}

int fram_reset_img_reboot_counter(SlotType slotType) {
    return 0;
}

int fram_read_bootloader_block_raw(uint8_t* buff, size_t max_size) {
    return 0;
}

int fram_arm_deployment_timer(bool arm) {
    return 0;
}

int fram_is_deployment_timer_armed(bool *armed) {
    return 0;
}

int fram_get_seconds_on_deployment_timer(uint32_t* seconds) {
    return 0;
}

int fram_set_seconds_on_deployment_timer(uint32_t seconds) {
    return 0;
}

int fram_increment_seconds_on_deployment_timer(uint32_t incrementSeconds) {
    return 0;
}

int fram_increment_reboot_counter(uint32_t* new_reboot_counter) {
    return 0;
}

int fram_update_seconds_since_epoch(uint32_t secondsSinceEpoch) {
    F_FILE* file = NULL;
    int result = open_fram_file(&file, SEC_SINCE_EPOCH_ADDR, "r+");
    if(result != 0) {
        return result;
    }
    size_t sizeWritten = f_write(&secondsSinceEpoch, sizeof(uint8_t), sizeof(uint32_t), file);
    if(sizeWritten != sizeof(uint32_t)) {
        result = -1;
    }
    close_fram_file(file);
    return result;
}

int fram_read_seconds_since_epoch(uint32_t* secondsSinceEpoch) {
    if(secondsSinceEpoch == NULL) {
        return -3;
    }
    F_FILE* file = NULL;
    int result = open_fram_file(&file, SEC_SINCE_EPOCH_ADDR, "r");
    if(result != 0) {
        return result;
    }
    size_t sizeRead = f_read(secondsSinceEpoch, sizeof(uint8_t), sizeof(uint32_t), file);
    if(sizeRead != sizeof(uint32_t)) {
        result = -1;
    }
    close_fram_file(file);
    return result;
}
