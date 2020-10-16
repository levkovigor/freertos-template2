#include "SDCardHandler.h"
#include "SDCardAccess.h"
#include "FileSystemMessage.h"
#include "SDCardHandlerPackets.h"

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/ipc/CommandMessage.h>
#include <fsfw/timemanager/Countdown.h>
#include <fsfw/tasks/PeriodicTaskIF.h>
#include <fsfw/timemanager/Stopwatch.h>

SDCardHandler::SDCardHandler(object_id_t objectId): SystemObject(objectId),
    commandQueue(QueueFactory::instance()->
    createMessageQueue(MAX_MESSAGE_QUEUE_DEPTH)),
    actionHelper(this, commandQueue) {
    IPCStore = objectManager->get<StorageManagerIF>(objects::IPC_STORE);
    countdown = new Countdown(0);
}


SDCardHandler::~SDCardHandler(){
    QueueFactory::instance()->deleteMessageQueue(commandQueue);
}


ReturnValue_t SDCardHandler::initialize() {
    ReturnValue_t result = actionHelper.initialize(commandQueue);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    return SystemObject::initialize();
}


ReturnValue_t SDCardHandler::initializeAfterTaskCreation() {
    periodMs = executingTask->getPeriodMs();
    countdown->setTimeout(0.85 * periodMs);
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::performOperation(uint8_t operationCode){
	CommandMessage message;
	countdown->resetTimer();
	// Check for first message
	ReturnValue_t result = commandQueue->receiveMessage(&message);
	if(result == MessageQueueIF::EMPTY) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	else if(result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	//Stopwatch stopwatch;
    VolumeId volumeToOpen = determineVolumeToOpen();
    // File system message received, open access to SD Card which will
    // be closed automatically on function exit.
    SDCardAccess sdCardAccess = SDCardAccess(volumeToOpen);
    result = handleAccessResult(sdCardAccess.accessResult);
    if(result == HasReturnvaluesIF::RETURN_FAILED) {
    	// No SD card could be opened.
    	return result;
    }

    // handle first message. Returnvalue ignored for now.
	result = handleMessage(&message);

	// Returnvalue ignored for now.
	return handleMultipleMessages(&message);
}

VolumeId SDCardHandler::determineVolumeToOpen() {
    if(not fileSystemWasUsedOnce) {
    	return preferedVolume;
    }
    else {
    	return activeVolume;
    }
}

ReturnValue_t SDCardHandler::handleAccessResult(ReturnValue_t accessResult) {
    if(accessResult == HasReturnvaluesIF::RETURN_OK){
    	fileSystemWasUsedOnce = true;
    }
    else if(accessResult == SDCardAccess::OTHER_VOLUME_ACTIVE) {
    	if(preferedVolume == SD_CARD_0) {
    		activeVolume = SD_CARD_1;
    	}
    	else {
    		activeVolume = SD_CARD_0;
    	}
    	fileSystemWasUsedOnce = true;
    	// what to do now? we lose old files? maybe a reboot would help..
    	triggerEvent(SD_CARD_SWITCHED, activeVolume, 0);
    }
    else {
    	// not good, reboot?
    	triggerEvent(SD_CARD_ACCESS_FAILED, 0, 0);
    	return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::handleMultipleMessages(CommandMessage *message) {
	ReturnValue_t status = HasReturnvaluesIF::RETURN_OK;
	for(uint8_t counter = 1;
			counter < MAX_FILE_MESSAGES_HANDLED_PER_CYCLE;
			counter++)
	{
 		ReturnValue_t result = commandQueue->receiveMessage(message);
		if(result == MessageQueueIF::EMPTY) {
			return HasReturnvaluesIF::RETURN_OK;
		}
		else if(result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}

		result = handleMessage(message);
		if(result != HasReturnvaluesIF::RETURN_OK) {
			status = result;
		}

	    if(countdown->hasTimedOut()) {
	    	return status;
	    }
	}
	return status;
}


ReturnValue_t SDCardHandler::handleMessage(CommandMessage* message) {
	ReturnValue_t result = actionHelper.handleActionMessage(message);
	if(result == HasReturnvaluesIF::RETURN_OK) {
	    return result;
	}

	return handleFileMessage(message);
}

ReturnValue_t SDCardHandler::executeAction(ActionId_t actionId,
        MessageQueueId_t commandedBy, const uint8_t *data, size_t size) {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    switch(actionId) {
    case(SELECT_ACTIVE_SD_CARD): {
        uint8_t volumeId;
        result = SerializeAdapter::deSerialize(&volumeId, &data, &size,
                SerializeIF::Endianness::BIG);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            actionHelper.finish(commandedBy, actionId, result);
            return HasReturnvaluesIF::RETURN_OK;
        }

        VolumeId enumeratedId;
        if(volumeId == 0) {
            enumeratedId = SD_CARD_0;
        }
        else {
            enumeratedId = SD_CARD_1;
        }
        if((activeVolume == SD_CARD_0 and enumeratedId == SD_CARD_1)
                or (activeVolume == SD_CARD_1 and enumeratedId == SD_CARD_0)) {
            int retval = switch_sd_card(enumeratedId);
            if(retval == F_NO_ERROR) {
                activeVolume = enumeratedId;
            }
            else {
                result = retval;
            }
        }

        actionHelper.finish(commandedBy, actionId, result);
        break;
    }
    case(PRINT_SD_CARD): {
        this->printSdCard();
        actionHelper.finish(commandedBy, actionId);
        break;
    }
    case(CLEAR_SD_CARD): {
        int retval = clear_sd_card();
        if(retval != F_NO_ERROR) {
            result = retval;
        }
        actionHelper.finish(commandedBy, actionId, result);
        break;
    }
    case(FORMAT_SD_CARD): {
        // formats the currently active filesystem!
        sif::info << "SDCardHandler::handleMessage: Formatting SD-Card "
                << activeVolume << "!" << std::endl;
        int retval = f_format(0, F_FAT32_MEDIA);
        if(retval != F_NO_ERROR) {
            sif::info << "SD-Card was formatted successfully!" << std::endl;
            result = retval;
        }
        actionHelper.finish(commandedBy, actionId, result);
        break;
    }
    case(REPORT_ACTIVE_SD_CARD): {
        ActivePreferedVolumeReport reply(activeVolume);
        result = actionHelper.reportData(commandedBy, actionId, &reply);
        actionHelper.finish(commandedBy, actionId, result);
        break;
    }
    case(REPORT_PREFERED_SD_CARD): {
        ActivePreferedVolumeReport reply(preferedVolume);
        result = actionHelper.reportData(commandedBy, actionId, &reply);
        actionHelper.finish(commandedBy, actionId, result);
        break;
    }
    default: {
        return CommandMessage::UNKNOWN_COMMAND;
    }
    }

    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t SDCardHandler::handleFileMessage(CommandMessage* message) {
    ReturnValue_t  result = HasReturnvaluesIF::RETURN_OK;
    switch(message->getCommand()) {
    case FileSystemMessage::CMD_CREATE_FILE: {
        result = handleCreateFileCommand(message);
        break;
    }
    case FileSystemMessage::CMD_DELETE_FILE: {
        result = handleDeleteFileCommand(message);
        break;
    }
    case FileSystemMessage::CMD_REPORT_FILE_ATTRIBUTES: {
        result = handleReportAttributesCommand(message);
        break;
    }
    case FileSystemMessage::CMD_CREATE_DIRECTORY: {
        result = handleCreateDirectoryCommand(message);
        break;
    }
    case FileSystemMessage::CMD_DELETE_DIRECTORY: {
        result = handleDeleteDirectoryCommand(message);
        break;
    }
    case FileSystemMessage::CMD_LOCK_FILE: {
        result = handleLockFileCommand(message, true);
        break;
    }
    case FileSystemMessage::CMD_UNLOCK_FILE: {
        result = handleLockFileCommand(message, false);
        break;
    }
    case FileSystemMessage::CMD_APPEND_TO_FILE: {
        result = handleAppendCommand(message);
        break;
    }
    case FileSystemMessage::CMD_FINISH_APPEND_TO_FILE: {
    	result = handleFinishAppendCommand(message);
    	break;
    }
    case FileSystemMessage::CMD_READ_FROM_FILE: {
        result = handleReadCommand(message);
        break;
    }
    default: {
        sif::debug << "SDCardHandler::handleFileMessage: "
                << "Invalid filesystem command!" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    }
    return result;
}


void SDCardHandler::subscribeForSdCardNotifications(MessageQueueId_t queueId) {
    sdCardNotificationRecipients.push_back(queueId);
}

MessageQueueId_t SDCardHandler::getCommandQueue() const{
    return commandQueue->getId();
}

void SDCardHandler::setTaskIF(PeriodicTaskIF *executingTask) {
	this->executingTask = executingTask;
}

ReturnValue_t SDCardHandler::getStoreData(store_address_t& storeId,
        ConstStorageAccessor& accessor,
        const uint8_t** ptr, size_t* size) {
    ReturnValue_t result = IPCStore->getData(storeId, accessor);
    if(result != HasReturnvaluesIF::RETURN_OK){
        // Should not happen!
        sif::error << "SDCardHandler::getStoreData: "
               <<  "Getting data failed!" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    *size = accessor.size();
    *ptr = accessor.data();
    return HasReturnvaluesIF::RETURN_OK;
}

void SDCardHandler::sendCompletionReply(bool success, ReturnValue_t errorCode,
        uint32_t errorParam) {
    CommandMessage reply;
    if(success) {
        FileSystemMessage::setSuccessReply(&reply);
    }
    else {
        FileSystemMessage::setFailureReply(&reply, errorCode, errorParam);
    }

    ReturnValue_t result = commandQueue->reply(&reply);
    if(result != HasReturnvaluesIF::RETURN_OK){
        if(result == MessageQueueIF::FULL) {
            // Configuration error.
            sif::error << "SDCardHandler::sendCompletionReply: "
                    <<" Queue of receiver is full!" << std::endl;
        }
    }
}




