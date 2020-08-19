/*
 * Service23FileManagement.cpp
 *
 *  Created on: 09.12.2019
 *      Author: Jakob Meier
 */

#include <mission/pus/Service23FileManagement.h>

#include <apid.h>
#include <pusIds.h>

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/tmtcpacket/pus/TmPacketStored.h>
#include <fsfw/action/ActionMessage.h>
#include <fsfw/memory/AcceptsMemoryMessagesIF.h>
#include <fsfw/ipc/QueueFactory.h>

Service23FileManagement::Service23FileManagement(object_id_t objectId_):
		PusServiceBase(objectId_,apid::SOURCE_OBSW,pus::PUS_SERVICE_23),
		packetSubCounter(0), fileSystemHandlerQueueId(0)  {
	commandQueue = QueueFactory::instance()->createMessageQueue(commandQueueDepth);
	IPCStore = objectManager->get<StorageManagerIF>(objects::IPC_STORE);
}

Service23FileManagement::~Service23FileManagement() {
	QueueFactory::instance()->deleteMessageQueue(commandQueue);
}

ReturnValue_t Service23FileManagement::handleRequest(uint8_t subservice) {
	switch(currentPacket.getSubService()){
	case Subservice::CREATE_FILE: {
		//identify queue of file system handler
		AcceptsMemoryMessagesIF* fileSystemHandlerObject =
		        objectManager->get<AcceptsMemoryMessagesIF>(fileSystemHandler);
		if(fileSystemHandlerObject!=NULL){
			fileSystemHandlerQueueId = fileSystemHandlerObject->getCommandQueue();
		}
		else {
			sif::error << "No target queue to file system handler found" << std::endl;
			return HasReturnvaluesIF::RETURN_FAILED;
		}

		store_address_t storeId;
		ReturnValue_t result = IPCStore->addData(&storeId,
		        currentPacket.getApplicationData(), currentPacket.getApplicationDataSize());
		if(result!=HasReturnvaluesIF::RETURN_OK){
			sif::error << "Adding data to IPC store failed" << std::endl;
			return RETURN_FAILED;
		}

		CommandMessage command;
		ActionMessage::setCommand(&command, Subservice::CREATE_FILE, storeId);
		result = commandQueue->sendMessage(fileSystemHandlerQueueId, &command);
		if(result != HasReturnvaluesIF::RETURN_OK){
			sif::error << " MessageQueue of SDCardHandler is full!" << std::endl;
		}
		return HasReturnvaluesIF::RETURN_OK;
	}
	case Subservice::DELETE_FILE: {
		return RETURN_OK;
	}
	default:
		return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
	}
}

ReturnValue_t Service23FileManagement::performService() {
	return HasReturnvaluesIF::RETURN_OK;
}

