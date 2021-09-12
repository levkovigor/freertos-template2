#include "Pca9554.h"
#include "devices/logicalAddresses.h"
#include "bsp_sam9g20/comIF/I2cDeviceComIF.h"
#include "bsp_sam9g20/comIF/cookies/I2cCookie.h"

#include "fsfw/osal/freertos/BinarySemaphore.h"
#include "fsfw/globalfunctions/bitutility.h"
#include "fsfw/objectmanager/ObjectManager.h"

#include "hal/Drivers/I2C.h"

Pca9554::Pca9554():
        i2cCookie(addresses::PVCH_PCA9554, 16) {
}

ReturnValue_t Pca9554::intialize() {
    //simplePca9554Init();
    i2cComIF = ObjectManager::instance()->get<I2cDeviceComIF>(objects::I2C_DEVICE_COM_IF);
    if(i2cComIF == nullptr) {
        sif::printWarning("PVCHTestTask::initialize: I2C ComIF invalid!\n");
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    // Initialize PCA9554 first
    // A0, A1 and A2 are all pulled high
    ReturnValue_t result = i2cComIF->initializeInterface(&i2cCookie);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::printWarning("PVCHTestTask::initialize: I2C ComIF initialization failed!\n");
    }
    semaphHandle = &i2cCookie.getSemaphoreObjectHandle();
    // Set ports as output
    result = blockingWriteWrapper(PCA9554Regs::CONFIG, 0x00);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    uint8_t* buf = nullptr;
    size_t readSize = 0;
    // Read back
    result = blockingReadbackWrapper(&buf, &readSize);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    //sif::printDebug("Pca9554: Read back I2C config register: 0x%02x\n", buf[0]);
    if(buf[0] != 0x00) {
        sif::printWarning("Pca9554: CONFIG register read back has failed\n");
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    // Set ports to high
    result = blockingWriteWrapper(PCA9554Regs::OUTPUT_PORT, 0xff);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    // Read back
    result = blockingReadbackWrapper(&buf, &readSize);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    //sif::printDebug("Pca9554: Read back I2C output port: 0x%02x\n", buf[0]);
    if(buf[0] != outputState) {
        sif::printWarning("Pca9554: Output Port register read back has failed\n");
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Pca9554::setAdcModeLow() {
    bitutil::bitSet(&outputState, 0);
    return blockingWriteWrapper(PCA9554Regs::OUTPUT_PORT, outputState);
}

ReturnValue_t Pca9554::clearAdcModeLow() {
    bitutil::bitClear(&outputState, 0);
    return blockingWriteWrapper(PCA9554Regs::OUTPUT_PORT, outputState);
}

ReturnValue_t Pca9554::setAdcModeHigh() {
    bitutil::bitSet(&outputState, 1);
    return blockingWriteWrapper(PCA9554Regs::OUTPUT_PORT, outputState);
}

ReturnValue_t Pca9554::clearAdcModeHigh() {
    bitutil::bitClear(&outputState, 1);
    return blockingWriteWrapper(PCA9554Regs::OUTPUT_PORT, outputState);
}

ReturnValue_t Pca9554::setCsLoadSw() {
    // Apparently the DLR does not want this output state to persist
    uint8_t tmpCmd = outputState;
    bitutil::bitSet(&tmpCmd, 2);
    return blockingWriteWrapper(PCA9554Regs::OUTPUT_PORT, tmpCmd);
}

ReturnValue_t Pca9554::clearCsLoadSw() {
    // Apparently the DLR does not want this output state to persist
    uint8_t tmpCmd = outputState;
    bitutil::bitClear(&tmpCmd, 2);
    return blockingWriteWrapper(PCA9554Regs::OUTPUT_PORT, tmpCmd);
}

ReturnValue_t Pca9554::setMuxSync() {
    // Apparently the DLR does not want this output state to persist
    uint8_t tmpCmd = outputState;
    bitutil::bitSet(&tmpCmd, 3);
    return blockingWriteWrapper(PCA9554Regs::OUTPUT_PORT, tmpCmd);
}

ReturnValue_t Pca9554::clearMuxSync() {
    // Apparently the DLR does not want this output state to persist
    uint8_t tmpCmd = outputState;
    bitutil::bitClear(&tmpCmd, 3);
    return blockingWriteWrapper(PCA9554Regs::OUTPUT_PORT, tmpCmd);
}

ReturnValue_t Pca9554::setAdcSync() {
    // Apparently the DLR does not want this output state to persist
    uint8_t tmpCmd = outputState;
    bitutil::bitSet(&tmpCmd, 4);
    return blockingWriteWrapper(PCA9554Regs::OUTPUT_PORT, tmpCmd);
}

ReturnValue_t Pca9554::clearAdcSync() {
    // Apparently the DLR does not want this output state to persist
    uint8_t tmpCmd = outputState;
    bitutil::bitClear(&tmpCmd, 4);
    return blockingWriteWrapper(PCA9554Regs::OUTPUT_PORT, tmpCmd);
}

void Pca9554::simplePca9554Init() {
    int result = 0;
    txBuf[0] = PCA9554Regs::CONFIG;
    txBuf[1] = 0x00;
    I2C_setTransferTimeout(1000);
    result = I2C_write(addresses::PVCH_PCA9554, txBuf.data(), 2);
    I2CdriverState drvState = I2C_getDriverState();
    sif::printInfo("Current driver state: 0x%02x\n", drvState);
    //int result = I2C_read(addresses::PVCH_PCA9554, txBuf.data(), 2);
    if(result != 0) {
        sif::printWarning("PVCHTestTask::simplePca9554Init: "
                "I2C_write failed with code %d\n", result);
    }
}

ReturnValue_t Pca9554::blockingWriteWrapper(uint8_t reg, uint8_t val) {
    txBuf[0] = reg;
    txBuf[1] = val;
    ReturnValue_t result = i2cComIF->sendMessage(&i2cCookie, txBuf.data(), 2);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::printWarning("Pca9554: Sending I2C failed\n");
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    // Semaphore will be unblocked by a transfer callback
    result = semaphHandle->acquire(SemaphoreIF::TimeoutType::WAITING, 50);
    if(result == HasReturnvaluesIF::RETURN_OK) {
        semaphHandle->release();
    }

    return i2cComIF->getSendSuccess(&i2cCookie);
}

ReturnValue_t Pca9554::blockingReadbackWrapper(uint8_t** buf, size_t *readSize) {
    ReturnValue_t result = i2cComIF->requestReceiveMessage(&i2cCookie, 1);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::printWarning("Pca9554: Error requesting reply\n");
    }
    // Semaphore will be unblocked by a transfer callback
    result = semaphHandle->acquire(SemaphoreIF::TimeoutType::WAITING, 50);
    if(result == HasReturnvaluesIF::RETURN_OK) {
        semaphHandle->release();
        auto transferStatus = i2cCookie.getI2cTransferStatusHandle();
        if(transferStatus != I2CtransferStatus::done_i2c) {
            sif::printWarning("PVCHTestTask:: I2C read back transfer failure\n");
        }
    }
    return i2cComIF->readReceivedMessage(&i2cCookie, buf, readSize);
}
