#include "PollingSequenceFactory.h"

#include <sam9g20/comIF/RS485Controller.h>

#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/devicehandlers/DeviceHandlerIF.h>
#include <fsfw/tasks/FixedTimeslotTaskIF.h>
#include <fsfwconfig/objects/systemObjectList.h>

ReturnValue_t pst::pollingSequenceInitDefault(
        FixedTimeslotTaskIF *thisSequence) {
	/* Length of a communication cycle */
	uint32_t length = thisSequence->getPeriodMs();

	thisSequence->addSlot(objects::PCDU_HANDLER,
	        length * 0, DeviceHandlerIF::PERFORM_OPERATION);
    thisSequence->addSlot(objects::PCDU_HANDLER,
            length * 0.4, DeviceHandlerIF::SEND_WRITE);
	thisSequence->addSlot(objects::PCDU_HANDLER,
	        length * 0.55, DeviceHandlerIF::GET_WRITE);
	thisSequence->addSlot(objects::PCDU_HANDLER,
	        length * 0.7,  DeviceHandlerIF::SEND_READ);
	thisSequence->addSlot(objects::PCDU_HANDLER,
	        length * 0.85, DeviceHandlerIF::GET_READ);

    thisSequence->addSlot(objects::GPS0_HANDLER,
            length * 0, DeviceHandlerIF::PERFORM_OPERATION);
    thisSequence->addSlot(objects::GPS0_HANDLER,
    		length * 0.4,  DeviceHandlerIF::SEND_WRITE);
    thisSequence->addSlot(objects::GPS0_HANDLER,
            length * 0.55,  DeviceHandlerIF::GET_WRITE);
    thisSequence->addSlot(objects::GPS0_HANDLER,
            length * 0.7,  DeviceHandlerIF::SEND_READ);
    thisSequence->addSlot(objects::GPS0_HANDLER,
            length * 0.85,  DeviceHandlerIF::GET_READ);

    thisSequence->addSlot(objects::GPS1_HANDLER,
            length * 0, DeviceHandlerIF::PERFORM_OPERATION);
    thisSequence->addSlot(objects::GPS1_HANDLER,
    		length * 0.4,  DeviceHandlerIF::SEND_WRITE);
	thisSequence->addSlot(objects::GPS1_HANDLER,
			length * 0.55,  DeviceHandlerIF::GET_WRITE);
	thisSequence->addSlot(objects::GPS1_HANDLER,
			length * 0.7,  DeviceHandlerIF::SEND_READ);
	thisSequence->addSlot(objects::GPS1_HANDLER,
			length * 0.85,  DeviceHandlerIF::GET_READ);

    thisSequence->addSlot(objects::DUMMY_HANDLER,
            length * 0, DeviceHandlerIF::PERFORM_OPERATION);
    thisSequence->addSlot(objects::DUMMY_HANDLER,
            length * 0.4,  DeviceHandlerIF::SEND_WRITE);
    thisSequence->addSlot(objects::DUMMY_HANDLER,
            length * 0.55, DeviceHandlerIF::GET_WRITE);
    thisSequence->addSlot(objects::DUMMY_HANDLER,
            length * 0.7,  DeviceHandlerIF::SEND_READ);
    thisSequence->addSlot(objects::DUMMY_HANDLER,
            length * 0.85, DeviceHandlerIF::GET_READ);

	if (thisSequence->checkSequence() == HasReturnvaluesIF::RETURN_OK) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "pst::pollingSequenceInitDefault: Sequence invalid!"
		        << std::endl;
#else
		sif::printError("pst::pollingSequenceInitDefault: "
		        "Sequence invalid!\n");
#endif
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

ReturnValue_t pst::pollingSequenceInitRS485(FixedTimeslotTaskIF *thisSequence) {
    /* Length of a communication cycle */
    uint32_t length = thisSequence->getPeriodMs();

    thisSequence->addSlot(objects::RS485_CONTROLLER, length * 0,
            RS485Steps::SYRLINKS_ACTIVE);
    thisSequence->addSlot(objects::RS485_CONTROLLER, length * 0.4,
            RS485Steps::PCDU_VORAGO_ACTIVE);
    thisSequence->addSlot(objects::RS485_CONTROLLER, length * 0.65,
            RS485Steps::PL_VORAGO_ACTIVE);
    thisSequence->addSlot(objects::RS485_CONTROLLER, length * 0.85,
            RS485Steps::PL_PIC24_ACTIVE);

    if (thisSequence->checkSequence() == HasReturnvaluesIF::RETURN_OK) {
        return HasReturnvaluesIF::RETURN_OK;
    }
    else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "pst::pollingSequenceInitRS485: Sequence invalid!"
                << std::endl;
#else
        sif::printError("pst::pollingSequenceInitRS485: Sequence invalid!\n");
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }
}

ReturnValue_t pst::pollingSequenceInitTest(FixedTimeslotTaskIF *thisSequence) {
	/* Length of a communication cycle */
	uint32_t length = thisSequence->getPeriodMs();

//    thisSequence->addSlot(objects::SPI_Test_PT1000, length * 0,
//            DeviceHandlerIF::PERFORM_OPERATION);
//    thisSequence->addSlot(objects::SPI_Test_PT1000, length * 0.4,
//            DeviceHandlerIF::SEND_WRITE);
//    thisSequence->addSlot(objects::SPI_Test_PT1000, length * 0.55,
//            DeviceHandlerIF::GET_WRITE);
//    thisSequence->addSlot(objects::SPI_Test_PT1000, length * 0.7,
//            DeviceHandlerIF::SEND_READ);
//    thisSequence->addSlot(objects::SPI_Test_PT1000, length * 0.85,
//            DeviceHandlerIF::GET_READ);
//
//    thisSequence->addSlot(objects::SPI_Test_Gyro, length * 0,
//            DeviceHandlerIF::PERFORM_OPERATION);
//    thisSequence->addSlot(objects::SPI_Test_Gyro, length * 0.4,
//    		DeviceHandlerIF::SEND_WRITE);
//    thisSequence->addSlot(objects::SPI_Test_Gyro, length * 0.55,
//    		DeviceHandlerIF::GET_WRITE);
//    thisSequence->addSlot(objects::SPI_Test_Gyro, length * 0.7,
//    		DeviceHandlerIF::SEND_READ);
//    thisSequence->addSlot(objects::SPI_Test_Gyro, length * 0.85,
//    		DeviceHandlerIF::GET_READ);

    thisSequence->addSlot(objects::SPI_Test_MGM, length * 0,
            DeviceHandlerIF::PERFORM_OPERATION);
    thisSequence->addSlot(objects::SPI_Test_MGM, length * 0.4,
            DeviceHandlerIF::SEND_WRITE);
    thisSequence->addSlot(objects::SPI_Test_MGM, length * 0.55,
            DeviceHandlerIF::GET_WRITE);
    thisSequence->addSlot(objects::SPI_Test_MGM, length * 0.7,
            DeviceHandlerIF::SEND_READ);
    thisSequence->addSlot(objects::SPI_Test_MGM, length * 0.85,
            DeviceHandlerIF::GET_READ);

//	thisSequence->addSlot(objects::ARDUINO_0,
//			length * 0,  DeviceHandlerIF::SEND_WRITE);
//	thisSequence->addSlot(objects::ARDUINO_0,
//			length * 0.3,  DeviceHandlerIF::GET_WRITE);
//	thisSequence->addSlot(objects::ARDUINO_0,
//			length * 0.5,  DeviceHandlerIF::SEND_READ);
//	thisSequence->addSlot(objects::ARDUINO_0,
//			length * 0.8,  DeviceHandlerIF::GET_READ);
//
//	thisSequence->addSlot(objects::ARDUINO_1,
//			length * 0,  DeviceHandlerIF::SEND_WRITE);
//	thisSequence->addSlot(objects::ARDUINO_1,
//			length * 0.2,  DeviceHandlerIF::GET_WRITE);
//	thisSequence->addSlot(objects::ARDUINO_1,
//			length * 0.4,  DeviceHandlerIF::SEND_READ);
//	thisSequence->addSlot(objects::ARDUINO_1,
//			length * 0.8,  DeviceHandlerIF::GET_READ);
//
//	thisSequence->addSlot(objects::ARDUINO_2,
//			length * 0,  DeviceHandlerIF::SEND_WRITE);
//	thisSequence->addSlot(objects::ARDUINO_2,
//			length * 0.2,  DeviceHandlerIF::GET_WRITE);
//	thisSequence->addSlot(objects::ARDUINO_2,
//			length * 0.4,  DeviceHandlerIF::SEND_READ);
//	thisSequence->addSlot(objects::ARDUINO_2,
//			length * 0.8,  DeviceHandlerIF::GET_READ);

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
#if FSFW_CPP_OSTREAM_ENABLED == 1
	    sif::error << "pst::pollingSequenceInitTest: Sequence invalid!"
	                    << std::endl;
#else
	    sif::printError("pst::pollingSequenceInitTest: Sequence invalid!\n");
#endif
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}
