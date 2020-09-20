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
		TRACE_ERROR("SDCardHandler::openFilesystem: fs_init failed with "
				"code %d\n\r", result);
		return result;
	}

	/* Register this task with filesystem */
	result = f_enterFS();
	if(result != F_NO_ERROR){
		TRACE_ERROR("SDCardHandler::openFilesystem: fs_enterFS failed with "
				"code %d\n\r", result);
		return result;
	}

	result = select_sd_card(volume);
	if(result != F_NO_ERROR){
		TRACE_ERROR("SDCardHandler::openFilesystem: SD Card %d not present or "
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
		TRACE_ERROR("SDCardHandler::openFilesystem: fs_delete failed with "
				"code %d\n\r", result);
		return result;
	}

	return hcc_mem_delete();
}



int select_sd_card(VolumeId volumeId){
	/* Initialize volID as safe */
	int result = f_initvolume(0, atmel_mcipdc_initfunc, volumeId);

	if((result != F_NO_ERROR) && (result != F_ERR_NOTFORMATTED)) {
		TRACE_ERROR("SDCardHandler::openFilesystem: fs_initvolume failed with "
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
			TRACE_ERROR("SDCardHandler::openFilesystem: fs_format failed with "
					"code %d\n\r", result);
			return result;
		}
	}

	return 0;
}
