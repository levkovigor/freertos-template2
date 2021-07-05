#include "Service23FileManagement.h"

#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/tmtcpacket/pus/tm.h>
#include <fsfw/memory/HasFileSystemIF.h>
#include <fsfw/action/ActionMessage.h>
#include <fsfw/objectmanager/ObjectManager.h>
#include <objects/systemObjectList.h>
#include <mission/memory/FileSystemMessage.h>

Service23FileManagement::Service23FileManagement(object_id_t objectId,
        uint16_t apid, uint8_t serviceId):
        CommandingServiceBase(objectId, apid, serviceId, NUM_PARALLEL_COMMANDS,
                COMMAND_TIMEOUT_SECONDS) {
}


Service23FileManagement::~Service23FileManagement() {
}


ReturnValue_t Service23FileManagement::isValidSubservice(uint8_t subservice) {
    switch(subservice) {
    case Subservice::CMD_CREATE_FILE:
    case Subservice::CMD_DELETE_FILE:
    case Subservice::CMD_LOCK_FILE:
    case Subservice::CMD_UNLOCK_FILE:
    case Subservice::CREATE_DIRECTORY:
    case Subservice::DELETE_DIRECTORY:
    case Subservice::CMD_REPORT_FILE_ATTRIBUTES:
    case Subservice::APPEND_TO_FILE:
    case Subservice::FINISH_APPEND_TO_FILE:
    case Subservice::CMD_READ_FROM_FILE:
    case Subservice::CMD_COPY_FILE: {
        return HasReturnvaluesIF::RETURN_OK;
    }
    default:
        return CommandingServiceBase::INVALID_SUBSERVICE;
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
	        ObjectManager::instance()->get<HasFileSystemIF>(*objectId);
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
    case(Subservice::CMD_CREATE_FILE):
    case(Subservice::CREATE_DIRECTORY):
    case(Subservice::CMD_DELETE_FILE):
    case(Subservice::DELETE_DIRECTORY):
    case(Subservice::APPEND_TO_FILE):
    case(Subservice::FINISH_APPEND_TO_FILE):
    case(Subservice::CMD_REPORT_FILE_ATTRIBUTES):
    case(Subservice::CMD_LOCK_FILE):
    case(Subservice::CMD_UNLOCK_FILE):
    case(Subservice::CMD_READ_FROM_FILE):
    case(Subservice::CMD_COPY_FILE): {
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
	case(Subservice::CMD_CREATE_FILE): {
	    FileSystemMessage::setCreateFileCommand(message, storeId);
		break;
	}
    case(Subservice::CMD_DELETE_FILE): {
        FileSystemMessage::setDeleteFileCommand(message, storeId);
        break;
    }
    case(Subservice::CMD_REPORT_FILE_ATTRIBUTES): {
        FileSystemMessage::setReportFileAttributesCommand(message, storeId);
        break;
    }
    case(Subservice::CMD_LOCK_FILE): {
        FileSystemMessage::setLockFileCommand(message, storeId);
        break;
    }
    case(Subservice::CMD_UNLOCK_FILE): {
        FileSystemMessage::setUnlockFileCommand(message, storeId);
        break;
    }
	case(Subservice::CREATE_DIRECTORY): {
	    FileSystemMessage::setCreateDirectoryCommand(message, storeId);
		break;
	}
	case(Subservice::DELETE_DIRECTORY): {
	    FileSystemMessage::setDeleteDirectoryCommand(message, storeId);
		break;
	}
	case(Subservice::FINISH_APPEND_TO_FILE): {
		FileSystemMessage::setFinishStopWriteCommand(message, storeId);
		break;
	}
    case(Subservice::APPEND_TO_FILE): {
        FileSystemMessage::setWriteCommand(message, storeId);
        break;
    }
	case(Subservice::CMD_READ_FROM_FILE): {
	    FileSystemMessage::setReadCommand(message, storeId);
		break;
	}
	case(Subservice::CMD_COPY_FILE): {
	    FileSystemMessage::setCopyCommand(message, storeId);
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
	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
	switch(replyId)  {
	case FileSystemMessage::COMPLETION_SUCCESS: {
		return CommandingServiceBase::EXECUTION_COMPLETE;
	}
	case FileSystemMessage::COMPLETION_FAILED: {
		failureParameter1 = FileSystemMessage::getFailureReply(reply);
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	case FileSystemMessage::REPLY_READ_FROM_FILE: {
	    bool readFinished = FileSystemMessage::getReadReply(reply, nullptr);
	    if(not readFinished) {
	        /* Another reply will follow. */
	        *isStep = true;
	    }
		return forwardFileSystemReply(reply, objectId,
				Subservice::REPLY_READ_FROM_FILE);
		break;
	}
	case FileSystemMessage::REPLY_READ_FINISHED_STOP: {
        return forwardFileSystemReply(reply, objectId,
                Subservice::REPLY_READ_STOPED_OR_FINISHED);
        break;
	}
	case(FileSystemMessage::REPLY_FINISH_APPEND): {
		return forwardFileSystemReply(reply, objectId,
				Subservice::FINISH_APPEND_REPLY);
		break;
	}
	case(FileSystemMessage::REPLY_REPORT_FILE_ATTRIBUTES): {
        return forwardFileSystemReply(reply, objectId,
                Subservice::REPLY_REPORT_FILE_ATTRIBUTES);
        break;
	}
	default:
		result = INVALID_REPLY;
	}
	return result;
}


void Service23FileManagement::handleUnrequestedReply(CommandMessage *reply) {
    Command_t replyId = reply->getCommand();
    switch(replyId) {
    case FileSystemMessage::REPLY_READ_FINISHED_STOP: {
        /* Should not really happen. */
        forwardFileSystemReply(reply, objects::NO_OBJECT,
                Subservice::REPLY_READ_STOPED_OR_FINISHED);
        break;
    }
    default:
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "Service23FileManagement::handleUnrequestedReply: "
                "Unknown reply with reply ID " << replyId << std::endl;
#else
        sif::printDebug( "Service23FileManagement::handleUnrequestedReply: "
                "Unknown reply with reply ID %hu\n", static_cast<unsigned short>(replyId));
#endif
    }
}


ReturnValue_t Service23FileManagement::addDataToStore(
        store_address_t* storeId, const uint8_t* tcData,
        size_t tcDataLen) {
    /* It is assumed the pointer to the tcData is passed unchanged, so we skip the objectId */
    ReturnValue_t result = IPCStore->addData(storeId,
            tcData + sizeof(object_id_t), tcDataLen - sizeof(object_id_t));
    if (result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "Service23FileManagement::addDataToStore: Failed to add data to "
                "IPC Store" << std::endl;
#else
        sif::printError("Service23FileManagement::addDataToStore: "
                "Failed to add data to IPC Store\n");
#endif
    }
    return result;
}

ReturnValue_t Service23FileManagement::forwardFileSystemReply(
		const CommandMessage* reply, object_id_t objectId,
		Subservice subservice) {
	store_address_t storeId = FileSystemMessage::getStoreId(reply);
	ConstStorageAccessor accessor(storeId);
	ReturnValue_t result = IPCStore->getData(storeId, accessor);
	if(result != RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "Service23FileManagement::forwardFileSystemReply: Could not retrieve data for "
		        "data reply for subservice" << subservice << "!";
#else
        sif::printError("Service23FileManagement::forwardFileSystemReply: Could not retrieve data "
                "for data reply for subservice %d!", static_cast<int>(subservice));
#endif
		return result;
	}
	result = sendTmPacket(subservice, objectId, accessor.data(),
	        accessor.size());
	if(result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "Service23FileManagement::handleReply: Could not send TM packet for "
		        "subservice" << subservice << "!";
#else
		sif::printError("Service23FileManagement::handleReply: Could not send TM packet for "
		        "subservice %d", static_cast<int>(subservice));
#endif
	}
	return HasReturnvaluesIF::RETURN_OK;
}
