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

/**
 * Create directory in specified path.
 * @param repository_path
 * @param dirname
 * @return
 * F_NO_ERROR (0) if the folder was created successfully, error code otherwise
 * with 6 being the error code for F_ERR_DUPLICATED
 */
int create_directory(const char* repository_path, const char* dirname);

/**
 * Change to certain directory
 * @param repository_path
 * @return
 * F_NO_ERROR (0) if the directory was changed, error code otherwise,
 * with 4 being the error code for F_ERR_INVALIDDIR
 */
int change_directory(const char* repository_path);

/**
 * Remove directory with specified name in specified path.
 * @param repository_path
 * @param dirname
 * @return
 * F_NO_ERROR(0) if the directory was removed successfully, error code otherwise
 * with 14 being the error code for F_ERR_NOTEMPTY
 */
int delete_directory(const char* repository_path, const char* dirname);

/**
 * Helper function to read whole file
 * @param file_ptr          Valid pointer to file.
 * @param buffer            Buffer to copy read data into.
 * @param bytes_to_read     Number of bytes to read.
 * @return Number of bytes read
 */
int read_whole_file(const char* repository_path, const char* file_name,
        uint8_t* buffer, const size_t max_buffer_size);

/***
 * Create a file. Data can be written to new file directly if desired.
 * @param repository_path
 * @param filename
 * @param initial_data      Pointer to data to write. Set to NULL if not needed.
 * @param initial_data_size Size of data to be written.
 * @return
 * Returns -2 if the path does not exists, -1 if the file already exists,
 * and the number of bytes written otherwise.
 */
int create_file(const char* repository_path, const char* filename,
        const uint8_t* initial_data, size_t initial_data_size);

int delete_file(const char* repository_path,
        const char* filename);

#ifdef __cplusplus
}
#endif

#endif /* SAM9G20_MEMORY_SDCARDAPI_H_ */
