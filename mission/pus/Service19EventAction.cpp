#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/events/EventManagerIF.h>
#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/events/EventMessage.h>
#include <fsfw/tmtcpacket/pus/TmPacketStored.h>
#include <tmtc/apid.h>
#include <tmtc/pusIds.h>
#include <mission/pus/Service19EventAction.h>

Service19EventAction::Service19EventAction(object_id_t objectId) :
    PusServiceBase(objectId, apid::SOURCE_OBSW, pus::PUS_SERVICE_19),
    packetSubCounter(0)
{
	eventQueue = QueueFactory::instance()->createMessageQueue();
}

Service19EventAction::~Service19EventAction()
{
}

ReturnValue_t Service19EventAction::performService()
{
	EventMessage message;
	ReturnValue_t status = eventQueue->receiveMessage(&message);
	uint8_t counter = 0;
	while (status == HasReturnvaluesIF::RETURN_OK)
	{
		if (counter > Service19EventAction::MAX_NUMBER_OF_REPORTS_PER_QUEUE)
		{
			break;
		}
		status = eventQueue->receiveMessage(&message);
		counter++;
	}

	return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t Service19EventAction::handleRequest()
{
	switch (currentPacket.getSubService())
	{
	default:
		return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
	}
}

/* In addition to the default PUSServiceBase initialization, this service needs to
be registered to the event manager to listen for events */
ReturnValue_t Service19EventAction::initialize() {
	EventManagerIF *manager = objectManager->get<EventManagerIF>(
		objects::EVENT_MANAGER);
	
	if (manager == NULL)
	{
		return RETURN_FAILED;
	}
	// register Service 19 as listener for events
	ReturnValue_t result = manager->registerListener(eventQueue->getId(), true);
	if (result != HasReturnvaluesIF::RETURN_OK)
	{
		return result;
	}

	return PusServiceBase::initialize();
}
