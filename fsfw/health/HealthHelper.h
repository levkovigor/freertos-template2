#ifndef FRAMEWORK_HEALTH_HEALTHHELPER_H_
#define FRAMEWORK_HEALTH_HEALTHHELPER_H_

#include "../events/EventManagerIF.h"
#include "../events/EventReportingProxyIF.h"
#include "../health/HasHealthIF.h"
#include "../health/HealthMessage.h"
#include "../health/HealthTableIF.h"
#include "../ipc/MessageQueueIF.h"
#include "../objectmanager/ObjectManagerIF.h"
#include "../returnvalues/HasReturnvaluesIF.h"

/**
 * Helper class for Objects that implement HasHealthIF
 *
 * It takes care of registering with the Health Table as well as handling health commands
 * (including replying to the sender) and updating the Health Table.
 *
 * If a parent is set in the ctor, the parent will be informed with a @c HEALTH_INFO message
 * about changes in the health state. Note that a @c HEALTH_INFO is only generated if the Health
 * changes, not for all @c HEALTH_SET commands received.
 *
 * It does NOT handle @c HEALTH_INFO messages
 */
class HealthHelper {
public:

	/**
	 * ctor
	 *
	 * @param owner
	 * @param objectId the object Id to use when communication with the HealthTable
	 */
	HealthHelper(HasHealthIF* owner, object_id_t objectId);

	virtual ~HealthHelper();

	/**
	 * Pointer to the Health Table
	 *
	 * only valid after initialize() has been called
	 */
	HealthTableIF *healthTable = nullptr;

	/**
	 * Proxy to forward events.
	 */
	EventReportingProxyIF* eventSender = nullptr;

	/**
	 * Try to handle the message.
	 *
	 * This function handles @c HEALTH_SET and @c HEALTH_READ commands.
	 * it updates the Health Table and generates a reply to the sender.
	 *
	 * @param message
	 * @return
	 * 			-@c RETURN_OK if the message was handled
	 * 			-@c RETURN_FAILED if the message could not be handled (ie it was not a @c HEALTH_SET or @c HEALTH_READ message)
	 */
	ReturnValue_t handleHealthCommand(CommandMessage *message);

	/**
	 * set the Health State
	 *
	 * The parent will be informed, if the Health changes
	 *
	 * @param health
	 */
	void setHealth(HasHealthIF::HealthState health);

	/**
	 * get Health State
	 *
	 * @return Health State of the object
	 */
	HasHealthIF::HealthState getHealth();

	/**
	 * @param parentQueue the Queue id of the parent object. Set to 0 if no parent present
	 */
	void setParentQueue(MessageQueueId_t parentQueue);

	/**
	 *
	 * @param parentQueue the Queue id of the parent object. Set to 0 if no parent present
	 * @return
	 * 			-@c RETURN_OK if the Health Table was found and the object could be registered
	 * 			-@c RETURN_FAILED else
	 */
	ReturnValue_t initialize(MessageQueueId_t parentQueue );

	ReturnValue_t initialize();

private:
	/**
	 * the object id to use when communicating with the Health Table
	 */
	object_id_t objectId;

	/**
	 * The Queue of the parent
	 */
	MessageQueueId_t parentQueue = MessageQueueIF::NO_QUEUE;

	/**
	 * The one using the healthHelper.
	 */
	HasHealthIF* owner;

	/**
	 * if the #parentQueue is not NULL, a @c HEALTH_INFO message will be sent to this queue
	 * @param health the health is passed as parameter so that the number of calls to the health table can be minimized
	 * @param oldHealth information of the previous health state.
	 */
	void informParent(HasHealthIF::HealthState health, HasHealthIF::HealthState oldHealth);

	void handleSetHealthCommand(CommandMessage *message);
};

#endif /* HEALTHHELPER_H_ */
