/**
 * @file EMAC_TestTask.h
 *
 * @date 19.11.2019
 */

#ifndef STM32_TMTCBRIDGE_EMAC_POLLINGTASK_H_
#define STM32_TMTCBRIDGE_EMAC_POLLINGTASK_H_

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>


/**
 * @brief Separate task to poll EMAC interface.
 * 		  Polled data is passed to the netif (lwIP)
 *
 *
 */
class EmacPollingTask:  public SystemObject, public ExecutableObjectIF, public HasReturnvaluesIF {
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
	static const uint8_t PERIODIC_HANDLE_TRIGGER = 5;
	uint8_t periodicHandleCounter;
};


#endif /* STM32_TMTCBRIDGE_EMAC_POLLINGTASK_H_ */
