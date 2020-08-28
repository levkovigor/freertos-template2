/**
 * @file SPI_ComTask.cpp
 * @date 21.02.2020
 * @author R. Mueller
 */

#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/osal/FreeRTOS/TaskManagement.h>
#include <config/devices/logicalAddresses.h>
#include <misc/archive/SpiPollingTask.h>
#include <sam9g20/comIF/GpioDeviceComIF.h>
#include <sam9g20/comIF/SpiDeviceComIF.h>


SpiPollingTask::SpiPollingTask(object_id_t objectId_):
	SystemObject(objectId_)
{
	communicationQueue = nullptr;
//	communicationQueue = QueueFactory::instance()->
//		createMessageQueue(SpiDeviceComIF::SPI_QUEUE_DEPTH,
//			CommunicationMessage::COMMUNICATION_MESSAGE_SIZE);
}

SpiPollingTask::~SpiPollingTask() {
}

ReturnValue_t SpiPollingTask::performOperation(uint8_t operationCode) {
	// Check for communication requests
	receiveSpiMessages();
	// Communicate with all connected SPI devices if a transfer was requested
	performPollingOperation();
	return RETURN_OK;
}

void SpiPollingTask::receiveSpiMessages() {

}

/**
 * Maybe we should request a context switch somewhere?
 * In any case, we have to measure how long this operation takes
 * for a large number of connected devices.
 *
 */
void SpiPollingTask::performPollingOperation() {
	// Each slave select gets an own binary semaphore as a signalling
	// mechanism for successful communication.
	BinarySemaphore semaphoreSS0;
	BinarySemaphore semaphoreSS1;
	BinarySemaphore semaphoreSS2;
	BinarySemaphore semaphoreSS3;
	BinarySemaphore semaphoreSS4Decoder1;
	BinarySemaphore semaphoreSS5Decoder2;
	BinarySemaphore semaphoreSS6Decoder3;
	BinarySemaphore semaphoreSS7Decoder4;

	SemaphorePack semaphorePackSingleSlaveSelect = {&semaphoreSS0, &semaphoreSS1, &semaphoreSS2, &semaphoreSS3};
	SemaphorePack semaphorePackDecoders = {&semaphoreSS4Decoder1, &semaphoreSS5Decoder2,
			&semaphoreSS6Decoder3, &semaphoreSS7Decoder4};

	// Send to devices on own slave select line first
	handleSendRequestBatch(&sendRequestsSingleSlaveSelect, semaphorePackSingleSlaveSelect);
	// Poll Decoder Chip Select 1
	GpioDeviceComIF::enableDecoderOutput1();
	handleSendRequestBatch(&sendRequestsDecOutput1, semaphorePackDecoders);
	// Check the tranfer status for singe slave select devices
	checkTransferResults(&sendRequestsSingleSlaveSelect, semaphorePackSingleSlaveSelect, false);
	// Check the tranfer status for decoder output 1
	checkTransferResults(&sendRequestsDecOutput1, semaphorePackDecoders);

	// Poll Decoder Chip Select 2
	GpioDeviceComIF::enableDecoderOutput2();
	handleSendRequestBatch(&sendRequestsDecOutput2, semaphorePackDecoders);
	// Check the tranfer status for decoder output 2
	checkTransferResults(&sendRequestsDecOutput2, semaphorePackDecoders);

	// Poll Decoder Chip Select 3
	GpioDeviceComIF::enableDecoderOutput3();
	handleSendRequestBatch(&sendRequestsDecOutput3, semaphorePackDecoders);
	// Check the tranfer status for decoder output 3
	checkTransferResults(&sendRequestsDecOutput3, semaphorePackDecoders);

	// Poll Decoder Chip Select 4
	GpioDeviceComIF::enableDecoderOutput4();
	handleSendRequestBatch(&sendRequestsDecOutput4, semaphorePackDecoders);
	// Check the tranfer status for decoder output 4
	checkTransferResults(&sendRequestsDecOutput4, semaphorePackDecoders);

	// Poll Decoder Chip Select 5
	GpioDeviceComIF::enableDecoderOutput5();
	handleSendRequestBatch(&sendRequestsDecOutput5, semaphorePackDecoders);
	// Check the tranfer status for decoder output 5
	checkTransferResults(&sendRequestsDecOutput5, semaphorePackDecoders);

	// Poll Decoder Chip Select 6
	GpioDeviceComIF::enableDecoderOutput6();
	handleSendRequestBatch(&sendRequestsDecOutput6, semaphorePackDecoders);
	// Check the tranfer status for decoder output 6
	checkTransferResults(&sendRequestsDecOutput6, semaphorePackDecoders);

	// Poll Decoder Chip Select 7
	GpioDeviceComIF::enableDecoderOutput7();
	handleSendRequestBatch(&sendRequestsDecOutput7, semaphorePackDecoders);
	// Check the tranfer status for decoder output 7
	checkTransferResults(&sendRequestsDecOutput7, semaphorePackDecoders);

	// Poll Decoder Chip Select 8
	GpioDeviceComIF::enableDecoderOutput8();
	handleSendRequestBatch(&sendRequestsDecOutput8, semaphorePackDecoders);
	// Check the tranfer status for decoder output 8
	checkTransferResults(&sendRequestsDecOutput8, semaphorePackDecoders);
}

void SpiPollingTask::handleSendRequestBatch(SendRequestBatch * sendRequests, SemaphorePack semaphPack) {
	if(*sendRequests->sendRequested1) {
		handleTransfer(sendRequests->sendRequested1, semaphPack.semaph1);
	}
	if(*sendRequests->sendRequested2) {
		handleTransfer(sendRequests->sendRequested2, semaphPack.semaph2);
	}
	if(*sendRequests->sendRequested3) {
		handleTransfer(sendRequests->sendRequested3, semaphPack.semaph3);
	}
	if(*sendRequests->sendRequested4) {
		handleTransfer(sendRequests->sendRequested4, semaphPack.semaph4);
	}
}

void SpiPollingTask::handleTransfer(bool * sendRequestPointer,
		BinarySemaphore * binSemaphore) {
	SpiInfo spiInfo = sendRequestMap[sendRequestPointer];
	SPItransfer spiTransfer;
	prepareSpiTransfer(&spiTransfer, &spiInfo, binSemaphore, spiInfo.slaveParameter);
	SPI_queueTransfer(&spiTransfer);
}

void SpiPollingTask::prepareSpiTransfer(SPItransfer * spiTransfer, SpiInfo *spiInfo,
		BinarySemaphore *binSemaphore, SPIslaveParameters *slaveParams) {
	spiTransfer->callback = SPIcallback;
	spiTransfer->readData = readBuffer + spiInfo->readBufferPosition;
	spiTransfer->writeData = const_cast<uint8_t *>(spiInfo->sendData);
	spiTransfer->transferSize = spiInfo->sendDataSize;
	spiTransfer->semaphore = binSemaphore->getSemaphore();
	spiTransfer->slaveParams = &slave0Params;
}

void SpiPollingTask::checkTransferResults(SendRequestBatch *sendRequests,
		SemaphorePack semaphPack, bool areDecoderDevices) {
	checkTransferResult(sendRequests->sendRequested1, semaphPack.semaph1, false);
	checkTransferResult(sendRequests->sendRequested2, semaphPack.semaph2, false);
	checkTransferResult(sendRequests->sendRequested3, semaphPack.semaph3, false);
	checkTransferResult(sendRequests->sendRequested4, semaphPack.semaph4, false);
}

void SpiPollingTask::checkTransferResult(bool * sendRequestBool,
		BinarySemaphore * binSemaph, bool isDecoderDevice) {
	// Try to take semaphore, blocks task if transfer has not finished yet
	// The semaphore is given back by the callback
	ReturnValue_t result = binSemaph->takeBinarySemaphoreTickTimeout(SPI_STANDARD_SEMAPHORE_TIMEOUT);
	if(result == BinarySemaphore::SEMAPHORE_TIMEOUT) {
		// somethings wrong, report error.
		// Some semaphores are used multiple times
		// I don't know if we can do it like that.
		if(isDecoderDevice) {
			if(binSemaph != NULL) {
				// The semaphore is used multiple times for decoder devices,
				// so it needs to be reset (deletes old one and creates new one)
				binSemaph->resetSemaphore();
			}
		}
	}
	else if(result == BinarySemaphore::SEMAPHORE_NOT_FOUND) {
		// config error
		error << "SPI Com Task: Configuration error, semaphore not found !";
	}
	else {
		SpiMessage spiMessage;
		*sendRequestBool = false;
		SpiInfo spiInfo = sendRequestMap[sendRequestBool];
		// don't really like this, have to copy data twice.
		// will propably switch to one read buffer here,
		// supplying only the pointer.
		if(spiInfo.storageType == ComType::RAW_RTD) {
			prepareSpiRtdMessage(&spiMessage, &spiInfo);

		}
		communicationQueue->reply(&spiMessage);
		binSemaph->giveBinarySemaphore();
	}
}

void SpiPollingTask::prepareSpiRtdMessage(SpiMessage * spiMessage, SpiInfo * spiInfo) {
	rtd_t rtdValue;
	spiMessage->setDataReplyRaw(spiInfo->logicalAddress,
			spiInfo->sendDataSize, spiInfo->readBufferPosition);
	memcpy(&rtdValue,readBuffer + spiInfo->readBufferPosition,sizeof(rtdValue));
	spiMessage->setUint32Data(rtdValue);
	spiMessage->setMessageType(CommunicationMessage::REPLY_DATA_RAW);
	// we propably wont have a temp sensor on an own chip select line later
	spiMessage->setMessageId(MESSAGE_TYPE::SPI_RTD);
}

ReturnValue_t SpiPollingTask::initialize() {
	slave0Params = {SPI_BUS1,mode0_spi,slave0_spi,SPI_DELAY_CS_LOW_TO_CLOCK,
			SPI_DELAY_CONSECUTIVE_BYTES, SPI_BUS_SPEED, SPI_POST_TRANSFER_DELAY};
	slave1Params = {SPI_BUS1,mode0_spi,slave1_spi,SPI_DELAY_CS_LOW_TO_CLOCK,
			SPI_DELAY_CONSECUTIVE_BYTES, SPI_BUS_SPEED, SPI_POST_TRANSFER_DELAY};
	slave2Params = {SPI_BUS1,mode0_spi,slave2_spi,SPI_DELAY_CS_LOW_TO_CLOCK,
			SPI_DELAY_CONSECUTIVE_BYTES, SPI_BUS_SPEED, SPI_POST_TRANSFER_DELAY};
	slave3Params = {SPI_BUS1,mode0_spi,slave3_spi,SPI_DELAY_CS_LOW_TO_CLOCK,
			SPI_DELAY_CONSECUTIVE_BYTES, SPI_BUS_SPEED, SPI_POST_TRANSFER_DELAY};
	slavePt1000Params = {SPI_BUS1,mode0_spi,slave4_spi,SPI_DELAY_CS_LOW_TO_CLOCK,
			SPI_DELAY_CONSECUTIVE_BYTES, SPI_BUS_SPEED, SPI_POST_TRANSFER_DELAY};
	// we propably should do all of this in a separate config function
	// or at least use an initializer function
	SpiInfo requestInfo;
	requestInfo.readBufferPosition = 0;
	requestInfo.slaveParameter = &slave0Params;
	requestInfo.logicalAddress = addresses::SPI_DLR_IRAS;
	sendRequestMap.insert(SendRequestEntry(&SPI_SS0_SendRequest, requestInfo));

	requestInfo.readBufferPosition = 20;
	requestInfo.slaveParameter = &slave1Params;
	requestInfo.logicalAddress = addresses::SPI_DLR_PVCH;
	sendRequestMap.insert(SendRequestEntry(&SPI_SS1_SendRequest, requestInfo));
	return RETURN_OK;
}

void SpiPollingTask::SPIcallback(SystemContext context,
		xSemaphoreHandle semaphore) {
	BaseType_t higherPriorityTaskAwoken = pdFALSE;
	ReturnValue_t result;
	//TRACE_INFO("SPI Callback reached\n\r");
	if(context == SystemContext::task_context) {
		result = BinarySemaphore::giveBinarySemaphore(semaphore);
	}
	else {
		result = BinarySemaphore::giveBinarySemaphoreFromISR(semaphore,
		        &higherPriorityTaskAwoken);
		if(higherPriorityTaskAwoken == pdPASS) {
		    SpiPollingTask::higherTaskUnlockedCounter++;
			//TRACE_INFO("SPI Test: Higher Priority Task awoken !");
		}
	}
	if(result != RETURN_OK) {
		TRACE_ERROR("SPI ComIF: Error in SPI Callback!");
	}
}
