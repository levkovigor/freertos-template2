/**
 * Contains virtualized FRAM module, using the SD card to emulate the FRAM for the AT91 board.
 * It is assumed that the file system has been set up before calling these functions!
 * Also see SDCardApi or the SDCardAccess class.
 */
#ifndef SAM9G20_COMMON_VIRTUALFRAMAPI_H_
#define SAM9G20_COMMON_VIRTUALFRAMAPI_H_

#include "SDCardApi.h"

#ifdef __cplusplus
extern "C" {
#endif

static const char* const VIRT_FRAM_NAME = "fram.bin";
static const char* const VIRT_FRAM_PATH = "misc";

/**
 * Call this to create the generic FRAM file if it does not exist yet.
 * This function will allocate the memory required for the critical block from the heap,
 * so this function should be called at software startup.
 * @return
 */
int create_generic_fram_file();

extern int set_hamming_check_flag();

extern int set_to_load_softwareupdate(bool enable, VolumeId volume);

#ifdef __cplusplus
}
#endif

#endif /* SAM9G20_COMMON_VIRTUALFRAMAPI_H_ */

