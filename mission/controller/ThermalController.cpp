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

ReturnValue_t ThermalController::initializeAfterTaskCreation() {
    ReturnValue_t result =
            ExtendedControllerBase::initializeAfterTaskCreation();
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "ThermalController::initializeAfterTaskCreation: Base"
                << " class initialization failed!" << std::endl;
    }
    HasLocalDataPoolIF* testHkbHandler = objectManager->get<HasLocalDataPoolIF>(
            ThermalSensors::ObjIds::TEST_HKB_HANDLER);
    if(testHkbHandler == nullptr) {
        sif::warning << "ThermalController::initializeAfterTaskCreation: Test"
                << " HKB Handler invalid!" << std::endl;
    }
    //testHkbHandler->getHkManagerHandle()->subscribeForUpdate()
    return result;
}
