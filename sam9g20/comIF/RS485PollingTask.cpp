#include <sam9g20/comIF/RS485PollingTask.h>

RS485PollingTask::RS485PollingTask(object_id_t objectId):
        SystemObject(objectId) {
}

ReturnValue_t RS485PollingTask::initializeInterface(CookieIF *cookie) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485PollingTask::sendMessage(CookieIF *cookie,
        const uint8_t *sendData, size_t sendLen) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485PollingTask::getSendSuccess(CookieIF *cookie) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485PollingTask::requestReceiveMessage(CookieIF *cookie,
        size_t requestLen) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485PollingTask::readReceivedMessage(CookieIF *cookie,
        uint8_t **buffer, size_t *size) {
    return HasReturnvaluesIF::RETURN_OK;
}
