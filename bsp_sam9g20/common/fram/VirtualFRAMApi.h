/**
 * Contains virtualized FRAM module, using the SD card to emulate the FRAM for the AT91 board.
 * It is assumed that the file system has been set up before calling these functions!
 * Also see SDCardApi or the SDCardAccess class.
 */
#ifndef SAM9G20_COMMON_VIRTUALFRAMAPI_H_
#define SAM9G20_COMMON_VIRTUALFRAMAPI_H_

#include "../SDCardApi.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

static const char* const VIRT_FRAM_NAME = "FRAM.BIN";
static const char* const VIRT_FRAM_PATH = "MISC";

/**
 * Start the virtualized FRAM by creating a FRAM file on the SD-Card. This function will
 * also take care of opening and closing the file system, all other functions
 * in this API will not do this!
 * @return
 */
int FRAM_start();

int fram_read_critical_block(uint8_t* buffer, const size_t max_size);

/**
 * Call this to create the generic FRAM file if it does not exist yet.
 * This function will allocate the memory required for the critical block from the heap,
 * so this function should be called at software startup.
 * @return
 */
int create_generic_fram_file();

int delete_generic_fram_file();

int fram_set_ham_check_flag();

int fram_set_to_load_softwareupdate(bool enable, VolumeId volume);

int fram_write_software_version(uint8_t software_version, uint8_t software_subversion,
        uint8_t sw_subsubversion);
int fram_read_software_version(uint8_t *software_version, uint8_t* software_subversion,
        uint8_t* sw_subsubversion);

int fram_increment_reboot_counter(uint32_t* new_reboot_counter);

int fram_is_deployment_timer_armed(bool *armed);
int fram_arm_deployment_timer(bool arm);

int fram_get_seconds_on_deployment_timer(uint32_t* seconds);
int fram_set_seconds_on_deployment_timer(uint32_t seconds);
int fram_increment_seconds_on_deployment_timer(uint32_t incrementSeconds);

#ifdef __cplusplus
}
#endif

#endif /* SAM9G20_COMMON_VIRTUALFRAMAPI_H_ */

