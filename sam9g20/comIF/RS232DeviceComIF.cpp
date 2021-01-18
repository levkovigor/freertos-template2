#include "RS232DeviceComIF.h"

#include <fsfw/serialize/SerializeAdapter.h>
#include <sam9g20/comIF/RS232PollingTask.h>
#include <fsfw/serviceinterface/ServiceInterface.h>

RS232DeviceComIF::RS232DeviceComIF(object_id_t objectId):
		SystemObject(objectId) {
	writeStruct.bus = bus0_uart;
	writeStruct.direction = write_uartDir;
	writeStruct.postTransferDelay = 0;
	writeStruct.result = &writeResult;
	writeStruct.callback = uartWriteCallback;
}

RS232DeviceComIF::~RS232DeviceComIF() {
}

ReturnValue_t RS232DeviceComIF::initializeInterface(CookieIF *cookie) {
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS232DeviceComIF::sendMessage(CookieIF *cookie,
		const uint8_t *sendData, size_t sendLen) {
	if(not RS232PollingTask::uart0Started) {
		// should not happen! The RS232 bus is started by the RS232PollingTask
	    // at program initialization.
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "RS232DeviceComIF::sendMessage: UART bus 0 not active" << std::endl;
#else
		sif::printError("RS232DeviceComIF::sendMessage: UART bus 0 not active\n");
#endif
		return RS232_INACTIVE;
	}

	writeStruct.writeData = const_cast<uint8_t*>(sendData);
	writeStruct.writeSize = sendLen;

	int retval = UART_queueTransfer(&writeStruct);
	if(retval == 0) {
		return HasReturnvaluesIF::RETURN_OK;
	}

	// Configuration error.
#if FSFW_CPP_OSTREAM_ENABLED == 1
	sif::error << "RS232DeviceComIF::sendMessage: UART_queueTransfer failed"
			<< "with code " << retval << std::endl;
#else
	sif::printError("RS232DeviceComIF::sendMessage: UART_queueTransfer failed"
            "with code %d\n", retval);
#endif
	triggerEvent(comconstants::RS232_SEND_FAILURE, retval, 0);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS232DeviceComIF::getSendSuccess(CookieIF *cookie) {
	// unless there is a configuration error, this should never fail.
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS232DeviceComIF::requestReceiveMessage(CookieIF *cookie,
		size_t requestLen) {
	// The Iridium will come on the same line all satellite telecommands are
	// received. If the Iridium device sends replies with a specific protocol,
	// the polling task needs to distinguish between CCSDS packets and those
	// special packets. Those packets then need to be sent to the ComIF via
	// messaging.
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS232DeviceComIF::readReceivedMessage(CookieIF *cookie,
		uint8_t **buffer, size_t *size) {
	return HasReturnvaluesIF::RETURN_OK;
}

void RS232DeviceComIF::uartWriteCallback(SystemContext context,
		xSemaphoreHandle sem) {
	RS232PollingTask::genericUartCallback(context, sem);
}
