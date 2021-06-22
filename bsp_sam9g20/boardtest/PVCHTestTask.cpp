#include "PVCHTestTask.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

PVCHTestTask::PVCHTestTask(object_id_t objectId): SystemObject(objectId) {
}

ReturnValue_t PVCHTestTask::initialize() {
    sif::printInfo("Hello PVCH!\n");
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t PVCHTestTask::performOperation(uint8_t opCode) {
    return HasReturnvaluesIF::RETURN_OK;
}


