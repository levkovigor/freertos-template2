#ifndef SAM9G20_COMMON_SDCARDAPI_H_
#define SAM9G20_COMMON_SDCARDAPI_H_

#include <hcc/api_fat.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SD_CARD_0 = 0,
    SD_CARD_1 = 1
} VolumeId;

/**
 * API as a thin abstraction layer for the HCC file API. Written in C
 * to be easily usable by the bootloader as well.
 * Unless specifically documented, all functions return 0 or F_NO_ERROR
 * for successfull execution.
 * @param volumeId
 */
int open_filesystem();
int close_filesystem();
int select_sd_card(VolumeId volumeId);
int switch_sd_card(VolumeId volumeId);

/**
 * Create directory in specified path.
 * @param repository_path   Path where the directory will be created.
 * If this path is NULL, the directory will be created in the current directory.
 * @param dirname
 * @return
 * F_NO_ERROR (0) if the folder was created successfully, error code otherwise
 * with 6 being the error code for F_ERR_DUPLICATED
 */
int create_directory(const char* repository_path, const char* dirname);

/**
 * Change to certain directory
 * @param repository_path
 * @param from_root Should be set to true for absolute paths, the current
 * working directory will be switched to root if this is the case before
 * changing to the repository path.
 * @return
 * F_NO_ERROR (0) if the directory was changed, error code otherwise,
 * with 4 being the error code for F_ERR_INVALIDDIR
 */
int change_directory(const char* repository_path, bool from_root);

/**
 * Remove directory with specified name in specified path. Must be empty.
 * @param repository_path Path where the directory resides in.
 * Set to NULL if directory is in current directory.
 * @param dirname
 * @return
 * F_NO_ERROR(0) if the directory was removed successfully, error code otherwise
 * with 14 being the error code for F_ERR_NOTEMPTY and 3 being the error
 * code for F_ERR_INVALIDDIR in case the target is not a directory.
 */
int delete_directory(const char* repository_path, const char* dirname);

/**
 * Delete the directory even if it is not empty by deleting all files.
 * Use with care, this is the rm -rf equivalent if the third parameter
 * is set to true!
 * @param repository_path
 * @param dirname
 * @param delete_subfolder_recursively If the folder contains other folders
 * and this parameter is set to true, delete these subfolders recursively.
 * @return
 * F_NO_ERROR(0) if folder was delected successfully. -1 if a folder
 * was found inside the folder to delete and recursive deletion is not
 * allowed. Other error codes otherwise.
 */
int delete_directory_force(const char* repository_path, const char* dirname,
        bool delete_subfolder_recursively);

/**
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

/**
 * Get generic file information.
 * @param filesize
 * @param locked
 * @return
 */
int get_file_info(const char* repository_path, const char* filename,
		size_t* filesize, bool* locked, uint16_t* cdate,
		uint16_t* ctime);

/**
 * Locks a file, making it read-only and preventing deletion.
 * @param repository_path
 * @param filename
 * @return
 */
int lock_file(const char* repository_path, const char* filename);

/**
 * Unlocks a file, enabling writing and allowing deletion.
 * @param repository_path
 * @param filename
 * @return
 */
int unlock_file(const char* repository_path, const char* filename);

/**
 * Delete a file which resides in a give path.
 * @param repository_path File is located at this path. Set to NULL to delete
 * file in current directory.
 * @param filename
 * @return
 * F_NO_ERROR (0) if the file was deleted successfully, otherwise error code
 * with 3 being the error code for F_ERR_INVALIDDIR if the target is
 * a directory.
 */
int delete_file(const char* repository_path,
        const char* filename);

/**
 * Helper function to read whole file
 * @param file_ptr          Valid pointer to file.
 * @param buffer            Buffer to copy read data into.
 * @param bytes_to_read     Number of bytes to read.
 * @return Number of bytes read
 */
int read_whole_file(const char* repository_path, const char* file_name,
        uint8_t* buffer, const size_t max_buffer_size);

/**
 * Clears the whole SD-Card by deleting all files and folders
 * except the SYSTEM folder. Use with care!
 * @return
 * F_NO_ERROR(0) is SD card was cleared successfully, error code if some
 * or all steps failed.
 */
int clear_sd_card();

#ifdef __cplusplus
}
#endif

#endif /* SAM9G20_COMMON_SDCARDAPI_H_ */
