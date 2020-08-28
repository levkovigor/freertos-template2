#include <logicalAddresses.h>

#include <sam9g20/comIF/SpiDeviceComIF.h>
#include <sam9g20/comIF/GpioDeviceComIF.h>

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/ipc/MutexHelper.h>
#include <fsfw/osal/FreeRTOS/Mutex.h>
#include <fsfw/timemanager/Stopwatch.h>
#include <fsfw/osal/FreeRTOS/TaskManagement.h>

extern "C" {
#include <utility/trace.h>
#include <peripherals/spi/spi_at91.h>
}

SpiDeviceComIF::SpiDeviceComIF(object_id_t objectId): SystemObject(objectId) {
}

SpiDeviceComIF::~SpiDeviceComIF() {}

ReturnValue_t SpiDeviceComIF::initialize() {
	ReturnValue_t result = SPI_start(spiBus,spiSlaves);
	if(result != RETURN_OK) {
		// TODO: we should also trigger an event here and maybe we should
	    // attempt restarts later.
		return SPI_INIT_FAILURE;
	}
	return RETURN_OK;
}

ReturnValue_t SpiDeviceComIF::checkAddress(address_t spiAddress) {
    switch(spiAddress) {
    // todo: add all designated spi addresses.
    case(addresses::SPI_Test_PT1000):
    case(addresses::SPI_Test_Gyro):
    case(addresses::SPI_ARDUINO_0):
    case(addresses::SPI_ARDUINO_1):
    case(addresses::SPI_ARDUINO_2):
    case(addresses::SPI_ARDUINO_3):
    case(addresses::SPI_ARDUINO_4):
        return RETURN_OK;
    default:
        return SPI_INVALID_ADDRESS;
    }
}

ReturnValue_t SpiDeviceComIF::initializeInterface(CookieIF *cookie)  {
	if(cookie == nullptr) {
		return NULLPOINTER;
	}

	SpiCookie * spiCookie = dynamic_cast<SpiCookie*>(cookie);
	address_t spiAddress = spiCookie->getAddress();
	if(ReturnValue_t result = checkAddress(spiAddress); result != RETURN_OK) {
		return result;
	}

	// Add to reply and cookie map.
	if(auto replyIter = spiMap.find(spiCookie->getAddress());
			replyIter == spiMap.end())
	{
		spiMap.emplace(spiCookie->getAddress(),
				SpiStruct(spiCookie, spiCookie->getMaxReplyLen()));
	}
	else {
		// This should be avoided, we want to avoid dynamic memory allocation
		// after all. This should not kill us if it happens though :-)
		// Already in map. readjust size and reassign supplied cookie.
		ReplyBuffer & existingReplyBuffer = replyIter->second.spiReplyBuffer;
		existingReplyBuffer.resize(spiCookie->getMaxReplyLen());
		existingReplyBuffer.shrink_to_fit();
		replyIter->second.spiCookie = spiCookie;
	}

	return RETURN_OK;
}

ReturnValue_t SpiDeviceComIF::performOperation(uint8_t operationCode) {
	// Could be used to check the bus health (and reset the bus if needed).
	return RETURN_OK;
}


ReturnValue_t SpiDeviceComIF::sendMessage(CookieIF *cookie,
		const uint8_t * sendData, size_t sendLen) {
	SpiCookie * spiCookie = dynamic_cast<SpiCookie *> (cookie);
	address_t spiAddress = spiCookie->getAddress();
	auto spiMapIter = spiMap.find(spiAddress);
	if(spiMapIter == spiMap.end()) {
	    return HasReturnvaluesIF::RETURN_FAILED;
	}

	ReturnValue_t result = spiSemaphore.acquire(
	        SemaphoreIF::TimeoutType::WAITING, 20);
	if(result != HasReturnvaluesIF::RETURN_OK) {
	    sif::warning << "SpiDeviceComIF::sendMessage: Semaphore unavailable "
	            "for long time!" << std::endl;
	}

	SPItransfer spiTransfer;
	prepareSpiTransfer(spiTransfer, spiMapIter, sendData, sendLen);
	// now we have to execute the GPIO logic.
	handleGpioDecoderSelect(spiCookie->getSlaveType());
	handleGpioOutputSwitching(spiCookie->getDemuxOutput());

	int driverResult = SPI_queueTransfer(&spiTransfer);
	handleSpiTransferInitResult(spiCookie,
	        static_cast<SpiTransferInitResult>(driverResult));

	return RETURN_OK;
}

void SpiDeviceComIF::prepareSpiTransfer(SPItransfer& spiTransfer,
        SpiIterator spiMapIter, const uint8_t* sendData,
        const size_t sendDataLen) {
    spiTransfer.callback = SPIcallback;
    spiTransfer.readData = spiMapIter->second.spiReplyBuffer.data();
    // Assign data cached and set in cookie.
    // Ugly cast, ISIS drivers want a non-const buffer for some reason.
    spiTransfer.writeData = const_cast<uint8_t*>(sendData);
    spiTransfer.transferSize = sendDataLen;
    spiMapIter->second.spiCookie->setTransferLen(sendDataLen);
    spiTransfer.semaphore = spiSemaphore.getSemaphore();
    spiTransfer.slaveParams = spiMapIter->
            second.spiCookie->getSpiParametersHandle();
}

ReturnValue_t SpiDeviceComIF::getSendSuccess(CookieIF *cookie) {
	SpiCookie * spiCookie = dynamic_cast<SpiCookie *> (cookie);
	uint32_t spiAddress = spiCookie->getAddress();
	auto spi_iter = spiMap.find(spiAddress);
	if(spi_iter == spiMap.end()) {
		sif::warning << "SPI ComIF: Invalid logical address " << std::hex
				<< spiCookie->getAddress() << std::endl;
		return RETURN_FAILED;
	}
	// Com Status has mutex protection
	// MutexHelper mutexLock(spi_cookie->getMutexHandle(), MutexIF::BLOCKING);
	if(spiCookie->getCurrentComStatus() == ComStatus::FAULTY) {
		return spiCookie->getErrorReturnValue();
	}
	else {
		return RETURN_OK;
	}
}

ReturnValue_t SpiDeviceComIF::requestReceiveMessage(CookieIF *cookie,
		size_t requestLen) {
	// SPI is full duplex, so a send operation will always be a read operation
	// too as bytes are transmitted in both directions.
	return RETURN_OK;
}

ReturnValue_t SpiDeviceComIF::readReceivedMessage(CookieIF *cookie,
		uint8_t** buffer, size_t* size) {
	SpiCookie* spiCookie = dynamic_cast<SpiCookie*> (cookie);
	uint32_t spiAddress = spiCookie->getAddress();

	auto spiIter = spiMap.find(spiAddress);
	if(spiIter == spiMap.end()) {
		sif::warning << "SPI ComIF: Invalid logical address " << std::hex
				<< spiCookie->getAddress() << std::endl;
		return RETURN_FAILED;
	}

	checkTransferResult(spiCookie, spiSemaphore);
	// Com Status has mutex protection
	//MutexHelper mutexLock(spiCookie->getMutexHandle(), 30);
	if(spiCookie->getCurrentComStatus() == ComStatus::TRANSFER_SUCCESS) {
		*buffer = spiIter->second.spiReplyBuffer.data();
		*size = spiCookie->getTransferLen();
		return RETURN_OK;
	}
	else if(spiCookie->getCurrentComStatus() ==
	        ComStatus::TRANSFER_INIT_SUCCESS) {
		*size = 0;
		return NO_REPLY_RECEIVED;
	}
	else {
		return spiCookie->getErrorReturnValue();
	}
}


void SpiDeviceComIF::checkTransferResult(SpiCookie* spiCookie,
        BinarySemaphore& binSemaph) {
    // Try to take semaphore, blocks task if transfer has not finished yet
    // The semaphore is given back by the callback
    ReturnValue_t result = binSemaph.acquireWithTickTimeout(
            SemaphoreIF::TimeoutType::WAITING,
            SPI_STANDARD_SEMAPHORE_TIMEOUT);
    if(result == BinarySemaphore::SEMAPHORE_TIMEOUT) {
        // DON'T IGNORE THIS WARNING!!!
        // If this occurs, consider making the communication timeout
        // number higher or consider setting individual times in the cookies
        // We have to draw the line between devices that just take a long
        // time and faulty communication.
        sif::warning << "Spi ComIF: After waiting for " <<
                SPI_STANDARD_SEMAPHORE_TIMEOUT << " ticks, the transfer "
                "was not completed!" << std::endl;
        result = SPI_COMMUNICATION_TIMEOUT;
    }
    else if(result != RETURN_OK) {
        sif::error << "Spi ComIF: Configuration error when taking semaphore!"
                << std::endl;
    }

    if(result != RETURN_OK) {
        spiCookie->setCurrentComStatus(ComStatus::FAULTY, result);
    }
    else {
        spiCookie->setCurrentComStatus(ComStatus::TRANSFER_SUCCESS);

    }
    binSemaph.release();
}


// Be careful with ostreams here, those can lead to crashes!
void SpiDeviceComIF::SPIcallback(SystemContext context,
        xSemaphoreHandle semaphore) {
    GpioDeviceComIF::disableDecoders();
    BaseType_t higherPriorityTaskAwoken = pdFALSE;
    ReturnValue_t result;
    //TRACE_INFO("SPI Callback reached\n\r");
    if(context == SystemContext::task_context) {
        result = BinarySemaphore::release(semaphore);
    }
    else {
        result = BinarySemaphore::releaseFromISR(semaphore,
                &higherPriorityTaskAwoken);
    }
    if(result != RETURN_OK) {
        if(result == BinarySemaphore::SEMAPHORE_INVALID) {
            TRACE_ERROR("SPIDeviceComIF::SPIcallback: "
                    "Semaph is nullpointer!\r\n");
        }
        else if(result == BinarySemaphore::SEMAPHORE_NOT_OWNED) {
            TRACE_ERROR("SPIDeviceComIF::SPIcallback: "
                    "Semaph not owned!\r\n");
        }
        else {
        	TRACE_ERROR("SPIDeviceComIF::SPIcallback: "
        	        "Unknown error occured!\r\n");
        }
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

void SpiDeviceComIF::handleSpiTransferInitResult(SpiCookie * spiCookie,
        SpiTransferInitResult driverResult) {
    switch(driverResult) {
    case(SpiTransferInitResult::INVALID_TRANSFER_STRUCT): {
    	spiCookie->setCurrentComStatus(ComStatus::FAULTY,
    			SPI_TRANSFER_INVALID_TRANSFER_STRUCT);
    	sif::error << "SpiDeviceComIF::handleSpiTransferInitResult: Invalid"
    	        " transfer struct." << std::endl;
    	spiSemaphore.release();
    	return;
    }
    case(SpiTransferInitResult::INVALID_DRIVER_PARAMS): {
    	spiCookie->setCurrentComStatus(ComStatus::FAULTY,
    			SPI_TRANSFER_INVALID_DRIVER_PARAMS);
    	spiSemaphore.release();
    	return;
    }
    case(SpiTransferInitResult::QUEUE_INIT_FAILURE): {
    	spiCookie->setCurrentComStatus(ComStatus::FAULTY,
    			SPI_TRANSFER_QUEUE_INIT_FAILURE);
    	spiSemaphore.release();
    	return;
    }
    case(SpiTransferInitResult::RETURN_OK):
    	spiCookie->setCurrentComStatus(ComStatus::TRANSFER_INIT_SUCCESS);
    	return;
    default:
    	sif::error << "Spi ComIF: Invalid transfer init return value "
		      << static_cast<int>(driverResult) << std::endl;
    	spiSemaphore.release();
    	spiCookie->setCurrentComStatus(ComStatus::FAULTY, RETURN_FAILED);
    	return;
    };
}

void SpiDeviceComIF::handleGpioOutputSwitching(DemultiplexerOutput demuxOutput) {
    switch(demuxOutput) {
    case(DemultiplexerOutput::OUTPUT_1):
        GpioDeviceComIF::enableDecoderOutput1();
        return;
    case(DemultiplexerOutput::OUTPUT_2):
        GpioDeviceComIF::enableDecoderOutput2();
        return;
    case(DemultiplexerOutput::OUTPUT_3):
        GpioDeviceComIF::enableDecoderOutput3();
        return;
    case(DemultiplexerOutput::OUTPUT_4):
        GpioDeviceComIF::enableDecoderOutput4();
        return;
    case(DemultiplexerOutput::OUTPUT_5):
        GpioDeviceComIF::enableDecoderOutput5();
        return;
    case(DemultiplexerOutput::OUTPUT_6):
        GpioDeviceComIF::enableDecoderOutput6();
        return;
    case(DemultiplexerOutput::OUTPUT_7):
        GpioDeviceComIF::enableDecoderOutput7();
        return;
    case(DemultiplexerOutput::OUTPUT_8):
        GpioDeviceComIF::enableDecoderOutput8();
        return;
    case(DemultiplexerOutput::OWN_SLAVE_SELECT):

        return;
    default:
        sif::error << "SPI ComIF: Invalid decoder output"
                 " used for GPIO switching!" << std::endl;
    }
}

void SpiDeviceComIF::handleGpioDecoderSelect(SlaveType slave) {
    switch(slave) {
    case(SlaveType::DEMULTIPLEXER_1):
        GpioDeviceComIF::enableDecoder1();
        pullDummySlaveSelectLow = true;
        return;
    case(SlaveType::DEMULTIPLEXER_2):
        GpioDeviceComIF::enableDecoder2();
        pullDummySlaveSelectLow = true;
        return;
    case(SlaveType::DEMULTIPLEXER_3):
        GpioDeviceComIF::enableDecoder3();
        pullDummySlaveSelectLow = true;
        return;
    case(SlaveType::DEMULTIPLEXER_4):
        GpioDeviceComIF::enableDecoder4();
        pullDummySlaveSelectLow = true;
        return;
    default:
        GpioDeviceComIF::disableDecoders();
        pullDummySlaveSelectLow = false;
        return;
    }
}

