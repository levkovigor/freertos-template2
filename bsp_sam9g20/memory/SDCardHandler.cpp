#include "SDCardHandler.h"
#include "SDCardHandlerPackets.h"
#include "SDCardAccess.h"
#include "SDCAccessManager.h"
#include "sdcardHandlerDefinitions.h"

#include "bsp_sam9g20/common/fram/FRAMApi.h"

#include "mission/memory/FileSystemMessage.h"

#include "fsfw/tasks/PeriodicTaskIF.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/storagemanager/StorageManagerIF.h"
#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/serviceinterface/ServiceInterface.h"


SDCardHandler::SDCardHandler(object_id_t objectId): SystemObject(objectId),
        commandQueue(QueueFactory::instance()->createMessageQueue(MAX_MESSAGE_QUEUE_DEPTH)),
        actionHelper(this, commandQueue), countdown(0), stateMachine(this, &countdown) {
    ipcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
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
    countdown.setTimeout(0.75 * periodMs);
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::performOperation(uint8_t operationCode) {
    /* Can be used to measure the time this function takes */
    // Stopwatch stopwatch;
    CommandMessage message;
    countdown.resetTimer();
    /* Check for first message */
    ReturnValue_t result = commandQueue->receiveMessage(&message);
    if(result == MessageQueueIF::EMPTY) {
        return HasReturnvaluesIF::RETURN_OK;
    }
    else if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    if(sdCardChangeOngoing) {
        bool changeSuccess = SDCardAccessManager::instance()->tryActiveSdCardChange();
        if(not changeSuccess) {
            /* TODO: Counter, generate event if it never works */
            return HasReturnvaluesIF::RETURN_OK;
        }
        sdCardChangeOngoing = false;
        actionHelper.finish(true, actionSender, currentAction, HasReturnvaluesIF::RETURN_OK);
        currentAction = -1;
        actionSender = MessageQueueIF::NO_QUEUE;
    }

    /* File system message received, open access to SD Card which will be closed automatically
    on function exit. */
    SDCardAccess sdCardAccess;
    result = handleSdCardAccessResult(sdCardAccess);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    /* Handle first message. Returnvalue ignored for now. */
    result = handleMessage(&message);
    if(countdown.hasTimedOut()) {
        return result;
    }

    /* Now we check if the state machine is busy. If it is, we drive it as long as possible
    for the rest of the task cycle. */
    while(countdown.isBusy()) {
        if(stateMachine.getInternalState() != SDCHStateMachine::States::IDLE) {
            driveStateMachine();
        }
        /* The state machine is IDLE so we can try to read more messages */
        else {
            /* This might also set the state machine to a non-idle state */
            result = handleNextMessage(&message);
            if(result == MessageQueueIF::EMPTY) {
                return HasReturnvaluesIF::RETURN_OK;
            }
        }

        if(sdCardChangeOngoing) {
            /* SD card change might have been requested in a message. Return to tear down
            SD card access */
            return HasReturnvaluesIF::RETURN_OK;
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
}

void SDCardHandler::driveStateMachine() {
    ReturnValue_t result = stateMachine.continueCurrentOperation();
    if(result == sdchandler::OPERATION_FINISHED) {
        stateMachine.resetAndSetToIdle();
    }
    else if(result != HasReturnvaluesIF::RETURN_OK) {
        /* If the state machine did not fail because of a pending SD card change operation
        we reset it */
        stateMachine.resetAndSetToIdle();
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
        if(stateMachine.getInternalState() != SDCHStateMachine::States::IDLE) {
            /* Some operation might be going on (file operation or SD card switching
            already happening), so we reject this command */
            actionHelper.finish(false, commandedBy, actionId, HasActionsIF::IS_BUSY);
            return HasReturnvaluesIF::RETURN_OK;
        }
        sdCardChangeOngoing = true;
        actionHelper.step(1, commandedBy, actionId, HasReturnvaluesIF::RETURN_OK);

        /* Cache this to generate finish reply later */
        actionSender = commandedBy;
        currentAction = actionId;
        break;
    }
    case(REPORT_ACTIVE_SD_CARD): {
        ActivePreferedVolumeReport reply(SDCardAccessManager::instance()->getActiveSdCard());
        result = actionHelper.reportData(commandedBy, actionId, &reply);
        if(result == HasReturnvaluesIF::RETURN_OK) {
            actionHelper.finish(true, commandedBy, actionId, result);
        }
        else {
            actionHelper.finish(false, commandedBy, actionId, result);
        }
        break;
    }
    case(SELECT_PREFERRED_SD_CARD): {
        /* TODO: Here, we should set the respective FRAM variable, which will be used by the
        SD card access manager on startup to determine which SD card to use */
        break;
    }
    case(REPORT_PREFERRED_SD_CARD): {
        /* TODO: We need to use the FRAM variable here instead */
        break;
    }
    case(PRINT_SD_CARD): {
        this->printSdCard();
        actionHelper.finish(true, commandedBy, actionId);
        break;
    }
    case(CLEAR_SD_CARD): {
        int retval = clear_sd_card();
        if(retval != F_NO_ERROR) {
            result = retval;
            actionHelper.finish(false, commandedBy, actionId, result);
            return HasReturnvaluesIF::RETURN_OK;
        }
        actionHelper.finish(true, commandedBy, actionId, result);
        break;
    }
    case(FORMAT_SD_CARD): {
        VolumeId currentVolumeId = SDCardAccessManager::instance()->getActiveSdCard();
        /* Formats the currently active filesystem! */
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "SDCardHandler::handleMessage: Formatting SD-Card " << currentVolumeId <<
                "!" << std::endl;
#else
        sif::printWarning("SDCardHandler::handleMessage: Formatting SD-Card %d!\n",
                currentVolumeId);
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
        else {
            actionHelper.finish(false, commandedBy, actionId, retval);
        }
        actionHelper.finish(true, commandedBy, actionId, result);
        break;
    }
    case(SET_LOAD_OBSW_UPDATE): {
        if (size < 1) {
            return HasActionsIF::INVALID_PARAMETERS;
        }
        if (data[0] > 1) {
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
        int retval = fram_set_to_load_softwareupdate(enable, volume);
        if (retval != 0) {
            return HasReturnvaluesIF::RETURN_FAILED;
        }

        break;
    }
    case(GET_LOAD_OBSW_UPDATE): {

        break;
    }
    case(CANCEL_SDCH_OPERATIONS): {
        stateMachine.resetAndSetToIdle();
        actionHelper.finish(true, commandedBy, actionId, HasReturnvaluesIF::RETURN_OK);
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
    case FileSystemMessage::CMD_COPY_FILE: {
        /* Might take multiple cycles, we store the sender */
        fileSystemSender = message->getSender();
        result = handleCopyCommand(message);
        break;
    }
    case FileSystemMessage::CMD_MOVE_FILE: {
        /* Not implemented yet */
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

ReturnValue_t SDCardHandler::handleSdCardAccessResult(SDCardAccess &sdCardAccess) {
    ReturnValue_t result = sdCardAccess.getAccessResult();
    if(result == SDCardAccess::SD_CARD_CHANGE_ONGOING) {
        /* Really should not happen.. */
        sif::printWarning("SDCardHandler::handleSdCardAccessResult: SD card change ongoing at"
                "wrong location!\n");
        sdCardChangeOngoing = true;
        bool changeSuccess = SDCardAccessManager::instance()->tryActiveSdCardChange();
        if(changeSuccess) {
            sdCardChangeOngoing = false;
            return HasReturnvaluesIF::RETURN_OK;
        }
        else {
            return HasReturnvaluesIF::RETURN_FAILED;
        }
    }
    else if(result != HasReturnvaluesIF::RETURN_OK) {
        /* Not good. */
#if OBSW_VERBOSE_LEVEL >= 1
        sif::printWarning("SDCardHandler::handleAccessResult: SD-Card access error!\n");
#endif
        triggerEvent(sdchandler::SD_CARD_ACCESS_FAILED, 0, 0);
        return result;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::handleDeleteFileCommand(CommandMessage* message){
    store_address_t storeId = FileSystemMessage::getStoreId(message);
    ConstStorageAccessor accessor(storeId);
    const uint8_t* ipcStoreBuffer = nullptr;
    size_t remainingSize = 0;
    ReturnValue_t result = getStoreData(storeId, accessor, &ipcStoreBuffer,
            &remainingSize);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    DeleteFileCommand command;
    /* Extract the repository path and the filename from the application data field */
    result = command.deSerialize(&ipcStoreBuffer,
            &remainingSize, SerializeIF::Endianness::BIG);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sendCompletionReply(false, result);
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    result = removeFile(command.getRepositoryPathRaw(),
            command.getFilenameRaw());
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sendCompletionReply(false, result);
    }
    else {
        sendCompletionReply();
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::handleReportAttributesCommand(
        CommandMessage* message) {
    store_address_t storeId = FileSystemMessage::getStoreId(message);
    ConstStorageAccessor accessor(storeId);
    const uint8_t* ipcStoreBuffer = nullptr;
    size_t remainingSize = 0;
    ReturnValue_t result = getStoreData(storeId, accessor, &ipcStoreBuffer,
            &remainingSize);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    FileAttributesCommand command;
    /* Extract the repository path and the filename from the application data field */
    result = command.deSerialize(&ipcStoreBuffer,
            &remainingSize, SerializeIF::Endianness::BIG);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sendCompletionReply(false, result);
        return result;
    }

    size_t filesize = 0;
    bool locked = false;
    int retval = get_file_info(command.getRepositoryPathRaw(),
            command.getFilenameRaw(), &filesize, &locked, nullptr, nullptr);
    if(retval != F_NO_ERROR) {
        result = HasReturnvaluesIF::RETURN_FAILED;
        sendCompletionReply(false, result, retval);
    }

    uint8_t* writePtr = nullptr;

    FileAttributesReply replyPacket(command.getRepoPath(),
            command.getFilename(), filesize, locked);
    size_t sizeToSerialize = replyPacket.getSerializedSize();
    result = ipcStore->getFreeElement(&storeId,
            sizeToSerialize, &writePtr);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sendCompletionReply(false, result);
        return result;
    }

    size_t serializedSize = 0;
    result = replyPacket.serialize(&writePtr, &serializedSize, sizeToSerialize,
            SerializeIF::Endianness::BIG);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sendCompletionReply(false, result);
        return result;
    }

    CommandMessage reply;
    FileSystemMessage::setReportFileAttributesReply(&reply, storeId);
    result = commandQueue->reply(&reply);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sendCompletionReply(false, result);
        return result;
    }
    sendCompletionReply();
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::handleCreateDirectoryCommand(
        CommandMessage* message){
    store_address_t storeId = FileSystemMessage::getStoreId(message);
    ConstStorageAccessor accessor(storeId);
    const uint8_t* ipcStoreBuffer = nullptr;
    size_t remainingSize = 0;
    ReturnValue_t result = getStoreData(storeId, accessor, &ipcStoreBuffer,
            &remainingSize);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    CreateDirectoryCommand command;
    /* Extract the repository path and the directory name from the application data field */
    result = command.deSerialize(&ipcStoreBuffer, &remainingSize,
            SerializeIF::Endianness::BIG);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sendCompletionReply(false, result);
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    result = createDirectory(command.getRepositoryPath(),
            command.getDirname());
    if (result != HasReturnvaluesIF::RETURN_OK) {
        /* If the folder already exists, count that as success.. */
        if(result != HasFileSystemIF::DIRECTORY_ALREADY_EXISTS) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SDCardHandler::handleCreateDirectoryCommand: Creating directory " <<
                    command.getDirname() << " failed." << std::endl;
#else
            sif::printError("SDCardHandler::handleCreateDirectoryCommand: Creating directory "
                    "%s failed.\n", command.getDirname());
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
            sendCompletionReply(false, result);
        }
    }

    sendCompletionReply();
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::handleDeleteDirectoryCommand(
        CommandMessage* message){
    store_address_t storeId = FileSystemMessage::getStoreId(message);
    ConstStorageAccessor accessor(storeId);
    const uint8_t* ipcStoreBuffer = nullptr;
    size_t remainingSize = 0;
    ReturnValue_t result = getStoreData(storeId, accessor, &ipcStoreBuffer,
            &remainingSize);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    DeleteDirectoryCommand command;
    /* Extract the repository path and the directory name from the
    application data field */
    result = command.deSerialize(&ipcStoreBuffer, &remainingSize,
            SerializeIF::Endianness::BIG);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sendCompletionReply(false, result);
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    result = removeDirectory(command.getRepositoryPath(),
            command.getDirname());
    if (result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "Deleting directory " << command.getDirname() << " failed." << std::endl;
#else
        sif::printError("Deleting directory %s failed.\n", command.getDirname());
#endif
        sendCompletionReply(false, result);
    }
    else {
        sendCompletionReply();
    }

    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::handleLockFileCommand(CommandMessage *message,
        bool lock) {
    store_address_t storeId = FileSystemMessage::getStoreId(message);
    ConstStorageAccessor accessor(storeId);
    size_t sizeRemaining = 0;
    const uint8_t* readPtr = nullptr;
    ReturnValue_t result = getStoreData(storeId, accessor, &readPtr,
            &sizeRemaining);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    LockFileCommand command;
    result = command.deSerialize(&readPtr, &sizeRemaining,
            SerializeIF::Endianness::BIG);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sendCompletionReply(false, result);
    }

    int retval = 0;
    if(lock) {
        retval = lock_file(command.getRepositoryPathRaw(),
                command.getFilenameRaw());
    }
    else {
        retval = unlock_file(command.getRepositoryPathRaw(),
                command.getFilenameRaw());
    }

    if(retval == F_NO_ERROR) {
        sendCompletionReply(true);
    }
    else {
        result = HasReturnvaluesIF::RETURN_FAILED;
        sendCompletionReply(false, result, retval);
    }
    return result;
}

ReturnValue_t SDCardHandler::printRepository(const char *repository) {
    int result = change_directory(repository, true);
    if(result != F_NO_ERROR) {
        return result;
    }

    F_FIND findResult;
    int fileFound = f_findfirst("*.*", &findResult);
    if(fileFound != F_NO_ERROR) {
        return HasReturnvaluesIF::RETURN_OK;
    }

    if(findResult.filename[0] == '.') {
        /* we are not in root, so the next search result is going to be the parent folder, and
        the third result is going to be the first file or directory. */
        f_findnext(&findResult);
        fileFound = f_findnext(&findResult);
    }

    for(int idx = 0; idx < 255; idx++) {
        if(idx > 0) {
            fileFound = f_findnext(&findResult);
        }

        if(fileFound != F_NO_ERROR) {
            break;
        }

        /* Check whether file object is directory or file. */
        if(change_directory(findResult.filename, false) == F_NO_ERROR) {
            change_directory("..", false);
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "D: " << findResult.filename << std::endl;
#else
            sif::printInfo("D: %s\n", findResult.filename);
#endif
        }
        else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "F: " << findResult.filename << std::endl;
#else
            sif::printInfo("F: %s\n", findResult.filename);
#endif
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::printFilesystemHelper(uint8_t recursionDepth) {
    F_FIND findResult;
    int fileFound = f_findfirst("*.*", &findResult);
    if(fileFound != F_NO_ERROR) {
        return HasReturnvaluesIF::RETURN_OK;
    }

    if(findResult.filename[0] == '.') {
        /* we are not in root, so the next search result is going to be the parent folder, and
        the third result is going to be the first file or directory. */
        f_findnext(&findResult);
        fileFound = f_findnext(&findResult);
    }

#if FSFW_CPP_OSTREAM_ENABLED == 0
    char subdirDepth[10];
    uint8_t subdirsLen = 0;
#endif

    for(uint8_t idx = 0; idx < 255; idx ++) {
        if(idx > 0) {
            fileFound = f_findnext(&findResult);
        }

        if(fileFound != F_NO_ERROR) {
            return HasReturnvaluesIF::RETURN_OK;
        }


        if(change_directory(findResult.filename, false) == F_NO_ERROR) {
            for(uint8_t j = 0; j < recursionDepth; j++) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
                sif::info << "-";
#else
                subdirsLen += sprintf(subdirDepth + subdirsLen , "-");
#endif
            }
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "D: " << findResult.filename << std::endl;
#else
            sif::printInfo("%sD: %s\n", subdirDepth, findResult.filename);
            subdirsLen = 0;
#endif
            printFilesystemHelper(recursionDepth + 1);
            change_directory("..", false);
        }
        else {
            for(uint8_t j = 0; j < recursionDepth; j++) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
                sif::info << "-";
#else
                subdirsLen += sprintf(subdirDepth + subdirsLen , "-");
#endif
            }
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "F: " << findResult.filename << std::endl;
#else
            sif::printInfo("%sF: %s\n", subdirDepth, findResult.filename);
            subdirsLen = 0;
#endif
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::printSdCard() {
    F_FIND findResult;
    int fileFound = 0;
    uint8_t recursionDepth = 0;
    f_chdir("/");
    /* find directories first */
    fileFound = f_findfirst("*.*", &findResult);
    if(fileFound != F_NO_ERROR) {
        /* might be empty. */
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "SD Card empty." << std::endl;
#else
        sif::printInfo("SD Card empty.\n");
#endif
        return HasReturnvaluesIF::RETURN_OK;
    }

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Printing SD Card: " << std::endl;
    sif::info << "F = File, D = Directory, - = Subdir Depth" << std::endl;
#else
    sif::printInfo("Printing SD Card: \n");
    sif::printInfo("F = File, D = Directory, - = Subdir Depth\n");
#endif

    for(int idx = 0; idx < 255; idx++) {
        if(idx > 0) {
            fileFound = f_findnext(&findResult);
        }

        if(fileFound != F_NO_ERROR) {
            break;
        }

        /* check whether file object is directory or file. */
        if(change_directory(findResult.filename, false) == F_NO_ERROR) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "D: " << findResult.filename << std::endl;
#else
            sif::printInfo("D: %s\n", findResult.filename);
#endif
            printFilesystemHelper(recursionDepth + 1);
            change_directory("..", false);
        }
        else {
            /* Normally files should have a three letter extension, but we always check whether
            there is a file without extension */
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "F: " << findResult.filename << std::endl;
#else
            sif::printInfo("F: %s\n", findResult.filename);
#endif
        }

    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::dumpSdCard() {
    // TODO: implement. This dumps the file structure of the SD card and will
    // be one of the most important functionalities for operators.
    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t SDCardHandler::removeFile(const char* repositoryPath,
        const char* filename, FileSystemArgsIF* args) {
    int result = delete_file(repositoryPath, filename);
    if(result == F_NO_ERROR) {
        return HasReturnvaluesIF::RETURN_OK;
    }
    else {
        return result;
    }
}

ReturnValue_t SDCardHandler::createFile(const char* dirname,
        const char* filename, const uint8_t* data, size_t size,
        FileSystemArgsIF* args) {
    int result = create_file(dirname, filename, data, size);
    if(result == -2) {
        return HasFileSystemIF::DIRECTORY_DOES_NOT_EXIST;
    }
    else if(result == -1) {
        return HasFileSystemIF::FILE_ALREADY_EXISTS;
    }
    else {
        //*bytesWritten = result;
        return HasReturnvaluesIF::RETURN_OK;
    }
}

ReturnValue_t SDCardHandler::createDirectory(const char* repositoryPath,
        bool createParentDirs, FileSystemArgsIF* args) {
    int result = create_directory(nullptr, repositoryPath);
    if(result == F_ERR_DUPLICATED) {
        return HasFileSystemIF::DIRECTORY_ALREADY_EXISTS;
    }
    else if(result != F_NO_ERROR) {
        return result;
    }

    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::renameFile(const char* repositoryPath, const char* oldFilename,
        const char* newFilename, FileSystemArgsIF* args) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::removeDirectory(const char* repositoryPath,
        bool deleteRecurively, FileSystemArgsIF* args) {
    int result = delete_directory(nullptr, repositoryPath);
    if(result == F_ERR_NOTEMPTY) {
        return HasFileSystemIF::DIRECTORY_NOT_EMPTY;
    }
    else if(result != F_NO_ERROR) {
        // should not happen (directory read only)
        return result;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::changeDirectory(const char* repositoryPath) {
    // change to root directory, all paths are going to be relative.
    int result = change_directory(repositoryPath, true);
    if(result == F_NO_ERROR) {
        return HasReturnvaluesIF::RETURN_OK;
    }
    else {
        if(result == F_ERR_INVALIDDIR) {
            return HasFileSystemIF::DIRECTORY_DOES_NOT_EXIST;
        }
        return result;
    }
}

ReturnValue_t SDCardHandler::getStoreData(store_address_t& storeId,
        ConstStorageAccessor& accessor,
        const uint8_t** ptr, size_t* size) {
    ReturnValue_t result = ipcStore->getData(storeId, accessor);
    if(result != HasReturnvaluesIF::RETURN_OK){
        // Should not happen!
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "SDCardHandler::getStoreData: Getting data failed!" << std::endl;
#else
        sif::printError("SDCardHandler::getStoreData: Getting data failed!\n");
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    *size = accessor.size();
    *ptr = accessor.data();
    return HasReturnvaluesIF::RETURN_OK;
}

void SDCardHandler::sendCompletionReply(bool success, ReturnValue_t errorCode,
        uint32_t errorParam) {
    return sendCompletionMessage(success, MessageQueueIF::NO_QUEUE, errorCode, errorParam);
}

void SDCardHandler::sendCompletionMessage(bool success, MessageQueueId_t queueId,
        ReturnValue_t errorCode, uint32_t errorParam) {
    CommandMessage reply;
    if(success) {
        FileSystemMessage::setSuccessReply(&reply);
    }
    else {
        FileSystemMessage::setFailureReply(&reply, errorCode, errorParam);
    }

    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    if(queueId == MessageQueueIF::NO_QUEUE) {
        result = commandQueue->reply(&reply);
    }
    else {
        result = commandQueue->sendMessage(queueId, &reply);
    }

    if(result != HasReturnvaluesIF::RETURN_OK){
        if(result == MessageQueueIF::FULL) {
            /* Configuration error. */
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SDCardHandler::sendCompletionReply: Queue of receiver is full!" <<
                    std::endl;
#else
            sif::printError("SDCardHandler::sendCompletionReply: Queue of receiver is full!\n");
#endif
        }
    }
}


ReturnValue_t SDCardHandler::handleReadCommand(CommandMessage* message) {
    store_address_t storeId = FileSystemMessage::getStoreId(message);
    ConstStorageAccessor accessor(storeId);
    size_t sizeRemaining = 0;
    const uint8_t* readPtr = nullptr;
    ReturnValue_t result = getStoreData(storeId, accessor, &readPtr,
            &sizeRemaining);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    ReadCommand command;
    result = command.deSerialize(&readPtr, &sizeRemaining,
            SerializeIF::Endianness::BIG);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    result = handleSequenceNumberRead(command.getSequenceNumber());
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    return handleReadReplies(command);
}

ReturnValue_t SDCardHandler::handleSequenceNumberRead(uint16_t sequenceNumber) {
    if(sequenceNumber == 0) {
        lastPacketReadNumber = 0;
    }
    else if((sequenceNumber == 1) and (lastPacketReadNumber != 0)) {
#if OBSW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "SDCardHandler::appendToFile: First sequence packet missed!" << std::endl;
#else
        sif::printDebug("SDCardHandler::appendToFile: First sequence packet missed!\n");
#endif
#endif
        triggerEvent(sdchandler::SEQUENCE_PACKET_MISSING_READ_EVENT, 0, 0);
        return SEQUENCE_PACKET_MISSING_READ;
    }
    else if((sequenceNumber - lastPacketReadNumber) > 1) {
#if OBSW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "SDCardHandler::appendToFile: Packet missing between "
                << sequenceNumber << " and " << lastPacketReadNumber << std::endl;
#else
        sif::printDebug("SDCardHandler::appendToFile: Packet missing between %hu and %hu\n",
                sequenceNumber, lastPacketReadNumber);
#endif
#endif
        triggerEvent(sdchandler::SEQUENCE_PACKET_MISSING_READ_EVENT,
                lastPacketReadNumber + 1, 0);
        return SEQUENCE_PACKET_MISSING_READ;
    }
    else {
        lastPacketReadNumber = sequenceNumber;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::handleReadReplies(ReadCommand& command) {
    // Open file for reading and get file size
    F_FILE* file = nullptr;
    size_t fileSize = 0;
    size_t sizeToRead = 0;
    currentReadPos = command.getSequenceNumber() * MAX_READ_LENGTH;
    ReturnValue_t result = openFileForReading(command.getRepositoryPathRaw(),
            command.getFilenameRaw(), &file, currentReadPos, &fileSize,
            &sizeToRead);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    bool readOpFinished = false;
    if(sizeToRead < MAX_READ_LENGTH) {
        readOpFinished = true;
    }

    // Generate and serialize the reply packet.
    ReadReply replyPacket(command.getRepoPath(), command.getFilename(),
            &file, sizeToRead);

    // Get space in IPC store to serialize packet.
    uint8_t* writePtr = nullptr;
    store_address_t storeId;
    result = ipcStore->getFreeElement(&storeId,
            replyPacket.getSerializedSize(), &writePtr);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        int retval = f_close(file);
        if(retval != F_NO_ERROR) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SDCardHandler::handleReadCommand: Closing file failed!" << std::endl;
#else
            sif::printError("SDCardHandler::handleReadCommand: Closing file failed!\n");
#endif
        }
        return result;
    }
    size_t serializedSize = 0;
    result = replyPacket.serialize(&writePtr, &serializedSize,
            sizeToRead,SerializeIF::Endianness::BIG);
    if(result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "SDCardHandler::handleReadReply: Serialization of file "
                << command.getFilename() << " failed" << std::endl;
#else
        sif::printWarning("SDCardHandler::handleReadReply: Reading from file %s failed\n",
                command.getFilename()->c_str());
#endif
        sendCompletionReply(false, result);
        int retval = f_close(file);
        if(retval != F_NO_ERROR) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SDCardHandler::handleReadCommand: Closing file failed!" << std::endl;
#else
            sif::printError("SDCardHandler::handleReadCommand: Closing file failed!\n");
#endif
        }
        return result;
    }

    // Generate the reply.
    {
        CommandMessage reply;
        if(readOpFinished) {
            FileSystemMessage::setReadReply(&reply, true, storeId);
        }
        else {
            FileSystemMessage::setReadReply(&reply, false, storeId);
        }

        result = commandQueue->reply(&reply);
        if(result != HasReturnvaluesIF::RETURN_OK){
            if(result == MessageQueueIF::FULL){
#if FSFW_CPP_OSTREAM_ENABLED == 1
                sif::debug << "SDCardHandler::sendDataReply: Could not send "
                        << "data reply, queue of receiver is full!" << std::endl;
#else
                sif::printDebug("SDCardHandler::sendDataReply: Could not send "
                        "data reply, queue of receiver is full!\n");
#endif
            }
        }
    }

    int retval = f_close(file);
    if(retval != F_NO_ERROR) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "SDCardHandler::handleReadCommand: Closing file failed!" << std::endl;
#else
        sif::printError("SDCardHandler::handleReadCommand: Closing file failed!\n");
#endif
    }

    if(readOpFinished) {
        CommandMessage reply;
        // TODO: implement packing this;
        FileSystemMessage::setReadFinishedReply(&reply, storeId);
        result = commandQueue->reply(&reply);
    }
    return result;
}


ReturnValue_t SDCardHandler::openFileForReading(const char* repositoryPath,
        const char* filename, F_FILE** file,
        size_t readPosition, size_t* fileSize, size_t* sizeToRead) {
    int result = changeDirectory(repositoryPath);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    *file = f_open(filename, "r");
    if (f_getlasterror() != F_NO_ERROR) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "SDCardHandler::readFile: Opening file failed with code "
                 << f_getlasterror() << std::endl;
#else
        sif::printError("SDCardHandler::readFile: Opening file failed with code %d\n",
                f_getlasterror());
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    *fileSize = f_filelength(filename);

    // Set correct size to read and read position
    if(readPosition > *fileSize) {
        // Configuration error.
        *sizeToRead = 0;
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "SDCardHandler::openFileForReading: Specified read"
                << " position larger than file size!" << std::endl;
#else
        sif::printWarning("SDCardHandler::openFileForReading: Specified read"
                " position larger than file size!\n");
#endif
        return HasReturnvaluesIF::RETURN_OK;
    }
    // This also covers the case *fileSize == readPosition
    else if(*fileSize - readPosition < MAX_READ_LENGTH) {
        result = f_seek(*file, readPosition, F_SEEK_SET);
        *sizeToRead = *fileSize - readPosition;
        if(result != F_NO_ERROR) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SDCardHandler::openFileForReading: Seeking read"
                    << " position failed with code" << result << std::endl;
#else
            sif::printError("SDCardHandler::openFileForReading: Seeking read"
                    " position failed with code %d!\n", result);
#endif
        }
    }
    else {
        result = f_seek(*file, readPosition, F_SEEK_SET);
        *sizeToRead = MAX_READ_LENGTH;
        if(result != F_NO_ERROR) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SDCardHandler::openFileForReading: Seeking read"
                    << " position failed with code" << result << std::endl;
#else
            sif::printError("SDCardHandler::openFileForReading: Seeking read"
                    " position failed with code %d!\n", result);
#endif
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
}

#include "SDCardHandler.h"
#include "SDCardAccess.h"
#include "SDCardHandlerPackets.h"
#include "sdcardHandlerDefinitions.h"
#include "SdCardHandlerArgs.h"

#include "fsfw/serviceinterface/ServiceInterface.h"
#include "bsp_sam9g20/memory/HCCFileGuard.h"
#include "mission/memory/FileSystemMessage.h"

#define SDC_FILE_WRITE_WIRETAPPING    0

ReturnValue_t SDCardHandler::handleCreateFileCommand(CommandMessage *message) {
    store_address_t storeId = FileSystemMessage::getStoreId(message);
    ConstStorageAccessor accessor(storeId);
    const uint8_t* readPtr = nullptr;
    size_t sizeRemaining = 0;
    ReturnValue_t result = getStoreData(storeId, accessor, &readPtr,
            &sizeRemaining);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    WriteCommand command(WriteCommand::WriteType::NEW_FILE);
    result = command.deSerialize(&readPtr, &sizeRemaining,
            SerializeIF::Endianness::BIG);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sendCompletionReply(false, result);
        return result;
    }

#if SDC_FILE_WRITE_WIRETAPPING == 1
    sif::printInfo("SDCardHandler::handleCreateFileCommand | Create file %s/%s\n",
            command.getRepositoryPath(), command.getFilename()
    );
#endif
    result = createFile(command.getRepositoryPath(), command.getFilename(),
            command.getFileData(), command.getFileSize(), nullptr);
    if(result == HasReturnvaluesIF::RETURN_OK) {
        sendCompletionReply();
    }
    else {
        sendCompletionReply(false, result);
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::handleAppendCommand(CommandMessage* message){
    store_address_t storeId = FileSystemMessage::getStoreId(message);
    ConstStorageAccessor accessor(storeId);
    size_t sizeRemaining = 0;
    const uint8_t* readPtr = nullptr;
    ReturnValue_t result = getStoreData(storeId, accessor, &readPtr,
            &sizeRemaining);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }


    WriteCommand command(WriteCommand::WriteType::APPEND_TO_FILE);
    result = command.deSerialize(&readPtr, &sizeRemaining,
            SerializeIF::Endianness::BIG);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    uint16_t packetSequenceIfMissing = 0;
    SdCardHandlerArgs sdArgs(&packetSequenceIfMissing);
#if SDC_FILE_WRITE_WIRETAPPING == 1
    sif::printInfo("SDCardHandler::handleAppendCommand | Append to file %s/%s\n",
            command.getRepositoryPath(), command.getFilename()
    );
#endif
    result = appendToFile(command.getRepositoryPath(), command.getFilename(), command.getFileData(),
            command.getFileSize(), command.getPacketNumber(),
            &sdArgs);
    if(result != HasReturnvaluesIF::RETURN_OK){
        if(result == SEQUENCE_PACKET_MISSING_WRITE) {
            sendCompletionReply(false, result, packetSequenceIfMissing);
        }
        else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SDCardHandler::handleWriteCommand: Writing to file " <<
                    command.getFilename()  << " failed." << std::endl;
#else
            sif::printError("SDCardHandler::handleWriteCommand: Writing to file %s failed\n",
                    command.getFilename());
#endif
            sendCompletionReply(false, result);
        }

    }
    else {
        sendCompletionReply();
    }

    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::appendToFile(const char* repositoryPath,
        const char* filename, const uint8_t* data, size_t size,
        uint16_t packetNumber,  FileSystemArgsIF* args) {
    ReturnValue_t result = changeDirectory(repositoryPath);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    uint16_t* packetSeqIfMissing = nullptr;
    SdCardHandlerArgs* sdArgs = dynamic_cast<SdCardHandlerArgs*>(args);
    if(sdArgs != nullptr) {
        packetSeqIfMissing = sdArgs->packetSeqIfMissing;
    }
    if(packetSeqIfMissing == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "SDCardHandler::appendToFile: Args invalid!"
                << std::endl;
#else
        sif::printError("SDCardHandler::appendToFile: Args invalid!\n");
        return HasReturnvaluesIF::RETURN_FAILED;
#endif
    }

    result = handleSequenceNumberWrite(packetNumber, packetSeqIfMissing);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }


    /* Try to open file. File should already exist, therefore "r+" for first
    packet. Subsequent packets are appended at the end of the file.
    Therefore file is opened in append mode. */
    F_FILE* file = nullptr;
    if(packetNumber == 0) {
        file = f_open(filename, "r+");
        if(file != nullptr) {
            result = f_seek(file, 0, F_SEEK_END);
            if(result != F_NO_ERROR) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
                sif::error << "SDCardHandler::appendToFile: f_seek failed with error code "
                        << result << "!" << std::endl;
#else
                sif::printError("SDCardHandler::appendToFile: f_seek failed with "
                        "error code %d!\n", result);
#endif
            }
        }
    }
    else {
        file = f_open(filename, "a");
    }

    /* File does not exist */
    if(file == nullptr) {
        result = f_getlasterror();
        if(result == F_ERR_NOTFOUND) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SDCardHandler::appendToFile: File to append to does not exist, "
                    "error code" << result << std::endl;
#else
            sif::printError("SDCardHandler::appendToFile: File to append to does not exist, "
                    "error code %d\n", result);
#endif
            return FILE_DOES_NOT_EXIST;
        }
        else if(result == F_ERR_LOCKED) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SDCardHandler::appendToFile: File to append to is "
                    "locked, error code" << result << std::endl;
#else
            sif::printError("SDCardHandler::appendToFile: File to append to is "
                    "locked, error code %d.", result);
#endif
            return FILE_LOCKED;
        }
        else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SDCardHandler::appendToFile: Opening file failed "
                    "with error code" << result << std::endl;
#else
            sif::printError("SDCardHandler::appendToFile: Opening file failed "
                    "with error code %d.\n", result);
#endif
        }

        return HasReturnvaluesIF::RETURN_FAILED;
    }

    HCCFileGuard fileHelper(&file);
    long numberOfItemsWritten = f_write(data, sizeof(uint8_t), size, file);
    /* If bytes written doesn't equal bytes to write, get the error */
    if (numberOfItemsWritten != (long) size) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "SDCardHandler::writeToFile: Not all bytes written,"
                << " f_write error code " << f_getlasterror() << std::endl;
#else
        sif::printError("SDCardHandler::writeToFile: Not all bytes written,"
                " f_write error code %d\n", f_getlasterror());
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::handleFinishAppendCommand(
        CommandMessage* message) {
    store_address_t storeId = FileSystemMessage::getStoreId(message);
    ConstStorageAccessor accessor(storeId);
    size_t sizeRemaining = 0;
    const uint8_t* readPtr = nullptr;
    ReturnValue_t result = getStoreData(storeId, accessor, &readPtr,
            &sizeRemaining);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    FinishAppendCommand finishAppendCommand;
    result = finishAppendCommand.deSerialize(&readPtr, &sizeRemaining,
            SerializeIF::Endianness::BIG);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    /* The file can be locked via the finish command optionally */
    if(finishAppendCommand.getLockFile()) {
        int retval = lock_file(finishAppendCommand.getRepositoryPathRaw(),
                finishAppendCommand.getFilenameRaw());
        if(retval != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SDCardHandler::handleFinishAppendCommand: Could"
                    << " not lock file, code " << result << "!" << std::endl;
#else
            sif::printError("SDCardHandler::handleFinishAppendCommand: Could"
                    " not lock file, code %d\n", result);
#endif
        }
    }

    /* Get file information for the reply packet. ctime and cdate not contained for now. */
    size_t fileSize = 0;
    bool locked = false;
    int retval = get_file_info(finishAppendCommand.getRepositoryPathRaw(),
            finishAppendCommand.getFilenameRaw(), &fileSize, &locked, nullptr,
            nullptr);
    if(retval != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "SDCardHandler::handleFinishAppendCommand: get_file_info"
                << " failed with error code " << retval << "!" << std::endl;
#else
        sif::printError("SDCardHandler::handleFinishAppendCommand: get_file_info"
                " failed with error code %d\n", retval);
#endif
    }

    return generateFinishAppendReply(finishAppendCommand.getRepoPath(),
            finishAppendCommand.getFilename(), fileSize, locked);
}


ReturnValue_t SDCardHandler::generateFinishAppendReply(RepositoryPath *repoPath,
        FileName *fileName, size_t filesize, bool locked) {
    store_address_t storeId;
    FinishAppendReply replyPacket(repoPath, fileName, lastPacketWriteNumber + 1, filesize, locked);

    uint8_t* ptr = nullptr;
    size_t serializedSize = 0;
    ReturnValue_t result = ipcStore->getFreeElement(&storeId,
            replyPacket.getSerializedSize(), &ptr);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        /* Reset last packet sequence number */
        lastPacketWriteNumber = UNSET_SEQUENCE;
        return result;
    }
    result = replyPacket.serialize(&ptr, &serializedSize,
            replyPacket.getSerializedSize(),
            SerializeIF::Endianness::BIG);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        lastPacketWriteNumber = UNSET_SEQUENCE;
        return result;
    }

    CommandMessage reply;
    FileSystemMessage::setFinishAppendReply(&reply, storeId);
    result = commandQueue->reply(&reply);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        lastPacketWriteNumber = UNSET_SEQUENCE;
        return result;
    }

#if OBSW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Append operation on file " << fileName->c_str() << " in repository " <<
            repoPath->c_str() << " finished." << std::endl;
    sif::info <<  "Filesize: " << filesize << ".";
    if(locked) {
        sif::info << " File was locked" << std::endl;
    }
    else {
        sif::info << " File was not locked" << std::endl;
    }
#else
    sif::printInfo("Append operation on file %s in repository %s finished\n", fileName->c_str(),
            repoPath->c_str());
    sif::printInfo("Filesize: %lu\n", static_cast<unsigned long>(filesize));
    if(locked) {
        sif::printInfo("File was locked.\n");
    }
    else {
        sif::printInfo("File was not locked\n");
    }
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* OBSW_VERBOSE_LEVEL >= 1 */
    lastPacketWriteNumber = UNSET_SEQUENCE;
    return result;

}


ReturnValue_t SDCardHandler::handleSequenceNumberWrite(uint16_t sequenceNumber,
        uint16_t* packetSeqIfMissing) {
    if(sequenceNumber == 0) {
        lastPacketWriteNumber = 0;
    }
    else if((sequenceNumber == 1) and (lastPacketWriteNumber != 0)) {
#if OBSW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "SDCardHandler::appendToFile: First sequence packet missed!" << std::endl;
#else
        sif::printDebug("SDCardHandler::appendToFile: First sequence packet missed!\n");
#endif
#endif
        triggerEvent(sdchandler::SEQUENCE_PACKET_MISSING_WRITE_EVENT, 0, 0);
        *packetSeqIfMissing = 0;
        return SEQUENCE_PACKET_MISSING_WRITE;
    }
    else if((sequenceNumber - lastPacketWriteNumber) > 1) {
#if OBSW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "SDCardHandler::appendToFile: Packet missing between "
                << sequenceNumber << " and " << lastPacketWriteNumber
                << std::endl;
#else
        sif::printDebug("SDCardHandler::appendToFile: Packet missing between %hu amd %hu\n",
                sequenceNumber ,lastPacketWriteNumber);
#endif
#endif
        triggerEvent(sdchandler::SEQUENCE_PACKET_MISSING_WRITE_EVENT,
                lastPacketWriteNumber + 1, 0);
        *packetSeqIfMissing = lastPacketWriteNumber + 1;
        return SEQUENCE_PACKET_MISSING_WRITE;
    }
    else {
        lastPacketWriteNumber = sequenceNumber;
    }
    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t SDCardHandler::handleCopyCommand(CommandMessage *message) {
    store_address_t storeId = FileSystemMessage::getStoreId(message);
    ConstStorageAccessor storeAccess(storeId);
    ReturnValue_t result = ipcStore->getData(storeId, storeAccess);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        /* Problems with IPC store, reject */
        sendCompletionReply(false, result);
    }

    CopyFileCommand copyCommand;
    size_t sizeToDeserialize = storeAccess.size();

    if(sizeToDeserialize == 0) {
        /* Empty packet, reject */
        sendCompletionReply(false, HasFileSystemIF::INVALID_PARAMETERS);
    }

    const uint8_t* dataPtr = storeAccess.data();
    result = copyCommand.deSerialize(&dataPtr, &sizeToDeserialize, SerializeIF::Endianness::BIG);

    if(result != HasReturnvaluesIF::RETURN_OK) {
        /* Notify about failed serialization */
        sendCompletionReply(false, result);
        return HasReturnvaluesIF::RETURN_OK;
    }

    /* Attempt to set state machine operation. The operation might take multiple cycles
    and the state machine will take core of reporting the operation success */
    if(not stateMachine.setCopyFileOperation(*copyCommand.getSourceRepoPath(),
            *copyCommand.getSourceFilename(), *copyCommand.getTargetRepoPath(),
            *copyCommand.getTargetFilename(), message->getSender())) {
        sendCompletionReply(false, HasFileSystemIF::IS_BUSY);
    }

    return HasReturnvaluesIF::RETURN_OK;
}

