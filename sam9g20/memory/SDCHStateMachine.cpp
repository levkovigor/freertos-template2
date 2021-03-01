#include "SDCHStateMachine.h"
#include "SDCardHandlerDefinitions.h"

#include <sam9g20/common/SDCardApi.h>
#include <fsfw/action/HasActionsIF.h>
#include <mission/memory/FileSystemMessage.h>
#include <sam9g20/memory/SDCAccessManager.h>


SDCHStateMachine::SDCHStateMachine(Countdown* ownerCountdown): ownerCountdown(ownerCountdown) {
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
        F_FILE* file;
        prepareCopyFileInformation(&file);

        break;
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
        RepositoryPath &targetRepo, FileName &targetName) {
    this->path1 = sourceRepo;
    this->fileName1 = sourceName;
    this->path2 = targetRepo;
    this->fileName2 = targetName;
    if (internalState != this->States::IDLE) {
        return false;
    }

    currentByteIdx = 0;
    stepCounter = 0;
    internalState = this->States::COPY_FILE;
    return true;
}

void SDCHStateMachine::reset() {
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
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}
