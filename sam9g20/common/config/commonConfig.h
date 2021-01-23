/**
 * Common configuration constants for both bootloader and main OBSW for iOBC and AT91 will
 * be stored here.
 */
#ifndef SAM9G20_AT91_COMMON_COMMONCONFIG_H_
#define SAM9G20_AT91_COMMON_COMMONCONFIG_H_

#ifdef __cplusplus
namespace config {
#endif

static const char* const BOOTLOADER_NAME =               "bl.bin";
static const char* const SW_SLOT_0_NAME =                "obsw_sl0.bin";
static const char* const SW_SLOT_1_NAME =                "obsw_sl1.bin";

static const char* const BL_HAMMING_NAME =               "bl_ham.bin";
static const char* const SW_NORFLASH_NAME =              "nor_ham.bin";
static const char* const SW_SLOT_0_HAMMING_NAME =        "sl0_hamm.bin";
static const char* const SW_SLOT_1_HAMMING_NAME =        "sl1_hamm.bin";

#ifdef __cplusplus
}
#endif

#endif /* SAM9G20_AT91_COMMON_COMMONCONFIG_H_ */
