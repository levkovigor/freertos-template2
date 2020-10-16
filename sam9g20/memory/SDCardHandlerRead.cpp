#include "SDCardHandler.h"
#include "SDCardHandlerPackets.h"
#include "FileSystemMessage.h"

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
#if OBSW_REDUCED_PRINTOUT == 0
        sif::debug << "SDCardHandler::appendToFile: First sequence "
                << "packet missed!" << std::endl;
#endif
        triggerEvent(SEQUENCE_PACKET_MISSING_READ_EVENT, 0, 0);
        return SEQUENCE_PACKET_MISSING_READ;
    }
    else if((sequenceNumber - lastPacketReadNumber) > 1) {
#if OBSW_REDUCED_PRINTOUT == 0
        sif::debug << "SDCardHandler::appendToFile: Packet missing between "
                << sequenceNumber << " and " << lastPacketReadNumber
                << std::endl;
#endif
        triggerEvent(SEQUENCE_PACKET_MISSING_READ_EVENT,
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
    if (result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "SDCardHandler::handleReadReply: Reading from file "
                << command.getFilename() << " failed" << std::endl;
        sendCompletionReply(false, result);
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    // Get space in IPC store to serialize packet.
    uint8_t* writePtr = nullptr;
    store_address_t storeId;
    result = IPCStore->getFreeElement(&storeId,
            replyPacket.getSerializedSize(), &writePtr);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        int retval = f_close(file);
        if(retval != F_NO_ERROR) {
            sif::error << "SDCardHandler::handleReadCommand: Closing file"
                    << " failed!" << std::endl;
        }
        return result;
    }
    size_t serializedSize = 0;
    result = replyPacket.serialize(&writePtr, &serializedSize,
            sizeToRead,SerializeIF::Endianness::BIG);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        int retval = f_close(file);
        if(retval != F_NO_ERROR) {
            sif::error << "SDCardHandler::handleReadCommand: Closing file"
                    << " failed!" << std::endl;
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
                sif::debug << "SDCardHandler::sendDataReply: Could not send "
                        << "data reply, queue of receiver is full!"
                        << std::endl;
            }
        }
    }

    int retval = f_close(file);
    if(retval != F_NO_ERROR) {
        sif::error << "SDCardHandler::handleReadCommand: Closing file"
                << " failed!" << std::endl;
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
        sif::error << "SDCardHandler::readFile: Opening file failed with code "
                 << f_getlasterror() << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    *fileSize = f_filelength(filename);

    // Set correct size to read and read position
    if(readPosition > *fileSize) {
        // Configuration error.
        *sizeToRead = 0;
        sif::warning << "SDCardHandler::openFileForReading: Specified read"
                << " position larger than file size!" << std::endl;
        return HasReturnvaluesIF::RETURN_OK;
    }
    // This also covers the case *fileSize == readPosition
    else if(*fileSize - readPosition < MAX_READ_LENGTH) {
        result = f_seek(*file, readPosition, F_SEEK_SET);
        *sizeToRead = *fileSize - readPosition;
        if(result != F_NO_ERROR) {
            sif::error << "SDCardHandler::openFileForReading: Seeking read"
                    << " position failed with code" << result << std::endl;
        }
    }
    else {
        result = f_seek(*file, readPosition, F_SEEK_SET);
        *sizeToRead = MAX_READ_LENGTH;
        if(result != F_NO_ERROR) {
            sif::error << "SDCardHandler::openFileForReading: Seeking read"
                    << " position failed with code" << result << std::endl;
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
}



