#include "I2cDeviceComIF.h"
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/osal/FreeRTOS/TaskManagement.h>
#include <fsfwconfig/devices/logicalAddresses.h>

extern "C" {
#include <at91/utility/trace.h>
#include <hal/Drivers/I2C.h>
}

#include "ComConstants.h"

I2cDeviceComIF::I2cDeviceComIF(object_id_t objectId): SystemObject(objectId),
		i2cCookie(nullptr) {
}

ReturnValue_t I2cDeviceComIF::initialize() {
	setTrace(3);
	int startResult = I2C_start(I2C_BUS_SPEED_HZ, I2C_TRANSFER_TIMEOUT);
	setTrace(5);
	if(startResult != RETURN_OK) {
		ReturnValue_t result = handleI2cInitError(
		        static_cast<I2cInitResult>(startResult));
		return result;
	}
	return RETURN_OK;
}

I2cDeviceComIF::~I2cDeviceComIF() {}

ReturnValue_t I2cDeviceComIF::initializeInterface(CookieIF * cookie) {
	if(cookie == nullptr) {
		return NULLPOINTER;
	}
	i2cCookie = dynamic_cast<I2cCookie*>(cookie);
	if(i2cCookie == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
	    sif::error << "I2cDeviceComIF:: Passed Cookie is not a I2C Cookie!" << std::endl;
#else
	    sif::printError("I2cDeviceComIF:: Passed Cookie is not a I2C Cookie!\n");
#endif
	    return NULLPOINTER;
	}

	address_t i2cAddress = i2cCookie->getAddress();
	if(ReturnValue_t result = checkAddress(i2cAddress); result != RETURN_OK) {
		return result;
	}

	if(auto i2cIter = i2cVectorMap.find(i2cAddress);
	        i2cIter == i2cVectorMap.end()) {
		i2cVectorMap.emplace(i2cAddress, vectorBuffer(i2cCookie->getMaxReplyLen()));
	}
	else {
		// Already in map. readjust size
		vectorBuffer & existingVector = i2cIter->second;
		existingVector.resize(i2cCookie->getMaxReplyLen());
		existingVector.shrink_to_fit();
	}

	return RETURN_OK;
}

ReturnValue_t I2cDeviceComIF::checkAddress(address_t address) {
	switch(address) {
	case(addresses::I2C_ARDUINO_0):
	case(addresses::I2C_ARDUINO_1):
	case(addresses::I2C_ARDUINO_2):
	case(addresses::I2C_ARDUINO_3):
	case(addresses::I2C_ARDUINO_4):
		return RETURN_OK;
	default:
		return I2C_INVALID_ADDRESS;
	}
}

ReturnValue_t I2cDeviceComIF::sendMessage(CookieIF *cookie,
        const uint8_t *sendData, size_t sendLen) {
	if(ReturnValue_t result = checkI2cDriver(); result != RETURN_OK) {
		return result;
	}

	if(sendData == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
	    sif::warning << "I2cDeviceComIF::sendMessage: Send data is nullptr" << std::endl;
#else
	    sif::printWarning("I2cDeviceComIF::sendMessage: Send data is nullptr\n");
#endif
	    return RETURN_FAILED;
	}

	if(sendLen == 0) {
		return RETURN_OK;
	}

	i2cCookie = dynamic_cast<I2cCookie*>(cookie);
	I2CgenericTransfer& i2cTransfer = i2cCookie->getI2cGenericTransferStructHandle();
	if(i2cCookie->getI2cComType() == I2cCommunicationType::CONSECUTIVE_WRITE_READ) {
		// This function prepares the generic i2cTransfer structure
		prepareI2cConsecutiveTransfer(i2cTransfer, sendData, sendLen);
	}
	else {
		prepareI2cWriteTransfer(i2cTransfer, sendData, sendLen);
	}

	// Take the semaphore, will be released by the callback.
	BinarySemaphore & binSemaph = i2cCookie->getSemaphoreObjectHandle();
	// We could reset the semaphore. However, one semaphore is used
	// for all communication steps, so we really should not do this...
	ReturnValue_t result = binSemaph.acquire();
	// If this happens, the semaphore might still be blocked from the
	// last transfer, the device is not connected or there is another issue
	if(result != RETURN_OK) {
		// Maybe we should give back the semaphore after some counters
		// but if this happens, generally there is something wrong with I2C
		// TODO(Robin): Talk with steffen about this.
		if(i2cCookie->semaphResetCounter == i2cCookie->SEMAPH_RESET_TRIGGER) {
			binSemaph.release();
			i2cCookie->semaphResetCounter = 1;
		} else {
			i2cCookie->semaphResetCounter ++;
			return I2C_WRITE_SEMAPHORE_BLOCKED;
		}
	}

	int transferInitResult = I2C_queueTransfer(&i2cTransfer);
	return handleI2cTransferInitResult(i2cCookie,
	        static_cast<I2cQueueTransferInitResult>(transferInitResult));
}

ReturnValue_t I2cDeviceComIF::getSendSuccess(CookieIF *cookie) {
	// Transfer status is checked in requestReceiveMessage. Less overhead.
	return RETURN_OK;
}

ReturnValue_t I2cDeviceComIF::requestReceiveMessage(CookieIF *cookie,
		size_t requestLen) {
	// For separate send/read: Request read here
	i2cCookie = dynamic_cast<I2cCookie*>(cookie);
	i2cCookie->setReceiveDataSize(requestLen);

	if(requestLen == 0 or i2cCookie->getI2cComType() ==
			I2cCommunicationType::CONSECUTIVE_WRITE_READ)
	{
		return RETURN_OK;
	}

	// check transfer status.
	I2CtransferStatus transferStatus = i2cCookie->getI2cTransferStatusHandle();
	switch(transferStatus) {
	case(I2CtransferStatus::done_i2c): {
		return requestReply(i2cCookie, requestLen);
	}

	case(I2CtransferStatus::pending_i2c):
		return RETURN_OK;
	case(I2CtransferStatus::readError_i2c):
		return I2C_READ_ERROR;
	case(I2CtransferStatus::writeError_i2c):
		return I2C_WRITE_ERROR;
	case(I2CtransferStatus::error_i2c):
		return I2C_TRANSFER_GENERAL_ERROR;
	case(I2CtransferStatus::timeoutError_i2c):
		return I2C_TRANSFER_TIMEOUT_ERROR;
	// This should never happen for separate read/write calls.
	case(I2CtransferStatus::writeDoneReadStarted_i2c):
	case(I2CtransferStatus::writeDone_i2c):
	default:
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "I2CDeviceComIF: Configuration error reading message with "
		        "error code "  << transferStatus << std::endl;
#else
		sif::printError("I2CDeviceComIF: Configuration error reading message with "
                "error code %d\n", transferStatus);
#endif
		return PROTOCOL_ERROR;
	}
	return RETURN_OK;
}

ReturnValue_t I2cDeviceComIF::requestReply(I2cCookie * i2cCookie,
		size_t requestLen) {
	address_t slaveAddress = i2cCookie->getAddress();
	I2CgenericTransfer & i2cTransfer =
			i2cCookie->getI2cGenericTransferStructHandle();
	ReturnValue_t result =
			prepareI2cReadTransfer(slaveAddress, i2cTransfer, requestLen);
	if(result != RETURN_OK) {
		return result;
	}
	// Take the semaphore, should have been released by the callback.
	result = i2cCookie->getSemaphoreObjectHandle().acquire(
	        SemaphoreIF::TimeoutType::WAITING, 10);
	if(result == SemaphoreIF::SEMAPHORE_TIMEOUT) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::warning << "I2cDeviceComIF::requestReply: Possible configuration"
				"error, semaphore timeout!" << std::endl;
#else
		sif::printWarning("I2cDeviceComIF::requestReply: Possible configuration"
                "error, semaphore timeout!\n");
#endif
		return result;
	}
	int transferInitResult = I2C_queueTransfer(&i2cTransfer);
	return handleI2cTransferInitResult(i2cCookie,
	        static_cast<I2cQueueTransferInitResult>(transferInitResult), false);
}

ReturnValue_t I2cDeviceComIF::readReceivedMessage(CookieIF *cookie,
		uint8_t **buffer, size_t* size) {
	*size = 0;
	i2cCookie = dynamic_cast<I2cCookie*>(cookie);
	I2CtransferStatus & transferStatus = i2cCookie->getI2cTransferStatusHandle();

	switch(transferStatus) {
	case(I2CtransferStatus::done_i2c): {
		return assignReply(i2cCookie, buffer, size);
	}
	case(I2CtransferStatus::writeDone_i2c):
	case(I2CtransferStatus::pending_i2c):
	case(I2CtransferStatus::writeDoneReadStarted_i2c):
		return DeviceCommunicationIF::NO_REPLY_RECEIVED;
	case(I2CtransferStatus::writeError_i2c):
		return I2C_WRITE_ERROR;
	case(I2CtransferStatus::readError_i2c):
		return I2C_READ_ERROR;
	case(I2CtransferStatus::error_i2c):
		return I2C_TRANSFER_GENERAL_ERROR;
	case(I2CtransferStatus::timeoutError_i2c):
		return I2C_TRANSFER_TIMEOUT_ERROR;
	default:
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::warning << "I2CDeviceComIF: Configuration Error Reading message with "
				"error code "  << transferStatus << std::endl;
#else
		sif::printWarning("I2CDeviceComIF: Configuration Error Reading message with "
                "error code %d\n", transferStatus);
#endif
		return PROTOCOL_ERROR;
	}
}

ReturnValue_t I2cDeviceComIF::assignReply(I2cCookie * i2cCookie,
		uint8_t** buffer, size_t * size) {
	address_t slaveAddress = i2cCookie->getAddress();
	BinarySemaphore & binSemaph = i2cCookie->getSemaphoreObjectHandle();
	// This should never happen if the transfer result is success.
	ReturnValue_t result = binSemaph.acquire();
	if(result != RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::warning << "I2cDeviceComIF::assignReply:"
				" Configuration Error, taking semaphore !" << std::endl;
#else
		sif::printWarning("I2cDeviceComIF::assignReply: Configuration Error, taking semaphore!\n");
#endif
		return result;
	}
	if(result = binSemaph.release(); result != RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "I2cDeviceComIF: Configuration Error, "
				"giving semaphore!" << std::endl;
#else
		sif::printError("I2cDeviceComIF: Configuration error, giving semaphore!\n");
#endif
		return result;
	}
	*buffer = i2cVectorMap[slaveAddress].data();
	*size = i2cCookie->getReceiveDataSize();
	i2cCookie->setReceiveDataSize(0);
	return result;
}

void I2cDeviceComIF::prepareI2cConsecutiveTransfer(I2CgenericTransfer& transferStruct,
		const uint8_t * writeData, uint32_t writeLen) {
	// somewhere here, we prepare and initiate a queue transfer
	address_t slaveAddress = i2cCookie->getAddress();
	prepareI2cGenericTransfer(slaveAddress, transferStruct);
	transferStruct.direction = I2Cdirection::writeRead_i2cDir;
	transferStruct.writeData = const_cast<uint8_t *>(writeData);
	transferStruct.writeSize = writeLen;
	transferStruct.writeReadDelay = i2cCookie->getWriteReadDelay();
	// read size is set by device handler beforehand !
	i2cVectorMap[slaveAddress].reserve(32);
	transferStruct.readData = i2cVectorMap[slaveAddress].data();
	transferStruct.readSize = i2cCookie->getReceiveDataSize();

}

void I2cDeviceComIF::prepareI2cWriteTransfer(I2CgenericTransfer& transferStruct,
		const uint8_t * writeData, size_t writeLen) {
	prepareI2cGenericTransfer(i2cCookie->getAddress(), transferStruct);
	transferStruct.direction = I2Cdirection::write_i2cDir;
	transferStruct.writeData = const_cast<uint8_t *>(writeData);
	transferStruct.writeSize = writeLen;

}

ReturnValue_t I2cDeviceComIF::prepareI2cReadTransfer(address_t slaveAddress,
		I2CgenericTransfer& transferStruct, size_t requestLen) {
	prepareI2cGenericTransfer(slaveAddress, transferStruct);
	transferStruct.direction = I2Cdirection::read_i2cDir;
	vectorBufferIter iter = i2cVectorMap.find(slaveAddress);
	if(iter == i2cVectorMap.end()) {
		return I2C_ADDRESS_NOT_IN_RECEIVE_MAP;
	}
	// The read buffer and size needs to be set by the device handler beforehand !
	transferStruct.readData = i2cVectorMap[slaveAddress].data();
	transferStruct.readSize = requestLen;
	return RETURN_OK;
}

void I2cDeviceComIF::prepareI2cGenericTransfer(address_t slaveAddress,
		I2CgenericTransfer& transferStruct) {
	transferStruct.slaveAddress = slaveAddress;
	// The semaphore will be taken during the transfer and given back by the callback
	// when the transfer has finished
	BinarySemaphore & deviceSemaphore = i2cCookie->getSemaphoreObjectHandle();
	transferStruct.semaphore = deviceSemaphore.getSemaphore();
	transferStruct.callback = I2cCallback;
}

ReturnValue_t I2cDeviceComIF::handleI2cInitError(I2cInitResult result) {
	if(result == I2cInitResult::I2C_PERIPHERAL_INIT_FAILURE) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "I2CDeviceComIF: Peripheral Initialization Failure!" << std::endl;
#else
		sif::printError("I2CDeviceComIF: Peripheral Initialization Failure!\n");
#endif
		return I2cDeviceComIF::I2C_PERIPHERAL_INIT_FAILURE;
	}
	else if(result == I2cInitResult::I2C_QUEUE_CREATION_FAILURE) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "I2CDeviceComIF: Queue Creation Failure!" << std::endl;
#else
		sif::printError("I2CDeviceComIF: Queue Creation Failure!\n");
#endif
		return I2cDeviceComIF::I2C_QUEUE_CREATION_FAILURE;
	}
	else if(result == I2cInitResult::I2C_TASK_CREATION_FAILURE) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "I2CDeviceComIF: Queue Creation Failure!" << std::endl;
#else
		sif::printError("I2CDeviceComIF: Queue Creation Failure!\n");
#endif
		return I2cDeviceComIF::I2C_TASK_CREATION_FAILURE;
	}
	else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "I2CDeviceComIF: Unknown error has occured!\n" << std::endl;
#else
		sif::printError("I2CDeviceComIF: Unknown error has occured!\n");
#endif

		return RETURN_FAILED;
	}
}

ReturnValue_t I2cDeviceComIF::handleI2cTransferInitResult(I2cCookie * comCookie,
		I2cQueueTransferInitResult driverResult, bool isSendRequest) {
	if(driverResult == I2cQueueTransferInitResult::I2C_INVALID_TRANSFER_PARAMETERS) {
		return I2cDeviceComIF::I2C_INVALID_TRANSFER_PARAMETERS;
	}
	else if(driverResult == I2cQueueTransferInitResult::I2C_QUEUE_TRANSFER_FAILED) {
		return I2cDeviceComIF::I2C_QUEUE_TRANSFER_INIT_FAILURE;
	}
	else if (driverResult == I2cQueueTransferInitResult::I2C_RETURN_OK){
		comCookie->messageSent = isSendRequest ? true : false;
		return RETURN_OK;
	}
	else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "I2CDeviceComIF: Unknown queue transfer init result!" << std::endl;
#else
		sif::printError("I2CDeviceComIF: Unknown queue transfer init result!\n");
#endif
		return RETURN_FAILED;
	}
}

ReturnValue_t I2cDeviceComIF::checkI2cDriver() {
	busCheckCounter ++;
	if(busCheckCounter == BUS_CHECK_TRIGGER) {
		I2CdriverState i2cDriverState = I2C_getDriverState();
		if(i2cDriverState == error_i2cState or
				i2cDriverState == uninitialized_i2cState) {
			triggerEvent(comconstants::I2C_DRIVER_ERROR_EVENT, 0, 0);
			// Restart driver.
			I2C_stop();
			I2C_start(I2C_BUS_SPEED_HZ, I2C_TRANSFER_TIMEOUT);
			return I2C_DRIVER_ERROR;
		}
		busCheckCounter = 1;
	}
	return RETURN_OK;
}

void I2cDeviceComIF::I2cCallback(SystemContext context,
		xSemaphoreHandle semaphore) {
	ReturnValue_t result;
	BaseType_t higherPriorityTaskAwoken = pdFALSE;
	// Give back the semaphore that was taken before initiating the transfer
	if(context == SystemContext::task_context) {
		result = BinarySemaphore::release(semaphore);
	}
	else {
		result = BinarySemaphore::releaseFromISR(semaphore,
				&higherPriorityTaskAwoken);
	}
	if(result != RETURN_OK) {
	    // This should never happen
		TRACE_ERROR("I2cDeviceComIF: Callback error!");
	}
	if(higherPriorityTaskAwoken == pdTRUE) {
		TaskManagement::requestContextSwitch(CallContext::ISR);
	}
}

// Use with care!
void I2cDeviceComIF::restartBus() {
	// Restart driver.
	I2C_stop();
	I2C_start(I2C_BUS_SPEED_HZ, I2C_TRANSFER_TIMEOUT);
}

