
/**
 * \file Service19EventActionService.h
 *
 */

#ifndef MISSION_PUS_SERVICE19EVENTAction_H_
#define MISSION_PUS_SERVICE19EVENTAction_H_

#include <fsfw/tmtcservices/PusServiceBase.h>

/**
 * \brief Report on-board events like information or errors
 *
 * Full Documentation: ECSS-E70-41A p.79
 * Documentation: Dissertation Baetz p.135,136
 *
 * This service provides for the reporting to the service user of information of
 * operational significance.
 *   a. reporting of failures or anomalies detected on-board;
 *	 b. reporting of autonomous on-board actions;
 *	 c. reporting of normal progress of operations and activities, e.g. detection of
 * events which are not anomalous (such as payload events), reaching of
 * predefined steps in an operation. Some reports can combine more than one of these events.
 *
 *	Minimum capabilities of this service:
 *
 *   - TM[5,1]: Normal/Progress Report
 *   - TM[5,2]: Error/Anomaly Report - Low Severity
 *	 - TM[5,3]: Error/Anomaly Report - Medium Severity
 *	 - TM[5,4]: Error/Anomaly Report - High Severity
 *
 *
 * Events can be translated by using translator files located in /config/objects/ and /config/events/
 * Description to events can be added by adding a comment behind the event definition with [//!<] as leading string
 *
 * Additional capabilities of this service:
 *
 *	  - TC[5,5]: Enable Event Report Generation (Req. 6)
 *	  - TC[5,6]: Disable Event Report Generation (Req. 5)
 *
 *
 * \ingroup pus_services
 */
class Service19EventAction : public PusServiceBase
{
public:
	Service19EventAction(object_id_t objectId, uint16_t apid);
	virtual ~Service19EventAction();

	/***
	* Check for events and generate event reports if required
	* @return
	*/
	virtual ReturnValue_t performService();

	/***
	 * Turn event generation on or off
	 * @return
	 */
	virtual ReturnValue_t handleRequest();

	/***
	 * The default PusServiceBase initialize has been overridden but is still executed
	 * Registers this service as a listener for events at the EventManager.
	 * @return
	 */
	virtual ReturnValue_t initialize();

	static const uint8_t MAX_NUMBER_OF_REPORTS_PER_QUEUE = 10;
	enum Subservice
	{

	};

	uint16_t packetSubCounter;
	MessageQueueIF *eventQueue;

private:
};

#endif /* MISSION_PUS_SERVICE5EVENTREPORTING_H_ */
