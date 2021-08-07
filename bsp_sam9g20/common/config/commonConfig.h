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

static const char* const BOOTLOADER_REPOSITORY =         "BIN/BL";
static const char* const SW_REPOSITORY =                 "BIN/OBSW";

static const char* const BOOTLOADER_NAME =              "bl.bin";
static const char* const SW_SLOT_0_NAME =               "obsw_sl0.bin";
static const char* const SW_SLOT_1_NAME =               "obsw_sl1.bin";

static const char* const BL_HAMMING_NAME =              "bl_ham.bin";
static const char* const SW_FLASH_HAMMING_NAME =        "nor_ham.bin";
static const char* const SW_SLOT_0_HAMMING_NAME =       "sl0_ham.bin";
static const char* const SW_SLOT_1_HAMMING_NAME =       "sl1_ham.bin";

#ifdef __cplusplus
}
#endif

#endif /* SAM9G20_AT91_COMMON_COMMONCONFIG_H_ */
