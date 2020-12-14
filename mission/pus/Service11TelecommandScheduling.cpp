#include "Service11TelecommandScheduling.h"
#include "etl/utility.h"


Service11TelecommandScheduling::Service11TelecommandScheduling(
        object_id_t objectId, uint16_t apid, uint8_t serviceId):
        PusServiceBase(objectId, apid, serviceId) { 


        // just to test:
        //--------------
        TelecommandStruct dummyCommand;
        dummyCommand.milliseconds = 100;
        dummyCommand.storeId = 1;
        uint32_t millisecKey = 100;

        telecommandMap.insert(etl::pair<uint32_t, TelecommandStruct>(millisecKey, dummyCommand));

        }


Service11TelecommandScheduling::~Service11TelecommandScheduling() { }


ReturnValue_t Service11TelecommandScheduling::handleRequest(
        uint8_t subservice) {

    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t Service11TelecommandScheduling::performService() {

    return HasReturnvaluesIF::RETURN_OK;
}
