#include "PVCHTestTask.h"

#include "devices/logicalAddresses.h"

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/globalfunctions/arrayprinter.h"

PVCHTestTask::PVCHTestTask(object_id_t objectId): SystemObject(objectId) {
}

PVCHTestTask::~PVCHTestTask() {

}

ReturnValue_t PVCHTestTask::initialize() {
    ReturnValue_t result = i2cMux.intialize();
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::printWarning("PCA9554 I2C Demultiplexer Init failed\n");
    }
    return result;
}



ReturnValue_t PVCHTestTask::performOperation(uint8_t opCode) {
    return HasReturnvaluesIF::RETURN_OK;
}

