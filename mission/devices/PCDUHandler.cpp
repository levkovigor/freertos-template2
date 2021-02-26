#include <mission/devices/PCDUHandler.h>

#include <fsfw/devicehandlers/AcceptsDeviceResponsesIF.h>
#include <fsfw/thermal/ThermalComponentIF.h>
#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/events/EventManager.h>

PCDUHandler::PCDUHandler(object_id_t objectId, object_id_t comIF,
		CookieIF * cookie):
		DeviceHandlerBase(objectId, comIF, cookie, &fdir),
		fdir(objectId, objectId) {
	eventQueue = QueueFactory::instance()->createMessageQueue();
}


PCDUHandler::~PCDUHandler() {
}


void PCDUHandler::doStartUp() {
	return;
}


void PCDUHandler::doShutDown() {
	return;
}


ReturnValue_t PCDUHandler::buildNormalDeviceCommand(DeviceCommandId_t* id) {
	return RETURN_OK;
}

ReturnValue_t PCDUHandler::buildTransitionDeviceCommand(DeviceCommandId_t* id) {
	return RETURN_OK;
}

ReturnValue_t PCDUHandler::buildCommandFromCommand(
		DeviceCommandId_t deviceCommand, const uint8_t* commandData,
		size_t commandDataLen) {
	switch(deviceCommand) {
	default:
		return RETURN_FAILED;
	}
}

void PCDUHandler::fillCommandAndReplyMap() {

}

ReturnValue_t PCDUHandler::scanForReply(const uint8_t *start, size_t len,
		DeviceCommandId_t *foundId, size_t *foundLen) {
	return RETURN_OK;
}

ReturnValue_t PCDUHandler::interpretDeviceReply(DeviceCommandId_t id,
		const uint8_t* packet) {
	switch(id){
	default:
		return DeviceHandlerIF::DEVICE_REPLY_INVALID;
	}
}



void PCDUHandler::setNormalDatapoolEntriesInvalid() {
}


void PCDUHandler::sendSwitchCommand(uint8_t switchNr, ReturnValue_t onOff) const {
	if(switchNr < sizeof(switchList)){
		//switchList[switchNr] = onOff;
	}
}

void PCDUHandler::sendFuseOnCommand(uint8_t fuseNr) const {
}

ReturnValue_t PCDUHandler::getSwitchState(uint8_t switchNr) const {
	if(switchNr < sizeof(switchList)){
		return switchList[switchNr];
	} else {
		return NO_SWITCH;
	}
}

uint32_t PCDUHandler::getTransitionDelayMs(Mode_t modeFrom, Mode_t modeTo) {
	return 2000;
}

ReturnValue_t PCDUHandler::getFuseState(uint8_t fuseNr) const {
	return RETURN_OK;
}

uint32_t PCDUHandler::getSwitchDelayMs(void) const {
	uint32_t switchDelayMs = 8000;
	return switchDelayMs;
}

MessageQueueId_t PCDUHandler::getEventReceptionQueue() {
	return fdir.getEventReceptionQueue();
}
