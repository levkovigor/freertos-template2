#ifndef SAM9G20_COMIF_COOKIES_I2C_COOKIE_H_
#define SAM9G20_COMIF_COOKIES_I2C_COOKIE_H_

#include <fsfw/devicehandlers/CookieIF.h>
#include <fsfw/devicehandlers/DeviceCommunicationIF.h>
#include <fsfw/osal/freertos/BinarySemaphore.h>

extern "C" {
#include <hal/Drivers/I2C.h>
}

#include "portmacro.h"

/**
 * @brief This enum specifieds how the I2C queue transfer is performed.
 * @details
 * If the exact read size is known beforehand, the write and read
 * parameters can be specified at once and the whole write/read operation
 * is performed consecutively without a separate queue transfer to receive data.
 */
enum I2cCommunicationType: uint8_t {
	UNKNOWN,
	SEPARATE_WRITE_READ,  //!< SEPARATE_WRITE_READ
	CONSECUTIVE_WRITE_READ//!< CONSECUTIVE_WRITE_READ
};

/**
 * @brief   Primary data structure to exchange information between device
 *          handlers and the I2C communication interface
 * @author 	R. Mueller
 * @ingroup comm
 */
class I2cCookie: public CookieIF {
public:
	/**
	 * Costructor for I2C transfers using the queueTransfer with binary semaphore
	 * as a signalling mechanism and separate write and read queue transfers
	 * @param address       7-bit I2C address
	 * @param maxReplyLen
	 * @param i2cComType_
	 * @param
	 */
	I2cCookie(address_t address, size_t maxReplyLen, I2cCommunicationType i2cComType_ =
			I2cCommunicationType::SEPARATE_WRITE_READ);

	virtual ~I2cCookie();

	static constexpr uint8_t SEMAPH_RESET_TRIGGER { 3 };
	bool messageSent = false;
	uint8_t semaphResetCounter { 0 };

	void setI2cComType(I2cCommunicationType newComType);
	I2cCommunicationType getI2cComType() const;

	address_t getAddress() const;
	size_t getMaxReplyLen() const;
	/**
	 * For consecutive transfers, the receive data size can be set with this
	 * setter function
	 */
	void setReceiveDataSize(size_t receiveDataSize_);
	size_t getReceiveDataSize() const;

	void setWriteReadDelay(portTickType delayTicks);
	void setWriteReadDelayMs(uint32_t delayMs);
	portTickType getWriteReadDelay() const;

	I2CgenericTransfer & getI2cGenericTransferStructHandle();

	/**
	 * Get the pointer to a variable storing the current I2C transfer progress
	 * @return Address of the I2C transfer status value
	 */
	I2CtransferStatus & getI2cTransferStatusHandle();

	BinarySemaphore & getSemaphoreObjectHandle();

private:
	address_t logicalAddress = 0;
	size_t maxReplyLen = 0;

	//! Used as a signalling mechanism for completed transfers.
	BinarySemaphore deviceSemaphore;
	I2cCommunicationType i2cComType; //!< Specifies type of communication.

	//! Current status of transfer is stored here (driver transfer state)
	I2CtransferStatus i2cTransferStatus = I2CtransferStatus::done_i2c;
	I2CgenericTransfer i2cTransferStruct;

	void setUpGenericI2cStruct(uint32_t receiveDataSize_ = 0);
};

#endif
