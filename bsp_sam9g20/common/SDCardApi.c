#include <at91/utility/trace.h>
#include <hcc/api_fat.h>
#include <hcc/api_mdriver_atmel_mcipdc.h>
#include <hcc/api_hcc_mem.h>
#include <bsp_sam9g20/common/SDCardApi.h>

#include <string.h>

int delete_file_system_object(const char* name);

int open_filesystem(){
	/* Initialize the memory to be used by the filesystem */
	hcc_mem_init();

	/* Initialize the filesystem */
	int result = fs_init();
	if(result != F_NO_ERROR){
		TRACE_ERROR("open_filesystem: fs_init failed with code %d\n\r", result);
		return result;
	}

	return result;
}

int select_sd_card(VolumeId volumeId, bool enterFs) {
    int result = 0;
    if(enterFs) {
        result = f_enterFS();
        if(result != F_NO_ERROR) {
            TRACE_ERROR("open_filesystem: fs_enterFS failed with code %d\n\r", result);
            return result;
        }
    }
    /* Initialize volID as safe */
    result = f_initvolume(0, atmel_mcipdc_initfunc, volumeId);

    if((result != F_NO_ERROR) && (result != F_ERR_NOTFORMATTED)) {
        TRACE_ERROR("select_sd_card: fs_initvolume failed with code %d\n\r", result);
        return result;
    }

    if(result == F_ERR_NOTFORMATTED) {
        TRACE_INFO("select_sd_card: Formatting SD-Card %d for Safe-FAT\r\n", volumeId);
        /**
         *  The file system has not been formatted to safeFat yet
         *  Therefore format filesystem now
         */
        result = f_format( 0, F_FAT32_MEDIA);
        if(result != F_NO_ERROR) {
            TRACE_ERROR("select_sd_card: fs_format failed with code %d\n\r", result);
            return result;
        }
    }
    return 0;
}

int close_filesystem(bool releaseFs, bool delVolume, VolumeId volumeId) {
    if(delVolume) {
        f_delvolume(volumeId);
    }

    if(releaseFs) {
        f_releaseFS();
    }

	int result = fs_delete();
	if(result != F_NO_ERROR) {
		TRACE_ERROR("close_filesystem: fs_delete failed with code %d\n\r", result);
		return result;
	}

	return hcc_mem_delete();
}


int switch_sd_card(VolumeId volumeId) {
	VolumeId oldVolumeId;
	if(volumeId == SD_CARD_0) {
		 oldVolumeId = SD_CARD_1;
	}
	else {
		oldVolumeId = SD_CARD_0;
	}
	int result = f_delvolume(oldVolumeId);
	if(result != 0) {
		TRACE_WARNING("switch_sd_card: f_delvolume failed with code %d\n\r", result);
	}

	// TODO: test whether we have to release the FS to switch the SD card.
	return select_sd_card(volumeId, false);
}

int create_directory(const char *repository_path, const char *dirname) {
    int result = 0;
    if(repository_path != NULL) {
        result = change_directory(repository_path, true);
        if(result != F_NO_ERROR){
            return result;
        }
    }

    result = f_mkdir(dirname);
    if(result == F_ERR_DUPLICATED) {
        // folder already exists, no diagnostic output here for now.
    }
    else if(result != F_NO_ERROR) {
        TRACE_ERROR("create_directory: f_mkdir failed with code %d!\n\r", result);
    }

    if(repository_path != NULL) {
        f_chdir("/");
    }
    return result;
}

int delete_directory(const char* repository_path, const char* dirname) {
    int result = 0;
    if(repository_path != NULL) {
        result = change_directory(repository_path, true);
        if(result != F_NO_ERROR){
            return result;
        }
    }

    result = f_rmdir(dirname);
    if(result == F_ERR_NOTEMPTY) {
        // folder not empty, no diagnostic output here for now.
    }
    else if(result != F_NO_ERROR) {
        TRACE_ERROR("delete_directory: f_rmdir failed with code %d!\n\r", result);
    }

    if(repository_path != NULL) {
        f_chdir("/");
    }
    return result;
}

int change_directory(const char* repository_path, bool from_root) {
    if(from_root) {
       f_chdir("/");
    }

    /* Change to the desired path */
    int result = f_chdir(repository_path);
    if(result == F_ERR_NOTFOUND){
        // directory does not exist, no diagnostic output here for now.
    }
    else if(result != F_NO_ERROR) {
        TRACE_ERROR("change_directory: f_chdir failed with code %d!\n\r", result);
    }
    return result;
}

int read_whole_file(const char* repository_path, const char* file_name,
        uint8_t *buffer, const size_t max_buffer_size) {
    int result = 0;
    if(repository_path != NULL) {
        result = change_directory(repository_path, true);
        if (result != F_NO_ERROR) {
            return 0;
        }
    }


    F_FILE* file = f_open(file_name, "r");
    if (f_getlasterror() != F_NO_ERROR) {
        TRACE_ERROR("read_whole_file: f_open failed with code %d\n\r", f_getlasterror());
        return 0;
    }

    long filesize = f_filelength(file_name);
    if(filesize == -1) {
        return 0;
    }

    uint8_t filecontent[filesize];
    size_t bytes_read = 0;
    if((size_t) filesize > max_buffer_size) {
        bytes_read = f_read(filecontent, sizeof(uint8_t),
                max_buffer_size, file);
    }
    else {
        bytes_read = f_read(filecontent, sizeof(uint8_t),
                filesize, file);
    }

    result = f_close(file);
    if (result != F_NO_ERROR) {
        TRACE_ERROR("read_whole_file: f_close failed with code %d\n\r",  f_getlasterror());
    }
    return bytes_read;
}

int create_file(const char* repository_path, const char* filename,
        const uint8_t* initial_data, size_t initial_data_size) {
    int result = 0;
    if(repository_path != NULL) {
        result = change_directory(repository_path, true);
        if(result != F_NO_ERROR){
            return -2;
        }
    }

    F_FIND exists;
    result = f_findfirst(filename, &exists);
    if(result == F_NO_ERROR) {
        // file already exists..
        return -1;
    }
    F_FILE* file = f_open(filename, "w");

    result = f_getlasterror();
    if(result != F_NO_ERROR) {
        return result;
    }

    size_t bytes_written = 0;
    if((file != NULL) && (initial_data != NULL)) {
        bytes_written = f_write(initial_data, sizeof(uint8_t),
                initial_data_size, file);
        if(bytes_written != initial_data_size) {
            TRACE_DEBUG("create_file: f_write did not write all bytes!\n\r");
        }
    }

    result = f_close(file);
    if (result != F_NO_ERROR) {
        TRACE_ERROR("create_file: f_close failed with code %d\n\r", result);
    }

    if(repository_path != NULL) {
        f_chdir("/");
    }

    return bytes_written;
}

int delete_file(const char* repository_path,
        const char* filename) {
    int result = 0;
    if(repository_path != NULL) {
        result = change_directory(repository_path, true);
        if(result != F_NO_ERROR){
            return result;
        }
    }

    return f_delete(filename);
}

int lock_file(const char* repository_path, const char* filename) {
	int result = 0;
    if(repository_path != NULL) {
        result = change_directory(repository_path, true);
        if(result != F_NO_ERROR) {
            return result;
        }
    }

    uint8_t current_attribute;
	result = f_getattr(filename, &current_attribute);
	if(result != F_NO_ERROR) {
		return result;
	}

	current_attribute |= F_ATTR_READONLY;

	return f_setattr(filename, current_attribute);
}


int unlock_file(const char* repository_path, const char* filename) {
	int result = 0;
    if(repository_path != NULL) {
        result = change_directory(repository_path, true);
        if(result != F_NO_ERROR) {
            return result;
        }
    }

    uint8_t current_attribute;
	result = f_getattr(filename, &current_attribute);
	if(result != F_NO_ERROR) {
		return result;
	}

	current_attribute &= ~(F_ATTR_READONLY);
	return f_setattr(filename, current_attribute);
}


int get_file_info(const char* repository_path, const char* filename,
		size_t* filesize, bool *locked, uint16_t* cdate,
		uint16_t* ctime) {
	int result = 0;
    if(repository_path != NULL) {
        result = change_directory(repository_path, true);
        if(result != F_NO_ERROR) {
            return result;
        }
    }

    f_open(filename, "r");

    result = f_getlasterror();
    if(result != F_NO_ERROR) {
        return result;
    }

    *filesize = f_filelength(filename);

    if(locked != NULL) {
    	uint8_t attribute;
    	result = f_getattr(filename, &attribute);
    	if(result != F_NO_ERROR) {
    		return result;
    	}

    	if((attribute & F_ATTR_READONLY) == F_ATTR_READONLY) {
    		*locked = true;
    	}
    	else {
    		*locked = false;
    	}
    }

    if((cdate != NULL) && (ctime != NULL)) {
    	result = f_gettimedate(filename, cdate, ctime);
    }
    return result;
}


int delete_directory_force(const char *repository_path, const char *dirname,
        bool delete_subfolder_recursively) {
    int status = F_NO_ERROR;
    // attempt normal delete first.
    int result = delete_directory(repository_path, dirname);
    if(result == F_NO_ERROR) {
        return result;
    }

    if(repository_path != NULL) {
        result = change_directory(repository_path, true);
        if(result != F_NO_ERROR) {
            return result;
        }
    }

    // normal delete failed, we need to delete all files in folder.
    F_FIND find_result;
    result = change_directory(dirname, false);
    if(result != F_NO_ERROR) {
        return result;
    }

    int file_found = f_findfirst("*.*", &find_result);
    if(file_found != F_NO_ERROR) {
        return file_found;
    }

    if(find_result.filename[0] == '.') {
        // we are not in root, so the next search result is going
        // to be the parent folder, and the third result is going to be
        // the first file or directory.
        f_findnext(&find_result);
        file_found = f_findnext(&find_result);
    }

    for(int idx = 0; idx < 255; idx++) {
        if(idx > 0) {
            file_found = f_findnext(&find_result);
        }

        if(file_found != F_NO_ERROR) {
            break;
        }

        result = delete_file_system_object(find_result.filename);
        if(result != F_NO_ERROR) {
            TRACE_ERROR("clear_sd_card: delete_file_system_object failed with "
                    "code %d!\n\r", result);
            status = result;
        }
    }

    // folder should be empty now
    change_directory("..", false);
    result = delete_directory(NULL, dirname);
    if(result != F_NO_ERROR) {
        status = result;
    }

    if(repository_path != NULL) {
        f_chdir("/");
    }
    return status;
}

int clear_sd_card() {
    F_FIND find_result;
    int file_found = 0;
    int result = F_NO_ERROR;
    int status = F_NO_ERROR;
    f_chdir("/");
    file_found = f_findfirst("*.*", &find_result);
    if(file_found != F_NO_ERROR) {
        return F_NO_ERROR;
    }

    // Skip SYSTEM folder.
    if(strncmp(find_result.filename, "SYSTEM", 6) == 0) {
        file_found = f_findnext(&find_result);
    }

    for(int idx = 0; idx < 255; idx++) {
        if(idx > 0) {
            file_found = f_findnext(&find_result);
        }

        // Skip SYSTEM folder.
        if(strncmp(find_result.filename, "SYSTEM", 6) == 0) {
            continue;
        }

        if(file_found != F_NO_ERROR) {
            break;
        }

        result = delete_file_system_object(find_result.filename);
        if(result != F_NO_ERROR) {
        	if(result == F_ERR_LOCKED) {
        		TRACE_WARNING("clear_sd_card: %s is protected!\n\r",
        				find_result.filename);
        	}
        	else {
                TRACE_ERROR("clear_sd_card: delete_file_system_object failed "
                		"with code %d!\n\r", result);
        	}

            status = result;
        }
    }
    return status;
}

int delete_file_system_object(const char* name) {
    int result = F_NO_ERROR;
    // Check whether it is a directory or a file
    if(change_directory(name, false) == F_NO_ERROR) {
        change_directory("..", false);
        result = delete_directory_force(NULL, name, true);
    }
    else {
        result = delete_file(NULL, name);
    }
    return result;
}

