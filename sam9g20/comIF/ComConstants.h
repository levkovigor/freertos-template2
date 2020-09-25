#ifndef SAM9G20_COMIF_UARTCONSTANTS_H_
#define SAM9G20_COMIF_UARTCONSTANTS_H_

#include <subsystemIdRanges.h>
#include <fsfw/events/Event.h>

namespace comconstants {
static constexpr uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::UART_COM_IF;

static constexpr Event RS232_SEND_FAILURE = MAKE_EVENT(0x00, SEVERITY::HIGH);
static constexpr Event RS232_POLLONG_ERROR = MAKE_EVENT(0x01, SEVERITY::MEDIUM);

}




#endif /* SAM9G20_COMIF_UARTCONSTANTS_H_ */
