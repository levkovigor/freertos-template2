#ifndef SAM9G20_COMIF_I2C_DEVICECOMIF_H_
#define SAM9G20_COMIF_I2C_DEVICECOMIF_H_

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfwconfig/OBSWConfig.h>
#include <sam9g20/comIF/cookies/I2cCookie.h>

#include <unordered_map>
#include <vector>

/**
 * @brief 	Communication interface used to communicate with I2C devices using
 * 			the ISIS drivers
 * @author 	R. Mueller
 */
class I2cDeviceComIF: public DeviceCommunicationIF, public SystemObject {
public:
	I2cDeviceComIF(object_id_t objectId);

	virtual ~I2cDeviceComIF();
	static constexpr uint32_t I2C_BUS_SPEED_HZ = 100000;
	//! Occasinally, bus state is checked
	static constexpr uint8_t BUS_CHECK_TRIGGER = 10;

	/**
	 * Set to portMAXDELAY for debugging, specified as 1/10th of ticks.
	 * Is set to one for values less than 1. Should be specified to
	 * portMAX_DELAY for debugging.
	 */
	static constexpr uint32_t I2C_TRANSFER_TIMEOUT =
			config::I2C_TRANSFER_TIMEOUT;

	static const uint8_t INTERFACE_ID = CLASS_ID::I2C_COM_IF;


	//! Returnvalues by ISIS driver start-up
	enum class I2cInitResult: int {
		I2C_QUEUE_CREATION_FAILURE = -3,
		I2C_PERIPHERAL_INIT_FAILURE = -2,
		I2C_TASK_CREATION_FAILURE = -1,
		RETURN_OK = 0
	};

	//! Returnvalues by ISIS driver queueTransfer initialization
	enum class I2cQueueTransferInitResult: int {
		I2C_INVALID_TRANSFER_PARAMETERS = -2,
		I2C_QUEUE_TRANSFER_FAILED = -1,
		RETURN_OK = 0
	};

	static constexpr ReturnValue_t I2C_DRIVER_ERROR = MAKE_RETURN_CODE(0x01);

	static constexpr ReturnValue_t I2C_QUEUE_CREATION_FAILURE = MAKE_RETURN_CODE(0xA1);
	static constexpr ReturnValue_t I2C_PERIPHERAL_INIT_FAILURE = MAKE_RETURN_CODE(0xA2);
	static constexpr ReturnValue_t I2C_TASK_CREATION_FAILURE = MAKE_RETURN_CODE(0xA3);

	static constexpr ReturnValue_t I2C_INVALID_TRANSFER_PARAMETERS = MAKE_RETURN_CODE(0xB1);
	static constexpr ReturnValue_t I2C_QUEUE_TRANSFER_INIT_FAILURE = MAKE_RETURN_CODE(0xB2);

	static constexpr ReturnValue_t I2C_WRITE_SEMAPHORE_BLOCKED = MAKE_RETURN_CODE(0xC1);
	static constexpr ReturnValue_t I2C_READ_ERROR = MAKE_RETURN_CODE(0xC2);
	static constexpr ReturnValue_t I2C_WRITE_ERROR = MAKE_RETURN_CODE(0xC3);
	static constexpr ReturnValue_t I2C_TRANSFER_GENERAL_ERROR = MAKE_RETURN_CODE(0xC4);
	static constexpr ReturnValue_t I2C_TRANSFER_TIMEOUT_ERROR = MAKE_RETURN_CODE(0xC5);

	static constexpr ReturnValue_t I2C_UNKNOWN_ERROR = MAKE_RETURN_CODE(0xD1);

	static constexpr ReturnValue_t I2C_INVALID_ADDRESS = MAKE_RETURN_CODE(0xE1);
	static constexpr ReturnValue_t I2C_ADDRESS_NOT_IN_RECEIVE_MAP = MAKE_RETURN_CODE(0xE2);

	/**
	 * Called at system object initialization. Sets up the I2C communication
	 * by using the ISIS drivers.
	 * @return
	 */
	ReturnValue_t initialize() override;

	ReturnValue_t initializeInterface(CookieIF * cookie) override;

	/** @brief 		DeviceCommunicationIF overrides
	 *  @details 	Refer to #DeviceCommunicationIF documentation
	 */
	ReturnValue_t sendMessage(CookieIF *cookie,const uint8_t *sendData,
			size_t sendLen) override;
	ReturnValue_t getSendSuccess(CookieIF *cookie) override;
	ReturnValue_t requestReceiveMessage(CookieIF *cookie,
	        size_t requestLen) override;
	ReturnValue_t readReceivedMessage(CookieIF *cookie, uint8_t **buffer,
			size_t *size) override;

	static void restartBus();
private:
	using vectorBuffer = std::vector<uint8_t>;
	using VectorBufferMap = std::unordered_map<address_t, vectorBuffer>;
	using vectorBufferIter = VectorBufferMap::iterator;

	//! This will be the primary data structure to store replies from devices.
	VectorBufferMap i2cVectorMap;
	//! This will hold the current cookie instance passed from DHB
	I2cCookie * i2cCookie = nullptr;

	uint8_t busCheckCounter = 1;

	ReturnValue_t checkAddress(address_t address);

	/**
	 * Request a reply with a specified request length
	 * @param i2cCookie
	 * @param requestLen
	 * @return
	 */
	ReturnValue_t requestReply(I2cCookie * i2cCookie, size_t requestLen);

	/**
	 * Transfer a received reply to the device handler
	 * @param i2cCookie
	 * @param buffer
	 * @param size
	 * @return
	 */
	ReturnValue_t assignReply(I2cCookie * i2cCookie, uint8_t ** buffer,
			size_t * size);

	void prepareI2cConsecutiveTransfer(I2CgenericTransfer& transferStruct,
	        const uint8_t * writeData, uint32_t writeLen);
	ReturnValue_t prepareI2cReadTransfer(address_t slaveAddress,
			I2CgenericTransfer& transferStruct, size_t requestLen);
	void prepareI2cWriteTransfer(I2CgenericTransfer& transferStruct,
			const uint8_t * writeData, size_t writeLen);
	void prepareI2cGenericTransfer(address_t slaveAddress,
			I2CgenericTransfer& transferStruct);

	ReturnValue_t handleI2cTransferInitResult(I2cCookie * comCookie,
	        I2cQueueTransferInitResult driverResult, bool isSendRequest = true);
	ReturnValue_t handleI2cInitError(I2cInitResult result);

	ReturnValue_t checkI2cDriver();

	/**
	 * This callback will be called when a I2C transfer has finished.
	 * @param context
	 * @param semaphore
	 */
	static void I2cCallback(SystemContext context, xSemaphoreHandle semaphore);
};

#endif /* SAM9G20_COMIF_I2C_DEVICECOMIF_H_ */
