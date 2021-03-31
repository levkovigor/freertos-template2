#include <mission/devices/GPSHandler.h>

#include <fsfw/osal/FreeRTOS/TaskManagement.h>
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <bsp_sam9g20/boardtest/TwiTestTask.h>
extern "C" {
#include <at91/utility/trace.h>
}

#include <cstdio>

TwiTestTask::TwiTestTask(object_id_t objectId,
        uint8_t slaveAddress):
		                SystemObject(objectId), slaveAddress(slaveAddress),
		                twiMode(twiModes::ECHO),comState(comStates::WRITE),
		                readingMode(readingModes::FRAMES) {
    initializeI2cTransferStruct();
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "twiTestTask object created!" << std::endl;
#else
#endif
    int retVal;
    setTrace(3);
    retVal = I2C_start(BUS_SPEED_HZ, I2C_TIMEOUT);
    setTrace(5);
    if(retVal != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "I2C Test Task: Error during I2C_start: " << retVal << std::endl;
#else
#endif
    }
}

void TwiTestTask::initializeI2cTransferStruct() {
    i2cTransfer.callback = nullptr;
    i2cTransfer.readData = nullptr;
    i2cTransfer.writeData = nullptr;
}

TwiTestTask::~TwiTestTask() {}

ReturnValue_t TwiTestTask::performOperation(uint8_t operationCode){
    switch(twiMode) {
    case(twiModes::ECHO): {
        performEchoOperation();
        break;
    }


    case(twiModes::ECHO_NON_BLOCKING): {
        performNonBlockingEcho();
        break;
    }

    case(twiModes::GPS): {
        performGpsOperation();
        break;
    }

    default: {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "I2C Test Task: Invalid mode" << std::endl;
#else
#endif
    }
    }
    return RETURN_OK;
}

// Echo operation (blocking)

void TwiTestTask::performEchoOperation() {
    if(comState == comStates::READ) {
        performI2cTestRead();
    }
    else if(comState == comStates::WRITE){
        performI2cTestWrite();
    }
}

void TwiTestTask::performI2cTestWrite() {
    ReturnValue_t result;
    char data[30] = "Hello, this is a message!";
    result = I2C_write(slaveAddress, (unsigned char*)data, 30);
    if(result == 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "I2C Test Task: Sent: " << data << std::endl;
#else
#endif
    }
    else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "I2C Test Task: Send failure with code: " << (int)result << std::endl;
#else
#endif
    }
    comState = comStates::READ; // perform read on next task cycle
}

void TwiTestTask::performI2cTestRead() {
    ReturnValue_t result;
    char recvBuffer[30];
    result = I2C_read(slaveAddress, (unsigned char*)recvBuffer, 30);
    if(result == 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "I2C Test Task: Received: " << (char*)recvBuffer << std::endl;
#else
#endif
    }
    else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "I2C Test Task: Send failure with code: " << (int)result << std::endl;
#else
#endif
    }

    comState = comStates::WRITE;
}

// Echo operation (non-blocking)

void TwiTestTask::performNonBlockingEcho() {
    //slaveAddress, (unsigned char*)data, 30
    if(comState == comStates::WRITE) {
        performNonBlockingWrite();
    }
    else if(comState == comStates::READ) {
        performNonBlockingRead();
    }
    else if(comState == comStates::WAIT_READ or comState==comStates::WAIT_WRITE) {
        Clock::getUptime(&startTime);
        checkComStatus();
        Clock::getUptime(&endTime);
        //info << "Com Status check took " << endTime - startTime << " milliseconds!" << std::endl;
    }
}

void TwiTestTask::performNonBlockingWrite() {
    CommandBuffer.reserve(testSize);
    std::copy(testString.begin(), testString.end(), CommandBuffer.data());
    i2cTransfer.slaveAddress = slaveAddress;
    i2cTransfer.callback = I2cCallback;
    i2cTransfer.direction = I2Cdirection::write_i2cDir;
    i2cTransfer.writeSize = testSize;
    i2cTransfer.writeData = CommandBuffer.data();
    i2cTransfer.result = &transferResult;
    i2cTransfer.semaphore = binSemaph.getSemaphore();
    ReturnValue_t result = binSemaph.acquire();
    // semaphore taken, previous transfer still in progress.
    if(result != RETURN_OK) {
        return;
    }
    //Clock::getUptime(&startTime);
    int i2cResult = I2C_queueTransfer(&i2cTransfer);
    if(i2cResult == RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "I2C Test Task: Sent: " << testString << std::endl;
#else
#endif
        comState = comStates::WAIT_WRITE;
    }
    else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "I2C Test Task: Send failure with code: " << (int)result << std::endl;
#else
#endif
    }
}

void TwiTestTask::performNonBlockingRead() {
    ReplyBuffer.reserve(testSize + 1);
    ReplyBuffer[testSize] = '\0';
    i2cTransfer.slaveAddress = slaveAddress;
    i2cTransfer.callback = I2cCallback;
    i2cTransfer.direction = I2Cdirection::read_i2cDir;
    i2cTransfer.readSize = testSize;
    i2cTransfer.readData = ReplyBuffer.data();
    i2cTransfer.result = &transferResult;
    i2cTransfer.semaphore = binSemaph.getSemaphore();
    ReturnValue_t result = binSemaph.acquire();
    // semaphore taken, previous transfer still in progress.
    if(result != RETURN_OK) {
        return;
    }
    //Clock::getUptime(&startTime);
    int i2cResult = I2C_queueTransfer(&i2cTransfer);
    if(i2cResult == RETURN_OK) {
        //info << "I2C Test Task: Sent: " << testString << std::endl;
        comState = comStates::WAIT_READ;
    }
    else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "I2C Test Task: Send failure with code: " << (int)result << std::endl;
#else
#endif
    }
}

void TwiTestTask::checkComStatus() {
    // try to take semaphore to check whether the write was successfull
    ReturnValue_t result = binSemaph.acquire();
    if(result == BinarySemaphore::SEMAPHORE_TIMEOUT) {
        cycleCount ++;
        if(cycleCount == 10) {
            comState = comStates::WRITE;
            cycleCount = 1;
        }
        return;
    }
    else if(result != RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "I2C Test Task: Configuration Error." << std::endl;
#else
#endif
    }
    else {
        if(comState == comStates::WAIT_READ) {
            // Check whether any reply has been received by checking the first letter
            if(ReplyBuffer[0] == 'H') {
                Clock::getUptime(&startTime);
#if FSFW_CPP_OSTREAM_ENABLED == 1
                sif::info << "I2C Task Reply: " << ReplyBuffer.data() << std::endl;
#else
#endif
                Clock::getUptime(&endTime);
                //info << "Read confirmation took "
                //		<< (int) cycleCount << " cycles." << std::endl;
                comState = comStates::WRITE;
            }
            else {
                return;
            }
        }
        else {
            //info << "Write confirmation took " << (int) cycleCount << " cycles." << std::endl;
            comState = comStates::READ;
        }
        cycleCount = 1;
        binSemaph.release();
    }
}


// GPS operations

void TwiTestTask::performGpsOperation() {
    if(comState == comStates::READ) {
        performI2cGpsRead();
    }
    else if(comState == comStates::WRITE){
        performI2cGpsWrite();
    }
}

void TwiTestTask::performI2cGpsWrite() {
    sendTrigger ++;
    if(sendTrigger == 40 /* !testCommandSent */) {
        prepareGpsConfigTest(1);
        ReturnValue_t result = I2C_write(slaveAddress,
                CommandBuffer.data(),gpsCommandSize);
        if(result != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "I2C Test Task: Test Command Send failed with returncode :"
                    << result << std::endl;
#else
#endif
        } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "I2C Test Task: Test command sent" << std::endl;
#else
#endif
        }
        sendTrigger = 0;
        //testCommandSent = true;
    }
    comState = comStates::READ;
}

// Reading I2C byte for byte to prevent problems with maximum buffer sizes of slave devices
void TwiTestTask::performI2cGpsRead() {
    uint8_t gpsBuffer[MAX_GPS_PACKET_SIZE];
    uint32_t startTime;
    Clock::getUptime(&startTime);
    if(readingMode == readingModes::BYTE_FOR_BYTE) {
        readI2cByteWise(gpsBuffer);
    }
    else {
        readI2cAsFrames(gpsBuffer);
    }
    uint32_t endTime;
    Clock::getUptime(&endTime);
    uint32_t elapsedTime = endTime - startTime;
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "I2C Test Task: Reading operation took "
            << std::dec << (int)elapsedTime << " milliseconds for "
            << (int) MAX_GPS_PACKET_SIZE << " bytes" << std::endl;
#else
#endif
    if ((int)elapsedTime > 50) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "I2C Test Task: Reading operation took longer than 50 ms with "
                << std::dec << (int)elapsedTime << " milliseconds for "
                << (int)MAX_GPS_PACKET_SIZE << " bytes" << std::endl;
#else
#endif
    }
    interpretGpsPacket(gpsBuffer);
    comState = comStates::WRITE;
}

void TwiTestTask::readI2cByteWise(uint8_t * buffer) {
    ReturnValue_t result;
    for(int i=0;i<MAX_GPS_PACKET_SIZE;i++) {
        result = I2C_read(slaveAddress,(unsigned char*)buffer + i,1);
        if(result != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "I2C Test Task: Send failure with code: " << (int) result << std::endl;
#else
#endif
            break;
        }
    }
}

void TwiTestTask::readI2cAsFrames(uint8_t *buffer) {
    ReturnValue_t result;
    uint8_t bufferIndex;
    for(int i=0;i<FRAME_READ_COUNT;i++) {
        bufferIndex = i * MAX_FRAME_SIZE;
        if(i == FRAME_READ_COUNT-1) {
            result = I2C_read(slaveAddress, (unsigned char*)buffer + bufferIndex,
                    LAST_READ_BYTE_NUMBER);
        }
        else {
            result = I2C_read(slaveAddress, (unsigned char*)buffer + bufferIndex,
                    MAX_FRAME_SIZE);
        }
        if(result != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "I2C Test Task: Send failure with code: " << (int) result << std::endl;
#else
#endif
            break;
        }
    }
}

void TwiTestTask::interpretGpsPacket(uint8_t *gpsBuffer) {
    if(gpsBuffer[0] == 0) {
        //sid::info << "I2C Test Task: Empty Packet received" << std::endl;
    }
    else if(gpsBuffer[0] == 0xA0) {
        //sid::info << "I2C Test Task: Binary message received" << std::endl;
        printTrigger ++;
        if(gpsBuffer[3] != 59) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "I2C Test Task: Binary reply received !" << std::endl;
#else
#endif
            printBufferAsHex(gpsBuffer,'\n',gpsBuffer[3] + 7);
            printTrigger = 0;
        }
    }
}

void TwiTestTask::printBufferAsHex(uint8_t * buffer, char endMarker,uint8_t maxBufferSize) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << std::uppercase << "[";
#else
#endif
    for(int i=0;i<maxBufferSize;i++) {
        if((char)buffer[i] == endMarker || i == maxBufferSize - 1) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "0x" << std::hex << (int)buffer[i] << "]" << std::dec << std::endl;
#else
#endif
            break;
        }
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "0x" << std::hex << (int)buffer[i] << ", "<< std::dec;
#else
#endif
        if(i % 24 == 0 && i > 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << std::endl;
#else
#endif
        }
    }
}

void TwiTestTask::prepareGpsConfigTest(uint8_t gpsFrequency_) {
    uint8_t selectUpdateRateId = 14;
    uint8_t checksum = 0;
    uint8_t updateRateCommand []= {0xA0, 0xA1, 0x00, 0x03,
            selectUpdateRateId, gpsFrequency_, 0x00, checksum, 0x0D, 0x0A};
    gpsCommandSize = sizeof(updateRateCommand);
    updateRateCommand[7] = GPSHandler::calcChecksum(updateRateCommand + 4, gpsCommandSize - 7);
    memcpy(CommandBuffer.data(),updateRateCommand,gpsCommandSize);
}

void TwiTestTask::I2cCallback(SystemContext context, xSemaphoreHandle semaphore) {
    BaseType_t higherPriorityTaskAwoken = pdFALSE;
    ReturnValue_t result = RETURN_FAILED;
    //TRACE_INFO("SPI Callback reached\n\r");
    if(context == SystemContext::task_context) {
        result = BinarySemaphore::release(semaphore);
        if(result != RETURN_OK) {
            TRACE_INFO("I2C Callback error !");
        }
    }
    else {
        result = BinarySemaphore::releaseFromISR(semaphore,
                &higherPriorityTaskAwoken);
        if(result != RETURN_OK) {
            TRACE_INFO("I2C Callback error !");
        }
        if(higherPriorityTaskAwoken == pdPASS) {
            TaskManagement::requestContextSwitch(CallContext::ISR);
            TRACE_INFO("SPI Test: Higher Priority Task awoken !");
        }
    }
}


