#include "ImageCopyingEngine.h"

#include <fsfw/serviceinterface/ServiceInterface.h>
#include <sam9g20/core/SoftwareImageHandler.h>

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
    if(sourceSlot != image::ImageSlot::SDC_SLOT_0 and sourceSlot != image::ImageSlot::SDC_SLOT_1) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    imageHandlerState = ImageHandlerStates::COPY_IMG_SDC_TO_FLASH;
    this->sourceSlot = sourceSlot;
    this->targetSlot = image::ImageSlot::FLASH;
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t ImageCopyingEngine::startSdcToSdcOperation(
        image::ImageSlot sourceSlot) {
    imageHandlerState = ImageHandlerStates::COPY_IMG_SDC_TO_SDC;
    this->sourceSlot = sourceSlot;
    if(sourceSlot == image::ImageSlot::SDC_SLOT_1){
    	targetSlot = image::ImageSlot::SDC_SLOT_0;
    }else{
    	targetSlot = image::ImageSlot::SDC_SLOT_1;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t ImageCopyingEngine::startFlashToSdcOperation(
        image::ImageSlot targetSlot) {
    if(targetSlot == image::ImageSlot::FLASH or targetSlot == image::ImageSlot::NONE) {
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
    hammingCode = false;
}


ReturnValue_t ImageCopyingEngine::prepareGenericFileInformation(
        VolumeId currentVolume, F_FILE** filePtr) {
    int result = 0;
    bool bootloader = false;
    if(sourceSlot == image::ImageSlot::BOOTLOADER_0 or
            sourceSlot == image::ImageSlot::BOOTLOADER_1) {
        bootloader = true;
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
                if(sourceSlot == image::ImageSlot::BOOTLOADER_0) {
                    currentFileSize = f_filelength(config::BOOTLOADER_NAME);
                }
                else {
                    currentFileSize = f_filelength(config::BOOTLOADER_2_NAME);
                }
            }
        }

        if(hammingCode) {
            *filePtr = f_open(config::BL_HAMMING_NAME, "r");
        }
        else {
            if(sourceSlot == image::ImageSlot::BOOTLOADER_0) {
                *filePtr = f_open(config::BOOTLOADER_NAME, "r");
            }
            else {
                *filePtr = f_open(config::BOOTLOADER_2_NAME, "r");
            }
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

    if(stepCounter == 0) {
        handleInfoPrintout(currentVolume);
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

	SDCardAccess sdCardAccess;
	F_FILE *binaryFile;
	long size = 8;
	long sizeToRead = 1;
	if(sourceSlot == image::ImageSlot::SDC_SLOT_0) {
		binaryFile = f_open(config::SW_SLOT_1_NAME, "w");
	}
	else if(sourceSlot == image::ImageSlot::SDC_SLOT_1) {
    	binaryFile = f_open(config::SW_SLOT_1_NAME, "w");
	}
	    // Get file information like binary size, open the file, seek correct
	    // position etc.

	ReturnValue_t result = prepareGenericFileInformation(
			sdCardAccess.currentVolumeId, &binaryFile);
	while(true){
		if(currentFileSize-currentByteIdx>imgBuffer->size()){
			sizeToRead=imgBuffer->size();
		}else{
			sizeToRead=currentFileSize-currentByteIdx;
		}
		f_seek(binaryFile, currentByteIdx, F_SEEK_SET);
		size_t bytesRead = f_read(imgBuffer, size, sizeToRead, binaryFile);
		currentByteIdx += bytesRead;

		if(currentByteIdx>=currentFileSize){
			reset();
	        lastFinishedState = imageHandlerState;
	        handleFinishPrintout();
	        return image::OPERATION_FINISHED;
		}
		if(countdown->hasTimedOut()) {
		        return  image::TASK_PERIOD_OVER_SOON;
		}
	}
    return HasReturnvaluesIF::RETURN_OK;
}

void ImageCopyingEngine::handleGenericInfoPrintout(const char * const board, char const* typePrint,
        char const* sourcePrint, char const* targetPrint) {
    if(board == nullptr) {
        return;
    }
    if(sourcePrint == nullptr or targetPrint == nullptr or typePrint == nullptr) {
        if(sourcePrint == nullptr) {
            sourcePrint = "unknown source";
        }
        if(targetPrint == nullptr) {
            targetPrint = "unknown target";
        }
        if(typePrint == nullptr) {
            typePrint = "unknown type";
        }
    }

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Copying " << board << " " << typePrint << " from " << sourcePrint << " to " <<
            targetPrint << ".." << std::endl;
    sif::info << "Binary size: " <<  currentFileSize << " bytes." << std::endl;
#else
    sif::printInfo("Copying %s %s from %s to %s..\n", board, typePrint, sourcePrint, targetPrint);
    sif::printInfo("Binary size: %lu bytes.\n", static_cast<unsigned long>(currentFileSize));
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */

}
