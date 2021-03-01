#include "SDCardHandler.h"
#include "SDCardHandlerPackets.h"
#include "sdcardHandlerDefinitions.h"

#include <fsfw/serviceinterface/ServiceInterface.h>
#include <mission/memory/FileSystemMessage.h>

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

    uint16_t packetSequenceIfMissing;
    result = appendToFile(command.getRepositoryPath(),
            command.getFilename(), command.getFileData(),
            command.getFileSize(), command.getPacketNumber(),
            &packetSequenceIfMissing);
    if(result != HasReturnvaluesIF::RETURN_OK){
        if(result == SEQUENCE_PACKET_MISSING_WRITE) {
            sendCompletionReply(false, result, packetSequenceIfMissing);
        }
        else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SDCardHandler::handleWriteCommand: Writing to file "
                    << command.getFilename()  << " failed." << std::endl;
#else
            sif::printError("SDCardHandler::handleWriteCommand: Writing to file %s failed.\n",
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
        uint16_t packetNumber, void* args) {
    ReturnValue_t result = changeDirectory(repositoryPath);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    uint16_t* packetSeqIfMissing = static_cast<uint16_t*>(args);
    if(packetSeqIfMissing == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "SDCardHandler::appendToFile: Args invalid!"
                << std::endl;
#else
        sif::printError("SDCardHandler::appendToFile: Args invalid!\n");
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
    result = f_getlasterror();
    if(result != F_NO_ERROR){
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
                    << "locked, error code" << result << std::endl;
#else
            sif::printError("SDCardHandler::appendToFile: File to append to is "
                    "locked, error code %d.", result);
#endif
            return FILE_LOCKED;
        }
        else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SDCardHandler::appendToFile: Opening file failed "
                    << "with error code" << result << std::endl;
#else
            sif::printError("SDCardHandler::appendToFile: Opening file failed "
                    "with error code %d.\n", result);
#endif
        }

        return HasReturnvaluesIF::RETURN_FAILED;
    }

    uint8_t sizeOfItems = sizeof(uint8_t);
    long numberOfItemsWritten = f_write(data, sizeOfItems, size, file);
    /* if bytes written doesn't equal bytes to write, get the error */
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

    /* Close file */
    result = f_close(file);

    if (result != F_NO_ERROR){
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "SDCardHandler::writeToFile: Closing failed, f_close "
                << "error code: " << result << std::endl;
#else
        sif::printError( "SDCardHandler::writeToFile: Closing failed, f_close "
                "error code: %d\n", result);
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

    // The file can be locked via the finish command optinally
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

    // Get file information for the reply packet. ctime and cdate not contained
    // for now.
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
    FinishAppendReply replyPacket(repoPath, fileName,
            lastPacketWriteNumber + 1, filesize, locked);

    uint8_t* ptr = nullptr;
    size_t serializedSize = 0;
    ReturnValue_t result = IPCStore->getFreeElement(&storeId,
            replyPacket.getSerializedSize(), &ptr);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        // Reset last packet sequence number
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
    sif::info << "Append operation on file " << fileName->c_str() << " in repository "
            << repoPath->c_str() << " finished." << std::endl;
    sif::info <<  "Filesize: " << filesize << ".";
    if(locked) {
        sif::info << " File was locked." << std::endl;
    }
    else {
        sif::info << " File was not locked." << std::endl;
    }
#else
    sif::printInfo("Append operation on file %s in repository %s finished.", fileName->c_str(),
            repoPath->c_str());
    sif::printInfo("Filesize: %lu. ", static_cast<unsigned long>(filesize));
    if(locked) {
        sif::printInfo("File was locked.\n");
    }
    else {
        sif::printInfo("File was not locked.\n");
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
    return HasReturnvaluesIF::RETURN_OK;
}
