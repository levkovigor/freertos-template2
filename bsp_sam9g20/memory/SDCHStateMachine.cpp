#include "SDCHStateMachine.h"
#include "sdcardHandlerDefinitions.h"
#include "SDCardHandler.h"
#include "SDCardAccess.h"
#include "SDCAccessManager.h"

#include <mission/memory/FileSystemMessage.h>
#include <bsp_sam9g20/common/SDCardApi.h>
#include <bsp_sam9g20/memory/HCCFileGuard.h>

#include <fsfw/timemanager/Countdown.h>


SDCHStateMachine::SDCHStateMachine(SDCardHandler* owner, Countdown* ownerCountdown):
        owner(owner), ownerCountdown(ownerCountdown) {
}

ReturnValue_t SDCHStateMachine::continueCurrentOperation() {
    switch(internalState) {
    case(this->States::IDLE): {
        break;
    }
    case(this->States::SPLITTING_FILE): {
        break;
    }
    case(this->States::COPY_FILE): {
        return handleGenericCopyOperation();
    }
    case(this->States::MOVE_FILE): {
        break;
    }
    default: {
        break;
    }
    }

    return HasReturnvaluesIF::RETURN_OK;
}

void SDCHStateMachine::resetAndSetToIdle() {
    internalState = this->States::IDLE;
    this->reset();
}

bool SDCHStateMachine::setCopyFileOperation(RepositoryPath &sourceRepo, FileName &sourceName,
        RepositoryPath &targetRepo, FileName &targetName, MessageQueueId_t recipient) {
    this->path1 = sourceRepo;
    this->fileName1 = sourceName;
    this->path2 = targetRepo;
    this->fileName2 = targetName;
    this->currentRecipient = recipient;
    if (internalState != this->States::IDLE) {
        return false;
    }

    currentByteIdx = 0;
    stepCounter = 0;
    internalState = this->States::COPY_FILE;
    return true;
}

void SDCHStateMachine::reset() {
    path1 = "";
    fileName1 = "";
    path2 = "";
    fileName2 = "";
    currentByteIdx = 0;
    currentFileSize = 0;
    stepCounter = 0;
    currentRecipient = MessageQueueIF::NO_QUEUE;
}

SDCHStateMachine::States SDCHStateMachine::getInternalState() const {
    return internalState;
}

ReturnValue_t SDCHStateMachine::prepareCopyFileInformation(F_FILE** filePtr) {
    if(filePtr == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    int result = 0;
    result = change_directory(path1.c_str(), true);
    if(result != F_NO_ERROR) {
        /* Changing directory failed! Repository might not exist, so we should just cancel
        the algorithm */
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    /* Current file size only needs to be cached once.
    Info output should only be printed once. */
    if(stepCounter == 0) {
        long readFileLength = f_filelength(fileName1.c_str());
        if(readFileLength < 0) {
            /* File might not exist so we should just cancel the algorithm */
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        currentFileSize = readFileLength;
    }

    *filePtr = f_open(fileName1.c_str(), "r");
    if(f_getlasterror() != F_NO_ERROR) {
        /* Opening file failed! */
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    /* Seek correct position in file. This needs to be done every time the file is reopened! */
    result = f_seek(*filePtr, currentByteIdx, F_SEEK_SET);
    if(result != F_NO_ERROR) {
        /* should not happen! */
        f_close(*filePtr);
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SDCHStateMachine::handleGenericCopyOperation() {
    F_FILE* readFile;
    ReturnValue_t result = prepareCopyFileInformation(&readFile);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    int retval = change_directory(path2.c_str(), true);
    if(retval != F_NO_ERROR) {
        /* Target repository might not exist */
        owner->sendCompletionMessage(false, currentRecipient,
                HasFileSystemIF::DIRECTORY_DOES_NOT_EXIST, 0);
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    F_FILE* writeFile = nullptr;
    if(stepCounter == 0) {
        /* Check whether file already exists */
        writeFile = f_open(fileName2.c_str(), "r");
        if(writeFile == nullptr) {
            /* File does not exist, we create it */
            writeFile = f_open(fileName2.c_str(), "a");
        }
        else {
            /* File does exist, so we reopen it in write mode, which truncates it to 0 length */
            f_close(writeFile);
            writeFile = f_open(fileName2.c_str(), "w");
        }
#if OBSW_VERBOSE_LEVEL >= 1
        sif::printInfo("Copying file %s/%s to %s/%s\n", path1.c_str(), fileName1.c_str(),
                path2.c_str(), fileName2.c_str());
#endif
    }
    else {
        /* Higher step counter so target file should already exist, open with append */
        writeFile = f_open(fileName2.c_str(), "a");
    }

    HCCFileGuard readHelper(&readFile);
    HCCFileGuard writeHelper(&writeFile);

    if(writeFile == nullptr) {
        /* Something went wrong */
        owner->sendCompletionMessage(false, currentRecipient,
                HasFileSystemIF::GENERIC_FILE_ERROR, f_getlasterror());
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    while(currentByteIdx < currentFileSize) {
        size_t sizeToCopy = 0;
        if(currentFileSize - currentByteIdx > fileBuffer.size()) {
            sizeToCopy = fileBuffer.size();
        }
        else {
            sizeToCopy = currentFileSize - currentByteIdx;
        }
        long sizeReadOrWritten = f_read(fileBuffer.data(), sizeof(uint8_t), sizeToCopy, readFile);
        if(sizeReadOrWritten != static_cast<long>(sizeToCopy)) {
            /* Read error */
            owner->sendCompletionMessage(false, currentRecipient,
                    HasFileSystemIF::GENERIC_FILE_ERROR, f_getlasterror());
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        sizeReadOrWritten = f_write(fileBuffer.data(), sizeof(uint8_t), sizeToCopy, writeFile);
        if(sizeReadOrWritten != static_cast<long>(sizeToCopy)) {
            /* Write error */
            owner->sendCompletionMessage(false, currentRecipient,
                    HasFileSystemIF::GENERIC_FILE_ERROR, f_getlasterror());
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        currentByteIdx += sizeToCopy;

        if(ownerCountdown->hasTimedOut()) {
            return sdchandler::TASK_PERIOD_OVER_SOON;
        }
    }

#if OBSW_VERBOSE_LEVEL >= 1
    sif::printInfo("Copy operation completed\n");
#endif
    this->resetAndSetToIdle();
    owner->sendCompletionMessage(true, currentRecipient);
    return sdchandler::OPERATION_FINISHED;
}
