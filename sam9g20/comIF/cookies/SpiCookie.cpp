#include <sam9g20/comIF/cookies/SpiCookie.h>

#include <fsfw/ipc/MutexFactory.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>

SpiCookie::SpiCookie(address_t logicalAddress, size_t maxReplyLen,
		SlaveType slaveType, DemultiplexerOutput demuxOutput,
		SPImode spiMode, uint8_t delayBetweenChars, uint32_t busSpeedHz,
		uint8_t delayBeforeSpck, portTickType postTransferDelay):
		spiAddress(logicalAddress), maxReplyLen(maxReplyLen),
		slaveType(slaveType), demuxOutput(demuxOutput) {
	spiMutex = MutexFactory::instance()->createMutex();
	setupSpiParameters(spiMode, delayBetweenChars, busSpeedHz,
			delayBeforeSpck, postTransferDelay);
}

SpiCookie::~SpiCookie() {
	MutexFactory::instance()->deleteMutex(spiMutex);
}

address_t SpiCookie::getAddress() const{
	return spiAddress;
}

size_t SpiCookie::getMaxReplyLen() const{
	return maxReplyLen;
}

void SpiCookie::setTransferLen(size_t transferLen) {
    this->transferLen = transferLen;
}

size_t SpiCookie::getTransferLen() const {
	return transferLen;
}

ComStatus SpiCookie::getCurrentComStatus() const {
	return comStatus;
}

ReturnValue_t SpiCookie::getErrorReturnValue() const {
	return errorReturnvalue;
}

MutexIF * SpiCookie::getMutexHandle() const {
	return spiMutex;
}

SlaveType SpiCookie::getSlaveType() const {
	return slaveType;
}

/**
 * @brief   Set the current com status.
 * For faulty states, a corrensponding return value can be passed.
 * @param comStatus
 * @param errorResult
 */
void SpiCookie::setCurrentComStatus(ComStatus comStatus,
		ReturnValue_t errorResult) {
	this->comStatus = comStatus;
	setErrorReturnValue(errorResult);
}

void SpiCookie::setErrorReturnValue(ReturnValue_t errorResult) {
	this->errorReturnvalue = errorResult;
}

DemultiplexerOutput SpiCookie::getDemuxOutput() const {
	return demuxOutput;
}

SPIslaveParameters * SpiCookie::getSpiParametersHandle() {
	return &spiParameters;
}

void SpiCookie::setupSpiParameters(SPImode spiMode, uint8_t delayBetweenChars,
		uint32_t busSpeedHz, uint8_t delayBeforeSpck,
		portTickType postTransferDelay) {
	spiParameters.bus = bus1_spi;
	determineSpiSlave(slaveType);
	if(busSpeedHz < SPI_MIN_BUS_SPEED) {
		sif::warning << "SPI Cookie: Specified bus speed to slow, setting to"
				<< " default value " << SPI_DEFAULT_SPEED << std::endl;
		spiParameters.busSpeed_Hz = SPI_DEFAULT_SPEED;
	}
	else if(busSpeedHz > SPI_MAX_BUS_SPEED) {
		sif::warning << "SPI Cookie: Specified bus speed to fast, setting to"
				<< " default value " <<  SPI_DEFAULT_SPEED << std::endl;
		spiParameters.busSpeed_Hz = SPI_DEFAULT_SPEED;
	}
	else {
		spiParameters.busSpeed_Hz = busSpeedHz;
	}

	spiParameters.mode = spiMode;
	spiParameters.dlybct = delayBetweenChars;
	spiParameters.dlybs = delayBeforeSpck;
	spiParameters.postTransferDelay = postTransferDelay;
}

void SpiCookie::determineSpiSlave(SlaveType slaveType) {
	switch(slaveType) {
	case(SlaveType::SLAVE_SELECT_0):
		spiParameters.slave = SPIslave::slave0_spi;
		return;
	case(SlaveType::SLAVE_SELECT_1):
		spiParameters.slave = SPIslave::slave1_spi;
		return;
	case(SlaveType::SLAVE_SELECT_2):
		spiParameters.slave = SPIslave::slave2_spi;
		return;
	case(SlaveType::DEMULTIPLEXER_1):
	case(SlaveType::DEMULTIPLEXER_2):
	case(SlaveType::DEMULTIPLEXER_3):
	case(SlaveType::DEMULTIPLEXER_4):
	    // The slave variable should not matter for demultiplexer devices.
	    // The slave select logic will be performed with GPIO pins.
	    // Still set to valid value because ISIS driver expects this.
	    // A special global boolean variable will be used to only pull the
	    // dummy slave select also used by ISIS (SPI1_NPCS3) low.
	    // Also see SpiDeviceComIF.h
	    spiParameters.slave = SPIslave::slave0_spi;
		return;
	default:
		// The device handler must check for the slave number, we can't / don't
		// want to use exceptions.
		slaveType = SlaveType::UNASSIGNED;
		demuxOutput = DemultiplexerOutput::UNASSIGNED;
		sif::error << "Spi Cookie: Config error, invalid slave "
		        "select!" << std::endl;
	}
}

