
#include "SpiTestTask.h"
#include <bsp_sam9g20/comIF/GpioDeviceComIF.h>
#include <fsfw/osal/FreeRTOS/BinarySemaphore.h>
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/tasks/TaskFactory.h>
#include <fsfw/globalfunctions/arrayprinter.h>
#include <fsfw/timemanager/Stopwatch.h>

extern "C" {
#include <board.h>
#include <at91/utility/trace.h>
#include <at91/peripherals/spi/spi_at91.h>
#include <at91/peripherals/aic/aic.h>
}

#ifdef ISIS_OBC_G20
#include <bsp_sam9g20/common/fram/CommonFRAM.h>
#include <hal/Storage/FRAM.h>
#include <bsp_sam9g20/common/fram/FRAMNoOs.h>
#endif

#include <bitset>
#include <cmath>

SpiTestTask::SpiTestTask(object_id_t objectId, SpiTestMode spiTestMode):
		                SystemObject(objectId), spiTestMode(spiTestMode) {
    // designated command byte. return largest value
    //writeData[0] = 'l';
    for(size_t i = 0; i < sizeof(writeData); i++) {
        writeData[i] = i;
        readData[i] = 0xFF;
    }
}

SpiTestTask::~SpiTestTask() {
    SPI_stop(bus0_spi);
}

ReturnValue_t SpiTestTask::performOperation(uint8_t operationCode) {
    if(initWait) {
        initWait = false;
        return HasReturnvaluesIF::RETURN_OK;
    }

    spiTestMode = SpiTestMode::IOBC_FRAM;

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
    else if(spiTestMode == SpiTestMode::AT91_LIB_BLOCKING or
            spiTestMode == SpiTestMode::AT91_LIB_DMA) {
        performAt91LibTest();
    }
    else if(spiTestMode == SpiTestMode::IOBC_FRAM) {
#ifdef ISIS_OBC_G20
        iobcFramTest();
#endif
    }
    return RETURN_OK;
}

void SpiTestTask::performBlockingSpiTest(SPIslave slave, uint8_t bufferPosition) {
    // Arduino Test / AT91SAM9G20-EK
    unsigned int i;
    SPIslaveParameters slaveParams = {};
    SPItransfer spiTransfer = {};
    int retval = SPI_start(SPI_bus, slave3_spi);
    if(retval != 0) {
        return;
    }

    writeData[0] = 0b1000'1111;
    writeData[1] = 0;
    transferSize = 2;

    slaveParams.bus    = SPI_bus;
    slaveParams.mode   = mode3_spi;
    slaveParams.slave  = slave0_spi;
    slaveParams.dlybs  = 6;
    slaveParams.dlybct = 100;
    slaveParams.busSpeed_Hz = 3'900'000;
    slaveParams.postTransferDelay = 0;

    spiTransfer.slaveParams = &slaveParams;
    spiTransfer.callback  = SPIcallback;
    spiTransfer.readData  = readData + bufferPosition;
    spiTransfer.writeData = writeData + bufferPosition;
    spiTransfer.transferSize = transferSize;



    retval = SPI_writeRead(&spiTransfer);
    if(retval != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "SPI Test 2: SPI_writeRead returned: " << (int)retval << std::endl;
#else

#endif
        while(1);
    }
    //uint32_t reg = AT91C_BASE_SPI1->SPI_CSR[0];
    //uint32_t mr = AT91C_BASE_SPI1->SPI_MR;

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

volatile At91TransferStates SpiTestTask::transferState = At91TransferStates::IDLE;

void SpiTestTask::performAt91LibTest() {
    At91Npcs cs = At91Npcs::NPCS_0;
    At91SpiBuses bus = At91SpiBuses::SPI_BUS_1;
    spiTestMode = SpiTestMode::AT91_LIB_DMA;

    // for L3GD20H board
    int retval = at91_spi_configure_driver(bus, cs, SpiModes::SPI_MODE_3, 3'900'000, 100, 6, 0);

    uint8_t whoAmIReg = 0b0000'1111;
    uint8_t readMask = 0b1000'0000;
    uint8_t sendBuf[10] = {0};
    sendBuf[0] = whoAmIReg | readMask;
    size_t sendLen = 2;
    if(retval != 0) {
        sif::printInfo("SpiTestTask::performAt91LibTest: cfg failed with %d\n", retval);
    }

    std::string halloString = "Hallo\r\n ";
    uint8_t recvBuffer[32] = {0};
    if(spiTestMode == SpiTestMode::AT91_LIB_BLOCKING) {
        retval = at91_spi_blocking_transfer_non_interrupt(bus, cs,
                reinterpret_cast<const uint8_t*>(sendBuf),
                reinterpret_cast<uint8_t*>(recvBuffer), sendLen);
        sif::printInfo("Register: %d\n\r", recvBuffer[1]);
        memset(recvBuffer, 0, sizeof(recvBuffer));

        // now configure config regs
        sendBuf[0] = 0b0000'1111;
        memset(sendBuf + 1, 0, 4);
        retval = at91_spi_blocking_transfer_non_interrupt(bus, cs,
                reinterpret_cast<const uint8_t*>(sendBuf),
                reinterpret_cast<uint8_t*>(recvBuffer), 5);

        // now read all the regs
        sendBuf[0] = 0b1110'0000;
        memset(sendBuf + 1, 0, 5);
        retval = at91_spi_blocking_transfer_non_interrupt(bus, cs,
                reinterpret_cast<const uint8_t*>(sendBuf),
                reinterpret_cast<uint8_t*>(recvBuffer), 5);
        arrayprinter::print(recvBuffer + 1, 6);
    }
    else if(spiTestMode == SpiTestMode::AT91_LIB_DMA) {
        if(oneshot) {
            at91_spi_configure_non_blocking_driver(bus, AT91C_AIC_PRIOR_LOWEST + 2);
            at91_spi_non_blocking_transfer(bus, cs,
                    reinterpret_cast<const uint8_t*>(sendBuf), recvBuffer,
                    2, spiIrqHandler, (void*) &transferState, true);
            int idx = 0;
            while(transferState != At91TransferStates::SPI_SUCCESS) {
                idx++;
            }

        }
    }
}

void SpiTestTask::spiIrqHandler(At91SpiBuses bus, At91TransferStates state, void* args) {
    if(state == SPI_SUCCESS) {
        auto state_flag = reinterpret_cast<volatile At91TransferStates*>(args);
        if (state_flag!= nullptr) {
            *state_flag = At91TransferStates::SPI_SUCCESS;
        }
    }
}

#ifdef ISIS_OBC_G20

#define FRAM_PRINTOUT 1

/* Test code for the CY15B104QI FRAM device */
void SpiTestTask::iobcFramTest() {
    int retval = 0;
    //iobcFramRawTest();
    if(oneshot) {
        retval = fram_start_no_os(&spiCallback,
                reinterpret_cast<void*>(const_cast<At91TransferStates*>(&transferState)));
        if(retval != 0) {
            sif::printWarning("FRAM start (NO OS) failed!\n");
        }
    }


    size_t len = 10;
    uint32_t address = CRITICAL_BLOCK_START_ADDR;
    retval = fram_read_no_os(readData, address, len);
    if(retval != 0) {
        sif::printWarning("FRAM read (NO OS) failed!\n");
    }

    while(true) {
        if(transferState == At91TransferStates::SPI_SUCCESS) {
            break;
        }
        else if(transferState == At91TransferStates::SPI_OVERRUN_ERROR) {
            sif::printWarning("Overrun error!\n");
            break;
        }
    }
    sif::printInfo("Printing critical block 10 bytes\n");
    arrayprinter::print(readData, len);

    len = 64;
    memset(readData, 0, len);
    transferState = At91TransferStates::IDLE;

    retval = fram_read_no_os(readData, address, len);
    if(retval != 0) {
        sif::printWarning("FRAM read (NO OS) failed!\n");
    }

    while(true) {
        if(transferState == At91TransferStates::SPI_SUCCESS) {
            break;
        }
        else if(transferState == At91TransferStates::SPI_OVERRUN_ERROR) {
            sif::printWarning("Overrun error!\n");
            break;
        }
    }
    sif::printInfo("Printing critical block 64 bytes\n");
    arrayprinter::print(readData, len);
    transferState = At91TransferStates::IDLE;

//    memset(readData, 0, 128);
//    len = 128;
//    retval = fram_read_no_os(readData, FRAM_END_ADDR - len, len);
//    if(retval != 0) {
//        sif::printWarning("FRAM read (NO OS) failed!\n");
//    }
//    while(true) {
//        if(transferState == At91TransferStates::SPI_SUCCESS) {
//            break;
//        }
//        else if(transferState == At91TransferStates::SPI_OVERRUN_ERROR) {
//            sif::printWarning("Overrun error!\n");
//            break;
//        }
//    }
//    transferState = At91TransferStates::IDLE;
//    sif::printInfo("Printing last 128 bytes of FRAM\n");
//    arrayprinter::print(readData, len);

    len = 64;
    for(size_t idx = 0; idx < len; idx ++) {
        writeData[idx] = idx + utilityCounter;
    }

    utilityCounter++;
    retval = fram_write_no_os(writeData, FRAM_END_ADDR - len, len);
    if(retval != 0) {
        sif::printWarning("FRAM write (NO OS) failed!\n");
    }
    while(true) {
        if(transferState == At91TransferStates::SPI_SUCCESS) {
            break;
        }
        else if(transferState == At91TransferStates::SPI_OVERRUN_ERROR) {
            sif::printWarning("Overrun error!\n");
            break;
        }
    }
    sif::printInfo("Wrote to last %d bytes of FRAM\n", len);
    transferState = At91TransferStates::IDLE;

    memset(readData, 0, len);
    retval = fram_read_no_os(readData, FRAM_END_ADDR - len, len);
    if(retval != 0) {
        sif::printWarning("FRAM read (NO OS) failed!\n");
    }
    while(true) {
        if(transferState == At91TransferStates::SPI_SUCCESS) {
            break;
        }
        else if(transferState == At91TransferStates::SPI_OVERRUN_ERROR) {
            sif::printWarning("Overrun error!\n");
            break;
        }
    }
    transferState = At91TransferStates::IDLE;
    sif::printInfo("Printing last %d bytes of FRAM\n", len);
    arrayprinter::print(readData, len);

}

void SpiTestTask::iobcFramRawTest() {
    Stopwatch stopwatch;
    int retval = 0;
    At91Npcs cs = At91Npcs::NPCS_0;
    At91SpiBuses bus = At91SpiBuses::SPI_BUS_0;

    /* This code was used to determine SPI properties used by ISIS */
    //    retval = FRAM_start();
    //    if(retval) {};
    //    retval = FRAM_read(readData, 0x0, 10);
    //    /* Value: 4098 -> SPI Mode 0 with NCPHA = 1 and SCBR = 0x10 */
    //    uint32_t framCsr = AT91C_BASE_SPI0->SPI_CSR[0];
    //    /* 8.256 MHz Speed */
    //    uint32_t framBaud = BOARD_MCK / 0x10;
    //    uint32_t scbrTest = SPI_SCBR(framBaud, BOARD_MCK) >> 8;
    //    (void) scbrTest;
    //    /* Value: 4279107585 -> DLYBCS = 0xff */
    //    uint32_t framMr = AT91C_BASE_SPI0->SPI_MR;
    //    if(framCsr) {};
    //    if(framMr) {};

    /* Write test data */


    //retval = FRAM_start();
    /*
    writeData[0] = 5;
    writeData[1] = 4;
    writeData[2] = 3;
    writeData[3] = 2;
    writeData[4] = 1;
    retval = FRAM_writeAndVerify(writeData, FRAM_END_ADDR - 5, 5);
    */

    /* For some reason, our custom implementation requires a little bit of dlybct in contrast
    to the ISIS implementation */
    /*
    unsigned int dlybs = SPI_DLYBS(10, BOARD_MCK);
    unsigned int dlybct = SPI_DLYBCT(65, BOARD_MCK);
    uint8_t dlybsField = dlybs >> 16;
    uint8_t dlybctField = dlybct >> 24;
    */

    retval = at91_spi_configure_driver(bus, cs, SpiModes::SPI_MODE_0, 8'256'000, 15, 1, 0xff);
    if(retval != 0) {
        sif::printWarning("SPI config failed with %d\n", retval);
    }

    //uint32_t customCsr = AT91C_BASE_SPI0->SPI_CSR[0];
    //uint32_t customMr = AT91C_BASE_SPI0->SPI_MR;
    uint8_t statusRegWrite = 0x01;
    uint8_t statusRegRead = 0x05;

    uint8_t statusRegValue = 0b0100'0000;
    writeData[0] = statusRegWrite;
    writeData[1] = statusRegValue;
    size_t sendLen = 2;
    retval = at91_spi_blocking_transfer_non_interrupt(bus, cs, writeData, readData, sendLen);
    if(retval != 0) {
        sif::printWarning("SPI write failed with %d\n", retval);
    }

    writeData[0] = statusRegRead;
    writeData[1] = 0x0;
    retval = at91_spi_blocking_transfer_non_interrupt(bus, cs, writeData, readData, sendLen);
    if(retval != 0) {
        sif::printWarning("SPI read failed with %d\n", retval);
    }

#if FRAM_PRINTOUT == 1
    sif::printInfo("FRAM Status Register: %d\n", readData[1]);
#endif

    /*
    INFO: | 20:17:23.005 | Printing critical block:
    INFO: | 20:17:23.010 | Page 1: 0x03, 0x00, 0x02, 0x00
    INFO: | 20:17:23.016 | Page 2: 0x7d, 0x00, 0x00, 0x00
    INFO: | 20:17:23.022 | Page 3: 0x51, 0x41, 0x5e, 0x60
     */
    uint8_t readOpReg = 0x03;
    uint8_t writeOpReg = 0x02;
    uint8_t writeEnableLatch = 0x06;
    uint32_t address = CRITICAL_BLOCK_START_ADDR;
    sendLen = 32;
    writeData[0] = readOpReg;
    writeData[1] = (address >> 16) & 0xff;
    writeData[2] = (address >> 8) & 0xff;
    writeData[3] = address & 0xff;
    memset(writeData + 4, 0, sendLen - 4);
    retval = at91_spi_blocking_transfer_non_interrupt(bus, cs, writeData, readData, sendLen);
    if(retval != 0) {
        sif::printWarning("SPI read failed with %d\n", retval);
    }

    address = FRAM_END_ADDR - 5;
    sendLen = 5 + 4;

    writeData[0] = writeEnableLatch;
    sendLen = 1;
    retval = at91_spi_blocking_transfer_non_interrupt(bus, cs, writeData, readData, sendLen);
    writeData[0] = writeOpReg;
    writeData[1] = (address >> 16) & 0xff;
    writeData[2] = (address >> 8) & 0xff;
    writeData[3] = address & 0xff;

    writeData[4] = 5;
    writeData[5] = 4;
    writeData[6] = 3;
    writeData[7] = 2;
    writeData[8] = 1;
    sendLen = 9;
#if FRAM_PRINTOUT == 1
    sif::printInfo("Writing 5-1 to FRAM end\n");
#endif
    retval = at91_spi_blocking_transfer_non_interrupt(bus, cs, writeData, readData, sendLen);
    if(retval != 0) {
        sif::printWarning("SPI write failed with %d\n", retval);
    }

    /*
    writeData[1] = address & 0xff;
    writeData[2] = (address >> 8) & 0xff;
    writeData[3] = (address >> 16) & 0xff;
    */
    writeData[0] = readOpReg;
    memset(writeData + 4, 0, sendLen - 4);
    retval = at91_spi_blocking_transfer_non_interrupt(bus, cs, writeData, readData, sendLen);
    if(retval != 0) {
        sif::printWarning("SPI read failed with %d\n", retval);
    }
#if FRAM_PRINTOUT == 1
    sif::printInfo("Reading FRAM end\n");
    arrayprinter::print(readData + 4, 5);
#endif

    at91_configure_csr(bus, cs, SPI_MODE_0, 8'256'000, 0, 0, 0);
    address = CRITICAL_BLOCK_START_ADDR;
    sendLen = 10;
    writeData[0] = readOpReg;
    writeData[1] = (address >> 16) & 0xff;
    writeData[2] = (address >> 8) & 0xff;
    writeData[3] = address & 0xff;
    memset(writeData + 4, 0, sendLen - 4);
    volatile At91TransferStates transferState = At91TransferStates::IDLE;
    retval = at91_spi_configure_non_blocking_driver(bus, 3);
    if(retval != 0) {
        sif::printWarning("SPI non-blocking config failed with %d\n", retval);
    }
    retval = at91_spi_non_blocking_transfer(bus, cs, writeData, readData, sendLen, &spiCallback,
            reinterpret_cast<void*>(const_cast<At91TransferStates*>(&transferState)), true);
    if(retval != 0) {
        sif::printWarning("SPI non-blocking transfer failed with %d\n", retval);
    }
    else {
        while(not transferState ) {}
    }
#if FRAM_PRINTOUT == 1
    sif::printInfo("Read critical block (non-blocking)\n");
    arrayprinter::print(readData + 4, 5);
#endif

    retval = at91_spi_blocking_transfer_non_interrupt(bus, cs, &writeEnableLatch, readData, 1);
//    retval = at91_spi_non_blocking_transfer(bus, cs, &writeEnableLatch, readData, 1,
//            &spiCallback, reinterpret_cast<void*>(const_cast<bool*>(&transferFinished)), true);
//    if(retval != 0) {
//        sif::printWarning("SPI blocking transfer failed with %d\n", retval);
//    }
//    else {
//        while(not transferFinished) {}
//   }
    //writeData[20] = writeEnableLatch;

    address = FRAM_END_ADDR - 5;
    writeData[0] = writeOpReg;
    writeData[1] = (address >> 16) & 0xff;
    writeData[2] = (address >> 8) & 0xff;
    writeData[3] = address & 0xff;
    writeData[4] = 1;
    writeData[5] = 2;
    writeData[6] = 3;
    writeData[7] = 4;
    writeData[8] = 5;
    sendLen = 9;
    transferState = At91TransferStates::IDLE;
    retval = at91_spi_non_blocking_transfer(bus, cs, writeData, NULL , sendLen,
            &spiCallback, reinterpret_cast<void*>(const_cast<At91TransferStates*>(&transferState)),
            false);
    at91_add_second_transfer(writeData + 4, NULL, 5);
    if(retval != 0) {
        sif::printWarning("SPI non-blocking transfer failed with %d\n", retval);
    }
    else {
        while(true) {
            if(transferState == At91TransferStates::SPI_OVERRUN_ERROR) {
                sif::printWarning("SPI overrun error\n");
            }
            else if(transferState == At91TransferStates::SPI_SUCCESS) {
                break;
            }
        }
    }

    writeData[0] = readOpReg;
    memset(writeData + 4, 0, 5);
    transferState = At91TransferStates::IDLE;
    retval = at91_spi_non_blocking_transfer(bus, cs, writeData, readData, sendLen, &spiCallback,
            reinterpret_cast<void*>(const_cast<At91TransferStates*>(&transferState)), true);
    if(retval != 0) {
        sif::printWarning("SPI non-blocking transfer failed with %d\n", retval);
    }
    else {
        while(true) {
            if(transferState == At91TransferStates::SPI_OVERRUN_ERROR) {
                sif::printWarning("SPI overrun error\n");
            }
            else if(transferState == At91TransferStates::SPI_SUCCESS) {
                break;
            }
        }
    }
#if FRAM_PRINTOUT
    sif::printInfo("Read FRAM end, should be 1-5 (non-blocking)\n");
    arrayprinter::print(readData + 4, 5);
#endif
}

void SpiTestTask::spiCallback(At91SpiBuses bus, At91TransferStates state, void *args) {
    At91TransferStates* transferState = static_cast<At91TransferStates*>(args);
    if(transferState != nullptr) {
        *transferState = state;
    }
}

#endif
