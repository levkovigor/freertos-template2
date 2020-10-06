#include "SDCardHandler.h"
#include "SDCardHandlerPackets.h"

#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/ipc/CommandMessage.h>
#include <fsfw/timemanager/Countdown.h>
#include <fsfw/timemanager/Stopwatch.h>
#include <sam9g20/memory/FileSystemMessage.h>
#include <sam9g20/memory/SDCardAccess.h>

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
    case FileSystemMessage::CREATE_FILE: {
        result = handleCreateFileCommand(message);
        if(result != HasReturnvaluesIF::RETURN_OK){
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        break;
    }
    case FileSystemMessage::DELETE_FILE: {
        result = handleDeleteFileCommand(message);
        if(result != HasReturnvaluesIF::RETURN_OK){
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        break;
    }
    case FileSystemMessage::CREATE_DIRECTORY: {
        result = handleCreateDirectoryCommand(message);
        if(result != HasReturnvaluesIF::RETURN_OK){
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        break;
    }
    case FileSystemMessage::DELETE_DIRECTORY: {
        result = handleDeleteDirectoryCommand(message);
        if(result != HasReturnvaluesIF::RETURN_OK){
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        break;
    }
    case FileSystemMessage::APPEND_TO_FILE: {
        result = handleAppendCommand(message);
        if(result != HasReturnvaluesIF::RETURN_OK){
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        break;
    }
    case FileSystemMessage::READ_FROM_FILE: {
        result = handleReadCommand(message);
        if(result != HasReturnvaluesIF::RETURN_OK){
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        break;
    }
    default: {
        sif::debug << "SDCardHandler::handleFileMessage: "
                << "Invalid filesystem command" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    }
    return HasReturnvaluesIF::RETURN_OK;
}


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
    /* Extract the repository path and the filename from the
        application data field */
    result = command.deSerialize(&ipcStoreBuffer,
            &remainingSize, SerializeIF::Endianness::BIG);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sendCompletionReply(false, result);
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    result = deleteFile(command.getRepositoryPath(), command.getFilename());
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sendCompletionReply(false, result);
    }
    else {
        sendCompletionReply();
    }
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
    // Extract the repository path and the directory name
    // from the application data field
    result = command.deSerialize(&ipcStoreBuffer, &remainingSize,
            SerializeIF::Endianness::BIG);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sendCompletionReply(false, result);
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    result = createDirectory(command.getRepositoryPath(),
            command.getDirname());
    if (result != HasReturnvaluesIF::RETURN_OK) {
    	// If the folder already exists, count that as success..
        if(result != HasFileSystemIF::DIRECTORY_ALREADY_EXISTS) {
            sif::error << "SDCardHandler::handleCreateDirectoryCommand: "
                    << "Creating directory " << command.getDirname()
                    << " failed" << std::endl;
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
    result = command.deSerialize(&ipcStoreBuffer, remainingSize);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sendCompletionReply(false, result);
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    result = deleteDirectory(command.getRepositoryPath(),
            command.getDirname());
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Deleting directory " << command.getDirname()
                                    << " failed" << std::endl;
        sendCompletionReply(false, result);
    }
    else {
        sendCompletionReply();
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

    result = appendToFile(command.getRepositoryPath(),
            command.getFilename(), command.getFileData(),
            command.getFileSize(), command.getPacketNumber());
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "SDCardHandler::handleWriteCommand: Writing to file "
                << command.getFilename()  << " failed" << std::endl;
        sendCompletionReply(false, result);
    }
    else {
        sendCompletionReply();
    }

    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t SDCardHandler::handleReadCommand(CommandMessage* message) {
    MessageQueueId_t receivedFromQueueId = message -> getSender();
    store_address_t storeId = FileSystemMessage::getStoreId(message);

    auto resultPair = IPCStore->getData(storeId);
    if(resultPair.first != HasReturnvaluesIF::RETURN_OK) {
        return resultPair.first;
    }

    size_t remainingSize = resultPair.second.size();
    const uint8_t* ipcStoreBuffer =  resultPair.second.data();
    ReadCommand command;
    ReturnValue_t result = command.deSerialize(&ipcStoreBuffer,
            &remainingSize, SerializeIF::Endianness::BIG);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    uint8_t tmData[readReplyMaxLen];
    size_t tmDataLen = 0;
    result = readFile(command.getRepositoryPath(), command.getFilename(),
            tmData, &tmDataLen);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Reading from file " << command.getFilename()
                            << " failed" << std::endl;
        sendCompletionReply(false, result);
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    return sendDataReply(receivedFromQueueId, tmData, tmDataLen);
}


ReturnValue_t SDCardHandler::sendDataReply(MessageQueueId_t receivedFromQueueId,
        uint8_t* tmData, size_t tmDataLen){
    store_address_t parameterAddress;
    ReturnValue_t result = IPCStore->addData(&parameterAddress, tmData, tmDataLen);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "SDCardHandler::sendDataReply: Failed to store data in "
                << "IPC store" << std::endl;
        result = IPCStore->deleteData(parameterAddress);
        if(result != HasReturnvaluesIF::RETURN_OK){
            sif::error << "SDCardHandler::sendDataReply:"
                    << "Failed to delete IPC store entry" << std::endl;
        }
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    CommandMessage reply;
    FileSystemMessage::setReadReply(&reply, parameterAddress);
    result = commandQueue->sendMessage(receivedFromQueueId, &reply);
    if(result != HasReturnvaluesIF::RETURN_OK){
        if(result == MessageQueueIF::FULL){
            sif::debug << "SDCardHandler::sendDataReply: Could not send data "
                    << "reply, queue of receiver is full!" << std::endl;
        }
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::readFile(const char* repositoryPath,
        const char* filename, uint8_t* tmData, size_t* tmDataLen) {
    int result = changeDirectory(repositoryPath);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    F_FILE* file = f_open(filename, "r");
    if (f_getlasterror() != F_NO_ERROR) {
        sif::error << "f_open pb: " << f_getlasterror() << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    long filesize = f_filelength(filename);
    // TODO: sanity check for file size. (limited)
    uint8_t filecontent[filesize];
    if (f_read(filecontent, sizeof(uint8_t), filesize, file) != filesize) {
        sif::error << "Not all items read" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    result = f_close(file);
    if (result != F_NO_ERROR) {
        sif::error << "f_close pb: " << result << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    ReadReply reply(repositoryPath,
            filename, filecontent, (uint16_t) filesize);
    result = reply.serialize(tmData, tmDataLen);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "Serialization of read reply failed." << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
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
        return result;
    }
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
        // we are not in root, so the next search result is going
        // to be the parent folder, and the third result is going to be
        // the first file or directory.
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

        // check whether file object is directory or file.
        if(change_directory(findResult.filename, false) == F_NO_ERROR) {
        	change_directory("..", false);
            sif::info << "D: " << findResult.filename << std::endl;
        }
        else {
            sif::info << "F: " << findResult.filename << std::endl;
        }

    }
    return HasReturnvaluesIF::RETURN_OK;
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



ReturnValue_t SDCardHandler::dumpSdCard() {
    // TODO: implement. This dumps the file structure of the SD card and will
    // be one of the most important functionalities for operators.
    return HasReturnvaluesIF::RETURN_OK;
}


void SDCardHandler::sendCompletionReply(bool success, ReturnValue_t errorCode) {
    CommandMessage reply;
    if(success) {
        FileSystemMessage::setSuccessReply(&reply);
    }
    else {
        FileSystemMessage::setFailureReply(&reply, errorCode);
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


ReturnValue_t SDCardHandler::appendToFile(const char* repositoryPath,
        const char* filename, const uint8_t* data, size_t size,
        uint16_t packetNumber, void* args) {
    int result = changeDirectory(repositoryPath);
    if(result != HasReturnvaluesIF::RETURN_OK){
        return result;
    }

    // todo: check whether packet number is a sequence.
    if(packetNumber == 0){
    	// todo: what if we miss packet number 0?
    	lastPacketNumber = 0;
    }
    else if((packetNumber == 1) and (lastPacketNumber != 0)) {
    	sif::debug << "SDCardHandler::appendToFile: First sequence "
    			<< "packet missed!" << std::endl;
    	return HasReturnvaluesIF::RETURN_FAILED;
    }
    else if((packetNumber - lastPacketNumber) > 1) {
    	sif::debug << "SDCardHandler::appendToFile: Packet missing between "
    			<< packetNumber << " and " << lastPacketNumber << std::endl;
    	return HasReturnvaluesIF::RETURN_FAILED;

    }
    else {
    	lastPacketNumber = packetNumber;
    }


    if(extendedDebugOutput) {
        sif::debug << "SDCardHandler::appendToFile: Packet with packet number: "
                << packetNumber << " received" << std::endl;
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
                sif::error << "SDCardHandler::appendToFile: f_seek failed with "
                        << "error code" << result << "!" << std::endl;
            }
        }
    }
    else {
        file = f_open(filename, "a");
    }

    /* File does not exist */
    result = f_getlasterror();
    if(result != F_NO_ERROR){
        if(result == F_ERR_NOTFOUND) {
            // File to append to does not exist
            // TODO: error codes
            sif::error << "SDCardHandler::appendToFile: File to append to "
                    << "does not exist, error code" << result
                    << std::endl;
        }
        else if(result == F_ERR_LOCKED) {
            // TODO: error codes
            sif::error << "SDCardHandler::appendToFile: File to append to is "
                    << "locked, error code" << result << std::endl;
        }
        else {
            sif::error << "SDCardHandler::appendToFile: Opening file failed "
                    << "with error code" << result << std::endl;
        }

        return HasReturnvaluesIF::RETURN_FAILED;
    }

    uint8_t sizeOfItems = sizeof(uint8_t);
    long numberOfItemsWritten = f_write(data, sizeOfItems, size, file);
    /* if bytes written doesn't equal bytes to write, get the error */
    if (numberOfItemsWritten != (long) size) {
        sif::error << "SDCardHandler::writeToFile: Not all bytes written,"
                << " f_write error code " << f_getlasterror() << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    /* Close file */
    result = f_close(file);

    if (result != F_NO_ERROR){
        sif::error << "SDCardHandler::writeToFile: Closing failed, f_close "
                << "error code: " << result << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCardHandler::deleteFile(const char* repositoryPath,
        const char* filename, void* args) {
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
        void* args) {
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
        const char* dirname){
    int result = create_directory(repositoryPath, dirname);
    if(result == F_ERR_DUPLICATED) {
        return HasFileSystemIF::DIRECTORY_ALREADY_EXISTS;
    }
    else if(result != F_NO_ERROR) {
        return result;
    }

    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t SDCardHandler::deleteDirectory(const char* repositoryPath,
        const char* dirname){
    int result = delete_directory(repositoryPath, dirname);
    if(result == F_ERR_NOTEMPTY) {
        return HasFileSystemIF::DIRECTORY_NOT_EMPTY;
    }
    else if(result != F_NO_ERROR) {
        // should not happen (directory read only)
        return result;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

MessageQueueId_t SDCardHandler::getCommandQueue() const{
    return commandQueue->getId();
}


ReturnValue_t SDCardHandler::printSdCard() {
    F_FIND findResult;
    int fileFound = 0;
    uint8_t recursionDepth = 0;
    f_chdir("/");
    // find directories first
    fileFound = f_findfirst("*.*", &findResult);
    if(fileFound != F_NO_ERROR) {
        // might be empty.
        sif::info << "SD Card empty." << std::endl;
        return HasReturnvaluesIF::RETURN_OK;
    }

    sif::info << "Printing SD Card: " << std::endl;
    sif::info << "F = File, D = Directory, - = Subdir Depth" << std::endl;

    for(int idx = 0; idx < 255; idx++) {
        if(idx > 0) {
            fileFound = f_findnext(&findResult);
        }

        if(fileFound != F_NO_ERROR) {
            break;
        }

        // check whether file object is directory or file.
        if(change_directory(findResult.filename, false) == F_NO_ERROR) {
            sif::info << "D: " << findResult.filename << std::endl;
            printHelper(recursionDepth + 1);
            change_directory("..", false);
        }
        else {
        	// Normally files should have a three letter extension, but
        	// we always check whether there is a file without extension
            sif::info << "F: " << findResult.filename << std::endl;
        }

    }

    return HasReturnvaluesIF::RETURN_OK;
}

void SDCardHandler::setTaskIF(PeriodicTaskIF *executingTask) {
	this->executingTask = executingTask;
}

ReturnValue_t SDCardHandler::printHelper(uint8_t recursionDepth) {
    F_FIND findResult;
    int fileFound = f_findfirst("*.*", &findResult);
    if(fileFound != F_NO_ERROR) {
        return HasReturnvaluesIF::RETURN_OK;
    }

    if(findResult.filename[0] == '.') {
        // we are not in root, so the next search result is going
        // to be the parent folder, and the third result is going to be
        // the first file or directory.
        f_findnext(&findResult);
        fileFound = f_findnext(&findResult);
    }

    for(uint8_t idx = 0; idx < 255; idx ++) {
        if(idx > 0) {
            fileFound = f_findnext(&findResult);
        }

        if(fileFound != F_NO_ERROR) {
            return HasReturnvaluesIF::RETURN_OK;
        }


        if(change_directory(findResult.filename, false) == F_NO_ERROR) {
            for(uint8_t j = 0; j < recursionDepth; j++) {
                sif::info << "-";
            }
            sif::info << "D: " << findResult.filename << std::endl;
            printHelper(recursionDepth + 1);
            change_directory("..", false);
        }
        else {
            for(uint8_t j = 0; j < recursionDepth; j++) {
                sif::info << "-";
            }
            sif::info << "F: " << findResult.filename << std::endl;
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
}
