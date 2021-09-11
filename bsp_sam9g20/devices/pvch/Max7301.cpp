#include "Max7301.h"
#include "devices/logicalAddresses.h"

#include "fsfw/tasks/TaskFactory.h"
#include "fsfw/objectmanager.h"
#include "bsp_sam9g20/comIF/SpiDeviceComIF.h"

Max7301::Max7301(Pca9554& i2cMux):
    i2cMux(i2cMux),
    spiCookie(addresses::SPI_DLR_PVCH, 16, SlaveType::PVCH, SPImode::mode0_spi) {
    spiComIF = ObjectManager::instance()->get<SpiDeviceComIF>(objects::SPI_DEVICE_COM_IF);
    if(spiComIF == nullptr) {
        sif::printWarning("Max7301: SPI communication interface is invalid\n");
    }
    ReturnValue_t result = spiComIF->initializeInterface(&spiCookie);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::printWarning("MAx7301: SPI ComIF initialization failed!\n");
    }
    spiSemaph = &spiComIF->getSpiSemaphoreHandle();
}

ReturnValue_t Max7301::initialize() {
    ReturnValue_t result = set(CmdAddr::CFG, RegisterData::NORMAL_OP);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    TaskFactory::delayTask(100);

    // configure all ports to output
    set(CmdAddr::P_CONF_7_TO_4, RegisterData::OUTPUT_ALL);
    TaskFactory::delayTask(100);
    set(CmdAddr::P_CONF_11_TO_8, RegisterData::OUTPUT_ALL);
    TaskFactory::delayTask(100);
    set(CmdAddr::P_CONF_15_TO_12, RegisterData::OUTPUT_ALL);
    TaskFactory::delayTask(100);
    set(CmdAddr::P_CONF_19_TO_16, RegisterData::OUTPUT_ALL);
    TaskFactory::delayTask(100);
    set(CmdAddr::P_CONF_23_TO_20, RegisterData::OUTPUT_ALL);
    TaskFactory::delayTask(100);
    set(CmdAddr::P_CONF_27_TO_24, RegisterData::OUTPUT_ALL);
    TaskFactory::delayTask(100);
    set(CmdAddr::P_CONF_31_TO_28, RegisterData::OUTPUT_ALL);
    TaskFactory::delayTask(100);

    // set all to low
    // D0-D7 -> 0x00
    set(CmdAddr::P4, RegisterData::SET_OUTPUT_HIGH);
    TaskFactory::delayTask(100);
    set(CmdAddr::P5, RegisterData::SET_OUTPUT_HIGH);
    TaskFactory::delayTask(100);
    set(CmdAddr::P6, RegisterData::SET_OUTPUT_HIGH);
    TaskFactory::delayTask(100);
    set(CmdAddr::P7, RegisterData::SET_OUTPUT_HIGH);
    TaskFactory::delayTask(100);
    set(CmdAddr::P8, RegisterData::SET_OUTPUT_HIGH);
    TaskFactory::delayTask(100);
    set(CmdAddr::P9, RegisterData::SET_OUTPUT_HIGH);
    TaskFactory::delayTask(100);
    set(CmdAddr::P10, RegisterData::SET_OUTPUT_HIGH);
    TaskFactory::delayTask(100);
    set(CmdAddr::P11, RegisterData::SET_OUTPUT_HIGH);
    TaskFactory::delayTask(100);
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Max7301::set(CmdAddr cmdAddr, RegisterData data) {
    ReturnValue_t result = i2cMux.clearCsLoadSw();
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    txBuf[0] = cmdAddr;
    txBuf[1] = data;
    result = spiComIF->sendMessage(&spiCookie, txBuf.data(), 2);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::printWarning("Max7301: SPI transfer failed\n");
        return result;
    }
    result = spiSemaph->acquire(SemaphoreIF::TimeoutType::WAITING, 50);
    if(result == HasReturnvaluesIF::RETURN_OK) {
        spiSemaph->release();
    }
    else {
        sif::printWarning("Max7301: SPI transfer timed out\n");
    }
    return i2cMux.setCsLoadSw();
}

ReturnValue_t Max7301::read(CmdAddr cmdAddr, uint8_t &readByte) {
    ReturnValue_t result = i2cMux.clearCsLoadSw();
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    txBuf[0] = cmdAddr | READ_MSK;
    txBuf[1] = 0x00;
    result = spiComIF->sendMessage(&spiCookie, txBuf.data(), 2);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::printWarning("Max7301: SPI transfer failed\n");
        return result;
    }

    result = spiSemaph->acquire(SemaphoreIF::TimeoutType::WAITING, 50);
    if(result == HasReturnvaluesIF::RETURN_OK) {
        spiSemaph->release();
    }
    else {
        sif::printWarning("Max7301: SPI transfer timed out\n");
    }
    result = spiComIF->getSendSuccess(&spiCookie);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    uint8_t rxBuf[2] = {};
    size_t readLen = 0;
    uint8_t* rxPtr = rxBuf;
    result = spiComIF->readReceivedMessage(&spiCookie, &rxPtr, &readLen);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    readByte = rxBuf[1];
    return HasReturnvaluesIF::RETURN_OK;
}
