#include "Service11TelecommandScheduling.h"

Service11TelecommandScheduling::Service11TelecommandScheduling(
        object_id_t objectId, uint16_t apid, uint8_t serviceId):
        PusServiceBase(objectId, apid, serviceId) {
}

Service11TelecommandScheduling::~Service11TelecommandScheduling() {
}

ReturnValue_t Service11TelecommandScheduling::handleRequest(
        uint8_t subservice) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service11TelecommandScheduling::performService() {
    return HasReturnvaluesIF::RETURN_OK;
}
