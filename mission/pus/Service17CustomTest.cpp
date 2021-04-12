#include "Service17CustomTest.h"

#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tmtcpacket/pus/TmPacketStoredPusA.h>


Service17CustomTest::Service17CustomTest(object_id_t object_id,
        uint16_t apid, uint8_t serviceId):
    Service17Test(object_id, apid, serviceId) {
}

Service17CustomTest::~Service17CustomTest() {
}

ReturnValue_t Service17CustomTest::handleRequest(uint8_t subservice) {
	switch(subservice) {
	case Subservice::CONNECTION_TEST:
	case Subservice::EVENT_TRIGGER_TEST: {
	    return Service17Test::handleRequest(subservice);
	}
	case CustomSubservice::ENABLE_PERIODIC_PRINT: {
		periodicPrintoutEnabled = true;
		break;
	}
	case CustomSubservice::DISABLE_PERIODIC_PRINT: {
	    periodicPrintoutEnabled = false;
	    break;
	}
	case CustomSubservice::TRIGGER_EXCEPTION: {
	    Service17CustomTest* exception = nullptr;
	    exception->performService();
	    break;
	}
	default:
	    return Service17Test::handleRequest(subservice);
	}
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service17CustomTest::performService() {
	if(periodicPrintoutEnabled) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
	    sif::info << "Service17CustomTest::performService: Alive!" << std::endl;
#else
	    sif::printInfo("Service17CustomTest::performService: Alive!\n");
#endif
	}
	return HasReturnvaluesIF::RETURN_OK;
}
