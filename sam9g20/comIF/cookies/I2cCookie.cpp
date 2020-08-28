#include <sam9g20/comIF/cookies/I2cCookie.h>

I2cCookie::I2cCookie(address_t address, size_t maxReplyLen_,
		I2cCommunicationType i2cComType_): logicalAddress(address),
		maxReplyLen(maxReplyLen_), i2cComType(i2cComType_) {
	setUpGenericI2cStruct();
}

address_t I2cCookie::getAddress() const {
	return logicalAddress;
}

size_t I2cCookie::getMaxReplyLen() const {
	return maxReplyLen;
}

void I2cCookie::setUpGenericI2cStruct(uint32_t receiveDataSize_) {
	i2cTransferStruct.semaphore = deviceSemaphore.getSemaphore();
	i2cTransferStruct.result = &i2cTransferStatus;
	i2cTransferStruct.readSize = receiveDataSize_;
	i2cTransferStruct.readData = nullptr;
	i2cTransferStruct.writeData = nullptr;
}

I2cCookie::~I2cCookie() {}


void I2cCookie::setReceiveDataSize(size_t receiveDataSize_) {
	i2cTransferStruct.readSize = receiveDataSize_;
}

I2cCommunicationType I2cCookie::getI2cComType() const {
	return i2cComType;
}

void I2cCookie::setI2cComType(I2cCommunicationType newComType) {
	i2cComType = newComType;
}

size_t I2cCookie::getReceiveDataSize() const {
	return i2cTransferStruct.readSize;
}

I2CgenericTransfer & I2cCookie::getI2cGenericTransferStructHandle() {
	return i2cTransferStruct;
}

I2CtransferStatus & I2cCookie::getI2cTransferStatusHandle() {
	return i2cTransferStatus;
}

BinarySemaphore & I2cCookie::getSemaphoreObjectHandle() {
	return deviceSemaphore;
}

void I2cCookie::setWriteReadDelay(portTickType delayTicks) {
	i2cTransferStruct.writeReadDelay = delayTicks;
}

void I2cCookie::setWriteReadDelayMs(uint32_t delayMs) {
	portTickType delayTicks = pdMS_TO_TICKS(delayMs);
	i2cTransferStruct.writeReadDelay = delayTicks;
}

portTickType I2cCookie::getWriteReadDelay() const {
	return i2cTransferStruct.writeReadDelay;
}
