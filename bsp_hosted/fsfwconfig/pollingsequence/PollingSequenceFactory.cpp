#include "PollingSequenceFactory.h"
#include <objects/systemObjectList.h>

#include <fsfw/objectmanager/ObjectManagerIF.h>
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/devicehandlers/DeviceHandlerIF.h>
#include <fsfw/tasks/FixedTimeslotTaskIF.h>

ReturnValue_t pst::pollingSequenceInitDefault(FixedTimeslotTaskIF *thisSequence)
{
	/* Length of a communication cycle */
	uint32_t length = thisSequence->getPeriodMs();

    thisSequence->addSlot(objects::DUMMY_HANDLER_0,
            length * 0,  DeviceHandlerIF::PERFORM_OPERATION);
    thisSequence->addSlot(objects::DUMMY_HANDLER_0,
            length * 0.4,  DeviceHandlerIF::SEND_WRITE);
    thisSequence->addSlot(objects::DUMMY_HANDLER_0,
            length * 0.55, DeviceHandlerIF::GET_WRITE);
    thisSequence->addSlot(objects::DUMMY_HANDLER_0,
            length * 0.7,  DeviceHandlerIF::SEND_READ);
    thisSequence->addSlot(objects::DUMMY_HANDLER_0,
            length * 0.85, DeviceHandlerIF::GET_READ);

	if (thisSequence->checkSequence() == HasReturnvaluesIF::RETURN_OK) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "PollingSequence::initialize has errors!" << std::endl;
#endif
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

