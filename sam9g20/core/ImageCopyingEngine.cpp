#include "ImageCopyingEngine.h"

#include <fsfw/serviceinterface/ServiceInterface.h>

ImageCopyingEngine::ImageCopyingEngine(SoftwareImageHandler *owner,
        Countdown *countdown, image::ImageBuffer *imgBuffer):
        owner(owner), countdown(countdown), imgBuffer(imgBuffer) {}

bool ImageCopyingEngine::getIsOperationOngoing() const {
    if(imageHandlerState == ImageHandlerStates::IDLE) {
        return false;
    }
    else {
        return true;
    }
}

ReturnValue_t ImageCopyingEngine::startSdcToFlashOperation(
        image::ImageSlot sourceSlot) {
    if(sourceSlot == image::ImageSlot::NORFLASH or sourceSlot == image::ImageSlot::NONE) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    imageHandlerState = ImageHandlerStates::COPY_IMG_SDC_TO_FLASH;
    this->sourceSlot = sourceSlot;
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t ImageCopyingEngine::startFlashToSdcOperation(
        image::ImageSlot targetSlot) {
    if(targetSlot == image::ImageSlot::NORFLASH or targetSlot == image::ImageSlot::NONE) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    imageHandlerState = ImageHandlerStates::COPY_IMG_FLASH_TO_SDC;
    this->sourceSlot = targetSlot;
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

void ImageCopyingEngine::reset() {
    internalState = GenericInternalState::IDLE;
    imageHandlerState = ImageHandlerStates::IDLE;
    sourceSlot = image::ImageSlot::NONE;
    targetSlot = image::ImageSlot::NONE;
    stepCounter = 0;
    currentByteIdx = 0;
    currentFileSize = 0;
    errorCount = 0;
    helperFlag1 = false;
    helperFlag2 = false;
    helperCounter1 = 0;
    helperCounter2 = 0;
    bootloader = false;
    hammingCode = false;
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
            if(hammingCode) {
                currentFileSize = f_filelength(config::BL_HAMMING_NAME);
            }
            else {
                currentFileSize = f_filelength(config::BOOTLOADER_NAME);
                // TODO: pass hammingCode flag to info printout
                handleInfoPrintout(currentVolume);
            }
        }

        if(hammingCode) {
            *filePtr = f_open(config::BL_HAMMING_NAME, "r");
        }
        else {
            *filePtr = f_open(config::BOOTLOADER_NAME, "r");
        }
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
            if(sourceSlot == image::ImageSlot::SDC_SLOT_0) {
                if(hammingCode) {
                    currentFileSize = f_filelength(config::SW_SLOT_0_HAMMING_NAME);
                }
                else {
                    currentFileSize = f_filelength(config::SW_SLOT_0_NAME);
                }
            }
            else if(sourceSlot == image::ImageSlot::SDC_SLOT_1) {
                if(hammingCode) {
                    currentFileSize = f_filelength(config::SW_SLOT_1_HAMMING_NAME);
                }
                else {
                    currentFileSize = f_filelength(config::SW_SLOT_1_NAME);
                }
            }
        }


        if(stepCounter == 0) {

#ifdef AT91SAM9G20_EK
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "Copying AT91 software image SD card " << currentVolume << " slot "
                    << static_cast<int>(sourceSlot) << " to AT91 NAND-Flash.." << std::endl;
#else
            sif::printInfo("Copying AT91 software image SD card %d slot %d to AT91 NAND-Flash..\n",
                    currentVolume, sourceSlot);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* AT91SAM9G20_EK */

#ifdef ISIS_OBC_G20
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "Copying iOBC software image SD card " << currentVolume << " slot "
                    << static_cast<int>(sourceSlot) << " to NOR-Flash.." << std::endl;
#else
            sif::printInfo("Copying iOBC software image SD card %d slot %d to NOR-Flash..\n",
                    currentVolume, sourceSlot);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* ISIS_OBC_G20 */

#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "Binary size: " <<  currentFileSize << " bytes." << std::endl;
#else
            sif::printInfo("Binary size: %lu bytes.\n",
                    static_cast<unsigned long>(currentFileSize));
#endif
        }

        if(sourceSlot == image::ImageSlot::SDC_SLOT_0) {
            *filePtr = f_open(config::SW_SLOT_0_NAME, "r");
        }
        else if(sourceSlot == image::ImageSlot::SDC_SLOT_1) {
            *filePtr = f_open(config::SW_SLOT_1_NAME, "r");
        }
    }

    if(f_getlasterror() != F_NO_ERROR) {
        // Opening file failed!
        char const* missingFile = nullptr;
        if(bootloader) {
            if(hammingCode) {
                missingFile = "Bootloader hamming code";
            }
            else {
                missingFile = "Bootloader";
            }
        }
        else {
            if(hammingCode) {
                missingFile = "OBSW hamming code";
            }
            else {
                missingFile = "OBSW file";
            }
        }
        if(missingFile != nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "ImageCopyingHelper::prepareGenericFileInformation: " << missingFile
                    << "file not found!" << std::endl;
#else
            sif::printError("ImageCopyingHelper::prepareGenericFileInformation: "
                    "%s not found!\n", missingFile);
#endif
        }
        return F_ERR_NOTFOUND;
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
    ssize_t bytesRead = f_read(imgBuffer->data(), sizeof(uint8_t), sizeToRead, *file);
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
        return image::TASK_PERIOD_OVER_SOON;
    }
    *sizeRead = static_cast<size_t>(bytesRead);
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t ImageCopyingEngine::copySdcImgToSdc() {
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
