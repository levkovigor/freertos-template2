#include "PVCHTestTask.h"

#include "devices/logicalAddresses.h"

#include "bsp_sam9g20/comIF/I2cDeviceComIF.h"
#include "bsp_sam9g20/comIF/cookies/I2cCookie.h"

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

PVCHTestTask::PVCHTestTask(object_id_t objectId): SystemObject(objectId) {
}

PVCHTestTask::~PVCHTestTask() {
    if(i2cCookie != nullptr) {
        delete(i2cCookie);
    }
}

ReturnValue_t PVCHTestTask::initialize() {
    i2cComIF = ObjectManager::instance()->get<I2cDeviceComIF>(objects::I2C_DEVICE_COM_IF);
    if(i2cComIF == nullptr) {
        sif::printWarning("PVCHTestTask::initialize: I2C ComIF invalid!\n");
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    // Initialize PCA9554 first
    // A0, A1 and A2 are all puled high
    address_t pca9554Address = addresses::PVCH_PCA9554;
    i2cCookie = new I2cCookie(pca9554Address, 16);
    ReturnValue_t result = i2cComIF->initializeInterface(i2cCookie);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::printWarning("PVCHTestTask::initialize: I2C ComIF initialization failed!\n");
    }
    txBuf[0] = PCA9554Regs::CONFIG;
    txBuf[1] = 0x00;
    result = i2cComIF->sendMessage(i2cCookie, txBuf.data(), 2);
    return HasReturnvaluesIF::RETURN_OK;
}



ReturnValue_t PVCHTestTask::performOperation(uint8_t opCode) {
    return HasReturnvaluesIF::RETURN_OK;
}


