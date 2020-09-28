#include "RS232PollingTask.h"

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/tmtcservices/TmTcMessage.h>
#include <fsfw/osal/FreeRTOS/BinarySemaphore.h>
#include <fsfw/osal/FreeRTOS/TaskManagement.h>

bool RS232PollingTask::uart0Started = false;

volatile size_t RS232PollingTask::transfer1bytesReceived = 0;
volatile size_t RS232PollingTask::transfer2bytesReceived = 0;

RS232PollingTask::RS232PollingTask(object_id_t objectId,
		object_id_t sharedRingBufferId):
        UartPollingBase(objectId, sharedRingBufferId, bus0_uart, &configBus0)
{
	configBus0.rxtimeout = RS232_SERIAL_TIMEOUT_BAUDTICKS;
	configBus0.baudrate = RS232_BAUD_RATE;
}

RS232PollingTask::~RS232PollingTask() {}


ReturnValue_t RS232PollingTask::initialize() {
    ReturnValue_t result = UartPollingBase::initialize();
    if(result == HasReturnvaluesIF::RETURN_OK) {
        uart0Started = true;
    }
    return result;
}


ReturnValue_t RS232PollingTask::performOperation(uint8_t operationCode) {
	initiateUartTransfers();
	while(true) {
		pollUart();
	}
	return RETURN_OK; // This should never be reached
}

void RS232PollingTask::initiateUartTransfers() {
	uartTransfer1.bus = bus0_uart;
	uartTransfer1.callback = uart1Callback;
	uartTransfer1.direction = read_uartDir;
	uartTransfer1.postTransferDelay = 0;
	uartTransfer1.readData = readBuffer1.data();
	uartTransfer1.readSize = RS232_MAX_SERIAL_FRAME_SIZE;
	uartTransfer1.result = &transfer1Status;
	uartSemaphore1.acquire();
	uartTransfer1.semaphore = uartSemaphore1.getSemaphore();

	uartTransfer2.bus = bus0_uart;
	uartTransfer2.callback = uart2Callback;
	uartTransfer2.direction = read_uartDir;
	uartTransfer2.postTransferDelay = 0;
	uartTransfer2.readData = readBuffer2.data();
	uartTransfer2.readSize = RS232_MAX_SERIAL_FRAME_SIZE;
	uartTransfer2.result = &transfer2Status;
	uartSemaphore2.acquire();
	uartTransfer2.semaphore = uartSemaphore2.getSemaphore();

	int result = UART_queueTransfer(&uartTransfer1);
	if(result != 0) {
		// config error
		sif::error << "TcSerialPollingTask::initiateUartTransfers: Config error"
				<< std::endl;
	}
	result = UART_queueTransfer(&uartTransfer2);
	if(result != 0) {
		// config error
		sif::error << "TcSerialPollingTask::initiateUartTransfers: Config error"
				<< std::endl;
	}
}

void RS232PollingTask::pollUart() {
    ReturnValue_t result = uartSemaphore1.acquire();
    if(result == HasReturnvaluesIF::RETURN_OK) {
        handleTransferCompletion(readBuffer1.data(), transfer1bytesReceived,
                transfer1Status, config::RS232_MUTEX_TIMEOUT);
        int retval = UART_queueTransfer(&uartTransfer1);
        if(retval != 0) {
            otherErrorCount++;
        }
    }

    result = uartSemaphore2.acquire();
    if(result == HasReturnvaluesIF::RETURN_OK) {
        handleTransferCompletion(readBuffer2.data(), transfer2bytesReceived,
                transfer2Status, config::RS232_MUTEX_TIMEOUT);
        int retval = UART_queueTransfer(&uartTransfer2);
        if(retval != 0) {
            otherErrorCount++;
        }
    }

    if((parityErrorCount > 0) or (overrunErrorCount > 0) or
            (framingErrorCount > 0) or (otherErrorCount > 0)) {
        generateErrorEventResetCounters();

    }
}


void RS232PollingTask::uart1Callback(SystemContext context,
		xSemaphoreHandle sem) {
	transfer1bytesReceived = UART_getPrevBytesRead(bus0_uart);
	genericUartCallback(context, sem);
}

void RS232PollingTask::uart2Callback(SystemContext context,
		xSemaphoreHandle sem) {
	transfer2bytesReceived = UART_getPrevBytesRead(bus0_uart);
	genericUartCallback(context, sem);
}

