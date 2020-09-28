#include <sam9g20/comIF/RS485DeviceComIF.h>

RS485DeviceComIF::RS485DeviceComIF(object_id_t objectId):
		SystemObject(objectId) {

}

ReturnValue_t RS485DeviceComIF::initializeInterface(CookieIF *cookie) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485DeviceComIF::sendMessage(CookieIF *cookie,
        const uint8_t *sendData, size_t sendLen) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485DeviceComIF::getSendSuccess(CookieIF *cookie) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485DeviceComIF::requestReceiveMessage(CookieIF *cookie,
        size_t requestLen) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485DeviceComIF::readReceivedMessage(CookieIF *cookie,
        uint8_t **buffer, size_t *size) {
    return HasReturnvaluesIF::RETURN_OK;
}

RS485DeviceComIF::~RS485DeviceComIF() {
}

