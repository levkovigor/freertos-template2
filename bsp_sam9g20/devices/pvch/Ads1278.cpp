#include "Ads1278.h"

#include "bsp_sam9g20/comIF/SpiDeviceComIF.h"
#include "bsp_sam9g20/comIF/GpioDeviceComIF.h"

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/tasks/TaskFactory.h"

Ads1278::Ads1278(Pca9554& i2cMux, addresses::LogAddr drdyPin, uint32_t drdyTimeout):
        i2cMux(i2cMux), drdyPin(drdyPin), drdyTimeout(drdyTimeout),
        spiCookie(addresses::SPI_DLR_PVCH, 16, SlaveType::PVCH, SPImode::mode1_spi) {
    GpioDeviceComIF::configurePin(drdyPin, GpioDeviceComIF::PinType::INPUT);
    spiComIF = ObjectManager::instance()->get<SpiDeviceComIF>(objects::SPI_DEVICE_COM_IF);
    if(spiComIF == nullptr) {
        sif::printError("Ads731: SPI communication interface invalid\n");
    }
}

ReturnValue_t Ads1278::setMode(Modes mode) {
    ReturnValue_t result = i2cMux.setAdcSync();
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    if(not isDrdy(DrdyLevel::HIGH, drdyTimeout)) {
        sif::printWarning("Ads1278::setMode: DRDY pin error\n");
        return PvchIF::ADC_DRDY_ERROR;
    }

    // set new mode
    switch (mode)
    {
        case Modes::HIGH_SPEED:
            i2cMux.clearAdcModeHigh();
            i2cMux.clearAdcModeLow();
            break;
        case Modes::HIGH_RESOLUTION:
            i2cMux.clearAdcModeHigh();
            i2cMux.setAdcModeLow();
            break;
        case Modes::LOW_POWER:
            i2cMux.setAdcModeHigh();
            i2cMux.clearAdcModeLow();
            break;
        case Modes::LOW_SPEED:
            i2cMux.setAdcModeHigh();
            i2cMux.setAdcModeLow();
            break;
    }

    // wait DRDY low
    if(not isDrdy(DrdyLevel::HIGH, drdyTimeout)) {
        sif::printWarning("Ads1278::setMode: DRDY pin error\n");
        return PvchIF::ADC_DRDY_ERROR;
    }

    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Ads1278::getData(int32_t &currentValue, int32_t &voltageValue) {
    // Set sync low
    ReturnValue_t result = i2cMux.clearAdcSync();
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    // check DRDY is high
    if(not isDrdy(DrdyLevel::HIGH, drdyTimeout)) {
        sif::printWarning("Ads1278::setMode: DRDY pin to high error\n");
        return PvchIF::ADC_DRDY_ERROR;
    }

    // Set sync high
    result = i2cMux.setAdcSync();
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    // wait DRDY low
    if(not isDrdy(DrdyLevel::LOW, drdyTimeout)) {
        sif::printWarning("Ads1278::setMode: DRDY pin to low error\n");
        return PvchIF::ADC_DRDY_ERROR;
    }


    std::array<uint8_t, 6> dummyTxData  = {};

    result = spiComIF->sendMessage(&spiCookie, dummyTxData.data(), dummyTxData.size());
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    // Semaphore will be unblocked by a transfer callback
    result = spiSemaph->acquire(SemaphoreIF::TimeoutType::WAITING, 50);
    if(result == HasReturnvaluesIF::RETURN_OK) {
        spiSemaph->release();
    }
    else {
        sif::printWarning("Adg731: SPI transfer timed out\n");
    }
    result = spiComIF->getSendSuccess(&spiCookie);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    uint8_t* rxBuf = nullptr;
    size_t readSize = 0;
    result = spiComIF->readReceivedMessage(&spiCookie, &rxBuf, &readSize);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    currentValue = (rxBuf[0] << 16) | (rxBuf[1] << 8) | rxBuf[2];
    voltageValue = (rxBuf[3] << 16) | (rxBuf[4] << 8) | rxBuf[5];

    sif::printDebug("Ads1278: Read Current Value: %d\n", currentValue);
    sif::printDebug("Ads1278: Read Voltage Value: %d\n", voltageValue);

    return HasReturnvaluesIF::RETURN_OK;
}

Ads1278::Modes Ads1278::getMode() const {
    return mode;
}

bool Ads1278::isDrdy(DrdyLevel level, uint16_t timeoutMs) {
    uint16_t counter = 0;
    while (GpioDeviceComIF::getGpio(drdyPin) != level) {
        TaskFactory::delayTask(1);
        counter++;
        if (counter >= timeoutMs) {
            sif::printWarning("Ads1278: DRDY timeout\n");
            return false;
        }
    }
    return true;
}
