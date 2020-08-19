#include <mission/devices/SunSensorHandler.h>

SunSensorHandler::SunSensorHandler(object_id_t objectId, object_id_t comIF,
        CookieIF *comCookie, uint8_t switchId):
        DeviceHandlerBase(objectId, comIF, comCookie), switchId(switchId) {
}

SunSensorHandler::~SunSensorHandler() {
}

void SunSensorHandler::doStartUp() {
}

void SunSensorHandler::doShutDown() {
}

ReturnValue_t SunSensorHandler::buildNormalDeviceCommand(
        DeviceCommandId_t *id) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SunSensorHandler::buildTransitionDeviceCommand(
        DeviceCommandId_t *id) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SunSensorHandler::buildCommandFromCommand(
        DeviceCommandId_t deviceCommand, const uint8_t *commandData,
        size_t commandDataLen) {
    return HasReturnvaluesIF::RETURN_OK;
}

void SunSensorHandler::fillCommandAndReplyMap() {
}

ReturnValue_t SunSensorHandler::scanForReply(const uint8_t *start,
        size_t remainingSize, DeviceCommandId_t *foundId, size_t *foundLen) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SunSensorHandler::interpretDeviceReply(DeviceCommandId_t id,
        const uint8_t *packet) {
    return HasReturnvaluesIF::RETURN_OK;
}

void SunSensorHandler::setNormalDatapoolEntriesInvalid() {
}

void SunSensorHandler::debugInterface(uint8_t positionTracker,
        object_id_t objectId, uint32_t parameter) {
}

uint32_t SunSensorHandler::getTransitionDelayMs(Mode_t modeFrom,
        Mode_t modeTo) {
    return 5000;
}

ReturnValue_t SunSensorHandler::getSwitches(const uint8_t **switches,
        uint8_t *numberOfSwitches) {
    return HasReturnvaluesIF::RETURN_OK;
}
