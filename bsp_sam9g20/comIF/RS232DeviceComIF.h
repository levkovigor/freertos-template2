#ifndef MISSION_COMIF_RS232DEVICECOMIF_H_
#define MISSION_COMIF_RS232DEVICECOMIF_H_

#include <fsfw/devicehandlers/DeviceCommunicationIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/osal/FreeRTOS/BinarySemaphore.h>

#include <fsfw/events/Event.h>

extern "C" {
#include <AT91SAM9G20.h>
#include <hal/Drivers/UART.h>
}

#include "ComConstants.h"

/**
 * @brief	This class will be used by the Iridium device to send commands.
 * @details
 * Please note that all RS232 data will be polled by a special task and
 * forwarded to a CCSDS distributer directly. Unless specifically implemented
 * in the polling task, packets targeted at the Irdidium handler will be
 * received via the messaging interfaces, instead of being polled in
 * this communication interface.
 */
class RS232DeviceComIF: public DeviceCommunicationIF, public SystemObject {
public:
	static constexpr uint8_t INTERFACE_ID = CLASS_ID::RS232_COM_IF;

	// Bus has not been started yet.
	static constexpr ReturnValue_t RS232_INACTIVE = MAKE_RETURN_CODE(0x00);

	RS232DeviceComIF(object_id_t objectId);
	virtual ~RS232DeviceComIF();

	virtual ReturnValue_t initializeInterface(CookieIF * cookie) override;
	virtual ReturnValue_t sendMessage(CookieIF *cookie,
			const uint8_t * sendData, size_t sendLen) override;
	virtual ReturnValue_t getSendSuccess(CookieIF *cookie) override;
	virtual ReturnValue_t requestReceiveMessage(CookieIF *cookie,
			size_t requestLen) override;
	virtual ReturnValue_t readReceivedMessage(CookieIF *cookie,
			uint8_t **buffer, size_t *size) override;

private:
	UARTgenericTransfer writeStruct;
	BinarySemaphore writeSemaphore;
	UARTtransferStatus writeResult = UARTtransferStatus::done_uart;

	static void uartWriteCallback(SystemContext context, xSemaphoreHandle sem);

};

#endif /* MISSION_COMMIF_DUMMYDEVICECOMIF_H_ */
