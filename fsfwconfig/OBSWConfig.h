#ifndef FSFWCONFIG_OBSWCONFIG_H_
#define FSFWCONFIG_OBSWCONFIG_H_

#ifdef __cplusplus
#include <cstdint>
#include <cstddef>

#include "returnvalues/classIds.h"
#include "events/subsystemIdRanges.h"
#include "objects/systemObjectList.h"

#else
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#endif

#include <portmacro.h>


#define OBSW_RS485_WITH_TERMINATION             1

//! All of these should be disabled for mission code but are very helpful
//! for development
#define OBSW_TRACK_FACTORY_ALLOCATION_SIZE   	1
#define OBSW_MONITOR_ALLOCATION    				1
#define OBSW_ADD_TEST_CODE                      1
#define OBSW_PRINT_MISSED_DEADLINES             1
#define OBSW_VERBOSE_LEVEL				        1

#define MAX_REPOSITORY_PATH_LENGTH 			    64
#define MAX_FILENAME_LENGTH 				    12

#ifdef __cplusplus
namespace config {
#endif

#if OBSW_MONITOR_ALLOCATION == 1
extern bool softwareInitializationComplete;
#endif

/* Hardcoded file names */
#ifdef AT91SAM9G20_EK
const char* const BOOTLOADER_REPOSITORY =         "BIN/AT91/BL";
const char* const SW_REPOSITORY =                 "BIN/AT91/OBSW";
#else
const char* const BOOTLOADER_REPOSITORY =         "BIN/IOBC/BL";
const char* const SW_REPOSITORY =                 "BIN/IOBC/OBSW";
#endif

const char* const BOOTLOADER_NAME =               "bl.bin";
const char* const SW_SLOT_0_NAME =                "obsw_sl1.bin";
const char* const SW_SLOT_1_NAME =                "obsw_sl2.bin";
const char* const SW_UPDATE_SLOT_NAME =           "obsw_up.bin";

const char* const BL_HAMMING_NAME =               "bl_ham.bin";
const char* const SW_SLOT_0_HAMMING_NAME =        "sl1_hamm.bin";
const char* const SW_SLOT_1_HAMMING_NAME =        "sl2_hamm.bin";
const char* const SW_UPDATE_HAMMING_NAME =        "up_hamm.bin";

static const uint32_t SD_CARD_ACCESS_MUTEX_TIMEOUT =    50;
static const uint8_t SD_CARD_MQ_DEPTH =                 20;
static const size_t SD_CARD_MAX_READ_LENGTH =           1024;

static const uint32_t OBSW_SERVICE_1_MQ_DEPTH =         10;

static const uint32_t RS232_BAUDRATE =                  230400;
static const size_t RS232_MAX_SERIAL_FRAME_SIZE =       1500;
// When performing timeout-based reading using DLE encoding, packet might
// be larger than 1500 bytes because of the transport layer.
static const size_t TRANSPORT_LAYER_ADDITION =          500;
static const uint32_t RS232_SERIAL_TIMEOUT_BAUDTICKS =  5;
static const uint16_t RS232_MUTEX_TIMEOUT =             20;

static const uint32_t RS485_REGULAR_BAUD =              115200;
static const uint32_t RS485_FAST_BAUD =                 115200;
static const size_t RS485_MAX_SERIAL_FRAME_SIZE =       1500;
static const uint32_t RS485_SERIAL_TIMEOUT_BAUDTICKS =  5;
static const uint16_t RS485_MUTEX_TIMEOUT =             20;

/**
 * Set timeout for I2C transfers, specified as 1/10th of ticks.
 * Is set to one for values less than 1. Set to portMAXDELAY for debugging.
 */
static const uint32_t I2C_TRANSFER_TIMEOUT =            portMAX_DELAY;

/**
 * Consider setting this higher if some SPI devices take a longer time
 * (should never take 40ms though..)
 */
static const uint32_t SPI_DEFAULT_TIMEOUT_MS =          40;

static const size_t USB_FRAME_SIZE =                    1500;
static const uint32_t MAX_STORED_TELECOMMANDS =         2000;


#ifdef __cplusplus
}
#endif

#endif /* FSFWCONFIG_OBSWCONFIG_H_ */
