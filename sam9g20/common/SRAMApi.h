/**
 * @brief   This file will contain the common API to access the SRAM0 memory block.
 * @details
 * This can be used for communication between the primary OBSW and the bootloader.
 */
#ifndef SAM9G20_COMMON_SRAMAPI_H_
#define SAM9G20_COMMON_SRAMAPI_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


static const uint32_t SRAM0_END = 0x204000;

/**
 * Possible values for the status field. The status field will be stored in the last
 * 4 bytes of the SRAM0 memory.
 */
static const int32_t SRAM_OK = 0;
static const int32_t SRAM_HAMMING_ERROR_SINGLE_BIT = 1;
static const int32_t SRAM_HAMMING_ERROR_ECC = 2;
static const int32_t SRAM_HAMMING_ERROR_MULTIBIT = 3;
static const int32_t SRAM_FRAM_ISSUES = 4;
static const int32_t SRAM_BOOTLOADER_INVALID = 5;

void set_sram0_status_field(int32_t status);
int32_t get_sram0_status_field();

#ifdef __cplusplus
}
#endif


#endif /* SAM9G20_COMMON_SRAMAPI_H_ */
