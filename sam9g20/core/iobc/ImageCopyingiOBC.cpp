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
        return copySdCardImageToNorFlash();
    }
    }
    return HasReturnvaluesIF::RETURN_OK;
}



ReturnValue_t ImageCopyingEngine::copySdCardImageToNorFlash() {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    if(internalState == GenericInternalState::IDLE) {
        internalState = GenericInternalState::STEP_1;
    }

    if(internalState == GenericInternalState::STEP_1) {
        result = handleNorflashErasure(bootloader);
        if(result == SoftwareImageHandler::TASK_PERIOD_OVER_SOON) {
            return result;
        }
        else if(result != HasReturnvaluesIF::RETURN_OK) {
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

ReturnValue_t ImageCopyingEngine::handleNorflashErasure() {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    if(bootloader) {
        sif::info << "ImageCopyingEngine::handleNorflashErasure: Deleting old"
                << " bootloader!" << std::endl;
        while(stepCounter < RESERVED_NOR_FLASH_SECTORS) {
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
    else {
        result = handleObswErasure();
    }

    return result;
}

ReturnValue_t ImageCopyingEngine::handleObswErasure() {
    sif::info << "ImageCopyingEngine::handleNorflashErasure: Deleting old"
            << " binary!" << std::endl;
    if(not helperFlag1) {
        SDCardAccess sdCardAccess;
        int result = change_directory(config::SW_REPOSITORY, true);
        if(result != F_NO_ERROR) {
            // The hardcoded repository does not exist. Exit!
            sif::error << "ImageCopyingHelper::handleErasingForObsw: Software"
                    << " repository does not exist. Cancelling erase operation"
                    << "!" << std::endl;
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        if(imageSlot == ImageSlot::IMAGE_0) {
            currentFileSize = f_filelength(config::SW_SLOT_0_NAME);
        }
        else if(imageSlot == ImageSlot::IMAGE_1) {
            currentFileSize = f_filelength(config::SW_SLOT_1_NAME);
        }
        else {
            currentFileSize = f_filelength(config::SW_UPDATE_SLOT_NAME);
        }
        helperFlag1 = true;
        helperCounter1 = 0;
        uint8_t requiredBlocks = 0;
        if(currentFileSize <=  NORFLASH_TOTAL_SMALL_SECTOR_MEM) {
            requiredBlocks = std::ceil(
                    static_cast<float>(currentFileSize) /
                    (NORFLASH_SMALL_SECTOR_SIZE));
        }
        else {
            requiredBlocks = NORFLASH_SMALL_SECTORS_NUMBER;
            requiredBlocks += std::ceil(
                    static_cast<float>(currentFileSize -
                            NORFLASH_TOTAL_SMALL_SECTOR_MEM) /
                    (NORFLASH_LARGE_SECTOR_SIZE));
        }
        helperCounter1 = requiredBlocks;

    }
    while(stepCounter < helperCounter1) {
        int result = NORFLASH_EraseSector(&NORFlash,
                getBaseAddress(stepCounter, nullptr));
        if(result != 0) {
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        stepCounter++;
        if(countdown->hasTimedOut()) {
            return SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
        }
    }
    if(stepCounter == helperCounter1) {
        // Reset counter and helper member.
        stepCounter = 0;
        helperFlag1 = false;
        helperCounter1 = 0;
    }
    return HasReturnvaluesIF::RETURN_OK;
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
        if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t ImageCopyingEngine::performNorCopyOperation(F_FILE** binaryFile) {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;

    // we will always copy in small buckets (8192 bytes)
    size_t sizeToRead = NORFLASH_SMALL_SECTOR_SIZE;
    if(currentFileSize - currentByteIdx < NORFLASH_SMALL_SECTOR_SIZE) {
        sizeToRead = currentFileSize - currentByteIdx;
    }

    // If data has been read but still needs to be copied, don't read.
    if(not helperFlag1) {
        size_t bytesRead = 0;
        // read length of NOR-Flash small section
        result = readFile(imgBuffer->data(), sizeToRead, &bytesRead,
                binaryFile);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }
        if(bytesRead < sizeToRead) {
            // should not happen..
            sif::error << "SoftwareImageHandler::performNorCopyOperation:"
                    << " Bytes read smaller than size to read!" << std::endl;
            return HasReturnvaluesIF::RETURN_FAILED;
        }
    }

    helperFlag1 = true;
    // we should consider a critical section here and extracting this
    // function to a special task with the highest priority so it can not
    // be interrupted.
    size_t offset = 0;
    size_t baseAddress = getBaseAddress(stepCounter, &offset);
    result = NORFLASH_WriteData(&NORFlash,  baseAddress + offset,
            imgBuffer->data(), sizeToRead);
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
            std::array<uint8_t, 7 * 4> armVectors;
            uint32_t currentArmVector = 0;
            NORFLASH_ReadData(&NORFlash, NORFLASH_SA0_ADDRESS,
                    armVectors.data(), armVectors.size());
            sif::debug << std::hex << std::setfill('0') << std::setw(8);
            sif::debug << "Written ARM vectors: " << std::endl;
            std::memcpy(&currentArmVector, armVectors.data(), 4);
            sif::debug << "1: 0x" << currentArmVector << std::endl;
            std::memcpy(&currentArmVector, armVectors.data() + 4, 4);
            sif::debug << "2: 0x" << currentArmVector << std::endl;
            std::memcpy(&currentArmVector, armVectors.data() + 8, 4);
            sif::debug << "3: 0x" << currentArmVector << std::endl;
            std::memcpy(&currentArmVector, armVectors.data() + 12, 4);
            sif::debug << "4: 0x" << currentArmVector << std::endl;
            std::memcpy(&currentArmVector, armVectors.data() + 16, 4);
            sif::debug << "5: 0x" << currentArmVector << std::endl;
            std::memcpy(&currentArmVector, armVectors.data() + 20, 4);
            sif::debug << "6: 0x" << currentArmVector << std::endl;
            std::memcpy(&currentArmVector, armVectors.data() + 24, 4);
            sif::debug << "7: 0x" << currentArmVector << std::endl;
            sif::debug << std::dec << std::setfill(' ');
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
                *offset = currentByteIdx % NORFLASH_SMALL_SECTOR_SIZE;
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
        if(internalState == GenericInternalState::STEP_1) {
            switch(stepCounter) {
            case(0): return NORFLASH_SA5_ADDRESS;
            case(1): return NORFLASH_SA6_ADDRESS;
            case(2): return NORFLASH_SA7_ADDRESS;
            case(3): return NORFLASH_SA8_ADDRESS;
            case(4): return NORFLASH_SA9_ADDRESS;
            case(5): return NORFLASH_SA10_ADDRESS;
            case(6): return NORFLASH_SA11_ADDRESS;
            case(7): return NORFLASH_SA12_ADDRESS;
            case(8): return NORFLASH_SA13_ADDRESS;
            case(9): return NORFLASH_SA14_ADDRESS;
            case(10): return NORFLASH_SA15_ADDRESS;
            case(11): return NORFLASH_SA16_ADDRESS;
            case(12): return NORFLASH_SA17_ADDRESS;
            case(13): return NORFLASH_SA18_ADDRESS;
            case(14): return NORFLASH_SA19_ADDRESS;
            case(15): return NORFLASH_SA20_ADDRESS;
            case(16): return NORFLASH_SA21_ADDRESS;
            case(17): return NORFLASH_SA22_ADDRESS;
            default: return NORFLASH_SA22_ADDRESS;
            }
        }
        else if(internalState == GenericInternalState::STEP_2) {
            switch(stepCounter) {
            case(0): return NORFLASH_SA5_ADDRESS;
            case(1): return NORFLASH_SA6_ADDRESS;
            case(2): return NORFLASH_SA7_ADDRESS;
            case(3): return NORFLASH_SA8_ADDRESS;
            case(4): return NORFLASH_SA9_ADDRESS;
            case(12): return NORFLASH_SA10_ADDRESS;
            case(20): return NORFLASH_SA11_ADDRESS;
            case(28): return NORFLASH_SA12_ADDRESS;
            case(36): return NORFLASH_SA13_ADDRESS;
            case(44): return NORFLASH_SA14_ADDRESS;
            case(52): return NORFLASH_SA15_ADDRESS;
            case(60): return NORFLASH_SA16_ADDRESS;
            case(68): return NORFLASH_SA17_ADDRESS;
            case(76): return NORFLASH_SA18_ADDRESS;
            case(84): return NORFLASH_SA19_ADDRESS;
            case(92): return NORFLASH_SA20_ADDRESS;
            case(100): return NORFLASH_SA21_ADDRESS;
            case(108): return NORFLASH_SA22_ADDRESS;
            default:
                if(stepCounter < 108 + 8) {
                    *offset = (stepCounter - 4 % 8) * NORFLASH_SMALL_SECTOR_SIZE;
                }
                if(stepCounter <= 12) return NORFLASH_SA10_ADDRESS;
                if(stepCounter <= 20) return NORFLASH_SA11_ADDRESS;
                if(stepCounter <= 28) return NORFLASH_SA12_ADDRESS;
                if(stepCounter <= 36) return NORFLASH_SA13_ADDRESS;
                if(stepCounter <= 44) return NORFLASH_SA14_ADDRESS;
                if(stepCounter <= 52) return NORFLASH_SA15_ADDRESS;
                if(stepCounter <= 60) return NORFLASH_SA16_ADDRESS;
                if(stepCounter <= 68) return NORFLASH_SA17_ADDRESS;
                if(stepCounter <= 76) return NORFLASH_SA18_ADDRESS;
                if(stepCounter <= 84) return NORFLASH_SA19_ADDRESS;
                if(stepCounter <= 92) return NORFLASH_SA20_ADDRESS;
                if(stepCounter <= 100) return NORFLASH_SA21_ADDRESS;
                if(stepCounter <= 108) return NORFLASH_SA22_ADDRESS;
            }
        }
    }
    return 0xffffffff;
}


