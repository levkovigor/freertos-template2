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

#ifdef __cplusplus
namespace config {
#endif

/* Hardcoded file names */
#ifdef AT91SAM9G20_EK
extern const char* BOOTLOADER_REPOSITORY;
extern const char* SW_REPOSITORY;
#else
extern char* BOOTLOADER_REPOSITORY;
extern const char* SW_REPOSITORY;
#endif

extern const char* BOOTLOADER_NAME;
extern const char* SW_SLOT_1_NAME;
extern const char* SW_SLOT_2_NAME;

extern const char* BL_HAMMING_NAME;
extern const char* SW_SLOT_1_HAMMING_NAME;
extern const char* SW_SLOT_2_HAMMING_NAME;

static constexpr uint32_t RS232_BAUDRATE = 230400;
static constexpr size_t RS232_MAX_SERIAL_FRAME_SIZE = 1500;
static constexpr uint32_t RS232_SERIAL_TIMEOUT_BAUDTICKS = 5;
static constexpr uint16_t RS232_MUTEX_TIMEOUT = 20;

static constexpr uint32_t RS485_REGULAR_BAUD = 115200;
static constexpr uint32_t RS485_FAST_BAUD = 115200;
static constexpr size_t RS485_MAX_SERIAL_FRAME_SIZE = 1500;
static constexpr uint32_t RS485_SERIAL_TIMEOUT_BAUDTICKS = 5;
static constexpr uint16_t RS485_MUTEX_TIMEOUT = 20;

/**
 * Set timeout for I2C transfers, specified as 1/10th of ticks.
 * Is set to one for values less than 1. Set to portMAXDELAY for debugging.
 */
static constexpr uint32_t I2C_TRANSFER_TIMEOUT = portMAX_DELAY;

/**
 * Consider setting this higher if some SPI devices take a longer time
 * (should never take 40ms though..)
 */
static constexpr uint32_t SPI_DEFAULT_TIMEOUT_MS = 40;

static constexpr size_t USB_FRAME_SIZE = 1500;
static constexpr uint32_t MAX_STORED_TELECOMMANDS = 2000;

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_OBSWCONFIG_H_ */
