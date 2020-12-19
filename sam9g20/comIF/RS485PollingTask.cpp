#include "RS485PollingTask.h"

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>




RS485PollingTask::RS485PollingTask(object_id_t objectId): SystemObject(objectId) {

}

ReturnValue_t RS485PollingTask::performOperation(uint8_t opCode) {
	UART_write(bus2_uart, reinterpret_cast<unsigned char*>(uartMessage), 6);
	sif::info << "RS485 Polling Task performOperation." << std::endl;
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485PollingTask::initialize() {
	UART_start(bus2_uart, configBusRS485);
	sif::info << "RS485 Polling Task initialized." << std::endl;
    return HasReturnvaluesIF::RETURN_OK;
}
