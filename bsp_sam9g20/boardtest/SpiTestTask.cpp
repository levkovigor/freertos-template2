#include <bsp_sam9g20/boardtest/SpiTestTask.h>
#include <fsfw/osal/FreeRTOS/BinarySemaphore.h>
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/tasks/TaskFactory.h>
#include <bsp_sam9g20/comIF/GpioDeviceComIF.h>

extern "C" {
#include <board.h>
#include <at91/utility/trace.h>
#include <at91/peripherals/spi/spi_at91.h>
}

#include <bitset>
#include <cmath>

SpiTestTask::SpiTestTask(object_id_t objectId, SpiTestMode spiTestMode):
		        SystemObject(objectId), spiTestMode(spiTestMode) {
    ReturnValue_t result = RETURN_FAILED;
    // Arduino Test / AT91SAM9G20-EK
    result = SPI_start(SPI_bus, slave2_spi);
    if(result != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "SPIandFRAMtest: SPI_start returned " << (int)result << std::endl;
#else
#endif
    }
    // designated command byte. return largest value
    //writeData[0] = 'l';
    for(uint8_t i=0; i<sizeof(writeData); i++) {
        writeData[i] = i;
        readData[i] = 0xFF;
    }

    //debug << "SPI Test: Bus started." << std::endl;
}

SpiTestTask::~SpiTestTask() {
    SPI_stop(bus0_spi);
}

ReturnValue_t SpiTestTask::performOperation(uint8_t operationCode) {
    if(initWait) {
        initWait = false;
        return HasReturnvaluesIF::RETURN_OK;
    }

    if(spiTestMode == SpiTestMode::BLOCKING) {
        performBlockingSpiTest(slave0_spi, 0);
    }
    else if(spiTestMode == SpiTestMode::NON_BLOCKING) {
        performNonBlockingSpiTest(slave0_spi, 0);
    }
    else if(spiTestMode == SpiTestMode::PT1000) {
        performBlockingPt1000Test();
    }
    else if(spiTestMode == SpiTestMode::GYRO) {
        performBlockingGyroTest();
    }
    else if(spiTestMode == SpiTestMode::MGM_LIS3) {
        performBlockingMgmTest();
    }
    return RETURN_OK;
}

void SpiTestTask::performBlockingSpiTest(SPIslave slave, uint8_t bufferPosition) {
    int retValInt = 0;
    unsigned int i;
    SPIslaveParameters slaveParams;
    SPItransfer spiTransfer;

    slaveParams.bus    = SPI_bus;
    slaveParams.mode   = mode2_spi;
    slaveParams.slave  = slave;
    slaveParams.dlybs  = 1;
    slaveParams.dlybct = 1;
    slaveParams.busSpeed_Hz = SPI_MIN_BUS_SPEED;
    slaveParams.postTransferDelay = 4;

    spiTransfer.slaveParams = &slaveParams;
    spiTransfer.callback  = SPIcallback;
    spiTransfer.readData  = readData + bufferPosition;
    spiTransfer.writeData = writeData + bufferPosition;
    spiTransfer.transferSize = transferSize;



    retValInt = SPI_writeRead(&spiTransfer);
    if(retValInt != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "SPI Test 2: SPI_writeRead returned: " << (int)retValInt << std::endl;
#else

#endif
        while(1);
    }

    GpioDeviceComIF::disableDecoders();
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "SPI Test 2: Received: ";
    sif::debug << std::dec << (int)readData[0] << std::endl;
#else
#endif
    readData[bufferPosition] = 0xFF;
    for(i=bufferPosition+1 ; i<spiTransfer.transferSize + bufferPosition; i++) {
        writeData[i]++;
        readData[i] = 0xFF;
    }
}

void SpiTestTask::performBlockingGyroTest() {

    SPIslaveParameters slaveParams;
    SPItransfer spiTransfer;

    slaveParams.bus = SPIbus::bus1_spi;
    slaveParams.mode = mode0_spi;
    slaveParams.slave = slave2_spi;
    decoderSSused = true;
    slaveParams.dlybs = 1;
    slaveParams.dlybct = 2;
    slaveParams.busSpeed_Hz = 3'900'000;
    slaveParams.postTransferDelay = 3;
    spiTransfer.slaveParams = &slaveParams;
    spiTransfer.callback  = SPIcallback;
    spiTransfer.readData  = readData;
    spiTransfer.writeData = writeData;


    if(not gyroConfigured) {
        spiTransfer.transferSize = 2;
        // activate SPI
        writeGyroRegister(spiTransfer, 0x7F, 0x00);

        bool pmuConfigured = false;
        uint8_t idx = 0;

        while((not pmuConfigured) and (idx < 10)) {
            configureGyroPmu(spiTransfer);
            TaskFactory::delayTask(2);
            uint8_t pmuConfig = readGyroPmu(spiTransfer);
            if(pmuConfig == 0b0000'0100) {
                pmuConfigured = true;
            }
            else {
                idx ++;
            }
            TaskFactory::delayTask(2);
        }

#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "Gyro booted successfully." << std::endl;
#else
#endif
        gyroConfigured = true;
    }

    // TODO: test burst write and read for configuration.
    // Burst write
    // 1. General configuration
    // 2. Range configuration
    writeData[0] = 0x42;
    static constexpr uint8_t GYRO_BWP_CONFIG = 0b0001'0000;
    // Configure 50 Hz output data rate, equals 10.7 Hz 3dB cutoff frequency
    // with OSR2.
    static constexpr uint8_t GYRO_ODR_CONFIG = 0b0000'0111;
    writeData[1] = GYRO_BWP_CONFIG | GYRO_ODR_CONFIG;
    // Burst configuration, target register is incremented automatically.
    static constexpr uint8_t RANGE_CONFIG = 0b0000'0100;

    writeData[2] = RANGE_CONFIG;
    spiTransfer.transferSize = 3;

    configureSpiDummySSIfNeeded();
    GpioDeviceComIF::enableDecoder1();
    GpioDeviceComIF::enableDecoderOutput4();
    int result = SPI_writeRead(&spiTransfer);
    if(result != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "SPI Test 2: SPI_writeRead returned: " << static_cast<int>(result)
	                    << std::endl;
#else
#endif
    }
    GpioDeviceComIF::disableDecoders();

    // Burst read, which reads the configuration and the range register
    // in one go.
    writeData[0] = GYRO_READ_MASK | 0x42;
    writeData[1] = 0x00;
    writeData[2] = 0x00;
    spiTransfer.transferSize = 3;

    configureSpiDummySSIfNeeded();
    GpioDeviceComIF::enableDecoder1();
    GpioDeviceComIF::enableDecoderOutput4();
    result = SPI_writeRead(&spiTransfer);
    if(result != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "SPI Test 2: SPI_writeRead returned: "
                << (int)result << std::endl;
#else
#endif
    }
    GpioDeviceComIF::disableDecoders();

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Gyro Config Register 0x42: 0b" <<
            std::bitset<8>(readData[1]) << std::endl;
    sif::info << "Gyro Range Config Register 0x43: 0b" <<
            std::bitset<8>(readData[2]) << std::endl;
#else
#endif

    readGyroSensorsBlockRead(spiTransfer);
}



void SpiTestTask::readGyroSensorsBlockRead(SPItransfer &spiTransfer) {
    spiTransfer.transferSize = 7;
    writeData[0] = 0x12 | GYRO_READ_MASK;
    std::memset(writeData + 1, 0, 6);

    configureSpiDummySSIfNeeded();
    GpioDeviceComIF::enableDecoder1();
    GpioDeviceComIF::enableDecoderOutput4();
    int result = SPI_writeRead(&spiTransfer);
    if(result != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "SPI Test 2: SPI_writeRead returned: " << (int)result << std::endl;
#else
#endif
    }
    GpioDeviceComIF::disableDecoders();

    int16_t angVelocityXDig = readData[2] << 8 | readData[1];
    int16_t angVelocityYDig = readData[4] << 8 | readData[3];
    int16_t angVelocityZDig = readData[6] << 8 | readData[5];

    uint16_t gyroRange = 125;
    // Angular velocities in degrees per second
    float angVelocityX = angVelocityXDig / std::pow(2, 15) * gyroRange;
    float angVelocityY = angVelocityYDig / std::pow(2, 15) * gyroRange;
    float angVelocityZ = angVelocityZDig / std::pow(2, 15) * gyroRange;

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Angular velocities in degrees per second:" << std::endl;
    sif::info << "X: " << angVelocityX << std::endl;
    sif::info << "Y: " << angVelocityY << std::endl;
    sif::info << "Z: " << angVelocityZ << std::endl;
#else
    sif::printInfo("Angular velocities in degrees per second:\n");
    sif::printInfo("X: %f\n", angVelocityX);
    sif::printInfo("Y: %f\n", angVelocityY);
    sif::printInfo("Z: %f\n", angVelocityZ);
#endif
}


void SpiTestTask::readGyroSensorsSeparateReads(SPItransfer &spiTransfer) {
    uint8_t gyroResult[6];
    spiTransfer.transferSize = 2;

    gyroResult[0] = readGyroRegister(spiTransfer, 0x13);
    gyroResult[1] = readGyroRegister(spiTransfer, 0x12);

    gyroResult[2] = readGyroRegister(spiTransfer, 0x15);
    gyroResult[3] = readGyroRegister(spiTransfer, 0x14);

    gyroResult[4] = readGyroRegister(spiTransfer, 0x17);
    gyroResult[5] = readGyroRegister(spiTransfer, 0x16);

    int16_t angVelocityXDig = gyroResult[0] << 8 | gyroResult[1];
    int16_t angVelocityYDig = gyroResult[2] << 8 | gyroResult[3];
    int16_t angVelocityZDig = gyroResult[4] << 8 | gyroResult[5];

    uint16_t gyroRange = 125;
    // Angular velocities in degrees per second
    float angVelocityX = angVelocityXDig / std::pow(2, 15) * gyroRange;
    float angVelocityY = angVelocityYDig / std::pow(2, 15) * gyroRange;
    float angVelocityZ = angVelocityZDig / std::pow(2, 15) * gyroRange;

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Angular velocities in degrees per second:" << std::endl;
    sif::info << "X: " << angVelocityX << std::endl;
    sif::info << "Y: " << angVelocityY << std::endl;
    sif::info << "Z: " << angVelocityZ << std::endl;
#else
    sif::printInfo("Angular velocities in degrees per second: \n");
    sif::printInfo("X: %f\n", angVelocityX);
    sif::printInfo("Y: %f\n", angVelocityY);
    sif::printInfo("Z: %f\n", angVelocityZ);
#endif
}

void SpiTestTask::configureSpiDummySSIfNeeded() {
    if(decoderSSused) {
        pullDummySlaveSelectLow = true;
    }
}

void SpiTestTask::configureGyroPmu(SPItransfer& spiTransfer) {
    writeGyroRegister(spiTransfer, 0x7E, 0x15);
}

uint8_t SpiTestTask::readGyroPmu(SPItransfer& spiTransfer) {
    return readGyroRegister(spiTransfer, 0x03);
}

void SpiTestTask::writeGyroRegister(SPItransfer &spiTransfer, uint8_t reg,
        uint8_t value) {
    writeData[0] = reg;
    writeData[1] = value;

    configureSpiDummySSIfNeeded();
    GpioDeviceComIF::enableDecoder1();
    GpioDeviceComIF::enableDecoderOutput4();
    int result = SPI_writeRead(&spiTransfer);
    if(result != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "SPI Test 2: SPI_writeRead returned: " << (int) result << std::endl;
#else
#endif
    }
    GpioDeviceComIF::disableDecoders();
}

uint8_t SpiTestTask::readGyroRegister(SPItransfer &spiTransfer, uint8_t reg) {
    uint8_t readMaskedReg = reg | GYRO_READ_MASK;
    writeData[0] = readMaskedReg;
    writeData[1] = 0x00;

    configureSpiDummySSIfNeeded();
    GpioDeviceComIF::enableDecoder1();
    GpioDeviceComIF::enableDecoderOutput4();
    int result = SPI_writeRead(&spiTransfer);
    if(result != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "SPI Test 2: SPI_writeRead returned: " << (int) result << std::endl;

#else
#endif
    }
    GpioDeviceComIF::disableDecoders();
    return readData[1];
}


void SpiTestTask::performBlockingPt1000Test() {
    SPIslaveParameters slaveParams;
    SPItransfer spiTransfer;
    slaveParams.bus = SPIbus::bus1_spi;
    slaveParams.mode = mode1_spi;
    slaveParams.slave = slave2_spi;
    slaveParams.dlybs = 5;
    slaveParams.dlybct = 5;
    slaveParams.busSpeed_Hz = 3'900'000;
    slaveParams.postTransferDelay = 0;
    decoderSSused = true;

    spiTransfer.slaveParams = &slaveParams;
    spiTransfer.callback  = SPIcallback;

    // send config.
    writeData[0] = 0x80;
    writeData[1] = 0b11000001;
    spiTransfer.readData  = readData;
    spiTransfer.writeData = writeData;

    spiTransfer.transferSize = 2;
    uint16_t idx = 0;
    uint16_t waitPeriod = 350;

    configureSpiDummySSIfNeeded();
    GpioDeviceComIF::enableDecoder1();
    GpioDeviceComIF::enableDecoderOutput3();
    //vTaskDelay(1);
    idx = waitPeriod;
    while(idx != 0) {
        idx--;
    }
    int result = SPI_writeRead(&spiTransfer);
    if(result != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "SPI Test 2: SPI_writeRead returned: " << (int) result << std::endl;
#else
#endif
    }

    GpioDeviceComIF::disableDecoders();
    //GpioDeviceComIF::enableDecoderOutput4();

    TaskFactory::delayTask(100);

    writeData[0] = 0;
    writeData[1] = 0;

    configureSpiDummySSIfNeeded();
    GpioDeviceComIF::enableDecoder1();
    GpioDeviceComIF::enableDecoderOutput3();
    idx = waitPeriod;
    while(idx != 0) {
        idx--;
    }
    result = SPI_writeRead(&spiTransfer);
    if(result != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "SPI Test 2: SPI_writeRead returned: " << (int) result << std::endl;
#else
#endif
    }
    GpioDeviceComIF::disableDecoders();

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Reply first byte: " << (int) readData[0] << std::endl;
    sif::info << "Reply second byte: " << (int) readData[1] << std::endl;
#else
#endif

    TaskFactory::delayTask(100);
    writeData[0] = 0x01;
    writeData[1] = 0x00;
    writeData[2] = 0x00;
    spiTransfer.transferSize = 3;

    configureSpiDummySSIfNeeded();
    GpioDeviceComIF::enableDecoder1();
    GpioDeviceComIF::enableDecoderOutput3();
    //vTaskDelay(1);
    idx = waitPeriod;
    while(idx != 0) {
        idx--;
    }
    result = SPI_writeRead(&spiTransfer);
    if(result != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "SPI Test 2: SPI_writeRead returned: " << (int) result << std::endl;
#else
#endif
    }
    GpioDeviceComIF::disableDecoders();
    //GpioDeviceComIF::enableDecoderOutput4();

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Reply first byte: " << (int) readData[0] << std::endl;
    sif::info << "Reply second byte: " << (int) readData[1] << std::endl;
    sif::info << "Reply third byte: " << (int) readData[2] << std::endl;
#else
#endif

    uint16_t adcCode = ((readData[1] << 8) | readData[2]) >> 1;

    bool faultBit = readData[2] & 0b0000'0001;
    if(faultBit == 1) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "PT1000: Fault bit set!" << std::endl;
#else
#endif
    }

    float approxTemp = adcCode / 32.0 - 256;
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Current temp: " << approxTemp << std::endl;
#else
    sif::printInfo("Current temp: %f\n", approxTemp);
#endif
}

void SpiTestTask::performBlockingMgmTest() {
    SPIslaveParameters slaveParams;
    SPItransfer spiTransfer;
    slaveParams.bus = SPIbus::bus1_spi;
    slaveParams.mode = mode0_spi;
    slaveParams.slave = slave1_spi;
    slaveParams.dlybs = 0;
    slaveParams.dlybct = 0;
    slaveParams.busSpeed_Hz = 3'900'000;
    slaveParams.postTransferDelay = 0;
    decoderSSused = false;

    //    AT91S_SPI *spi = AT91C_BASE_SPI1;
    //    if(slaveParams.mode == mode0_spi or slaveParams.mode == mode1_spi) {
    //        spi->SPI_CSR[1] &= ~(1UL << 0);
    //    }
    //    else {
    //        spi->SPI_CSR[1] |= 1UL;
    //    }
    //
    //    if(slaveParams.mode == mode0_spi or slaveParams.mode == mode2_spi) {
    //        spi->SPI_CSR[1] |= (1UL << 1);
    //    }
    //    else {
    //        spi->SPI_CSR[1] &= ~(1UL << 1);
    //    }

    spiTransfer.slaveParams = &slaveParams;
    spiTransfer.callback  = SPIcallback;

    const uint8_t LIS3_READ_MASK = 0b1000'0000;
    // read WHO AM I register
    writeData[0] = 0b0000'1111 | LIS3_READ_MASK;
    writeData[1] = 0x00;
    spiTransfer.readData  = readData;
    spiTransfer.writeData = writeData;

    spiTransfer.transferSize = 2;

    configureSpiDummySSIfNeeded();
    int result = SPI_writeRead(&spiTransfer);

    if(result != 0) {

    }

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Reply first byte: " << (int) readData[0] << std::endl;
    sif::info << "Reply second byte: " << (int) readData[1] << std::endl;
#else
#endif
}

void SpiTestTask::performNonBlockingSpiTest(SPIslave slave, uint8_t bufferPosition) {
    int retValInt = 0;

    slaveParams.bus    = SPI_bus;
    slaveParams.mode   = mode0_spi;
    slaveParams.slave  = slave;
    slaveParams.dlybs  = 2;
    slaveParams.dlybct = 10;
    slaveParams.busSpeed_Hz = SPI_MIN_BUS_SPEED;
    slaveParams.postTransferDelay = 1;

    spiTransfer.slaveParams = &slaveParams;
    spiTransfer.callback  = SPIcallback;
    spiTransfer.readData  = readData + bufferPosition;
    spiTransfer.writeData = writeData + bufferPosition;
    spiTransfer.transferSize = transferSize;


    BinarySemaphore binarySemaphore = BinarySemaphore();
    ReturnValue_t result = binarySemaphore.acquireWithTickTimeout(
            SemaphoreIF::TimeoutType::WAITING, 10);
    spiTransfer.semaphore  = binarySemaphore.getSemaphore();

    if(result == RETURN_FAILED) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "SPI Test: xSemaphoreTake failed !" << std::endl;
#else
#endif
        return;
    }
    retValInt = SPI_queueTransfer(&spiTransfer);
    if(retValInt != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "SPI Test: SPI_queueTransfer returned: " << (int) retValInt << std::endl;
#else
#endif
        return;
    }
    else {
        // Block on the semaphore waiting for the transfer to finish
        // timeout 10 ticks
        result = binarySemaphore.acquireWithTickTimeout(
                SemaphoreIF::TimeoutType::WAITING, 10);
        if(result == RETURN_FAILED) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::warning << "SPI Test: xSemaphoreTake failed" << std::endl;
#else
#endif
            return;
        }
        result = binarySemaphore.release();
    }

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "SPI Test Slave " << (int) slave << " Received: ";
    sif::debug << std::dec << (int)readData[bufferPosition] << std::endl;
#else
#endif
    readData[bufferPosition] = 0xFF;
    for(uint8_t i=bufferPosition; i<spiTransfer.transferSize + bufferPosition; i++) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << std::dec << " " << (int)readData[i] << " ";
#else
#endif
        writeData[i]++;
        readData[i] = 0xFF;
    }
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << std::endl;
#else
#endif
}

void SpiTestTask::SPIcallback(SystemContext context, xSemaphoreHandle semaphore) {
    BaseType_t higherPriorityTaskAwoken = pdFALSE;
    ReturnValue_t result;
    //TRACE_INFO("SPI Callback reached\n\r");
    if(context == task_context) {
        result = BinarySemaphore::release(semaphore);
    }
    else {
        result = BinarySemaphore::releaseFromISR(semaphore,
                &higherPriorityTaskAwoken);
        if(higherPriorityTaskAwoken == pdPASS) {
            TRACE_INFO("SPI Test: Higher Priority Task awoken !");
        }
    }
    if(result == RETURN_OK) {
        // perform some success operation ,e.g. setting a flag
    }
    else {
        TRACE_ERROR("SPI Test: Error in SPI Callback");
    }
}
