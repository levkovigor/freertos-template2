#include "PVCHHandler.h"
#include "objects/systemObjectList.h"

#include "fsfw/devicehandlers/DeviceCommunicationIF.h"

PVCHHandler::PVCHHandler(object_id_t objectId, object_id_t pvchWorkerId, uint8_t switchId):
        /* DeviceHandlerBase(objectId, objects::SPI_DEVICE_COM_IF, comCookie), */
        ExtendedControllerBase(objectId, objects::NO_OBJECT), pvchWorker(pvchWorkerId),
        switchId(switchId) {
}

PVCHHandler::~PVCHHandler() {
}

//void PVCHHandler::doStartUp() {
//}
//
//void PVCHHandler::doShutDown() {
//}
//
//ReturnValue_t PVCHHandler::buildNormalDeviceCommand(
//		DeviceCommandId_t *id) {
//	return RETURN_OK;
//}
//
//ReturnValue_t PVCHHandler::buildTransitionDeviceCommand(
//		DeviceCommandId_t *id) {
//	return RETURN_OK;
//}
//
//ReturnValue_t PVCHHandler::buildCommandFromCommand(
//		DeviceCommandId_t deviceCommand, const uint8_t *commandData,
//		size_t commandDataLen) {
//	return RETURN_OK;
//}
//
//void PVCHHandler::fillCommandAndReplyMap() {
//}
//
//ReturnValue_t PVCHHandler::scanForReply(const uint8_t *start,
//		size_t remainingSize, DeviceCommandId_t *foundId, size_t *foundLen) {
//	return RETURN_OK;
//}
//
//ReturnValue_t PVCHHandler::interpretDeviceReply(
//		DeviceCommandId_t id, const uint8_t *packet) {
//	return RETURN_OK;
//}
//
//void PVCHHandler::setNormalDatapoolEntriesInvalid() {
//}
//
//void PVCHHandler::debugInterface(uint8_t positionTracker, object_id_t objectId,
//		uint32_t parameter) {
//}
//
//uint32_t PVCHHandler::getTransitionDelayMs(Mode_t modeFrom,
//		Mode_t modeTo) {
//	return 5000;
//}
//
//ReturnValue_t PVCHHandler::getSwitches(const uint8_t **switches,
//		uint8_t *numberOfSwitches) {
//	return DeviceHandlerBase::NO_SWITCH;
//}
