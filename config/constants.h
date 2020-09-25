#ifndef CONFIG_TMTC_TMTCSIZE_H_
#define CONFIG_TMTC_TMTCSIZE_H_

#include <cstdint>
#include <cstddef>
#include <portmacro.h>

#define RS485_WITH_TERMINATION 1

namespace config {
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

static constexpr size_t USB_FRAME_SIZE = 1500;
static constexpr uint32_t MAX_STORED_TELECOMMANDS = 2000;
}

#endif /* CONFIG_TMTC_TMTCSIZE_H_ */
