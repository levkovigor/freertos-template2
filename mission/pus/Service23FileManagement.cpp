#include <mission/pus/Service23FileManagement.h>

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <config/objects/systemObjectList.h>
#include <fsfw/tmtcpacket/pus/TmPacketStored.h>
#include <fsfw/memory/HasFileSystemIF.h>
#include <fsfw/memory/FileSystemMessage.h>
#include <fsfw/action/ActionMessage.h>

Service23FileManagement::Service23FileManagement(object_id_t objectId,
        uint16_t apid, uint8_t serviceId):
        CommandingServiceBase(objectId, apid, serviceId, 4,60) {
}


Service23FileManagement::~Service23FileManagement() {
}


ReturnValue_t Service23FileManagement::isValidSubservice(uint8_t subservice) {
    switch(subservice){
    case Subservice::CREATE_FILE:
    case Subservice::DELETE_FILE:
    case Subservice::CREATE_DIRECTORY:
    case Subservice::DELETE_DIRECTORY:
    case Subservice::WRITE:
    case Subservice::READ:
        sif::info << "Service 23 detected valid subservice" << std::endl;
        return HasReturnvaluesIF::RETURN_OK;
    default:
        return HasReturnvaluesIF::RETURN_FAILED;
    }
}


ReturnValue_t Service23FileManagement::getMessageQueueAndObject(
		uint8_t subservice, const uint8_t *tcData, size_t tcDataLen,
		MessageQueueId_t *id, object_id_t *objectId) {
    if(tcDataLen < sizeof(object_id_t)) {
        return CommandingServiceBase::INVALID_TC;
    }

    SerializeAdapter::deSerialize(objectId, &tcData, &tcDataLen,
            SerializeIF::Endianness::BIG);
	return checkInterfaceAndAcquireMessageQueue(id,objectId);
}


ReturnValue_t Service23FileManagement::checkInterfaceAndAcquireMessageQueue(
        MessageQueueId_t* messageQueueToSet, object_id_t* objectId) {
	// check hasActionIF property of target
	HasFileSystemIF* possibleTarget =
	        objectManager->get<HasFileSystemIF>(*objectId);
	if(possibleTarget == nullptr){
	    return CommandingServiceBase::INVALID_OBJECT;

	}
	*messageQueueToSet = possibleTarget->getCommandQueue();
	return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t Service23FileManagement::prepareCommand(CommandMessage* message,
		uint8_t subservice, const uint8_t *tcData, size_t tcDataLen,
		uint32_t *state, object_id_t objectId) {
	ReturnValue_t result;
	if(subservice == Subservice::CREATE_FILE){

	}
	if(subservice == Subservice::DELETE_FILE){
		result = prepareDeleteFileCommand(message, tcData, tcDataLen);
	}
	else if(subservice == Subservice::CREATE_DIRECTORY){
		result = prepareCreateDirectoryCommand(message, tcData, tcDataLen);
	}
	else if(subservice == Subservice::DELETE_DIRECTORY){
		result = prepareDeleteDirectoryCommand(message, tcData, tcDataLen);
	}
	else if(subservice ==Subservice::WRITE){
		result = prepareWriteCommand(message, tcData, tcDataLen);
	}
	else if(subservice ==Subservice::READ){
		result = prepareReadCommand(message, tcData, tcDataLen);
	}
	else {
		result = RETURN_FAILED;
	}

	return result;

}


ReturnValue_t Service23FileManagement::prepareDeleteFileCommand(
		CommandMessage *message, const uint8_t *tcData, uint32_t tcDataLen) {
	store_address_t parameterAddress;
	/* Add data without the objectId to the IPC store */
	ReturnValue_t result = IPCStore->addData(&parameterAddress, tcData + sizeof(object_id_t), tcDataLen - sizeof(object_id_t));
	if(result == HasReturnvaluesIF::RETURN_OK){
		FileSystemMessage::setDeleteFileCommand(message, parameterAddress);
	}
	else{
		sif::info << "Failed to add data to IPC Store" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t Service23FileManagement::prepareCreateDirectoryCommand(
		CommandMessage *message, const uint8_t *tcData, uint32_t tcDataLen) {
	store_address_t parameterAddress;
	/* Add data without the objectId to the IPC store */
	ReturnValue_t result = IPCStore->addData(&parameterAddress, tcData + sizeof(object_id_t), tcDataLen - sizeof(object_id_t));
	if(result == HasReturnvaluesIF::RETURN_OK){
		FileSystemMessage::setCreateDirectoryCommand(message, parameterAddress);
	}
	else{
		sif::info << "Failed to add data to IPC Store" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t Service23FileManagement::prepareDeleteDirectoryCommand(
		CommandMessage *message, const uint8_t *tcData, uint32_t tcDataLen) {
	store_address_t parameterAddress;
	/* Add data without the objectId to the IPC store */
	ReturnValue_t result = IPCStore->addData(&parameterAddress, tcData + sizeof(object_id_t), tcDataLen - sizeof(object_id_t));
	if(result == HasReturnvaluesIF::RETURN_OK){
		FileSystemMessage::setDeleteDirectoryCommand(message, parameterAddress);
	}
	else{
		sif::info << "Failed to add data to IPC Store" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t Service23FileManagement::prepareWriteCommand(
		CommandMessage *message, const uint8_t *tcData,
		uint32_t tcDataLen) {

	// store additional parameters into the Inter Process Communication Store
	store_address_t parameterAddress;
	ReturnValue_t result = IPCStore->addData(&parameterAddress, tcData + sizeof(object_id_t), tcDataLen - sizeof(object_id_t));
	if(result == HasReturnvaluesIF::RETURN_OK){
		// setWriteCommand expects a Command Message and a store address pointing to the write packet
		FileSystemMessage::setWriteCommand(message, parameterAddress);
	}
	else{
		sif::info << "Failed to add data to IPC Store" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t Service23FileManagement::prepareReadCommand(
		CommandMessage *message, const uint8_t *tcData,
		uint32_t tcDataLen) {

	store_address_t parameterAddress;
	ReturnValue_t result = IPCStore->addData(&parameterAddress, tcData + sizeof(object_id_t),
			tcDataLen - sizeof(object_id_t));
	if (result == HasReturnvaluesIF::RETURN_OK) {
		FileSystemMessage::setReadCommand(message, parameterAddress);
	} else {
		sif::info << "Failed to add data to IPC Store" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t Service23FileManagement::handleReply(const CommandMessage* reply,
		Command_t previousCommand, uint32_t* state,
		CommandMessage* optionalNextCommand, object_id_t objectId,
		bool* isStep) {
	Command_t reply_id = reply->getCommand();
	ReturnValue_t result;
	switch(reply_id)  {
	case FileSystemMessage::COMPLETION_SUCCESS: {
		result = EXECUTION_COMPLETE;
		break;
	}
	case FileSystemMessage::READ_REPLY: {
		store_address_t storeId = FileSystemMessage::getStoreId(reply);
		size_t size = 0;
		const uint8_t * buffer = NULL;
		result = IPCStore->getData(storeId, &buffer, &size);
		if(result != RETURN_OK) {
			sif::error << "Service 23: Could not retrieve data for data reply";
			return result;
		}
		sendTmPacket(Subservice::READ_REPLY, objectId, buffer, size);
		result = IPCStore ->deleteData(storeId);
		break;
	}
	case FileSystemMessage::COMPLETION_FAILED: {
		result = HasReturnvaluesIF::RETURN_FAILED;
		break;
	}
	default:
		result = INVALID_REPLY;
	}
	return result;
}

