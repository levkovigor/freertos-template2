#include "SDCardHandler.h"
#include "SDCardHandlerPackets.h"
#include "FileSystemMessage.h"

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

    result = deleteFile(command.getRepositoryPathRaw(),
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
    /* Extract the repository path and the filename from the
        application data field */
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
    result = IPCStore->getFreeElement(&storeId,
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
    result = command.deSerialize(&ipcStoreBuffer, &remainingSize,
            SerializeIF::Endianness::BIG);
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

ReturnValue_t SDCardHandler::dumpSdCard() {
    // TODO: implement. This dumps the file structure of the SD card and will
    // be one of the most important functionalities for operators.
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

