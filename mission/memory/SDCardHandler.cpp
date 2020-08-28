#include <mission/memory/SDCardHandler.h>
#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/ipc/CommandMessage.h>
#include <fsfw/action/ActionMessage.h>


SDCardHandler::SDCardHandler(object_id_t objectId_):SystemObject(objectId_){
	commandQueue = QueueFactory::instance()->createMessageQueue(queueDepth);
	IPCStore = objectManager->get<StorageManagerIF>(objects::IPC_STORE);
}

SDCardHandler::~SDCardHandler(){
	QueueFactory::instance()->deleteMessageQueue(commandQueue);
}

ReturnValue_t SDCardHandler::performOperation(uint8_t operationCode) {
	CommandMessage command;
	MessageQueueId_t receivedFrom = 0;
	ReturnValue_t result = commandQueue->receiveMessage(&command,&receivedFrom);
	if(result == HasReturnvaluesIF::RETURN_OK){
		store_address_t storeId = ActionMessage::getStoreId(&command);
		size_t size = 0;
		const uint8_t* buffer = NULL;
		result = IPCStore->getData(storeId, &buffer, &size);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			sif::error << "SDCardHandler failed to read data from IPC store";
			return result;
		}
		IPCStore->deleteData(storeId);
	}
	return HasReturnvaluesIF::RETURN_OK;
}

MessageQueueId_t SDCardHandler::getCommandQueue() const{
	return commandQueue->getId();
}

ReturnValue_t SDCardHandler::handleMemoryLoad(uint32_t address, const uint8_t* data, uint32_t size, uint8_t** dataPointer){
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::handleMemoryDump(uint32_t address, uint32_t size, uint8_t** dataPointer, uint8_t* dumpTarget){
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::setAddress(uint32_t* startAddress){
	return HasReturnvaluesIF::RETURN_FAILED;
}
