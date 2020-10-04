#include <mission/pus/Service23FileManagement.h>

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <config/objects/systemObjectList.h>
#include <fsfw/tmtcpacket/pus/TmPacketStored.h>
#include <fsfw/memory/HasFileSystemIF.h>
#include <fsfw/memory/FileSystemMessage.h>
#include <fsfw/action/ActionMessage.h>

Service23FileManagement::Service23FileManagement(object_id_t objectId,
        uint16_t apid, uint8_t serviceId):
        CommandingServiceBase(objectId, apid, serviceId,
        NUM_PARALLEL_COMMANDS, COMMAND_TIMEOUT_SECONDS) {
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
	store_address_t storeId;
    switch(subservice) {
    case(Subservice::CREATE_FILE):
    case(Subservice::CREATE_DIRECTORY):
    case(Subservice::DELETE_FILE):
    case(Subservice::DELETE_DIRECTORY):
    case(Subservice::WRITE):
    case(Subservice::READ): {
        result = addDataToStore(&storeId, tcData, tcDataLen);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }
        break;
    }

    default: {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    }

	switch(subservice) {
	case(Subservice::CREATE_FILE): {
	    FileSystemMessage::setCreateFileCommand(message, storeId);
		break;
	}
	case(Subservice::CREATE_DIRECTORY): {
	    FileSystemMessage::setCreateDirectoryCommand(message, storeId);
		break;
	}
	case(Subservice::DELETE_FILE): {
	    FileSystemMessage::setDeleteFileCommand(message, storeId);
		break;
	}
	case(Subservice::DELETE_DIRECTORY): {
	    FileSystemMessage::setDeleteDirectoryCommand(message, storeId);
		break;
	}
	case(Subservice::WRITE): {
	    FileSystemMessage::setWriteCommand(message, storeId);
		break;
	}
	case(Subservice::READ): {
	    FileSystemMessage::setReadCommand(message, storeId);
		break;
	}
	}

	return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t Service23FileManagement::handleReply(const CommandMessage* reply,
		Command_t previousCommand, uint32_t* state,
		CommandMessage* optionalNextCommand, object_id_t objectId,
		bool* isStep) {
	Command_t replyId = reply->getCommand();
	ReturnValue_t result;
	switch(replyId)  {
	case FileSystemMessage::COMPLETION_SUCCESS: {
		return CommandingServiceBase::EXECUTION_COMPLETE;
	}
	case FileSystemMessage::COMPLETION_FAILED: {
		failureParameter1 = FileSystemMessage::getFailureReply(reply);
		return HasReturnvaluesIF::RETURN_FAILED;
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

	default:
		result = INVALID_REPLY;
	}
	return result;
}

ReturnValue_t Service23FileManagement::addDataToStore(
        store_address_t* storeId, const uint8_t* tcData,
        size_t tcDataLen) {
    // It is assumed the pointer to the tcData is passed unchanged,
    // so we skip the objectId
    store_address_t parameterAddress;
    ReturnValue_t result = IPCStore->addData(&parameterAddress,
            tcData + sizeof(object_id_t), tcDataLen - sizeof(object_id_t));
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Service23FileManagement::addDataToStore: Failed to add "
                << "data to IPC Store" << std::endl;
    }
    return result;
}
