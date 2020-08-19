#include <stm32/config/pollingsequence/PollingSequenceFactory.h>
#include <stm32/config/objects/systemObjectList.h>
#include <fsfw/objectmanager/ObjectManagerIF.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/devicehandlers/DeviceHandlerIF.h>
#include <fsfw/tasks/FixedTimeslotTaskIF.h>

ReturnValue_t pst::pollingSequenceInitDefault(FixedTimeslotTaskIF *thisSequence)
{
	/* Length of a communication cycle */
	uint32_t length = thisSequence->getPeriodMs();

	thisSequence->addSlot(objects::PCDU_HANDLER,
			length * 0.15, DeviceHandlerIF::SEND_WRITE);
    thisSequence->addSlot(objects::GPS0_HANDLER,
    		length * 0.3,  DeviceHandlerIF::SEND_WRITE);
    thisSequence->addSlot(objects::GPS1_HANDLER,
    		length * 0.4,  DeviceHandlerIF::SEND_WRITE);

    /* GET_WRITE and SEND_READ not used yet*/
	thisSequence->addSlot(objects::PCDU_HANDLER,
			length * 0.5, DeviceHandlerIF::GET_WRITE);
	thisSequence->addSlot(objects::GPS0_HANDLER,
			length * 0.5,  DeviceHandlerIF::GET_WRITE);
	thisSequence->addSlot(objects::GPS1_HANDLER,
			length * 0.5,  DeviceHandlerIF::GET_WRITE);

	thisSequence->addSlot(objects::PCDU_HANDLER,
			length * 0.55,  DeviceHandlerIF::SEND_READ);
	thisSequence->addSlot(objects::GPS0_HANDLER,
			length * 0.55,  DeviceHandlerIF::SEND_READ);
	thisSequence->addSlot(objects::GPS1_HANDLER,
			length * 0.55,  DeviceHandlerIF::SEND_READ);

	thisSequence->addSlot(objects::PCDU_HANDLER,
			length * 0.7, DeviceHandlerIF::GET_READ);
	thisSequence->addSlot(objects::GPS0_HANDLER,
			length * 0.8,  DeviceHandlerIF::GET_READ);
	thisSequence->addSlot(objects::GPS1_HANDLER,
			length * 0.9,  DeviceHandlerIF::GET_READ);

    thisSequence->addSlot(objects::DUMMY_HANDLER,
            length * 0,  DeviceHandlerIF::SEND_WRITE);
    thisSequence->addSlot(objects::DUMMY_HANDLER,
            length * 0.2, DeviceHandlerIF::GET_WRITE);
    thisSequence->addSlot(objects::DUMMY_HANDLER,
            length * 0.4,  DeviceHandlerIF::SEND_READ);
    thisSequence->addSlot(objects::DUMMY_HANDLER,
            length * 0.8, DeviceHandlerIF::GET_READ);

	if (thisSequence->checkSequence() == HasReturnvaluesIF::RETURN_OK) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
		sif::error << "PollingSequence::initialize has errors!" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

ReturnValue_t pst::pollingSequenceInitTest(FixedTimeslotTaskIF *thisSequence) {
	/* Length of a communication cycle */
	uint32_t length = thisSequence->getPeriodMs();

	thisSequence->addSlot(objects::ARDUINO_0,
			length * 0,  DeviceHandlerIF::SEND_WRITE);
	thisSequence->addSlot(objects::ARDUINO_0,
			length * 0.3,  DeviceHandlerIF::GET_WRITE);
	thisSequence->addSlot(objects::ARDUINO_0,
			length * 0.5,  DeviceHandlerIF::SEND_READ);
	thisSequence->addSlot(objects::ARDUINO_0,
			length * 0.8,  DeviceHandlerIF::GET_READ);

	thisSequence->addSlot(objects::ARDUINO_1,
			length * 0,  DeviceHandlerIF::SEND_WRITE);
	thisSequence->addSlot(objects::ARDUINO_1,
			length * 0.2,  DeviceHandlerIF::GET_WRITE);
	thisSequence->addSlot(objects::ARDUINO_1,
			length * 0.4,  DeviceHandlerIF::SEND_READ);
	thisSequence->addSlot(objects::ARDUINO_1,
			length * 0.8,  DeviceHandlerIF::GET_READ);

	thisSequence->addSlot(objects::ARDUINO_2,
			length * 0,  DeviceHandlerIF::SEND_WRITE);
	thisSequence->addSlot(objects::ARDUINO_2,
			length * 0.2,  DeviceHandlerIF::GET_WRITE);
	thisSequence->addSlot(objects::ARDUINO_2,
			length * 0.4,  DeviceHandlerIF::SEND_READ);
	thisSequence->addSlot(objects::ARDUINO_2,
			length * 0.8,  DeviceHandlerIF::GET_READ);

//	thisSequence->addSlot(objects::ARDUINO_3,
//			length * 0,  DeviceHandlerIF::SEND_WRITE);
//	thisSequence->addSlot(objects::ARDUINO_3,
//			length * 0.2,  DeviceHandlerIF::GET_WRITE);
//	thisSequence->addSlot(objects::ARDUINO_3,
//			length * 0.4,  DeviceHandlerIF::SEND_READ);
//	thisSequence->addSlot(objects::ARDUINO_3,
//			length * 0.8,  DeviceHandlerIF::GET_READ);
//
//	thisSequence->addSlot(objects::ARDUINO_4,
//			length * 0,  DeviceHandlerIF::SEND_WRITE);
//	thisSequence->addSlot(objects::ARDUINO_4,
//			length * 0.2,  DeviceHandlerIF::GET_WRITE);
//	thisSequence->addSlot(objects::ARDUINO_4,
//			length * 0.4,  DeviceHandlerIF::SEND_READ);
//	thisSequence->addSlot(objects::ARDUINO_4,
//			length * 0.8,  DeviceHandlerIF::GET_READ);

	if (thisSequence->checkSequence() == HasReturnvaluesIF::RETURN_OK) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
		sif::error << "PollingSequence::initialize has errors!" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}
