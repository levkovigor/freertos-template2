#include "PVCHTestTask.h"

#include "devices/logicalAddresses.h"

#include "bsp_sam9g20/comIF/I2cDeviceComIF.h"
#include "bsp_sam9g20/comIF/cookies/I2cCookie.h"

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/globalfunctions/arrayprinter.h"

PVCHTestTask::PVCHTestTask(object_id_t objectId): SystemObject(objectId) {
}

PVCHTestTask::~PVCHTestTask() {
    if(i2cCookie != nullptr) {
        delete(i2cCookie);
    }
}

ReturnValue_t PVCHTestTask::initialize() {
    //simplePca9554Init();
    i2cComIF = ObjectManager::instance()->get<I2cDeviceComIF>(objects::I2C_DEVICE_COM_IF);
    if(i2cComIF == nullptr) {
        sif::printWarning("PVCHTestTask::initialize: I2C ComIF invalid!\n");
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    // Initialize PCA9554 first
    // A0, A1 and A2 are all pulled high
    address_t pca9554Address = addresses::PVCH_PCA9554;
    i2cCookie = new I2cCookie(pca9554Address, 16);
    ReturnValue_t result = i2cComIF->initializeInterface(i2cCookie);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::printWarning("PVCHTestTask::initialize: I2C ComIF initialization failed!\n");
    }
    BinarySemaphore& semaphHandle = i2cCookie->getSemaphoreObjectHandle();
    // Set ports as output
    txBuf[0] = PCA9554Regs::CONFIG;
    txBuf[1] = 0xff;

    result = i2cComIF->sendMessage(i2cCookie, txBuf.data(), 2);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::printWarning("PVCHTestTask::initialize: Configuring ports as output failed!\n");
    }
    // Semaphore will be unblocked on transfer finish
    result = semaphHandle.acquire();
    if(result == HasReturnvaluesIF::RETURN_OK) {
        semaphHandle.release();
        auto transferStatus = i2cCookie->getI2cTransferStatusHandle();
        if(transferStatus != I2CtransferStatus::done_i2c) {
            sif::printWarning("PVCHTestTask:: Configuring PCA9554 config reg failed!\n");
            return HasReturnvaluesIF::RETURN_FAILED;
        }
    }

    // Read back registers to check whether it was set correctly
    result = i2cComIF->requestReceiveMessage(i2cCookie, 2);
    // Semaphore will be unblocked on transfer finish
    result = semaphHandle.acquire();
    if(result == HasReturnvaluesIF::RETURN_OK) {
        semaphHandle.release();
        auto transferStatus = i2cCookie->getI2cTransferStatusHandle();
        if(transferStatus != I2CtransferStatus::done_i2c) {
            sif::printWarning("PVCHTestTask:: Reading back PCA9554 config reg failed!\n");
        }
    }

    txBuf[0] = PCA9554Regs::OUTPUT_PORT;
    txBuf[1] = 0x5;
    result = i2cComIF->sendMessage(i2cCookie, txBuf.data(), 2);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::printWarning("PVCHTestTask::initialize: Setting all outputs high failed!\n");
    }
    // Semaphore will be unblocked on transfer finish
    result = semaphHandle.acquire();
    if(result == HasReturnvaluesIF::RETURN_OK) {
        semaphHandle.release();
        auto transferStatus = i2cCookie->getI2cTransferStatusHandle();
        if(transferStatus != I2CtransferStatus::done_i2c) {
            sif::printWarning("PVCHTestTask:: Configuring PCA9554 output reg failure!\n");
        }
    }

    // Read back
    result = i2cComIF->requestReceiveMessage(i2cCookie, 1);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::printWarning("PVCHTestTask::initialize: Reading output reg failed!\n");
    }
    // Semaphore will be unblocked on transfer finish
    result = semaphHandle.acquire();
    if(result == HasReturnvaluesIF::RETURN_OK) {
        semaphHandle.release();
        auto transferStatus = i2cCookie->getI2cTransferStatusHandle();
        if(transferStatus != I2CtransferStatus::done_i2c) {
            sif::printWarning("PVCHTestTask:: Reading back PCA9554 output reg failure!\n");
        }
    }
    uint8_t* buf = nullptr;
    size_t readSize = 0;
    result = i2cComIF->readReceivedMessage(i2cCookie, &buf, &readSize);
    sif::printInfo("Read back I2C output port: ");
    arrayprinter::print(buf, readSize);
    return HasReturnvaluesIF::RETURN_OK;
}



ReturnValue_t PVCHTestTask::performOperation(uint8_t opCode) {
    return HasReturnvaluesIF::RETURN_OK;
}

void PVCHTestTask::simplePca9554Init() {
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
