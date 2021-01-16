#include "Service11TelecommandScheduling.h"
#include "etl/utility.h"
//#include <fsfw/returnvalues/HasReturnvaluesIF.h>	// this is probably not needed
#include <fsfw/serialize/SerializeAdapter.h>


Service11TelecommandScheduling::Service11TelecommandScheduling(
        object_id_t objectId, uint16_t apid, uint8_t serviceId):
        PusServiceBase(objectId, apid, serviceId) {

}


Service11TelecommandScheduling::~Service11TelecommandScheduling() { }


ReturnValue_t Service11TelecommandScheduling::handleRequest(
        uint8_t subservice) {

	// storage for the raw data to be received
	    const uint8_t* pRawData = nullptr;	// "(non-const) pointer to const unsigned 8-bit int"
	    size_t size = 0;

	// get serialized data packet
	ReturnValue_t ret = this->currentPacket.getData(&pRawData, &size);
	if (ret != RETURN_OK){
		return ret;
	}
	else if (pRawData == nullptr){
		//NOTE: Don't know whether this is necessary, prevents nullpointer crashes though
		return RETURN_FAILED;
	}

	//TODO: parse the duration (first 4 bytes here)
	uint32_t parsedDuration = 1;


	//test
	uint32_t object = 0;

	ret = SerializeAdapter::deSerialize<uint32_t>(&object, &pRawData, &size, SerializeIF::Endianness::BIG);
	if (ret != RETURN_OK){
		return ret;
	}



	// get store address
	store_address_t addr = this->currentPacket.getStoreAddress();	// this can be done nicer
	if (addr.raw == storeId::INVALID_STORE_ADDRESS){
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	// instanciate tcStruct & insert into multimap
	TelecommandStruct tc(parsedDuration, addr);
	telecommandMap.insert(std::pair<uint32_t, TelecommandStruct>(parsedDuration, tc));


    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t Service11TelecommandScheduling::performService() {

	//sif::info << "Service11TelecommandScheduling performing." << std::endl;

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

	return res;

}
