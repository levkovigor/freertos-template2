#ifndef SAM9G20_COMIF_UARTCONSTANTS_H_
#define SAM9G20_COMIF_UARTCONSTANTS_H_

#include <subsystemIdRanges.h>
#include <classIds.h>
#include <fsfw/events/Event.h>

namespace comconstants {
static constexpr uint8_t UART_SUBSYSTEM_ID = SUBSYSTEM_ID::UART_COM_IF;

//! P1: Number of occured uart errors.
//! First byte: Parity Error Count.
//! Second byte: Overrun Error Count.
//! Third byte: Framing Error Count.
//! Fourth byte: Other Error Count.
//! P2: Last error (for other error count, e.g. mutex timeout)
static const Event RS232_POLLING_ERROR = EVENT::makeEvent(UART_SUBSYSTEM_ID,
        0x01, SEVERITY::MEDIUM);
static const Event RS485_POLLING_ERROR = EVENT::makeEvent(UART_SUBSYSTEM_ID,
        0x03, SEVERITY::MEDIUM);

static const Event RS232_SEND_FAILURE = EVENT::makeEvent(UART_SUBSYSTEM_ID,
        0x00, SEVERITY::HIGH);
static const Event RS485_SEND_FAILURE = EVENT::makeEvent(UART_SUBSYSTEM_ID,
        0x02, SEVERITY::HIGH);

static const uint8_t I2C_SUBSYSTEM_ID = SUBSYSTEM_ID::I2C_COM_IF;

static const Event I2C_DRIVER_ERROR_EVENT = EVENT::makeEvent(I2C_SUBSYSTEM_ID,
        0x00, SEVERITY::HIGH);
}

#endif /* SAM9G20_COMIF_UARTCONSTANTS_H_ */
