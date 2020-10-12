#include "ThermalController.h"

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
