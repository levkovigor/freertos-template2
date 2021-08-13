#ifndef SAM9G20_COMIF_UARTCONSTANTS_H_
#define SAM9G20_COMIF_UARTCONSTANTS_H_

#include <fsfw/events/Event.h>
#include <fsfwconfig/events/subsystemIdRanges.h>
#include <fsfwconfig/returnvalues/classIds.h>

namespace comconstants {
static constexpr uint8_t UART_SUBSYSTEM_ID = SUBSYSTEM_ID::UART_COM_IF;

//! P1: Number of occured uart errors.
//! First byte: Parity Error Count.
//! Second byte: Overrun Error Count.
//! Third byte: Framing Error Count.
//! Fourth byte: Other Error Count.
//! P2: Last error (for other error count, e.g. mutex timeout)
static const Event RS232_POLLING_ERROR = event::makeEvent(UART_SUBSYSTEM_ID,
        0x01, severity::MEDIUM);
static const Event RS485_POLLING_ERROR = event::makeEvent(UART_SUBSYSTEM_ID,
        0x03, severity::MEDIUM);

static const Event RS232_SEND_FAILURE = event::makeEvent(UART_SUBSYSTEM_ID,
        0x00, severity::HIGH);
static const Event RS485_SEND_FAILURE = event::makeEvent(UART_SUBSYSTEM_ID,
        0x02, severity::HIGH);

static const uint8_t I2C_SUBSYSTEM_ID = SUBSYSTEM_ID::I2C_COM_IF;

static const Event I2C_DRIVER_ERROR_EVENT = event::makeEvent(I2C_SUBSYSTEM_ID,
        0x00, severity::HIGH);
}

#endif /* SAM9G20_COMIF_UARTCONSTANTS_H_ */
