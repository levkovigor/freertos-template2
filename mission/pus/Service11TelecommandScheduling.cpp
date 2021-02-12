#include "Service11TelecommandScheduling.h"
#include "etl/utility.h"
//#include <fsfw/returnvalues/HasReturnvaluesIF.h>	// this is probably not needed
#include <fsfw/serialize/SerializeAdapter.h>
#include <fsfw/serviceinterface/ServiceInterfacePrinter.h>

#include <objects/systemObjectList.h>
#include <fsfw/tmtcservices/TmTcMessage.h>

#include <ctime>


Service11TelecommandScheduling::Service11TelecommandScheduling(
        object_id_t objectId, uint16_t apid, uint8_t serviceId):
        PusServiceBase(objectId, apid, serviceId) {

}


Service11TelecommandScheduling::~Service11TelecommandScheduling() { }


ReturnValue_t Service11TelecommandScheduling::handleRequest(uint8_t subservice) {

    if (subservice == Subservice::INSERT_ACTIVITY){
        return this->handleRequest_InsertActivity();
    }


    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t Service11TelecommandScheduling::performService() {

    //sif::info << "Service11TelecommandScheduling performing." << std::endl;

    // get current time as UNIX timestamp
    uint32_t tCurrent = static_cast<uint32_t>(std::time(nullptr));

    for (auto it = telecommandMap.begin(); it != telecommandMap.end() && true; ++it) {

        if (it->first <= tCurrent){
            // release tc
            TmTcMessage releaseMsg(it->second.storeId);
            auto sendRet = this->requestQueue->sendMessage(recipientMsgQueueId, &releaseMsg, false);

            if (sendRet != HasReturnvaluesIF::RETURN_OK){
                return sendRet;
            }

            telecommandMap.erase(it);
        }
        else {
            break;  //save some time as multimap is sorted anyway
        }
    }

    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t Service11TelecommandScheduling::initialize() {

    ReturnValue_t res = PusServiceBase::initialize();
    if (res != HasReturnvaluesIF::RETURN_OK) {
        return res;
    }

    tcStore = objectManager->get<StorageManagerIF>(objects::TC_STORE);
    if (not tcStore) {
        return ObjectManagerIF::CHILD_INIT_FAILED;
    }

    tcRecipient = objectManager->get<AcceptsTelecommandsIF>(objects::CCSDS_PACKET_DISTRIBUTOR);
    if (not tcRecipient){
        return ObjectManagerIF::CHILD_INIT_FAILED;
    }
    recipientMsgQueueId = tcRecipient->getRequestQueue();

    return res;
}



ReturnValue_t Service11TelecommandScheduling::handleRequest_InsertActivity() {

    // storage for the raw data to be received
    const uint8_t* pRawData = nullptr;  // "(non-const) pointer to const unsigned 8-bit int"

    // get serialized data packet
    pRawData = this->currentPacket.getApplicationData();
    if (pRawData == nullptr) {
        return RETURN_FAILED;
    }

    size_t appDataSize = this->currentPacket.getApplicationDataSize();

    //DEBUG
    sif::printInfo("currentPacket size: %d\n", this->currentPacket.getApplicationDataSize());


    uint32_t deserializedTimestamp = 0;

    // for better understanding:
    // pRawData: raw data buffer to be de-serialized
    // size: remaining size of buffer to de-serialize. Is decreased by function (until 0 I assume)
    // I assume: Function de-serializes byte-wise until remaining size is 0
    ReturnValue_t ret = SerializeAdapter::deSerialize<uint32_t>(&deserializedTimestamp,
            &pRawData, &appDataSize, SerializeIF::Endianness::BIG);
    if (ret != RETURN_OK){
        return ret;
    }

    // deserializedTimestamp should now be the correct UNIX timestamp in sec,
    // without any further bit-shifting etc.


    // get store address
    store_address_t addr = this->currentPacket.getStoreAddress();   // this can be done nicer
    if (addr.raw == storeId::INVALID_STORE_ADDRESS) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    TelecommandStruct tc(deserializedTimestamp, addr);
    auto it = telecommandMap.insert(std::pair<uint32_t, TelecommandStruct>(deserializedTimestamp, tc));
    if (it == telecommandMap.end()){
        return HasReturnvaluesIF::RETURN_FAILED;
    }



    return HasReturnvaluesIF::RETURN_OK;
}
