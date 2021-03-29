#ifndef SAM9G20_PUS_SERVICE9CUSTOMTIMEMANAGEMENT_H_
#define SAM9G20_PUS_SERVICE9CUSTOMTIMEMANAGEMENT_H_

#include <fsfw/pus/Service9TimeManagement.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>

#define USE_EPOCH_TO_SET_RTT_RTC

#ifndef USE_EPOCH_TO_SET_RTT_RTC
#define USE_ISIS_TIME_TO_SET_RTT_RTC
#endif

class Service9CustomTimeManagement: public Service9TimeManagement {
public:
	// Epoch DOW is Thursday, which is Index 5 in the ISIS driver, but we need
	// to substract 1 to do proper calculations
	static constexpr uint8_t EPOCH_DAY_OF_WEEK_FOR_CALC = 4;
	static constexpr uint32_t SECS_PER_DAY = 86400;

	static constexpr Event ISIS_CLOCK_SET_FAILURE = MAKE_EVENT(128, severity::LOW); //!< Clock could not be set. P1 New uptime. P2: Old uptime.

	Service9CustomTimeManagement(object_id_t objectId, uint16_t apid,
			uint8_t serviceId);

	ReturnValue_t setTime() override;
private:
	int setIsisClock(timeval& timeval);
};



#endif /* MISSION_PUS_SERVICEPACKETS_SERVICE9CUSTOMTIMEMANAGEMENT_H_ */
