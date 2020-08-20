#include <sam9g20/tmtcbridge/TcSerialPollingTask.h>
#include <sam9g20/tmtcbridge/TmTcSerialBridge.h>

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/tmtcservices/AcceptsTelecommandsIF.h>
#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/tmtcservices/TmTcMessage.h>
#include <fsfw/globalfunctions/arrayprinter.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


TcSerialPollingTask::TcSerialPollingTask(object_id_t objectId,
		object_id_t tcBridge, size_t frameSize, object_id_t sharedRingBufferId,
		uint16_t serialTimeoutBaudticks):
		SystemObject(objectId), tcBridge(tcBridge),
		sharedRingBufferId(sharedRingBufferId) {

//	if(frameSize == 0) {
//		// Default value: Set maximum reception buffer size.
//		this->frameSize = MAX_RECEPTION_BUFFER_SIZE;
//	}
//	else if(frameSize < MAX_RECEPTION_BUFFER_SIZE) {
//		this->frameSize = frameSize;
//	}
//
//	maxNumberOfStoredPackets = this->frameSize / 12;
//	pusParser = new PusParser(maxNumberOfStoredPackets, false);
//
//	recvBuffer.reserve(this->frameSize);
//	recvBuffer.resize(this->frameSize);

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
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return result;
}


ReturnValue_t TcSerialPollingTask::performOperation(uint8_t operationCode) {
	while(true) {
		pollUart();
	}

	return RETURN_OK; // This should never be reached
}

void TcSerialPollingTask::pollUart() {
	//ReturnValue_t result = pollTc();
	//if(result == RETURN_OK) {
		//handleTc();
	//}
	ringBufferPrototypePoll();
}



// prototype for RS485 polling task. it won't do anything else other than
// polling serial data.
void TcSerialPollingTask::ringBufferPrototypePoll() {
	// with 20 bytes as "bucket" size
	uint8_t* writePtr = nullptr;
	ReturnValue_t result = sharedRingBuffer->
	        lockRingBufferMutex(MutexIF::TimeoutType::WAITING, 5);
	if(result != HasReturnvaluesIF::RETURN_OK) {
	    // mutex might be blocked, config error?
	    return;
	}

	result = sharedRingBuffer->getFreeElement(&writePtr, DMA_BUCKET_READ_SIZE);
	if(result != HasReturnvaluesIF::RETURN_OK) {
		// not enough data available. trigger event or overwrite data?
	    // in any case, don't forget to unlock the mutex if the function returns
	    // here!
	}
	sharedRingBuffer->unlockRingBufferMutex();

	// blocking read.
	result = UART_read(bus0_uart, writePtr, DMA_BUCKET_READ_SIZE);
	if(result != HasReturnvaluesIF::RETURN_OK) {
		// handle driver errors here
	}

	// zero out bytes in case of timeout.
	size_t prevBytesRead = UART_getPrevBytesRead(bus0_uart);
	if(prevBytesRead < DMA_BUCKET_READ_SIZE) {
		size_t bytesToZeroOut = DMA_BUCKET_READ_SIZE - prevBytesRead;
		std::memset(writePtr + prevBytesRead, 0, bytesToZeroOut);
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

//ReturnValue_t TcSerialPollingTask::pollTc() {
//	// TODO: Non-Blocking IO would be nice. Could be done by utilising
//	// queueTransfer which uses DMA. Data propably needs to be stored
//	// in a ring buffer then, and fixed sized frames should be used.
//	// Right now, the read function is timeout based, so it depends
//	// on data being sent either packed or with enough break between the packets
//	// to handle tranfer to the last packet to the software bus
//
//	// When using non-blocking IO and a ring buffer, a callback is used to
//	// notify the task of packets to handle. The reception would propably still
//	// be timeout based. next queueTransfer needs to be called from callback
//
//	// Actually, that would require a lot of static variables, because
//	// no arguments can be passed to the callback..
//	ReturnValue_t result = UART_read(bus0_uart, recvBuffer.data(), frameSize);
//	taskENTER_CRITICAL(); // @suppress("Function cannot be resolved")
//	if(result == RETURN_OK) {
//		handleSuccessfulTcRead();
//	}
//	else if(result == 16) {
//		result = handleOverrunError();
//	}
//	else {
//		sif::error << "Serial Polling: Error with code " << std::dec
//				   << (int) result << " on bus 0" << std::endl;
//	}
//	taskEXIT_CRITICAL(); // @suppress("Function cannot be resolved")
//	return result;
//}

//void TcSerialPollingTask::handleSuccessfulTcRead() {
//	size_t recvSize = UART_getPrevBytesRead(bus0_uart);
//	pusParser->parsePusPackets(recvBuffer.data(), recvSize);
//}

//void TcSerialPollingTask::handleTc() {
//	auto fifo = pusParser->fifo();
//	if(fifo->empty()) {
//		return;
//	}
//
//	TmTcSerialBridge * tmtcBridge =
//			objectManager->get<TmTcSerialBridge>(tcBridge);
//	if(tmtcBridge == nullptr) {
//		sif::error << "SerialPollingTask: TmTcBridge invalid!" << std::endl;
//		return;
//	}
//	tmtcBridge->registerCommConnect();
//
//	while(not fifo->empty()) {
//		PusParser::indexSizePair indexSizePair = pusParser->getNextFifoPair();
//		transferPusToSoftwareBus(indexSizePair.first, indexSizePair.second);
//	}
//}
//
//void TcSerialPollingTask::transferPusToSoftwareBus(uint16_t recvBufferIndex,
//		uint16_t packetSize) {
//	store_address_t storeId = 0;
//	ReturnValue_t result = tcStore->addData(&storeId,
//			recvBuffer.data() + recvBufferIndex, packetSize);
//	// printTelecommand(recvBuffer + recvBufferIndex, packetSize);
//	if (result != RETURN_OK) {
//		sif::debug << "TcSerialPollingTask::transferPusToSoftwareBus: Data "
//				"storage failed" << std::endl;
//		sif::debug << "Packet size: " << packetSize << std::endl;
//		return;
//	}
//	TmTcMessage message(storeId);
//	result  = MessageQueueSenderIF::sendMessage(targetTcDestination, &message);
//	if (result != RETURN_OK) {
//		sif::error << "Serial Polling: Sending message to queue failed"
//				<< std::endl;
//		tcStore->deleteData(storeId);
//	}
//}

//void TcSerialPollingTask::printTelecommand(uint8_t* tcPacket,
//		uint16_t packetSize) {
//	arrayprinter::print(tcPacket, packetSize);
//}


