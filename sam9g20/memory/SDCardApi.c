#include "SDCardApi.h"

#include <at91/utility/trace.h>
#include <hcc/api_fat.h>
#include <hcc/api_mdriver_atmel_mcipdc.h>
#include <hcc/api_hcc_mem.h>


int open_filesystem(VolumeId volume){
	/* Initialize the memory to be used by the filesystem */
	hcc_mem_init();

	/* Initialize the filesystem */
	int result = fs_init();
	if(result != F_NO_ERROR){
		TRACE_ERROR("open_filesystem: fs_init failed with "
				"code %d\n\r", result);
		return result;
	}

	/* Register this task with filesystem */
	result = f_enterFS();
	if(result != F_NO_ERROR){
		TRACE_ERROR("open_filesystem: fs_enterFS failed with "
				"code %d\n\r", result);
		return result;
	}

	result = select_sd_card(volume);
	if(result != F_NO_ERROR){
		TRACE_ERROR("open_filesystem: SD Card %d not present or "
				"defect.\n\r", volume);
		return result;
	}
	return result;
}

int close_filesystem(VolumeId volumeId) {
	f_delvolume(volumeId);
	f_releaseFS();
	int result = fs_delete();
	if(result != 0) {
		TRACE_ERROR("close_filesystem: fs_delete failed with "
				"code %d\n\r", result);
		return result;
	}

	return hcc_mem_delete();
}


int select_sd_card(VolumeId volumeId){
	/* Initialize volID as safe */
	int result = f_initvolume(0, atmel_mcipdc_initfunc, volumeId);

	if((result != F_NO_ERROR) && (result != F_ERR_NOTFORMATTED)) {
		TRACE_ERROR("select_sd_card: fs_initvolume failed with "
				"code %d\n\r", result);
		return result;
	}

	if(result == F_ERR_NOTFORMATTED) {
		/**
		 *  The file system has not been formatted to safeFat yet
		 *  Therefore format filesystem now
		 */
		result = f_format( 0, F_FAT32_MEDIA );
		if(result != F_NO_ERROR) {
			TRACE_ERROR("select_sd_card: fs_format failed with "
					"code %d\n\r", result);
			return result;
		}
	}
	return 0;
}

int change_directory(const char* repository_path) {
    int result;
    /* Because all paths are relative to the root directory,
    go to root directory here */
    result = f_chdir("/");
    if(result != F_NO_ERROR){
        TRACE_ERROR("change_directory: f_chdir failed with code %d\n\r",
                result);
        return result;
    }
    /* Change to the desired path */
    if(*repository_path != '\0'){
        result = f_chdir(repository_path);
        if(result != F_NO_ERROR){
            TRACE_ERROR("change_directory: f_chdir failed with code %d\n\r",
                            result);
        }
    }
    return result;
}

int read_whole_file(const char* repository_path, const char* file_name,
        uint8_t *buffer, const size_t max_buffer_size) {
    int result = change_directory(repository_path);
    if (result != F_NO_ERROR) {
        return 0;
    }

    F_FILE* file = f_open(file_name, "r");
    if (f_getlasterror() != F_NO_ERROR) {
        TRACE_ERROR("read_whole_file: f_open failed with code %d\n\r",
                f_getlasterror());
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
        TRACE_ERROR("read_whole_file: f_close failed with code %d\n\r",
                f_getlasterror());
    }
    return bytes_read;
}
