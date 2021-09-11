#ifndef SAM9G20_COMIF_SPIDEVICECOMIF_H_
#define SAM9G20_COMIF_SPIDEVICECOMIF_H_

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/devicehandlers/DeviceCommunicationIF.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/osal/freertos/BinarySemaphore.h>
#include <fsfw/osal/freertos/Mutex.h>
#include <OBSWConfig.h>
#include <bsp_sam9g20/comIF/cookies/SpiCookie.h>

extern "C" {
#include <hal/Drivers/SPI.h>
}

#include <map>
#include <vector>

/**
 * @brief 	This interface handles the communication with SPI devices, using
 * 			the ISIS SPI drivers.
 * @author	R. Mueller
 * @ingroup comm
 */
class SpiDeviceComIF: public DeviceCommunicationIF,
		public SystemObject,
		public ExecutableObjectIF
{
public:
	SpiDeviceComIF(object_id_t objectId);
	virtual ~SpiDeviceComIF();

	static constexpr uint8_t NUMBER_OF_SINGLE_SLAVE_SELECTS = 4;
	static constexpr uint8_t NUMBER_OF_DEMULTIPLEXER_OUTPUTS = 8;
	//! Default semaphore timout in milliseconds
	static constexpr TickType_t SPI_STANDARD_SEMAPHORE_TIMEOUT =
	        config::SPI_DEFAULT_TIMEOUT_MS;
	//! Default mutex timeout in milliseconds
	static constexpr TickType_t SPI_STANDARD_MUTEX_TIMEOUT = 20;

	static constexpr uint8_t INTERFACE_ID = CLASS_ID::SPI_COM_IF;
	static constexpr uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::SPI_COM_IF;

	static constexpr Event SPI_START_FAILURE = MAKE_EVENT(0x00, severity::HIGH);
	static constexpr ReturnValue_t SPI_INIT_FAILURE = MAKE_RETURN_CODE(0x01);
	static constexpr ReturnValue_t SPI_COMMUNICATION_TIMEOUT =
	        MAKE_RETURN_CODE(0x02);
	static constexpr ReturnValue_t SPI_BLOCKED_ON_WRITE = MAKE_RETURN_CODE(0x03);

	static constexpr ReturnValue_t SPI_TRANSFER_SEMAPHORE_BLOCKED =
	        MAKE_RETURN_CODE(0xA1);
	static constexpr ReturnValue_t SPI_TRANSFER_INVALID_TRANSFER_STRUCT =
	        MAKE_RETURN_CODE(0xA2);
	static constexpr ReturnValue_t SPI_TRANSFER_INVALID_DRIVER_PARAMS =
	        MAKE_RETURN_CODE(0xA3);
	static constexpr ReturnValue_t SPI_TRANSFER_QUEUE_INIT_FAILURE =
	        MAKE_RETURN_CODE(0xA4);

	static constexpr ReturnValue_t SPI_INVALID_ADDRESS = MAKE_RETURN_CODE(0xE1);

    enum class SpiTransferInitResult {
        INVALID_TRANSFER_STRUCT = -3,
        INVALID_DRIVER_PARAMS = -2,
        QUEUE_INIT_FAILURE = -1,
        RETURN_OK = 0
    };

    /** ExecutableObjectIF overrides */
	ReturnValue_t performOperation(uint8_t opCode) override;
	ReturnValue_t initialize() override;

	/** DeviceCommunicationIF overrides */
	ReturnValue_t initializeInterface(CookieIF * cookie) override;
	ReturnValue_t sendMessage(CookieIF *cookie, const uint8_t * sendData,
			size_t sendLen) override;
	ReturnValue_t getSendSuccess(CookieIF *cookie) override;
	ReturnValue_t requestReceiveMessage(CookieIF *cookie,
	        size_t requestLen) override;
	ReturnValue_t readReceivedMessage(CookieIF *cookie,
			uint8_t **readData, size_t *readLen) override;

	/**
	 * Check whether the SPI address actually exists. Don't forget
	 * to add all SPI devices in the check
	 * @param spiAddress
	 * @return
	 */
	static ReturnValue_t checkAddress(address_t spiAddress);
	BinarySemaphore& getSpiSemaphoreHandle();

private:
	BinarySemaphore spiSemaphore;

	using ReplyBuffer = std::vector<uint8_t>;
	struct SpiStruct {
		//! Default constructor, assigns cookie pointer and intializes vector
		//! with max reply length
		SpiStruct(SpiCookie * spiCookie, uint32_t maxReplyLen):
				spiCookie(spiCookie), spiReplyBuffer(maxReplyLen) {}

		SpiCookie * spiCookie = nullptr;
		//! Reply Buffer is cleared automatically on struct destruction.
		ReplyBuffer spiReplyBuffer;
	};
	using SpiMap = std::map<address_t, SpiStruct>;
	using SpiIterator = SpiMap::iterator;

	//! This will be the primary data structure to store replies and a
	//! pointer to the cookie.
	SpiMap spiMap;

	//! Both buses will be active.
	SPIbus spiBus = SPIbus::both_spi;
	/**
	 * Software slave selects won't be used, instead a GPIO logic
	 * for slave select switching will be implemented with the demultiplexers
	 * of the housekeeping board. Therefore, only the hardware slave selects
	 * of the SPI1 bus will be activated.
	 */
	SPIslave spiSlaves = SPIslave::slave2_spi;

	//! Used for debugging purposes
	uint8_t transferCounter = 0;

	void handleGpioOutputSwitching(DemultiplexerOutput demuxOutput);
	void handleGpioDecoderSelect(SlaveType slave);

	void handleTransfer(bool& spiBoolean, BinarySemaphore& binSemaph);
	void handleTransfer(SpiCookie * spiCookie, bool&spiBoolean,
	        BinarySemaphore& spiSemaph);
	void prepareSpiTransfer(SPItransfer& spiTransfer,
	        SpiIterator spiMapIter, const uint8_t* sendData,
	        const size_t sendDataLen);
	void handleSpiTransferInitResult(SpiCookie * spiCookie,
			SpiTransferInitResult driverResult);

	/* Transfer check helpers */
	void checkTransferResult(SpiCookie * spiCookie,
	        BinarySemaphore& binSemaph);
	/**
	 * This callback will be called when the transfer is completed.
	 * @param context This will always be isr
	 * @param semaphore Semaphore which was assigned on transfer init
	 */
	static void SPIcallback(SystemContext context,
			xSemaphoreHandle semaphore);
};

#endif /* SAM9G20_COMIF_SPIDEVICECOMIF_H_ */
