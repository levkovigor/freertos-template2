#include "SDCardHandler.h"
#include "SDCardAccess.h"
#include "SDCAccessManager.h"
#include "SDCardHandlerPackets.h"

#include <mission/memory/FileSystemMessage.h>

#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/ipc/CommandMessage.h>
#include <fsfw/timemanager/Countdown.h>
#include <fsfw/tasks/PeriodicTaskIF.h>
#include <fsfw/timemanager/Stopwatch.h>


#ifdef ISIS_OBC_G20
#include <sam9g20/common/FRAMApi.h>
#else
#include <sam9g20/common/VirtualFRAMApi.h>
#endif


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
    /* This sets up the access manager. On the iOBC this will also cache
    the currently prefered SD card from the FRAM. */
    SDCardAccessManager::create();
    periodMs = executingTask->getPeriodMs();
    /* This prevents the task from blocking other low priority tasks (self-suspension) */
    countdown->setTimeout(0.75 * periodMs);
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::performOperation(uint8_t operationCode) {
    /* Can be used to measure the time this function takes */
    // Stopwatch stopwatch;
	CommandMessage message;
	countdown->resetTimer();
	/* Check for first message */
	ReturnValue_t result = commandQueue->receiveMessage(&message);
	if(result == MessageQueueIF::EMPTY) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	else if(result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

    /* File system message received, open access to SD Card which will be closed automatically
    on function exit. */
    SDCardAccess sdCardAccess;
    result = handleAccessResult(sdCardAccess.getAccessResult());
    if(result != HasReturnvaluesIF::RETURN_OK) {
    	/* No SD card could be opened. */
    	return result;
    }

    /* Handle first message. Returnvalue ignored for now. */
	result = handleMessage(&message);
    if(countdown->hasTimedOut()) {
        return result;
    }

    bool allMessagesDone = false;
    while(countdown->isBusy()) {
        /* Handle messages and internal state machine in an alternating way */
        if(not allMessagesDone) {
            result = handleNextMessage(&message);
        }

        if(result == MessageQueueIF::EMPTY) {
            allMessagesDone = true;
        }

        /* If there is something to do, perform one step */
        if(stateMachine.getInternalState() != SDCHStateMachine::States::IDLE) {
            performStateMachineStep();
        }

        if(allMessagesDone and stateMachine.getInternalState() == SDCHStateMachine::States::IDLE) {
            /* Nothing to do, no reason to block the CPU anymore */
            return HasReturnvaluesIF::RETURN_OK;
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::handleAccessResult(ReturnValue_t accessResult) {
    if(accessResult == HasReturnvaluesIF::RETURN_OK) {
        return accessResult;
    }
    if(accessResult == SDCardAccess::SD_CARD_CHANGE_ONGOING) {
        return SDCardAccess::SD_CARD_CHANGE_ONGOING;
    }
    else {
    	/* Not good. */
#if OBSW_VERBOSE_LEVEL >= 1
        sif::printWarning("SDCardHandler::handleAccessResult: SD-Card access error!\n");
#endif
    	triggerEvent(SD_CARD_ACCESS_FAILED, 0, 0);
    	return HasReturnvaluesIF::RETURN_FAILED;
    }
}

ReturnValue_t SDCardHandler::handleNextMessage(CommandMessage *message) {
    ReturnValue_t result = commandQueue->receiveMessage(message);
    if(result == MessageQueueIF::EMPTY) {
        return result;
    }
    else if(result != HasReturnvaluesIF::RETURN_OK) {
#if OBSW_VERBOSE_LEVEL >= 1
        sif::printWarning("SDCardHandler::handleNextMessage: Error receiving message!\n");
#endif
        return result;
    }

    return handleMessage(message);
}


ReturnValue_t SDCardHandler::handleMessage(CommandMessage* message) {
	ReturnValue_t result = actionHelper.handleActionMessage(message);
	if(result == HasReturnvaluesIF::RETURN_OK) {
	    return result;
	}

	result = handleFileMessage(message);
	if(result != HasReturnvaluesIF::RETURN_OK) {
#if OBSW_VERBOSE_LEVEL >= 1
	    sif::printWarning("SDCardHandler::handleMessage: Invalid message type!\n");
#endif
	}
    return result;
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
        /* Formats the currently active filesystem! */
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "SDCardHandler::handleMessage: Formatting SD-Card " << activeVolume <<
                "!" << std::endl;
#else
        sif::printWarning("SDCardHandler::handleMessage: Formatting SD-Card %d!\n", activeVolume);
#endif
        int retval = f_format(0, F_FAT32_MEDIA);
        if(retval != F_NO_ERROR) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "SD-Card was formatted successfully!" << std::endl;
#else
            sif::printInfo("SD-Card was formatted successfully!\n");
#endif
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
        VolumeId currentActiveVoume = SDCardAccessManager::instance()->getActiveSdCard();
        ActivePreferedVolumeReport reply(currentActiveVoume);
        result = actionHelper.reportData(commandedBy, actionId, &reply);
        actionHelper.finish(commandedBy, actionId, result);
        break;
    }
    case(SET_LOAD_OBSW_UPDATE): {
        if (size < 1) {
            return HasActionsIF::INVALID_PARAMETERS;
        }
        if (data[0] != 0 or data[0] != 1) {
            return HasActionsIF::INVALID_PARAMETERS;
        }
        bool enable = data[0];
        VolumeId volume = SD_CARD_0;
        if (enable ) {
            if (size < 2) {
                return HasActionsIF::INVALID_PARAMETERS;
            }
            if ((data[1] != SD_CARD_0) or (data[1] != SD_CARD_1)) {
                return HasActionsIF::INVALID_PARAMETERS;
            }
            volume = static_cast<VolumeId>(data[1]);
        }
        int retval = set_to_load_softwareupdate(enable, volume);
        if (retval != 0) {
            return HasReturnvaluesIF::RETURN_FAILED;
        }

        break;
    }
    case(GET_LOAD_OBSW_UPDATE): {

        break;
    }
    default: {
        return CommandMessage::UNKNOWN_COMMAND;
    }
    }

    return HasReturnvaluesIF::RETURN_OK;
}

void SDCardHandler::performStateMachineStep() {
    return;
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
    case FileSystemMessage::CMD_COPY_FILE: {
        result = handleCopyCommand(message);
        break;
    }
    case FileSystemMessage::CMD_READ_FROM_FILE: {
        result = handleReadCommand(message);
        break;
    }
    default: {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "SDCardHandler::handleFileMessage: Invalid filesystem command!" << std::endl;
#else
        sif::printDebug("SDCardHandler::handleFileMessage: Invalid filesystem command!\n");
#endif
        return CommandMessageIF::UNKNOWN_COMMAND;
    }
    }
    return result;
}

#ifdef ISIS_OBC_G20
void SDCardHandler::subscribeForSdCardNotifications(MessageQueueId_t queueId) {
    sdCardNotificationRecipients.push_back(queueId);
}
#endif

MessageQueueId_t SDCardHandler::getCommandQueue() const{
    return commandQueue->getId();
}

void SDCardHandler::setTaskIF(PeriodicTaskIF *executingTask) {
	this->executingTask = executingTask;
}



