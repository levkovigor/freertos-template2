#include "IridiumHandler.h"

IridiumHandler::IridiumHandler(object_id_t objectId, object_id_t deviceCom,
        CookieIF *comCookie):
        DeviceHandlerBase(objectId, deviceCom, comCookie) {
}

void IridiumHandler::doStartUp() {
}

void IridiumHandler::doShutDown() {
}

ReturnValue_t IridiumHandler::buildNormalDeviceCommand(DeviceCommandId_t *id) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t IridiumHandler::buildTransitionDeviceCommand(
        DeviceCommandId_t *id) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t IridiumHandler::buildCommandFromCommand(
        DeviceCommandId_t deviceCommand, const uint8_t *commandData,
        size_t commandDataLen) {
    return HasReturnvaluesIF::RETURN_OK;
}

void IridiumHandler::fillCommandAndReplyMap() {
}

ReturnValue_t IridiumHandler::scanForReply(const uint8_t *start,
        size_t remainingSize, DeviceCommandId_t *foundId, size_t *foundLen) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t IridiumHandler::interpretDeviceReply(DeviceCommandId_t id,
        const uint8_t *packet) {
    return HasReturnvaluesIF::RETURN_OK;
}

void IridiumHandler::setNormalDatapoolEntriesInvalid() {
}
