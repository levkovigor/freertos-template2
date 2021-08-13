#ifndef SAM9G20_TMTCBRIDGE_EMACPOLLINGTASK_H_
#define SAM9G20_TMTCBRIDGE_EMACPOLLINGTASK_H_

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>

extern "C" {
#include <at91/utility/trace.h>
}

/**
 * @brief Separate task to poll EMAC interface.
 * 		  Polled data is passed to the netif (lwIP)
 */
class EmacPollingTask:  public SystemObject,
		public ExecutableObjectIF, public HasReturnvaluesIF {
public:
	EmacPollingTask(object_id_t objectId_);
	virtual ~EmacPollingTask();

	virtual ReturnValue_t initialize();

	/**
	 * Executed periodically (set task priority and periodicity in init_mission)
	 * @param operationCode
	 * @return
	 */
	virtual ReturnValue_t performOperation(uint8_t operationCode = 0);
private:
	/* these values specify the periodicity of the checks */
	static const uint8_t CONNECTOR_CHECK_TRIGGER = 40;
	static const uint8_t DISCONNECT_CHECK_TRIGGER = 200;

	void checkEthernetConnection();
	void checkForDisconnect();

	uint8_t connectorCheckCounter;
	uint8_t disconnectCheckCounter;

};


#endif /* SAM9G20_TMTCBRIDGE_EMACPOLLINGTASK_H_ */
