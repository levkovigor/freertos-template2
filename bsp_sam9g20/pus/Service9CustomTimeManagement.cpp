#include <OBSWConfig.h>

#include <bsp_sam9g20/pus/Service9CustomTimeManagement.h>

#include <fsfw/pus/servicepackets/Service9Packets.h>
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/timemanager/CCSDSTime.h>

extern "C" {
#include <hal/Timing/Time.h>
}

Service9CustomTimeManagement::Service9CustomTimeManagement(object_id_t objectId,
		uint16_t apid, uint8_t serviceId):
		Service9TimeManagement(objectId, apid, serviceId) {}

int Service9CustomTimeManagement::setIsisClock(timeval& timeval) {
#ifdef USE_ISIS_TIME_TO_SET_RTT_RTC
    Clock::TimeOfDay_t timeOfDayFsfw;
    Clock::getDateAndTime(&timeOfDayFsfw);
	// set the time of RTT / RTC as well
	Time newTime;
	newTime.seconds = timeOfDay.second;
	newTime.minutes = timeOfDay.minute;
	newTime.hours = timeOfDay.hour;
	uint32_t daysSinceEpoch = timeval.tv_sec / SECS_PER_DAY;
	// Special calculation to calculate the day of week for ISIS drivers.
	newTime.day = ((daysSinceEpoch + EPOCH_DAY_OF_WEEK_FOR_CALC) % 7) + 1;
	newTime.date = timeOfDay.day;
	newTime.month = timeOfDay.month;
	newTime.year = timeOfDay.year;
	return Time_set(&newTime);
#else
	return Time_setUnixEpoch(timeval.tv_sec);
#endif
}

ReturnValue_t Service9CustomTimeManagement::setTime() {
    timeval newTime;
	TimePacket timePacket(currentPacket.getApplicationData(),
			currentPacket.getApplicationDataSize());
	size_t foundLen = 0;
	ReturnValue_t result = CCSDSTime::convertFromCcsds(&newTime,
            timePacket.getTime(), &foundLen, timePacket.getTimeSize());
	if(result != RETURN_OK) {
		triggerEvent(CLOCK_SET_FAILURE, result, 0);
		return result;
	}

	timeval formerTime;
	Clock::getClock_timeval(&formerTime);

	result = Clock::setClock(&newTime);
	if(result == RETURN_OK) {
#if OBSW_VERBOSE_LEVEL >= 1
	    sif::printInfo("Clock set to new value!\n");
#endif
		triggerEvent(CLOCK_SET, formerTime.tv_sec, newTime.tv_sec);
	}
	else {
		triggerEvent(CLOCK_SET_FAILURE, result, 0);
	}

	int retval = setIsisClock(newTime);
	if(retval != 0) {
		triggerEvent(ISIS_CLOCK_SET_FAILURE, formerTime.tv_sec, newTime.tv_sec);
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	return HasReturnvaluesIF::RETURN_OK;
}
