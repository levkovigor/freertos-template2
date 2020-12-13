#include "RS485PollingTask.h"

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>




RS485PollingTask::RS485PollingTask(object_id_t objectId): SystemObject(objectId) {

}

ReturnValue_t RS485PollingTask::performOperation(uint8_t opCode) {
	sif::info << "RS485 Polling Task performOperation." << std::endl;
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485PollingTask::initialize() {
	sif::info << "RS485 Polling Task initialized." << std::endl;
    return HasReturnvaluesIF::RETURN_OK;
}
