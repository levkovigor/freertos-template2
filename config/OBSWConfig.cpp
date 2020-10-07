#include "OBSWConfig.h"

#ifdef __cplusplus
namespace config {
#endif

/* Hardcoded file names */
#ifdef AT91SAM9G20_EK
const char* BOOTLOADER_REPOSITORY = "BIN/AT91/BL";
const char* SW_REPOSITORY = "BIN/AT91/OBSW";
#else
const char* BOOTLOADER_REPOSITORY = "BIN/IOBC/BL";
const char* SW_REPOSITORY = "BIN/IOBC/OBSW";
#endif

const char* BOOTLOADER_NAME = "bl.bin";
const char* SW_SLOT_0_NAME = "obsw_sl1.bin";
const char* SW_SLOT_1_NAME = "obsw_sl2.bin";
const char* SW_UPDATE_SLOT_NAME = "obsw_up.bin";

const char* BL_HAMMING_NAME = "bl_ham.bin";
const char* SW_SLOT_0_HAMMING_NAME = "sl1_hamm.bin";
const char* SW_SLOT_1_HAMMING_NAME = "sl2_hamm.bin";
const char* SW_UPDATE_HAMMING_NAME ="up_hamm.bin";

#ifdef __cplusplus
}
#endif
