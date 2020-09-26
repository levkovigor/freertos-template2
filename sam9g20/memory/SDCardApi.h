#ifndef SAM9G20_MEMORY_SDCARDAPI_H_
#define SAM9G20_MEMORY_SDCARDAPI_H_

#include <hcc/api_fat.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SD_CARD_0 = 0,
    SD_CARD_1 = 1
} VolumeId;

// Unless specifically documented, the functions return 0 or F_NO_ERROR
// for successfull execution.

/**
 * API to open and close the SD card filesystem. Written in C
 * to be easily usable by the bootloader as well.
 * @param volumeId
 */
int open_filesystem(VolumeId volumeId);
int close_filesystem(VolumeId volumeId);
int select_sd_card(VolumeId volumeId);
int change_directory(const char* repository_path);

/**
 * Helper function to read whole file
 * @param file_ptr          Valid pointer to file.
 * @param buffer            Buffer to copy read data into.
 * @param bytes_to_read     Number of bytes to read.
 * @return Number of bytes read
 */
int read_whole_file(const char* repository_path, const char* file_name,
        uint8_t* buffer, const size_t max_buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* SAM9G20_MEMORY_SDCARDAPI_H_ */
