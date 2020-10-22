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
        break;
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
    countdown->resetTimer();

    // 1. step: Find out bootloader size

    // NOR-Flash:
    // 5 small NOR-Flash sectors will be reserved and clear
    // for now: 8192 * 4 = 40960 bytes

    // Erase the 4 small sectors first. measure how long that takes.


    if(internalState == GenericInternalState::STEP_1) {
        int retval = 0;
        ReturnValue_t result = handleNorflashErasure(bootloader);
        if(result == SoftwareImageHandler::TASK_PERIOD_OVER_SOON) {
            return result;
        }
    }

    internalState = GenericInternalState::STEP_2;
    if(countdown->hasTimedOut()) {
        return SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
    }

    SDCardAccess sdCardAccess(SD_CARD_0);
    F_FILE * binaryFile;

    ReturnValue_t result = prepareGenericFileInformation(
            sdCardAccess.currentVolumeId, &binaryFile);

    size_t sizeToRead = 2048;
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

    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t ImageCopyingEngine::handleNorflashErasure(bool bootloader) {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    if(stepCounter == 0) {
        result = NORFLASH_EraseSector(&NORFlash, NORFLASH_SA0_ADDRESS);
        if(result != 0) {
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        stepCounter++;
    }
    if(countdown->hasTimedOut()) {
        return SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
    }

    if(stepCounter == 1) {
        result = NORFLASH_EraseSector(&NORFlash, NORFLASH_SA1_ADDRESS);
        if(result != 0) {
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        stepCounter++;
    }
    if(countdown->hasTimedOut()) {
        return SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
    }

    if(stepCounter == 2) {
        result = NORFLASH_EraseSector(&NORFlash, NORFLASH_SA2_ADDRESS);
        if(result != 0) {
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        stepCounter++;
    }
    if(countdown->hasTimedOut()) {
        return SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
    }

    if(stepCounter == 3) {
        result = NORFLASH_EraseSector(&NORFlash, NORFLASH_SA3_ADDRESS);
        if(result != 0) {
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        stepCounter++;
    }
    if(countdown->hasTimedOut()) {
        return SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
    }

    if(stepCounter == 4) {
        result = NORFLASH_EraseSector(&NORFlash, NORFLASH_SA4_ADDRESS);
        if(result != 0) {
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        stepCounter++;
    }
    if(countdown->hasTimedOut()) {
        return SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
    }

    return result;
}

