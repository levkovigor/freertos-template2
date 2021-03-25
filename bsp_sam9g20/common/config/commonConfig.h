/**
 * Common configuration constants for both bootloader and main OBSW for iOBC and AT91 will
 * be stored here.
 */
#ifndef SAM9G20_AT91_COMMON_COMMONCONFIG_H_
#define SAM9G20_AT91_COMMON_COMMONCONFIG_H_

typedef enum {
    SDC_SLOT_0,
    SDC_SLOT_1
} SdSlots;

#ifdef __cplusplus
namespace config {
#endif

static const char* const BOOTLOADER_NAME =              "BL.BIN";
static const char* const SW_SLOT_0_NAME =               "OBSW_SL0.BIN";
static const char* const SW_SLOT_1_NAME =               "OBSW_SL1.BIN";

static const char* const BL_HAMMING_NAME =              "BL_HAM.BIN";
static const char* const SW_FLASH_HAMMING_NAME =        "NOR_HAM.BIN";
static const char* const SW_SLOT_0_HAMMING_NAME =       "SL0_HAM.bin";
static const char* const SW_SLOT_1_HAMMING_NAME =       "SL1_HAM.bin";

#ifdef __cplusplus
}
#endif

#endif /* SAM9G20_AT91_COMMON_COMMONCONFIG_H_ */
