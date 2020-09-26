#include "SDCardHandler.h"
#include "SDCardHandlerPackets.h"

#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/ipc/CommandMessage.h>
#include <fsfw/memory/FileSystemMessage.h>
#include <sam9g20/memory/SDCardAccess.h>

SDCardHandler::SDCardHandler(object_id_t objectId):SystemObject(objectId) {
    commandQueue = QueueFactory::instance()->createMessageQueue(queueDepth);
    IPCStore = objectManager->get<StorageManagerIF>(objects::IPC_STORE);
}


SDCardHandler::~SDCardHandler(){
    QueueFactory::instance()->deleteMessageQueue(commandQueue);
}

ReturnValue_t SDCardHandler::performOperation(uint8_t operationCode){
	CommandMessage message;

	// Check for first message
	ReturnValue_t result = commandQueue->receiveMessage(&message);
	if(result == MessageQueueIF::EMPTY) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	else if(result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

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
    	return preferredVolume;
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
    	if(preferredVolume == SD_CARD_0) {
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
	}
	return status;
}


ReturnValue_t SDCardHandler::handleMessage(CommandMessage* message) {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;

    switch(message->getCommand()) {
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
    case FileSystemMessage::WRITE: {
        result = handleWriteCommand(message);
        if(result != HasReturnvaluesIF::RETURN_OK){
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        break;
    }
    case FileSystemMessage::READ: {
        result = handleReadCommand(message);
        if(result != HasReturnvaluesIF::RETURN_OK){
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        break;
    }
    default: {
        sif::error << "SDCardHandler::handleMessages: "
                << "Invalid filesystem command" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    }

    return HasReturnvaluesIF::RETURN_OK;
}

MessageQueueId_t SDCardHandler::getCommandQueue() const{
    return commandQueue->getId();
}



ReturnValue_t SDCardHandler::writeToFile(const char* repositoryPath,
        const char* filename, const uint8_t* data, size_t size,
        uint16_t packetNumber){
    int result;

    result = changeDirectory(repositoryPath);
    if(result != HasReturnvaluesIF::RETURN_OK){
        return result;
    }

    sif::debug << "SDCardHandler: Packet to write with packet number: "
            << packetNumber << " received" << std::endl;

    /**
     *  Try to open file
     *  If file doesn't exist a new file is created
     *  For the first packet the file should be opened in write mode
     *  Subsequent packets are appended at the end of the file. Therefore file
     *  is opened in append mode
     */
    if(packetNumber == 0){
        file = f_open(filename, "w");
    }
    else{
        file = f_open(filename, "a");
    }

    /* Write to file if opening was successful or file has already been opened */
    if(f_getlasterror() == F_NO_ERROR){
        uint8_t sizeOfItems = sizeof(uint8_t);
        long numberOfItemsWritten = f_write(data, sizeOfItems, size, file);
        /* if bytes written doesn't equal bytes to write, get the error */
        if (numberOfItemsWritten != (long) size) {
            sif::error << "f_write pb: " << f_getlasterror() << std::endl;
            return HasReturnvaluesIF::RETURN_FAILED;
        }

        /* only after flushing can data be considered safe */
        f_flush(file);
        if(f_getlasterror() != F_NO_ERROR){
            sif::error << "f_flush pb: " << f_getlasterror() << std::endl;
            return HasReturnvaluesIF::RETURN_FAILED;
        }

        result = f_close(file);
        if (result != F_NO_ERROR){
            sif::error << "f_close pb: " << result << std::endl;
            return HasReturnvaluesIF::RETURN_FAILED;
        }
    }
    else{
        sif::error << "f_open pb: " << f_getlasterror() << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}


// TODO: Move low level stuff to C API so bootloader or other files can use
// them directly.
ReturnValue_t SDCardHandler::deleteFile(const char* repositoryPath,
        const char* filename){
    int result;
    result = changeDirectory(repositoryPath);
    if(result != HasReturnvaluesIF::RETURN_OK){
        return result;
    }
    result = f_delete(filename);
    if(result != F_NO_ERROR){
        sif::error << "f_delete pb: " << result  << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t SDCardHandler::createFile(const char* dirname,
        const char* filename, const uint8_t* data, size_t size){
    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t SDCardHandler::createDirectory(const char* repositoryPath,
        const char* dirname){
    int result;
    result = changeDirectory(repositoryPath);
    if(result != HasReturnvaluesIF::RETURN_OK){
        return result;
    }
    result = f_mkdir(dirname);
    if(result == F_ERR_DUPLICATED) {
        // folder already exists
        return FOLDER_ALREADY_EXISTS;
    }
    else if(result != F_NO_ERROR) {
        sif::error << "SDCardHandler::createDirectory: f_mkdir failed with "
                << "code" << result;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t SDCardHandler::deleteDirectory(const char* repositoryPath,
        const char* dirname){
    int result;
    result = changeDirectory(repositoryPath);
    if(result != HasReturnvaluesIF::RETURN_OK){
        return result;
    }
    result = f_rmdir(dirname);
    if(result != F_NO_ERROR){
        sif::error << "f_rmdir pb: " << result  << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
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


ReturnValue_t SDCardHandler::sendDataReply(MessageQueueId_t receivedFromQueueId,
        uint8_t* tmData, size_t tmDataLen){
    store_address_t parameterAddress;
    ReturnValue_t result = IPCStore->addData(&parameterAddress, tmData, tmDataLen);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sif::error << "Failed to store data in IPC store" << std::endl;
        result = IPCStore->deleteData(parameterAddress);
        if(result != HasReturnvaluesIF::RETURN_OK){
            sif::error << "Failed to delete IPC store entry" << std::endl;
        }
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    CommandMessage reply;
    FileSystemMessage::setReadReply(&reply, parameterAddress);
    result = commandQueue->sendMessage(receivedFromQueueId, &reply);
    if(result != HasReturnvaluesIF::RETURN_OK){
        if(result == MessageQueueIF::FULL){
            sif::info << "SD card handler fails to send data reply. "
                    "Queue of receiver is  full" << std::endl;
        }
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t SDCardHandler::handleDeleteFileCommand(CommandMessage* message){
    //MessageQueueId_t receivedFromQueueId = message->getSender();
    store_address_t storeId = FileSystemMessage::getStoreId(message);
    size_t ipcStoreBufferSize = 0;
    const uint8_t* ipcStoreBuffer = nullptr;
    ReturnValue_t result = IPCStore->getData(storeId, &ipcStoreBuffer,
            &ipcStoreBufferSize);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::info << "SDCardHandler::handleDeleteFileCommand: "
               << "Invalid IPC storage ID" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    DeleteFileCommand command;
    /* Extract the repository path and the filename from the
        application data field */
    result = command.deSerialize(&ipcStoreBuffer, ipcStoreBufferSize);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sendCompletionReply(false, result);
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    result = deleteFile(command.getRepositoryPath(), command.getFilename());
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "SDCardHandler::handleDeleteFileCommand: Deleting file "
                << command.getFilename() << " failed" << std::endl;
        sendCompletionReply(false, result);
    }
    else {
        sendCompletionReply();
    }

    if (IPCStore->deleteData(storeId) != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "SDCardHandler::handleDeleteFileCommand: "
                << "Failed to delete data in IPC store" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t SDCardHandler::handleCreateDirectoryCommand(
        CommandMessage* message){
    //MessageQueueId_t receivedFromQueueId = message->getSender();
    store_address_t storeId = FileSystemMessage::getStoreId(message);

    auto returnPair = IPCStore->getData(storeId);
    if (returnPair.first != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "SDCardHandler::handleCreateDirectoryCommand: "
                << "Invalid IPC storage ID" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    const uint8_t* ipcStoreBuffer = returnPair.second.data();
    size_t ipcStoreBufferSize = returnPair.second.size();
    CreateDirectoryCommand command;
    // Extract the repository path and the directory name
    // from the application data field
    ReturnValue_t result = command.deSerialize(&ipcStoreBuffer,
            ipcStoreBufferSize);
    if(result != HasReturnvaluesIF::RETURN_OK){
        sendCompletionReply(false, result);
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    result = createDirectory(command.getRepositoryPath(),
            command.getDirname());
    if (result != HasReturnvaluesIF::RETURN_OK) {
    	// If the folder already exists, count that as  success..
        if(result != FOLDER_ALREADY_EXISTS) {
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
    //MessageQueueId_t receivedFromQueueId = message->getSender();
    store_address_t storeId = FileSystemMessage::getStoreId(message);
    size_t ipcStoreBufferSize = 0;
    const uint8_t* ipcStoreBuffer = NULL;
    ReturnValue_t result = IPCStore->getData(storeId, &ipcStoreBuffer,
            &ipcStoreBufferSize);
    if (result == HasReturnvaluesIF::RETURN_OK) {
        DeleteDirectoryCommand command;
        /* Extract the repository path and the directory name from the
        application data field */
        result = command.deSerialize(&ipcStoreBuffer, ipcStoreBufferSize);
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
        if (IPCStore->deleteData(storeId) != HasReturnvaluesIF::RETURN_OK) {
            sif::error << "Failed to delete data in IPC store" << std::endl;
            return HasReturnvaluesIF::RETURN_FAILED;
        }
    } else {
        sif::info << "Invalid IPC storage ID" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t SDCardHandler::handleWriteCommand(CommandMessage* message){
    //MessageQueueId_t receivedFromQueueId = message->getSender();
    store_address_t storeId = FileSystemMessage::getStoreId(message);
    size_t ipcStoreBufferSize = 0;
    const uint8_t* ipcStoreBuffer = NULL;
    ReturnValue_t result = IPCStore->getData(storeId, &ipcStoreBuffer,
            &ipcStoreBufferSize);
    if(result == HasReturnvaluesIF::RETURN_OK){
        WriteCommand command;
        command.deSerialize(ipcStoreBuffer, ipcStoreBufferSize);
        result = writeToFile(command.getRepositoryPath(), command.getFilename(),
                command.getFileData(), command.getFileSize(),
                command.getPacketNumber());
        if(result != HasReturnvaluesIF::RETURN_OK){
            sif::info << "Writing to file " << command.getFilename()
                    << " failed" << std::endl;
            sendCompletionReply(false, result);
        }
        else{
            sendCompletionReply();
        }
        if(IPCStore->deleteData(storeId) != HasReturnvaluesIF::RETURN_OK){
            sif::error << "Failed to delete data in IPC store" << std::endl;
            return HasReturnvaluesIF::RETURN_FAILED;
        }
    }
    else{
        sif::info << "Invalid IPC storage ID" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t SDCardHandler::handleReadCommand(CommandMessage* message) {
    MessageQueueId_t receivedFromQueueId = message -> getSender();
    store_address_t storeId = FileSystemMessage::getStoreId(message);
    size_t ipcStoreBufferSize = 0;
    const uint8_t* ipcStoreBuffer = NULL;
    ReturnValue_t result = IPCStore->getData(storeId, &ipcStoreBuffer,
            &ipcStoreBufferSize);
    if (result == HasReturnvaluesIF::RETURN_OK) {
        ReadCommand command;
        result = command.deSerialize(&ipcStoreBuffer, ipcStoreBufferSize);
        if (result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }

        uint8_t tmData[readReplyMaxLen];
        size_t tmDataLen = 0;
        result = read(command.getRepositoryPath(), command.getFilename(),
                tmData, &tmDataLen);
        if (result != HasReturnvaluesIF::RETURN_OK) {
            sif::error << "Reading from file " << command.getFilename()
                    << " failed" << std::endl;
            sendCompletionReply(false, result);
        } else {
            sendDataReply(receivedFromQueueId, tmData, tmDataLen);
        }
        if (IPCStore->deleteData(storeId) != HasReturnvaluesIF::RETURN_OK) {
            sif::error << "Failed to delete data in IPC store" << std::endl;
            return HasReturnvaluesIF::RETURN_FAILED;
        }
    } else {
        sif::info << "Invalid IPC storage ID" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t SDCardHandler::read(const char* repositoryPath,
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
    int result = change_directory(repositoryPath);
    if(result == F_NO_ERROR) {
        return HasReturnvaluesIF::RETURN_OK;
    }
    else {
        return result;
    }
}

