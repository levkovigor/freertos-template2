#include <mission/pus/Service9TimeManagement.h>
#include <fsfw/timemanager/CCSDSTime.h>

#include <apid.h>
#include <pusIds.h>

#include <fsfw/events/EventManagerIF.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <mission/pus/servicepackets/Service9Packets.h>


Service9TimeManagement::Service9TimeManagement(object_id_t objectId_) :
	PusServiceBase(objectId_,apid::SOURCE_OBSW,pus::PUS_SERVICE_9) {

}

Service9TimeManagement::~Service9TimeManagement() {
}

ReturnValue_t Service9TimeManagement::performService() {
	return RETURN_OK;
}

ReturnValue_t Service9TimeManagement::handleRequest(uint8_t subservice) {
	switch(subservice){
		case SUBSERVICE::SET_TIME:{
			ReturnValue_t result = setTime();
			return result;
		}
		default:
			return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
		}
}


ReturnValue_t Service9TimeManagement::setTime() {
	Clock::TimeOfDay_t timeToSet;
	TimePacket timePacket(currentPacket.getApplicationData(),
			currentPacket.getApplicationDataSize());
	ReturnValue_t result = CCSDSTime::convertFromCcsds(&timeToSet,
			timePacket.getTime(), timePacket.getTimeSize());
	if(result != RETURN_OK) {
		sif::error << "Service 9: Could not convert CCSDS to "
				 "Time of Day format" << std::endl;
		return result;
	}
	sif::info << "Service 9 Setting Clock" << std::endl;
	uint32_t formerUptime;
	Clock::getUptime(&formerUptime);
	result = Clock::setClock(&timeToSet);
	if(result == RETURN_OK) {
		sif::info << "Service 9: Clock set"  <<  std::endl;
		uint32_t newUptime;
		Clock::getUptime(&newUptime);
		triggerEvent(CLOCK_SET,newUptime,formerUptime);
		return RETURN_OK;
	}
	else {
		sif::error << "Service 9: Setting time failed" << std::endl;
		return RETURN_FAILED;
	}
}
