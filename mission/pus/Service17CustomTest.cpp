#include "Service17CustomTest.h"

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tmtcpacket/pus/TmPacketStored.h>


Service17CustomTest::Service17CustomTest(object_id_t object_id,
        uint16_t apid, uint8_t serviceId):
    Service17Test(object_id, apid, serviceId) {
}

Service17CustomTest::~Service17CustomTest() {
}

ReturnValue_t Service17CustomTest::handleRequest(uint8_t subservice) {
	switch(subservice){
	case CustomSubservice::ENABLE_PERIODIC_PRINT: {
		periodicPrintoutEnabled = true;
		break;
	}
	case CustomSubservice::DISABLE_PERIODIC_PRINT: {
	    periodicPrintoutEnabled = false;
	    break;
	}
	default:
	    return Service17Test::handleRequest(subservice);
	}
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service17CustomTest::performService() {
	if(periodicPrintoutEnabled) {
	    sif::info << "Service17CustomTest::performService: Alive!" << std::endl;
	}
	return HasReturnvaluesIF::RETURN_OK;
}
