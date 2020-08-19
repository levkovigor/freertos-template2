#include <mission/pus/Service17Test.h>

#include <apid.h>
#include <pusIds.h>

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tmtcpacket/pus/TmPacketStored.h>

//#include <fsfw/osal/FreeRTOS/TaskManagement.h>

Service17Test::Service17Test(object_id_t object_id):
    PusServiceBase(object_id, apid::SOURCE_OBSW, pus::PUS_SERVICE_17),
    packetSubCounter(0) {
}

Service17Test::~Service17Test() {
}

ReturnValue_t Service17Test::handleRequest(uint8_t subservice) {
	switch(subservice){
	case Subservice::CONNECTION_TEST: {
		TmPacketStored connectionPacket(apid::SOURCE_OBSW, pus::PUS_SERVICE_17,
		        Subservice::CONNECTION_TEST_REPORT, packetSubCounter++);
		connectionPacket.sendPacket(requestQueue->getDefaultDestination(),
				requestQueue->getId());
		return HasReturnvaluesIF::RETURN_OK;
	}
	case Subservice::EVENT_TRIGGER_TEST: {
		TmPacketStored connectionPacket(apid::SOURCE_OBSW, pus::PUS_SERVICE_17,
		        Subservice::CONNECTION_TEST_REPORT, packetSubCounter++);
		connectionPacket.sendPacket(requestQueue->getDefaultDestination(),
				requestQueue->getId());
		triggerEvent(TEST, 1234, 5678);
		return RETURN_OK;
	}
	case Subservice::MULTIPLE_EVENT_TRIGGER_TEST: {
		for(int i=0; i < 5; i++) {
			triggerEvent(TEST,1234,5678);
		}
		performMassEventTesting = true;
		return RETURN_OK;
	}
	case Subservice::MULTIPLE_CONNECTION_TEST: {
		for(int i=0;i<12;i++) {
			TmPacketStored connectionPacket(apid::SOURCE_OBSW,
					pus::PUS_SERVICE_17, Subservice::CONNECTION_TEST_REPORT,
					packetSubCounter++);
			connectionPacket.sendPacket(requestQueue->getDefaultDestination(),
					requestQueue->getId());
		}
		performMassConnectionTesting = true;
		return RETURN_OK;
	}
	default:
		return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
	}
}

ReturnValue_t Service17Test::performService() {
	//sif::info << "Great, I am still alive!" << std::endl;
	//sif::info << TaskManagement::getTaskStackHighWatermark() << std::endl;
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
			TmPacketStored connection_packet(apid::SOURCE_OBSW,
					pus::PUS_SERVICE_17,Subservice::CONNECTION_TEST_REPORT,
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
