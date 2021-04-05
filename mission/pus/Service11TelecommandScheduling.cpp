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
        PusServiceBase(objectId, apid, serviceId) { }


Service11TelecommandScheduling::~Service11TelecommandScheduling() { }


ReturnValue_t Service11TelecommandScheduling::handleRequest(uint8_t subservice) {

    switch(subservice){
    case Subservice::INSERT_ACTIVITY:
        return this->handleRequest_InsertActivity();
    case Subservice::DELETE_ACTIVITY:
        return this->handleRequest_DeleteActivity();
    case Subservice::TIMESHIFT_ACTIVITY:
        return this->handleRequest_TimeshiftActivity();
    default:
        break;
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
            TmTcMessage releaseMsg(it->second.storeAddr);
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

    // Get de-serialized Timestamp
    uint32_t deserializedTimestamp = 0;
    auto ret = GetDeserializedTimestamp(deserializedTimestamp);
    if (ret != RETURN_OK){
        return ret;
    }

    // DEBUG
    sif::printInfo("Deserialized Timestamp: %d\n", deserializedTimestamp);


    // Get store address
    store_address_t addr = this->currentPacket.getStoreAddress();
    if (addr.raw == storeId::INVALID_STORE_ADDRESS) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    // Insert, if sched. time is above margin
    uint32_t tNow = static_cast<uint32_t>(std::time(nullptr));
    if (deserializedTimestamp - tNow <= this->TIME_MARGIN) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    TelecommandStruct tc(deserializedTimestamp, addr);
    auto it = telecommandMap.insert(std::pair<uint32_t, TelecommandStruct>(deserializedTimestamp, tc));
    if (it == telecommandMap.end()){
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t Service11TelecommandScheduling::handleRequest_DeleteActivity() {

    // get timestamp
    uint32_t deserializedTimestamp = 0;
    auto ret = this->GetDeserializedTimestamp(deserializedTimestamp);
    if (ret != RETURN_OK) {
        return ret;
    }

    // retrieve all corresponding timestamps, if any
    // multiple timestamp keys can be inside map
    auto range = telecommandMap.equal_range(deserializedTimestamp);

    if (range.first == range.second) {
        // no corresponding timestamp keys found
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    // get store address
    store_address_t addr = this->currentPacket.getStoreAddress();
    if (addr.raw == storeId::INVALID_STORE_ADDRESS) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    // erase position from multimap, if storeAddress is equal to currentPacket's storeAddress
    for (auto& it = range.first; it != range.second; ++it) {
        if (it->second.storeAddr == addr) {
            telecommandMap.erase(it);
        }
    }

    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t Service11TelecommandScheduling::handleRequest_TimeshiftActivity() {

    // get timestamp
    uint32_t deserializedTimestamp = 0;
    auto ret = this->GetDeserializedTimestamp(deserializedTimestamp);
    if (ret != RETURN_OK) {
        return ret;
    }

    // get store address
    store_address_t addr = this->currentPacket.getStoreAddress();
    if (addr.raw == storeId::INVALID_STORE_ADDRESS) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    uint32_t tNow = static_cast<uint32_t>(std::time(nullptr));

    // find store address as value in Map (this is inefficient)
    for (auto it = telecommandMap.begin(); it != telecommandMap.end(); ++it) {
        if (it->second.storeAddr == addr) {
            telecommandMap.erase(it);

            TelecommandStruct tc(deserializedTimestamp, addr);
            auto insertIt = telecommandMap.insert(std::pair<uint32_t, TelecommandStruct>(deserializedTimestamp, tc));
            if (insertIt == telecommandMap.end()){
                return HasReturnvaluesIF::RETURN_FAILED;
            }
            return HasReturnvaluesIF::RETURN_OK;
        }
    }

    return HasReturnvaluesIF::RETURN_OK;
}



ReturnValue_t Service11TelecommandScheduling::GetDeserializedTimestamp(uint32_t& timestamp) {

    // storage for the raw data to be received
    const uint8_t* pRawData = nullptr;  // "(non-const) pointer to const unsigned 8-bit int"

    // get serialized data packet
    pRawData = this->currentPacket.getApplicationData();
    if (pRawData == nullptr) {
        return RETURN_FAILED;
    }

    size_t appDataSize = this->currentPacket.getApplicationDataSize();

    // for better understanding:
    // pRawData: raw data buffer to be de-serialized
    // size: remaining size of buffer to de-serialize. Is decreased by function (until 0 I assume)
    // => Function de-serializes byte-wise until remaining size is 0
    ReturnValue_t ret = SerializeAdapter::deSerialize<uint32_t>(&timestamp,
            &pRawData, &appDataSize, SerializeIF::Endianness::BIG);
    if (ret != RETURN_OK){
        return ret;
    }

    // deserializedTimestamp should now be the correct UNIX timestamp in sec,
    // without any further bit-shifting etc.

    return HasReturnvaluesIF::RETURN_OK;
}



