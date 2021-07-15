#include "SpiDeviceComIF.h"
#include "GpioDeviceComIF.h"

#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/ipc/MutexGuard.h>
#include <fsfw/osal/freertos/Mutex.h>
#include <fsfw/timemanager/Stopwatch.h>
#include <fsfw/osal/freertos/TaskManagement.h>
#include <devices/logicalAddresses.h>

extern "C" {
#include <at91/utility/trace.h>
#include <at91/peripherals/spi/spi_at91.h>
}

SpiDeviceComIF::SpiDeviceComIF(object_id_t objectId): SystemObject(objectId) {}

SpiDeviceComIF::~SpiDeviceComIF() {}

ReturnValue_t SpiDeviceComIF::initialize() {
    ReturnValue_t result = SPI_start(spiBus,spiSlaves);
    if(result != RETURN_OK) {
        triggerEvent(SPI_START_FAILURE, 0, 0);
        return SPI_INIT_FAILURE;
    }
    return RETURN_OK;
}

ReturnValue_t SpiDeviceComIF::checkAddress(address_t spiAddress) {
    switch(spiAddress) {
    // todo: add all designated spi addresses.
    case(addresses::SPI_Test_PT1000):
    case(addresses::SPI_Test_Gyro):
    case(addresses::SPI_Test_MGM):
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
    SpiCookie * spiCookie = dynamic_cast<SpiCookie*>(cookie);
    if(spiCookie == nullptr) {
        return NULLPOINTER;
    }
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
    SpiCookie* spiCookie = dynamic_cast<SpiCookie *> (cookie);
    if(spiCookie == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    address_t spiAddress = spiCookie->getAddress();
    auto spiMapIter = spiMap.find(spiAddress);
    if(spiMapIter == spiMap.end()) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    ReturnValue_t result = spiSemaphore.acquire(
            SemaphoreIF::TimeoutType::WAITING, SPI_STANDARD_SEMAPHORE_TIMEOUT);
    if(result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "SpiDeviceComIF::sendMessage: Semaphore unavailable "
                "for long time!" << std::endl;
#else
        sif::printWarning("SpiDeviceComIF::sendMessage: Semaphore unavailable "
                "for long time!\n");
#endif
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
    if(spiCookie == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    uint32_t spiAddress = spiCookie->getAddress();
    auto spiIter = spiMap.find(spiAddress);
    if(spiIter == spiMap.end()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "SPI ComIF: Invalid logical address " << std::hex
                << spiCookie->getAddress() << std::endl;
#else
        sif::printWarning( "SPI ComIF: Invalid logical address %lu\n", spiCookie->getAddress());
#endif
        return RETURN_FAILED;
    }
    // Com Status has mutex protection
    MutexGuard mutexLock(spiCookie->getMutexHandle(),
            MutexIF::TimeoutType::WAITING,
            SPI_STANDARD_MUTEX_TIMEOUT);
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
    if(spiCookie == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    uint32_t spiAddress = spiCookie->getAddress();

    auto spiIter = spiMap.find(spiAddress);
    if(spiIter == spiMap.end()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "SpiDeviceComIF::readReceivedMessage: Invalid logical address " << std::hex
                << spiCookie->getAddress() << std::endl;
#else
        sif::printWarning("SpiDeviceComIF::readReceivedMessage: Invalid logical address %lu\n",
                spiCookie->getAddress());
#endif
        return RETURN_FAILED;
    }

    checkTransferResult(spiCookie, spiSemaphore);
    // Com Status has mutex protection
    MutexGuard mutexLock(spiCookie->getMutexHandle(),
            MutexIF::TimeoutType::WAITING,
            SPI_STANDARD_MUTEX_TIMEOUT);
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
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "Spi ComIF: After waiting for " << SPI_STANDARD_SEMAPHORE_TIMEOUT
                << " ticks, the transfer was not completed!" << std::endl;
#else
        sif::printWarning("Spi ComIF: After waiting for %lu ticks, "
                "the transfer was not completed!\n", SPI_STANDARD_SEMAPHORE_TIMEOUT);
#endif
        result = SPI_COMMUNICATION_TIMEOUT;
    }
    else if(result != RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "SpiDeviceComIF::checkTransferResult: "
                "Configuration error when taking semaphore!" << std::endl;
#else
        sif::printError("SpiDeviceComIF::checkTransferResult: "
                "Configuration error when taking semaphore!\n");
#endif

    }

    // Com Status has mutex protection
    MutexGuard mutexLock(spiCookie->getMutexHandle(),
            MutexIF::TimeoutType::WAITING,
            SPI_STANDARD_MUTEX_TIMEOUT);
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
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
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
        // Request context switch at exit of ISR like recommended by
        // FreeRTOS.
        TaskManagement::requestContextSwitch(CallContext::ISR);
    }
}

void SpiDeviceComIF::handleSpiTransferInitResult(SpiCookie * spiCookie,
        SpiTransferInitResult driverResult) {
    switch(driverResult) {
    case(SpiTransferInitResult::INVALID_TRANSFER_STRUCT): {
        spiCookie->setCurrentComStatus(ComStatus::FAULTY,
                SPI_TRANSFER_INVALID_TRANSFER_STRUCT);
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "SpiDeviceComIF::handleSpiTransferInitResult: Invalid"
                " transfer struct." << std::endl;
#else
        sif::printError("SpiDeviceComIF::handleSpiTransferInitResult: Invalid"
                " transfer struct.\n");
#endif
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
    case(SpiTransferInitResult::RETURN_OK): {
        spiCookie->setCurrentComStatus(ComStatus::TRANSFER_INIT_SUCCESS);
        return;
    }

    default: {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "Spi ComIF: Invalid transfer init return value "
                << static_cast<int>(driverResult) << std::endl;
#else
        sif::printError("Spi ComIF: Invalid transfer init return value %d\n", driverResult);
#endif
        spiSemaphore.release();
        spiCookie->setCurrentComStatus(ComStatus::FAULTY, RETURN_FAILED);
        return;
    }

    };
}

void SpiDeviceComIF::handleGpioOutputSwitching(DemultiplexerOutput demuxOutput) {
    switch(demuxOutput) {
    case(DemultiplexerOutput::OUTPUT_1): {
        GpioDeviceComIF::enableDecoderOutput1();
        return;
    }
    case(DemultiplexerOutput::OUTPUT_2): {
        GpioDeviceComIF::enableDecoderOutput2();
        return;
    }
    case(DemultiplexerOutput::OUTPUT_3): {
        GpioDeviceComIF::enableDecoderOutput3();
        return;
    }
    case(DemultiplexerOutput::OUTPUT_4): {
        GpioDeviceComIF::enableDecoderOutput4();
        return;
    }
    case(DemultiplexerOutput::OUTPUT_5): {
        GpioDeviceComIF::enableDecoderOutput5();
        return;
    }
    case(DemultiplexerOutput::OUTPUT_6): {
        GpioDeviceComIF::enableDecoderOutput6();
        return;
    }
    case(DemultiplexerOutput::OUTPUT_7): {
        GpioDeviceComIF::enableDecoderOutput7();
        return;
    }
    case(DemultiplexerOutput::OUTPUT_8): {
        GpioDeviceComIF::enableDecoderOutput8();
        return;
    }
    case(DemultiplexerOutput::OWN_SLAVE_SELECT): {
        return;
    }
    default: {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "SPI ComIF: Invalid decoder output"
                " used for GPIO switching!" << std::endl;
#else
        sif::printError("SPI ComIF: Invalid decoder output"
                " used for GPIO switching!\n");
#endif
    }

    }
}

void SpiDeviceComIF::handleGpioDecoderSelect(SlaveType slave) {
    switch(slave) {
    case(SlaveType::DEMULTIPLEXER_1): {
        GpioDeviceComIF::enableDecoder1();
        pullDummySlaveSelectLow = true;
        return;
    }
    case(SlaveType::DEMULTIPLEXER_2): {
        GpioDeviceComIF::enableDecoder2();
        pullDummySlaveSelectLow = true;
        return;
    }
    case(SlaveType::DEMULTIPLEXER_3): {
        GpioDeviceComIF::enableDecoder3();
        pullDummySlaveSelectLow = true;
        return;
    }
    case(SlaveType::DEMULTIPLEXER_4): {
        GpioDeviceComIF::enableDecoder4();
        pullDummySlaveSelectLow = true;
    }
    return;
    default: {
        GpioDeviceComIF::disableDecoders();
        pullDummySlaveSelectLow = false;
        return;
    }
    }
}

