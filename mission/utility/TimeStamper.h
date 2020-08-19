#ifndef MISSION_UTILITY_TIMESTAMPER_H_
#define MISSION_UTILITY_TIMESTAMPER_H_

#include <fsfw/timemanager/TimeStamperIF.h>
#include <fsfw/timemanager/CCSDSTime.h>
#include <fsfw/objectmanager/SystemObject.h>

/**
 * @brief
 * @ingroup utility
 */
class TimeStamper: public TimeStamperIF, public SystemObject {
public:
	TimeStamper(object_id_t objectId);
	virtual ReturnValue_t addTimeStamp(uint8_t* buffer, const uint8_t maxSize);
};

#endif /* MISSION_UTILITY_TIMESTAMPER_H_ */
