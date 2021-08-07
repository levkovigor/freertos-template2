#ifndef SAM9G20_COMIF_COOKIES_SPI_COOKIE_H_
#define SAM9G20_COMIF_COOKIES_SPI_COOKIE_H_

#include <fsfw/devicehandlers/CookieIF.h>
#include <fsfw/ipc/MutexIF.h>
#include <fsfw/osal/freertos/BinarySemaphore.h>

extern "C" {
#include <hal/Drivers/SPI.h>
#include <board.h>
}

#include "portmacro.h"

enum class ComStatus: uint8_t {
	IDLE,
	TRANSFER_INIT_SUCCESS,
	TRANSFER_SUCCESS,
	FAULTY //!< Errors in communication
};

enum class SlaveType: uint8_t {
	UNASSIGNED,
	// Pin Name: SPI_NPCS0
	SLAVE_SELECT_0,
	// Pin Name: SPI_NPCS1
	SLAVE_SELECT_1,
	// Pin Name: SPI_NPCS2
	SLAVE_SELECT_2,
	DEMULTIPLEXER_1,
	DEMULTIPLEXER_2,
	DEMULTIPLEXER_3,
	DEMULTIPLEXER_4,
};

enum class DemultiplexerOutput: uint8_t {
	UNASSIGNED,
	OWN_SLAVE_SELECT,
	OUTPUT_1,
	OUTPUT_2,
	OUTPUT_3,
	OUTPUT_4,
	OUTPUT_5,
	OUTPUT_6,
	OUTPUT_7,
	OUTPUT_8,
};

/**
 * @brief   Primary data structure to exchange information between device
 *          handlers and the SPI communication interface
 * @author	R. Mueller
 * @ingroup comm
 */
class SpiCookie: public CookieIF {
public:
	static constexpr uint32_t SPI_DEFAULT_SPEED =
	        (SPI_MAX_BUS_SPEED + SPI_MIN_BUS_SPEED) / 2;

	SpiCookie(address_t logicalAddress, size_t maxReplyLen,
			SlaveType slaveType, DemultiplexerOutput demuxOutput,
			SPImode spiMode, uint8_t delayBetweenChars = 1,
			uint32_t busSpeedHz = SPI_DEFAULT_SPEED ,
			uint8_t delayBeforeSpck = 1, portTickType postTransferDelay = 0);

	virtual ~SpiCookie();

	address_t getAddress() const;
	size_t getMaxReplyLen() const;

	void setTransferLen(size_t transferLen);
	size_t getTransferLen() const;

	ComStatus getCurrentComStatus() const;
	void setCurrentComStatus(ComStatus comStatus,
			ReturnValue_t errorResult = HasReturnvaluesIF::RETURN_OK);

	ReturnValue_t getErrorReturnValue() const;
	void setErrorReturnValue(ReturnValue_t errorResult);

	MutexIF * getMutexHandle() const;
	SPIslaveParameters * getSpiParametersHandle();

	SlaveType getSlaveType() const;
	DemultiplexerOutput getDemuxOutput() const;

private:
	address_t spiAddress = 0;
	size_t maxReplyLen = 0;

	//! This will be initialized by passing a parameter struct into
	//! the constructor
	SPIslaveParameters spiParameters;

	//! Current Com Status
	ComStatus comStatus = ComStatus::IDLE;

	//! For faulty communication, return value will be stored here
	ReturnValue_t errorReturnvalue = HasReturnvaluesIF::RETURN_FAILED;

	//! Specify whether a single slave select line or a multiplexer is used.
	SlaveType slaveType = SlaveType::UNASSIGNED;
	//! SPI is multiplexed, this variable stores which multiplexer
	DemultiplexerOutput demuxOutput = DemultiplexerOutput::UNASSIGNED;

	//! This mutex protects the assignment in sendMessage()
	MutexIF * spiMutex = nullptr;

	//! This flag is set to true when there is data to send.
	size_t transferLen = 0;

	void setupSpiParameters(SPImode spi_mode, uint8_t delay_between_chars,
			uint32_t bus_speed_hz, uint8_t delay_before_spck,
			portTickType post_transfer_delay);
	void determineSpiSlave(SlaveType slave_type);
};


#endif /* BSP_COMIF_COOKIES_SPI_COOKIE_H_ */
