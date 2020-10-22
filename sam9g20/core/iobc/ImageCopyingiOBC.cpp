#include "../ImageCopyingEngine.h"
#include <fsfw/timemanager/Countdown.h>

extern "C" {
#include <sam9g20/common/FRAMApi.h>
#include <hal/Storage/NORflash.h>
}

ReturnValue_t ImageCopyingEngine::continueCurrentOperation() {
    switch(imageHandlerState) {
    case(ImageHandlerStates::IDLE): {
        return HasReturnvaluesIF::RETURN_OK;
    }
    case(ImageHandlerStates::COPY_SDC_IMG_TO_FLASH): {
        return copySdCardImageToNorFlash();
    }
    case(ImageHandlerStates::REPLACE_SDC_IMG): {
        break;
    }
    case(ImageHandlerStates::COPY_FLASH_IMG_TO_SDC): {
        break;
    }
    case(ImageHandlerStates::COPY_FRAM_BL_TO_FLASH): {
        break;
    }
    case(ImageHandlerStates::COPY_SDC_BL_TO_FLASH): {
        break;
    }
    }
    return HasReturnvaluesIF::RETURN_OK;
}



ReturnValue_t ImageCopyingEngine::copySdCardImageToNorFlash() {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    if(internalState == GenericInternalState::STEP_1) {
        result = handleNorflashErasure(bootloader);
        if(result == SoftwareImageHandler::TASK_PERIOD_OVER_SOON) {
            return result;
        }
    }

    internalState = GenericInternalState::STEP_2;
    if(countdown->hasTimedOut()) {
        return SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
    }

    if(internalState == GenericInternalState::STEP_2) {
        result = handleSdToNorCopyOperation();

    }

    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t ImageCopyingEngine::handleNorflashErasure(bool bootloader) {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;

    if(bootloader) {
        while(stepCounter <= RESERVED_NOR_FLASH_SECTORS) {
            result = NORFLASH_EraseSector(&NORFlash,
                    getBaseAddress(stepCounter, nullptr));
            if(result != 0) {
                return HasReturnvaluesIF::RETURN_FAILED;
            }
            stepCounter++;
            if(countdown->hasTimedOut()) {
                return SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
            }
        }
        if(stepCounter == RESERVED_NOR_FLASH_SECTORS) {
            stepCounter = 0;
        }
    }

    return result;
}

uint32_t ImageCopyingEngine::getBaseAddress(uint8_t stepCounter,
        size_t* offset) {
    if(bootloader) {
        if(internalState == GenericInternalState::STEP_1) {
            switch(stepCounter) {
            case(0): return NORFLASH_SA0_ADDRESS;
            case(1): return NORFLASH_SA1_ADDRESS;
            case(2): return NORFLASH_SA2_ADDRESS;
            case(3): return NORFLASH_SA3_ADDRESS;
            case(4): return NORFLASH_SA4_ADDRESS;
            }
        }
        else if(internalState == GenericInternalState::STEP_2) {
            uint8_t baseIdx = currentByteIdx / NORFLASH_SMALL_SECTOR_SIZE;
            if(offset != nullptr) {
                *offset = currentByteIdx & NORFLASH_SMALL_SECTOR_SIZE;
            }
            switch(baseIdx) {
            case(0): return NORFLASH_SA0_ADDRESS;
            case(1): return NORFLASH_SA1_ADDRESS;
            case(2): return NORFLASH_SA2_ADDRESS;
            case(3): return NORFLASH_SA3_ADDRESS;
            case(4): return NORFLASH_SA4_ADDRESS;
            }
        }
    }
    else {
    }

    return 0xffffffff;
}

ReturnValue_t ImageCopyingEngine::handleSdToNorCopyOperation() {
    SDCardAccess sdCardAccess;
    F_FILE * binaryFile;

    ReturnValue_t result = prepareGenericFileInformation(
            sdCardAccess.currentVolumeId, &binaryFile);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    while(true) {
        result = performNorCopyOperation(&binaryFile);

    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t ImageCopyingEngine::performNorCopyOperation(F_FILE** binaryFile) {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    size_t bytesRead = 0;
    size_t sizeToRead = NORFLASH_SMALL_SECTOR_SIZE;
    if(currentFileSize - currentByteIdx < NORFLASH_SMALL_SECTOR_SIZE) {
        sizeToRead = currentFileSize - currentByteIdx;
    }

    // If data has been read but still needs to be copied, don't read.
    if(not helperFlag1) {
        // read length of NOR-Flash small section
        result = readFile(imgBuffer->data(), sizeToRead, &bytesRead,
                binaryFile);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }
    }
    helperFlag1 = true;


    // we should consider a critical section here and extracting this
    // function to a special task with the highest priority so it can not
    // be interrupted.
    size_t offset = 0;
    size_t baseAddress = getBaseAddress(stepCounter, &offset);
    result = NORFLASH_WriteData(&NORFlash,  baseAddress + offset,
            imgBuffer->data(), bytesRead);
    if(result != 0) {
        errorCount++;
        if(errorCount >= 3) {
            // if writing to NAND failed 5 times, exit.
            sif::error << "SoftwareImageHandler::copyBootloaderToNor"
                    << "Flash: Write error!" << std::endl;
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        // Maybe SD card is busy, so try in next cycle..
        return SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
    }

    // bucket write success
    currentByteIdx += sizeToRead;
    stepCounter ++;
    // Set this flag to false so that the next bucket can be read from the
    // SD card.
    helperFlag1 = false;


    if(currentByteIdx >= currentFileSize) {
        // operation finished.
#if OBSW_REDUCED_PRINTOUT == 0
        if(bootloader) {
            sif::info << "Copying bootloader to NOR-Flash finished with "
                    << stepCounter << " steps!" << std::endl;
        }
        else {
            sif::info << "Copying OBSW image to NOR-Flash finished with "
                    << stepCounter << " steps!" << std::endl;
        }
#endif

        // cache last finished state.
        lastFinishedState = imageHandlerState;
        reset();
        return SoftwareImageHandler::OPERATION_FINISHED;
    }
    if(countdown->hasTimedOut()) {
        return SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
    }
    return result;
}
