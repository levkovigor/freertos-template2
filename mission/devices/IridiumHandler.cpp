#include "IridiumHandler.h"

IridiumHandler::IridiumHandler(object_id_t objectId, object_id_t deviceCom,
        CookieIF *comCookie):
        DeviceHandlerBase(objectId, deviceCom, comCookie) {
}

void IridiumHandler::doStartUp() {
    switch(internalState) {
    case(InternalStates::NONE): {
        internalState = InternalStates::START_WAIT_FOR_PING_RESPONSE;
        break;
    }
    case(InternalStates::START_WAIT_FOR_PING_RESPONSE): {
        if(commandExecuted) {
            internalState = InternalStates::START_CONFIGURE;
        }
        break;
    }
    default:
        break;
    }
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

void IridiumHandler::calculateIsuChecksum(uint8_t *data, size_t dataSize) {
    size_t dataSum = 0;
    for(size_t idx = 0; idx < dataSize; idx ++) {
        dataSum += data[idx];
    }
    // Extract two least significant bytes. Those are the checksum
    data[dataSize] = dataSum >> 8 & 0xff;
    data[dataSize + 1] = dataSum & 0xff;
}
