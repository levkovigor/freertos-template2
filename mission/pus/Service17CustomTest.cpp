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
	case CustomSubservice::MULTIPLE_EVENT_TRIGGER_TEST: {
		for(int i=0; i < 5; i++) {
			triggerEvent(TEST,1234,5678);
		}
		performMassEventTesting = true;
		return RETURN_OK;
	}
	case CustomSubservice::MULTIPLE_CONNECTION_TEST: {
		for(int i=0;i<12;i++) {
			TmPacketStored connectionPacket(apid, serviceId,
			        Subservice::CONNECTION_TEST_REPORT,
					packetSubCounter++);
			connectionPacket.sendPacket(requestQueue->getDefaultDestination(),
					requestQueue->getId());
		}
		performMassConnectionTesting = true;
		return RETURN_OK;
	}
	default:
	    return Service17Test::handleRequest(subservice);
	}
}

ReturnValue_t Service17CustomTest::performService() {
	if(performMassEventTesting && counter < 3)
	{
		for(int i=0;i<12;i++) {
			triggerEvent(TEST,1234,5678);
		}
	counter ++;
	}

	if(performMassConnectionTesting && counter < 3)
	{
		for(int i=0;i<12;i++) {
			TmPacketStored connection_packet(apid,
					serviceId, Subservice::CONNECTION_TEST_REPORT,
					packetSubCounter++);
			connection_packet.sendPacket(requestQueue->getDefaultDestination(),
			        requestQueue->getId());
		}
	counter ++;
	}

	if(counter == 3)
	{
		performMassEventTesting = false;
		performMassConnectionTesting = false;
		counter = 0;
	}
	return HasReturnvaluesIF::RETURN_OK;
}
