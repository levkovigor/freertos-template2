#include "SDCardHandler.h"
#include "SDCardHandlerPackets.h"
#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/ipc/CommandMessage.h>
#include <fsfw/memory/FileSystemMessage.h>

SDCardHandler::SDCardHandler(object_id_t objectId_):SystemObject(objectId_){
    commandQueue = QueueFactory::instance()->createMessageQueue(queueDepth);
    IPCStore = objectManager->get<StorageManagerIF>(objects::IPC_STORE);
}


SDCardHandler::~SDCardHandler(){
    QueueFactory::instance()->deleteMessageQueue(commandQueue);
}


ReturnValue_t SDCardHandler::performOperation(uint8_t operationCode){
    if(fileSystemInitialized == false){
        ReturnValue_t result = initializeFileSystem();
        if(result == HasReturnvaluesIF::RETURN_OK){
            fileSystemInitialized = true;
        }
        else{
            return HasReturnvaluesIF::RETURN_FAILED;
        }
    }
    CommandMessage message;
    ReturnValue_t result = commandQueue->receiveMessage(&message);
    if(result == HasReturnvaluesIF::RETURN_OK){
        switch(message.getCommand()){
            case FileSystemMessage::DELETE_FILE: {
                result = handleDeleteFileCommand(&message);
                if(result != HasReturnvaluesIF::RETURN_OK){
                    return HasReturnvaluesIF::RETURN_FAILED;
                }
                break;
            }
            case FileSystemMessage::CREATE_DIRECTORY: {
                result = handleCreateDirectoryCommand(&message);
                if(result != HasReturnvaluesIF::RETURN_OK){
                    return HasReturnvaluesIF::RETURN_FAILED;
                }
                break;
            }
            case FileSystemMessage::DELETE_DIRECTORY: {
                result = handleDeleteDirectoryCommand(&message);
                if(result != HasReturnvaluesIF::RETURN_OK){
                    return HasReturnvaluesIF::RETURN_FAILED;
                }
                break;
            }
            case FileSystemMessage::WRITE: {
                result = handleWriteCommand(&message);
                if(result != HasReturnvaluesIF::RETURN_OK){
                    return HasReturnvaluesIF::RETURN_FAILED;
                }
                break;
            }
            case FileSystemMessage::READ: {
                result = handleReadCommand(&message);
                if(result != HasReturnvaluesIF::RETURN_OK){
                    return HasReturnvaluesIF::RETURN_FAILED;
                }
                break;
            }
            default:{
                sif::info << "Invalid filesystem command" << std::endl;
                break;
            }
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
}


MessageQueueId_t SDCardHandler::getCommandQueue() const{
    return commandQueue->getId();
}


ReturnValue_t SDCardHandler::initializeFileSystem(){
    /* Initialize the memory to be used by the filesystem */
    hcc_mem_init();

    /* Initialize the filesystem */
    int result = fs_init();
    if(result != F_NO_ERROR){
        sif::error << "fs_init pb: " << result << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    /* Register this task with filesystem */
    result = f_enterFS();
    if(result != F_NO_ERROR){
        sif::error << "f_enterFS pb: " << result << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    if(result == HasReturnvaluesIF::RETURN_OK){
        result = selectSDCard(volumeID::SD_CARD_0);
        if(result != HasReturnvaluesIF::RETURN_OK){
            sif::error << "SD Card " << volumeID::SD_CARD_0 << " not present or defect" << std::endl;
            return HasReturnvaluesIF::RETURN_FAILED;
            /* Try to access the second sd card */
            result = selectSDCard(volumeID::SD_CARD_1);
            if(result != HasReturnvaluesIF::RETURN_OK){
                sif::error << "SD Card " << volumeID::SD_CARD_1 << " not present or defect" << std::endl;
                return HasReturnvaluesIF::RETURN_FAILED;
            }
        }
    }
    else{
        sif::error << "Initialization of filesystem failed" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t SDCardHandler::selectSDCard(int volumeID){
    /* Initialize volID as safe */
    int result = f_initvolume(0, atmel_mcipdc_initfunc, volumeID);

    if((result != F_NO_ERROR) && (result != F_ERR_NOTFORMATTED)){
        sif::error << "f_initvolume pb: " << result << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    if(result == F_ERR_NOTFORMATTED){
        /**
         *  The file system has not been formatted to safeFat yet
         *  Therefore format filesystem now
         */
        result = f_format( 0, F_FAT32_MEDIA );
        if(result != F_NO_ERROR){
            sif::error << "f_format pb: " << result << std::endl;
            return HasReturnvaluesIF::RETURN_FAILED;
        }
    }

    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t SDCardHandler::writeToFile(const char* repositoryPath,
        const char* filename, const uint8_t* data, size_t size,
        uint16_t packetNumber){
    int result;

    result = changeDirectory(repositoryPath);
    if(result != HasReturnvaluesIF::RETURN_OK){
        return result;
    }

    sif::debug << "SDCardHandler: Packet to write with packet number: " << packetNumber << " received" << std::endl;

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


ReturnValue_t SDCardHandler::deleteFile(const char* repositoryPath, const char* filename){
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


ReturnValue_t SDCardHandler::createDirectory(const char* repositoryPath, const char* dirname){
    int result;
    result = changeDirectory(repositoryPath);
    if(result != HasReturnvaluesIF::RETURN_OK){
        return result;
    }
    result = f_mkdir(dirname);
    if(result != F_NO_ERROR){
        sif::error << "f_mkdir pb: " << result  << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t SDCardHandler::deleteDirectory(const char* repositoryPath, const char* dirname){
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


void SDCardHandler::sendCompletionReply(MessageQueueId_t receivedFromQueueId, Command_t completionStatus){
    CommandMessage reply;
    FileSystemMessage::setCompletionReply(&reply, completionStatus);
    ReturnValue_t result = commandQueue->sendMessage(receivedFromQueueId, &reply);
    if(result != HasReturnvaluesIF::RETURN_OK){
        if(result == MessageQueueIF::FULL){
            sif::error << "SD Card Handler fails to send reply. Queue of receiver is full" << std::endl;
        }
    }
}


ReturnValue_t SDCardHandler::sendDataReply(MessageQueueId_t receivedFromQueueId, uint8_t* tmData, uint8_t tmDataLen){
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
            sif::info << "SD card handler fails to send data reply. Queue of receiver is  full" << std::endl;
        }
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t SDCardHandler::handleDeleteFileCommand(CommandMessage* message){
    MessageQueueId_t receivedFromQueueId = message->getSender();
    store_address_t storeId = FileSystemMessage::getStoreId(message);
    size_t ipcStoreBufferSize = 0;
    const uint8_t* ipcStoreBuffer = NULL;
    ReturnValue_t result = IPCStore->getData(storeId, &ipcStoreBuffer, &ipcStoreBufferSize);
    if (result == HasReturnvaluesIF::RETURN_OK) {
        DeleteFileCommand command;
        /* Extract the repository path and the filename from the application data field */
        result = command.deSerialize(&ipcStoreBuffer, ipcStoreBufferSize);
        if(result != HasReturnvaluesIF::RETURN_OK){
            sendCompletionReply(receivedFromQueueId,
                                FileSystemMessage::COMPLETION_FAILED);
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        result = deleteFile(command.getRepositoryPath(), command.getFilename());
        if (result != HasReturnvaluesIF::RETURN_OK) {
            sif::error << "Deleting file " << command.getFilename() << " failed"
                    << std::endl;
            sendCompletionReply(receivedFromQueueId,
                    FileSystemMessage::COMPLETION_FAILED);
        } else {
            sendCompletionReply(receivedFromQueueId,
                                FileSystemMessage::COMPLETION_SUCCESS);
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


ReturnValue_t SDCardHandler::handleCreateDirectoryCommand(CommandMessage* message){
    MessageQueueId_t receivedFromQueueId = message->getSender();
    store_address_t storeId = FileSystemMessage::getStoreId(message);
    size_t ipcStoreBufferSize = 0;
    const uint8_t* ipcStoreBuffer = NULL;
    ReturnValue_t result = IPCStore->getData(storeId, &ipcStoreBuffer, &ipcStoreBufferSize);
    if (result == HasReturnvaluesIF::RETURN_OK) {
        CreateDirectoryCommand command;
        /* Extract the repository path and the directory name from the application data field */
        result = command.deSerialize(&ipcStoreBuffer, ipcStoreBufferSize);
        if(result != HasReturnvaluesIF::RETURN_OK){
            sendCompletionReply(receivedFromQueueId,
                                FileSystemMessage::COMPLETION_FAILED);
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        result = createDirectory(command.getRepositoryPath(), command.getDirname());
        if (result != HasReturnvaluesIF::RETURN_OK) {
            sif::error << "Creating directory " << command.getDirname() << " failed"
                    << std::endl;
            sendCompletionReply(receivedFromQueueId,
                    FileSystemMessage::COMPLETION_FAILED);
        } else {
            sendCompletionReply(receivedFromQueueId,
                                FileSystemMessage::COMPLETION_SUCCESS);
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


ReturnValue_t SDCardHandler::handleDeleteDirectoryCommand(CommandMessage* message){
    MessageQueueId_t receivedFromQueueId = message->getSender();
    store_address_t storeId = FileSystemMessage::getStoreId(message);
    size_t ipcStoreBufferSize = 0;
    const uint8_t* ipcStoreBuffer = NULL;
    ReturnValue_t result = IPCStore->getData(storeId, &ipcStoreBuffer, &ipcStoreBufferSize);
    if (result == HasReturnvaluesIF::RETURN_OK) {
        DeleteDirectoryCommand command;
        /* Extract the repository path and the directory name from the application data field */
        result = command.deSerialize(&ipcStoreBuffer, ipcStoreBufferSize);
        if(result != HasReturnvaluesIF::RETURN_OK){
            sendCompletionReply(receivedFromQueueId,
                                FileSystemMessage::COMPLETION_FAILED);
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        result = deleteDirectory(command.getRepositoryPath(), command.getDirname());
        if (result != HasReturnvaluesIF::RETURN_OK) {
            sif::error << "Deleting directory " << command.getDirname() << " failed"
                    << std::endl;
            sendCompletionReply(receivedFromQueueId,
                    FileSystemMessage::COMPLETION_FAILED);
        } else {
            sendCompletionReply(receivedFromQueueId,
                                FileSystemMessage::COMPLETION_SUCCESS);
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
    MessageQueueId_t receivedFromQueueId = message->getSender();
    store_address_t storeId = FileSystemMessage::getStoreId(message);
    size_t ipcStoreBufferSize = 0;
    const uint8_t* ipcStoreBuffer = NULL;
    ReturnValue_t result = IPCStore->getData(storeId, &ipcStoreBuffer, &ipcStoreBufferSize);
    if(result == HasReturnvaluesIF::RETURN_OK){
        WriteCommand command;
        command.deSerialize(ipcStoreBuffer, ipcStoreBufferSize);
        result = writeToFile(command.getRepositoryPath(), command.getFilename(),
                command.getFileData(), command.getFileSize(),
                command.getPacketNumber());
        if(result != HasReturnvaluesIF::RETURN_OK){
            sif::info << "Writing to file " << command.getFilename() << " failed" << std::endl;
            sendCompletionReply(receivedFromQueueId, FileSystemMessage::COMPLETION_FAILED);
        }
        else{
            sendCompletionReply(receivedFromQueueId, FileSystemMessage::COMPLETION_SUCCESS);
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
    ReturnValue_t result = IPCStore->getData(storeId, &ipcStoreBuffer, &ipcStoreBufferSize);
    if (result == HasReturnvaluesIF::RETURN_OK) {
        ReadCommand command;
        result = command.deSerialize(&ipcStoreBuffer, ipcStoreBufferSize);
        if (result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }

        uint8_t tmData[readReplyMaxLen];
        uint32_t tmDataLen = 0;
        result = read(command.getRepositoryPath(), command.getFilename(), tmData, &tmDataLen);
        if (result != HasReturnvaluesIF::RETURN_OK) {
            sif::error << "Reading from file " << command.getFilename() << " failed"
                    << std::endl;
            sendCompletionReply(receivedFromQueueId,
                    FileSystemMessage::COMPLETION_FAILED);
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
        const char* filename, uint8_t* tmData, uint32_t* tmDataLen) {
    int result;
    F_FILE* file;

    result = changeDirectory(repositoryPath);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    file = f_open(filename, "r");
    if (f_getlasterror() != F_NO_ERROR) {
        sif::error << "f_open pb: " << f_getlasterror() << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    long filesize = f_filelength(filename);
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


ReturnValue_t SDCardHandler::changeDirectory(const char* repositoryPath){
    int result;
    /* Because all paths are relative to the root directory, go to root directory here */
    result = f_chdir("/");
    if(result != F_NO_ERROR){
        sif::error << "f_chdir pb: " << result << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    /* Change to the location of the directory to delete */
    if(*repositoryPath != '\0'){
        result = f_chdir(repositoryPath);
        if(result != F_NO_ERROR){
            sif::error << "f_chdir pb: " << result << std::endl;
            return HasReturnvaluesIF::RETURN_FAILED;
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
}
