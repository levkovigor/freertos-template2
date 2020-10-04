#ifndef CONFIG_OBSWCONFIG_H_
#define CONFIG_OBSWCONFIG_H_

#ifdef __cplusplus
#include <cstdint>
#include <cstddef>
#else
#include <stdint.h>
#include <stddef.h>
#endif

#include <portmacro.h>

#define DISPLAY_FACTORY_ALLOCATION_SIZE     0
#define RS485_WITH_TERMINATION              1
#define ADD_TEST_CODE                       1
#define PRINT_MISSED_DEADLINES              1

//! If this is not enabled, filenames may only have
//! a length of 8 (+3 bytes extension, e.g. .bin or .csv)
#define USE_LONGFILENAMES                   0
#define MAX_REPOSITORY_PATH_LENGTH          64
//! If USE_LONGFILENAMES is set to 1, this will specify
//! the maximum file name length allowed.
#if USE_LONGFILENAMES == 1
#define MAX_FILENAME_LENGTH                 16
#else
//! 8 bytes name + 1 bytes for dot + 3 byte extension.
#define MAX_FILENAME_LENGTH                 12
#endif

#ifdef __cplusplus
namespace config {
#endif

/* Hardcoded file names */
#ifdef AT91SAM9G20_EK
extern const char* BOOTLOADER_REPOSITORY;
extern const char* SW_REPOSITORY;
#else
extern const char* BOOTLOADER_REPOSITORY;
extern const char* SW_REPOSITORY;
#endif

extern const char* BOOTLOADER_NAME;
extern const char* SW_SLOT_0_NAME;
extern const char* SW_SLOT_1_NAME;

extern const char* BL_HAMMING_NAME;
extern const char* SW_SLOT_0_HAMMING_NAME;
extern const char* SW_SLOT_1_HAMMING_NAME;

static const uint32_t RS232_BAUDRATE = 230400;
static const size_t RS232_MAX_SERIAL_FRAME_SIZE = 1500;
static const uint32_t RS232_SERIAL_TIMEOUT_BAUDTICKS = 5;
static const uint16_t RS232_MUTEX_TIMEOUT = 20;

static const uint32_t RS485_REGULAR_BAUD = 115200;
static const uint32_t RS485_FAST_BAUD = 115200;
static const size_t RS485_MAX_SERIAL_FRAME_SIZE = 1500;
static const uint32_t RS485_SERIAL_TIMEOUT_BAUDTICKS = 5;
static const uint16_t RS485_MUTEX_TIMEOUT = 20;

/**
 * Set timeout for I2C transfers, specified as 1/10th of ticks.
 * Is set to one for values less than 1. Set to portMAXDELAY for debugging.
 */
static const uint32_t I2C_TRANSFER_TIMEOUT = portMAX_DELAY;

/**
 * Consider setting this higher if some SPI devices take a longer time
 * (should never take 40ms though..)
 */
static const uint32_t SPI_DEFAULT_TIMEOUT_MS = 40;

static const size_t USB_FRAME_SIZE = 1500;
static const uint32_t MAX_STORED_TELECOMMANDS = 2000;


#ifdef __cplusplus
}
#endif

#endif /* CONFIG_OBSWCONFIG_H_ */
