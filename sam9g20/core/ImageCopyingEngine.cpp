#include "ImageCopyingEngine.h"
#include <fsfwconfig/OBSWConfig.h>
#include <fsfw/serviceinterface/ServiceInterface.h>

ImageCopyingEngine::ImageCopyingEngine(SoftwareImageHandler *owner,
        Countdown *countdown, SoftwareImageHandler::ImageBuffer *imgBuffer):
        owner(owner), countdown(countdown), imgBuffer(imgBuffer) {}

bool ImageCopyingEngine::getIsOperationOngoing() const {
    if(imageHandlerState == ImageHandlerStates::IDLE) {
        return false;
    }
    else {
        return true;
    }
}

void ImageCopyingEngine::setHammingCodeCheck(bool enableHammingCodeCheck) {
    this->performHammingCodeCheck = enableHammingCodeCheck;
}

void ImageCopyingEngine::enableExtendedDebugOutput(bool enableMoreOutput) {
    this->extendedDebugOutput = enableMoreOutput;
}

ReturnValue_t ImageCopyingEngine::startSdcToFlashOperation(
        ImageSlot imageSlot) {
    imageHandlerState = ImageHandlerStates::COPY_SDC_IMG_TO_FLASH;
    this->imageSlot = imageSlot;
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t ImageCopyingEngine::startFlashToSdcOperation(
        ImageSlot imageSlot) {
    imageHandlerState = ImageHandlerStates::COPY_FLASH_IMG_TO_SDC;
    this->imageSlot = imageSlot;
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t ImageCopyingEngine::startBootloaderToFlashOperation(
        bool fromFRAM) {
    bootloader = true;
    if(fromFRAM) {
#ifdef ISIS_OBC_G20
        imageHandlerState = ImageHandlerStates::COPY_FRAM_BL_TO_FLASH;
#else
        return HasReturnvaluesIF::RETURN_FAILED;
#endif
    }
    else {
        imageHandlerState = ImageHandlerStates::COPY_SDC_BL_TO_FLASH;
    }
    return HasReturnvaluesIF::RETURN_OK;
}


ImageCopyingEngine::ImageHandlerStates
ImageCopyingEngine::getImageHandlerState() const {
    return imageHandlerState;
}

GenericInternalState ImageCopyingEngine::getInternalState() const {
    return internalState;
}

ImageCopyingEngine::ImageHandlerStates
ImageCopyingEngine::getLastFinishedState() const {
    return lastFinishedState;
}

void ImageCopyingEngine::setActiveSdCard(SdCard sdCard) {
    this->activeSdCard = sdCard;
}

void ImageCopyingEngine::reset() {
    internalState = GenericInternalState::IDLE;
    imageHandlerState = ImageHandlerStates::IDLE;
    stepCounter = 0;
    currentByteIdx = 0;
    currentFileSize = 0;
    errorCount = 0;
    helperFlag1 = false;
    helperFlag2 = false;
    helperCounter1 = 0;
    helperCounter2 = 0;
    bootloader = false;
}


ReturnValue_t ImageCopyingEngine::prepareGenericFileInformation(
        VolumeId currentVolume, F_FILE** filePtr) {
    int result = 0;
    if(bootloader) {
        result = change_directory(config::BOOTLOADER_REPOSITORY, true);
        if(result != F_NO_ERROR) {
            // changing directory failed!
            return HasReturnvaluesIF::RETURN_FAILED;
        }

        // Current file size only needs to be cached once.
        // Info output should only be printed once.
        if(stepCounter == 0) {
            currentFileSize = f_filelength(config::BOOTLOADER_NAME);
            handleInfoPrintout(currentVolume);
        }

        *filePtr = f_open(config::BOOTLOADER_NAME, "r");
    }
    else {
        result = change_directory(config::SW_REPOSITORY, true);
        if(result != F_NO_ERROR) {
            // changing directory failed!
            return HasReturnvaluesIF::RETURN_FAILED;
        }

        // Current file size only needs to be cached once.
        // Info output should only be printed once.
        if(stepCounter == 0) {
            if(imageSlot == ImageSlot::IMAGE_0) {
                currentFileSize = f_filelength(config::SW_SLOT_0_NAME);
            }
            else if(imageSlot == ImageSlot::IMAGE_1) {
                currentFileSize = f_filelength(config::SW_SLOT_1_NAME);
            }
            else if(imageSlot == ImageSlot::IMAGE_1) {
                currentFileSize = f_filelength(config::SW_UPDATE_SLOT_NAME);
            }
        }


        if(stepCounter == 0) {

#ifdef AT91SAM9G20_EK
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "Copying AT91 software image SD card " << currentVolume << " slot "
                    << static_cast<int>(imageSlot) << " to AT91 NAND-Flash.." << std::endl;
#else
            sif::printInfo("Copying AT91 software image SD card %d slot %d to AT91 NAND-Flash..\n",
                    currentVolume, imageSlot);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* AT91SAM9G20_EK */

#ifdef ISIS_OBC_G20
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "Copying iOBC software image SD card " << currentVolume << " slot "
                    << static_cast<int>(imageSlot) << " to NOR-Flash.." << std::endl;
#else
            sif::printInfo("Copying iOBC software image SD card %d slot %d to NOR-Flash..\n",
                    currentVolume, imageSlot);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* ISIS_OBC_G20 */

#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "Binary size: " <<  currentFileSize << " bytes." << std::endl;
#else
            sif::printInfo("Binary size: %lu bytes.\n",
                    static_cast<unsigned long>(currentFileSize));
#endif
        }

        if(imageSlot == ImageSlot::IMAGE_0) {
            *filePtr = f_open(config::SW_SLOT_0_NAME, "r");
        }
        else if(imageSlot == ImageSlot::IMAGE_1) {
            *filePtr = f_open(config::SW_SLOT_1_NAME, "r");
        }
        else {
            *filePtr = f_open(config::SW_UPDATE_SLOT_NAME, "r");
        }
    }

    if(f_getlasterror() != F_NO_ERROR) {
        // Opening file failed!
        if(bootloader) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "ImageCopyingHelper::prepareGenericFileInformation: "
                    << "Bootloader file not found!" << std::endl;
#else
            sif::printError("ImageCopyingHelper::prepareGenericFileInformation: "
                    "Bootloader file not found!\n");
#endif
        }
        else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "ImageCopyingHelper::prepareGenericFileInformation: "
                    << "OBSW file not found!" << std::endl;
#else
            sif::printError("ImageCopyingHelper::prepareGenericFileInformation: "
                    "OBSW file not found!\n");
#endif
        }
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    // Seek correct position in file. This needs to be done every time
    // the file is reopened!
    result = f_seek(*filePtr, currentByteIdx, F_SEEK_SET);
    if(result != F_NO_ERROR) {
        // should not happen!
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t ImageCopyingEngine::readFile(uint8_t *buffer, size_t sizeToRead,
        size_t *sizeRead, F_FILE** file) {
    ssize_t bytesRead = f_read(imgBuffer->data(), sizeof(uint8_t),
            sizeToRead, *file);
    if(bytesRead < 0) {
        errorCount++;
        // if reading a file failed 3 times, exit.
        if(errorCount >= 3) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "ImageCopyingHelper::performNandCopyAlgorithm: "
                    << "Reading file failed!" << std::endl;
#else
            sif::printError("ImageCopyingHelper::performNandCopyAlgorithm: "
                    "Reading file failed!\n");
#endif
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        // reading file failed. retry next cycle
        return SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
    }
    *sizeRead = static_cast<size_t>(bytesRead);
    return HasReturnvaluesIF::RETURN_OK;
}

void ImageCopyingEngine::handleInfoPrintout(int currentVolume) {
#if OBSW_VERBOSE_LEVEL >= 1
#ifdef AT91SAM9G20_EK
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Copying AT91 bootloader on SD card "
            << currentVolume << " to AT91 NAND-Flash.." << std::endl;
#else
    sif::printInfo("Copying AT91 bootloader on SD card %d to AT91 NAND-Flash..\n",
            currentVolume);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* AT91SAM9G20_EK */

#ifdef ISIS_OBC_G20
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Copying iOBC bootloader on SD card "
            << currentVolume << " to NOR-Flash.." << std::endl;
#else
    sif::printInfo("Copying iOBC bootloader on SD card %d to NOR-Flash..\n", currentVolume);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* ISIS_OBC_G20 */

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Bootloader size: " <<  currentFileSize << " bytes." << std::endl;
#else
    sif::printInfo("Bootloader size: %lu bytes.", static_cast<unsigned long>(currentFileSize));
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* OBSW_VERBOSE_LEVEL >= 1 */
}
