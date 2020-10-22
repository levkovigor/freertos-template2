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
                    getAddressToDelete(stepCounter));
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

uint32_t ImageCopyingEngine::getAddressToDelete(uint8_t stepCounter) {
    if(bootloader) {
        switch(stepCounter) {
        case(0): return NORFLASH_SA0_ADDRESS;
        case(1): return NORFLASH_SA1_ADDRESS;
        case(2): return NORFLASH_SA2_ADDRESS;
        case(3): return NORFLASH_SA3_ADDRESS;
        case(4): return NORFLASH_SA4_ADDRESS;
        default:
            return 0xffffffff;
        }
    }
    else {
        return 0xffffffff;
    }
}

ReturnValue_t ImageCopyingEngine::handleSdToNorCopyOperation() {
    SDCardAccess sdCardAccess;
    F_FILE * binaryFile;

    ReturnValue_t result = prepareGenericFileInformation(
            sdCardAccess.currentVolumeId, &binaryFile);

    size_t sizeToRead = 2048;
    if(currentFileSize - currentByteIdx < 2048) {
        sizeToRead = currentFileSize - currentByteIdx;
    }

    while(true) {
        // read length of NOR-Flash small section
        ssize_t bytesRead = f_read(imgBuffer->data(), sizeof(uint8_t),
                sizeToRead, binaryFile);

        // we should consider a critical section here and extracting this
        // function to a special task with the highest priority so it can not
        // be interrupted.
        result = NORFLASH_WriteData(&NORFlash, NORFLASH_SA0_ADDRESS,
                imgBuffer->data(), bytesRead);
        if(result != 0) {
            return HasReturnvaluesIF::RETURN_FAILED;
        }
    }
}
