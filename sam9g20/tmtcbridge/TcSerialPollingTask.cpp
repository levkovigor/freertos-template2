#include <sam9g20/tmtcbridge/TcSerialPollingTask.h>
#include <sam9g20/tmtcbridge/TmTcSerialBridge.h>

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/tmtcservices/TmTcMessage.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <fsfw/osal/FreeRTOS/BinarySemaphore.h>
#include <fsfw/osal/FreeRTOS/TaskManagement.h>


volatile uint8_t TcSerialPollingTask::transfer1bytesReceived = 0;
volatile uint8_t TcSerialPollingTask::transfer2bytesReceived = 0;

TcSerialPollingTask::TcSerialPollingTask(object_id_t objectId,
		object_id_t tcBridge, object_id_t sharedRingBufferId,
		uint16_t serialTimeoutBaudticks, size_t frameSize):
		SystemObject(objectId), tcBridge(tcBridge),
		sharedRingBufferId(sharedRingBufferId) {
	configBus0.rxtimeout = serialTimeoutBaudticks;
}

TcSerialPollingTask::~TcSerialPollingTask() {}


ReturnValue_t TcSerialPollingTask::initialize() {
	tcStore = objectManager->get<StorageManagerIF>(objects::TC_STORE);
	if (tcStore == nullptr) {
		sif::error << "TcSerialPollingTask::initialize: TC Store uninitialized!"
				<< std::endl;
		return ObjectManagerIF::CHILD_INIT_FAILED;
	}

	AcceptsTelecommandsIF* tcTarget =
			objectManager->get<AcceptsTelecommandsIF>(tcBridge);
	if(tcTarget == nullptr) {
		sif::error << "Serial Polling Task: Could not set TC destination"
				<< std::endl;
		return RETURN_FAILED;
	}
	//targetTcDestination = tcTarget->getRequestQueue();
	configBus0.baudrate = BAUD_RATE;
	ReturnValue_t result = UART_start(bus0_uart,configBus0);
	if (result != RETURN_OK) {
		sif::error << "Serial Polling: UART_start init error with code " <<
				(int) result << std::endl;
	}

	sharedRingBuffer = objectManager->get<SharedRingBuffer>(sharedRingBufferId);
	if(sharedRingBuffer == nullptr) {
		sif::error << "TcSerialPollingTask::initialize: Passed ring buffer"
				" invalid !" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return result;
}


ReturnValue_t TcSerialPollingTask::performOperation(uint8_t operationCode) {
	initiateUartTransfers();
	while(true) {
		pollUart();
	}
	return RETURN_OK; // This should never be reached
}

void TcSerialPollingTask::initiateUartTransfers() {
	uartTransfer1.bus = bus0_uart;
	uartTransfer1.callback = uart1Callback;
	uartTransfer1.direction = read_uartDir;
	uartTransfer1.postTransferDelay = 0;
	uartTransfer1.readData = readBuffer1.data();
	uartTransfer1.readSize = SERIAL_FRAME_MAX_SIZE;
	uartTransfer1.result = &transfer1Status;
	uartSemaphore1.acquire();
	uartTransfer1.semaphore = uartSemaphore1.getSemaphore();

	uartTransfer2.bus = bus0_uart;
	uartTransfer2.callback = uart2Callback;
	uartTransfer2.direction = read_uartDir;
	uartTransfer2.postTransferDelay = 0;
	uartTransfer2.readData = readBuffer2.data();
	uartTransfer2.readSize = SERIAL_FRAME_MAX_SIZE;
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
		sif::error << "TcSerialPollingTask::initiateUartTransfers: Config error"
				<< std::endl;
		// config error
	}
}

void TcSerialPollingTask::pollUart() {
	ReturnValue_t result = uartSemaphore1.acquire();
	if(result == HasReturnvaluesIF::RETURN_OK) {
	    // todo: handle lock error
		sharedRingBuffer->lockRingBufferMutex(MutexIF::TimeoutType::WAITING, 2);
		sharedRingBuffer->writeData(readBuffer1.data(), transfer1bytesReceived);
		sharedRingBuffer->unlockRingBufferMutex();
		if(transfer1Status != done_uart) {
			// todo: handle error here
		}
		int retval = UART_queueTransfer(&uartTransfer1);
		if(retval != 0) {
			// todo: config error debug output
		}
	}

	result = uartSemaphore2.acquire();
	if(result == HasReturnvaluesIF::RETURN_OK) {
	    // todo: handle lock error
		sharedRingBuffer->lockRingBufferMutex(MutexIF::TimeoutType::WAITING, 2);
		sharedRingBuffer->writeData(readBuffer2.data(), transfer2bytesReceived);
		sharedRingBuffer->unlockRingBufferMutex();
		if(transfer2Status != done_uart) {
		    // todo: handle error here
		}
		int retval = UART_queueTransfer(&uartTransfer2);
		if(retval != 0) {
		    // todo: config error debug output
		}
	}
}


ReturnValue_t TcSerialPollingTask::handleOverrunError() {
	sif::error << "Serial Polling: Overrun error on bus 0" << std::endl;
	size_t recvSize = UART_getPrevBytesRead(bus0_uart);
	sif::error << "Read " << recvSize << " bytes." << std::endl;
	sif::error << "Packet not read for now. Consider still reading the packet !"
			<< std::endl;
	// use RETURN_OK to enable packet handling if packet is still read.
	// todo: if this occurs too often, disable printout and maybe restart bus?
	return RETURN_FAILED;
}

void TcSerialPollingTask::uart1Callback(SystemContext context,
		xSemaphoreHandle sem) {
	transfer1bytesReceived = UART_getPrevBytesRead(bus0_uart);
	genericUartCallback(context, sem);
}

void TcSerialPollingTask::uart2Callback(SystemContext context,
		xSemaphoreHandle sem) {
	transfer2bytesReceived = UART_getPrevBytesRead(bus0_uart);
	genericUartCallback(context, sem);
}

void TcSerialPollingTask::genericUartCallback(SystemContext context,
		xSemaphoreHandle sem) {
	BaseType_t higherPriorityTaskAwoken = pdFALSE;
	if(context == SystemContext::task_context) {
		BinarySemaphore::release(sem);
	}
	else {
		BinarySemaphore::releaseFromISR(sem,
				&higherPriorityTaskAwoken);
	}
	if(context == SystemContext::isr_context and
			higherPriorityTaskAwoken == pdPASS) {
		// After some research, I have found out that each interrupt causes
		// a higher priority task to awaken. I assume that the ISR is called
		// from a separate driver internal task/queue. I assume this
		// is expected behaviour as this task has a relatively high
		// priority and has blocking elements.
		TaskManagement::requestContextSwitch(CallContext::ISR);
	}
}


