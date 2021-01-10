#include "iobc_boot_sd.h"
#include <bootloaderConfig.h>
#include <fatfs_config.h>
#include <memories/MEDSdcard.h>
#include <string.h>
#include <utility/trace.h>
#include <iobc/norflash/iobc_norflash.h>

#if USE_TINY_FS == 0
#include <sam9g20/common/SDCardApi.h>
#else
#include <tinyfatfs/tff.h>
#endif

#define MAX_LUNS        1
Media medias[MAX_LUNS];

#if USE_TINY_FS == 0
int copy_with_hcc_lib(BootSelect boot_select);
#else
int copy_with_tinyfatfs_lib(BootSelect boot_select);
#endif

int copy_sdcard_binary_to_sdram(BootSelect boot_select) {
#if USE_TINY_FS == 0
	return copy_with_hcc_lib(boot_select);
#else
	return copy_with_tinyfatfs_lib(boot_select);
#endif
}


#if USE_TINY_FS == 0
int copy_with_hcc_lib(BootSelect boot_select) {
	VolumeId current_filesystem = SD_CARD_0;
	if (boot_select == BOOT_SD_CARD_1_UPDATE) {
		current_filesystem = SD_CARD_1;
	}
	int result = open_filesystem(current_filesystem);
	if(result != 0) {
		// should not happen..
		result = open_filesystem(SD_CARD_1);
		if(result != 0) {
			// not good at all. boot from NOR-Flash instead
			return -1;
		}
	}

	switch(boot_select) {
	case(BOOT_SD_CARD_0_UPDATE): {
		// get repostiory
		char bin_folder_name[16];
		// hardcoded
		//int result = read_sdc_bin_folder_name(bin_folder_name);
		if(result != 0) {
			// not good
		}
		result = change_directory(bin_folder_name, true);
		if(result != F_NO_ERROR) {
			// not good
		}
		char binary_name[16];
		char hamming_name[16];
		// hardcoded
		//result = read_sdc1sl1_bin_names(binary_name, hamming_name);
		if(result != 0) {

		}
		// read in buckets.. multiple of 256. maybe 10 * 256 bytes?
		F_FILE* file = f_open(binary_name, "r");
		if (f_getlasterror() != F_NO_ERROR) {
			// opening file failed!
			return -1;
		}
		// get total file size.
		long filesize = f_filelength(binary_name);
		size_t current_idx = 0;
		size_t read_bucket_size = 10 * 256;
		while(true) {
			// we are close to the end of the file and don't have to read
			// the full bucket size
			if(filesize - current_idx < read_bucket_size) {
				read_bucket_size = filesize - current_idx;
			}
			// copy to SDRAM directly
			size_t bytes_read = f_read(
					(void *) (SDRAM_DESTINATION + current_idx),
					sizeof(uint8_t),
					read_bucket_size,
					file);
			if(bytes_read == -1) {
				// this should definitely not happen
				// we need to ensure we will not be locked in a permanent loop.
			}
			else if(bytes_read < 10 * 256) {
				// should not happen!
			}

			// now we could perform the hamming check on the RAM code directly
			// If the last bucket is smaller than 256, we pad with 0
			// and assume the hamming code calculation was performed with
			// padding too.
			current_idx += bytes_read;
			if(current_idx >= filesize) {
				break;
			}

		}
		break;
	}

	case(BOOT_SD_CARD_1_UPDATE): {
		break;
	}
	case(BOOT_NOR_FLASH): {
		return -1;
	}
	}
	close_filesystem(current_filesystem);
}
#else

int copy_with_tinyfatfs_lib(BootSelect boot_select) {
	FATFS fs;
	FIL fileObject;

	const int ID_DRV = DRV_MMC;
	FRESULT res = 0;
	MEDSdcard_Initialize(&medias[ID_DRV], 0);
	memset(&fs, 0, sizeof(FATFS));  // Clear file system object
	res = f_mount(0, &fs);
	if( res != FR_OK ) {
		printf("f_mount pb: 0x%X\n\r", res);
		return 0;
	}

	char file_name [strlen(SW_REPOSITORY) + strlen(SW_UPDATE_FILE_NAME) + 1];
	snprintf(file_name, sizeof (file_name), "%s%s", SW_REPOSITORY,
		SW_UPDATE_FILE_NAME);

	TRACE_INFO("Copy \"%s\" from SdCard %u to SDRAM\n\r", file_name,
			(unsigned int) boot_select);
	res = f_open(&fileObject, file_name, FA_OPEN_EXISTING|FA_READ);
	if( res != FR_OK ) {
		TRACE_ERROR("f_open read pb: 0x%X\n\r", res);
		return 0;
	}
	size_t ByteRead;
    res = f_read(&fileObject, (void*)(SDRAM_DESTINATION), OBSW_MAX_SIZE, &ByteRead);
    if(res != FR_OK) {
        TRACE_ERROR("f_read pb: 0x%X\n\r", res);
        return 0;
    }
	return 0;
}

#endif

