#include "ThermalController.h"
#include <mission/devices/devicedefinitions/ThermalSensorPacket.h>

ThermalController::ThermalController(object_id_t objectId):
        ExtendedControllerBase(objectId, objects::NO_OBJECT),
        thermalControllerSet(objectId) {
}

ReturnValue_t ThermalController::handleCommandMessage(
        CommandMessage *message) {
    return HasReturnvaluesIF::RETURN_OK;
}

void ThermalController::performControlOperation() {

}

void ThermalController::handleChangedDataset(sid_t sid,
        store_address_t storeId) {
    if(sid == sid_t(TSensorDefinitions::ObjIds::TEST_HKB_HANDLER,
            TSensorDefinitions::THERMAL_SENSOR_SET_ID)) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "Update registered!" << std::endl;
#else
        sif::printInfo("Update registered!\n");
#endif
    }
}

ReturnValue_t ThermalController::initializeAfterTaskCreation() {
    ReturnValue_t result =
            ExtendedControllerBase::initializeAfterTaskCreation();
    if(result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "ThermalController::initializeAfterTaskCreation: Base"
                << " class initialization failed!" << std::endl;
#else
        sif::printError("ThermalController::initializeAfterTaskCreation: Base"
                " class initialization failed!\n");
#endif
    }
    HasLocalDataPoolIF* testHkbHandler = objectManager->get<HasLocalDataPoolIF>(
            TSensorDefinitions::ObjIds::TEST_HKB_HANDLER);
    //LocalDataPoolManager* hkManager = testHkbHandler->getHkManagerHandle();
    //testHkbHandler->getDataSetHandle()
    if(testHkbHandler == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "ThermalController::initializeAfterTaskCreation: Test"
                << " HKB Handler invalid!" << std::endl;
#else
        sif::printWarning("ThermalController::initializeAfterTaskCreation: Test"
                " HKB Handler invalid!\n");
#endif
    }
    // Test normal notifications without data packet first.
    testHkbHandler->getSubscriptionInterface()->subscribeForSetUpdateMessage(
            TSensorDefinitions::THERMAL_SENSOR_SET_ID,
            this->getObjectId(), commandQueue->getId(), false);

    return result;
}

ReturnValue_t ThermalController::checkModeCommand(Mode_t mode,
        Submode_t submode, uint32_t *msToReachTheMode) {
    return HasReturnvaluesIF::RETURN_OK;
}
