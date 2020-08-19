#ifndef MISSION_PUS_SERVICE9TIMEMANAGEMENT_H_
#define MISSION_PUS_SERVICE9TIMEMANAGEMENT_H_

#include <fsfw/tmtcservices/PusServiceBase.h>

class Service9TimeManagement: public PusServiceBase{
public:

	/**
	 * 	@brief This service provides the capability to set the on-board time.
	 */
	Service9TimeManagement(object_id_t objectId_);

	virtual ~Service9TimeManagement();

	ReturnValue_t performService() override;

	/**
	 * 	@brief Sets the onboard-time by retrieving the time to set from TC[9,128].
	 *
	 */
	ReturnValue_t handleRequest(uint8_t subservice) override;
private:

	ReturnValue_t setTime();
	enum SUBSERVICE {
		SET_TIME = 128 //!< [EXPORT] : [COMMAND] Time command in ASCII, CUC or CDS format
	};


	static constexpr uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::PUS_SERVICE_9;
	static constexpr Event CLOCK_SET = MAKE_EVENT(0, SEVERITY::INFO); //!< Clock has been set. P1: New Uptime. P2: Old Uptime
};



#endif /* MISSION_PUS_SERVICE9TIMEMANAGEMENT_H_ */
