#include "../ImageCopyingEngine.h"
#include <sam9g20/common/config/commonIOBCConfig.h>

#include <fsfw/timemanager/Countdown.h>
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/globalfunctions/CRC.h>

extern "C" {
#include <sam9g20/common/FRAMApi.h>
#include <hal/Storage/NORflash.h>
#include <memories/norflash/NorFlashApi.h>
}


ReturnValue_t ImageCopyingEngine::continueCurrentOperation() {
    switch(imageHandlerState) {
    case(ImageHandlerStates::IDLE): {
        return HasReturnvaluesIF::RETURN_OK;
    }
    case(ImageHandlerStates::COPY_IMG_SDC_TO_FLASH): {
        return copySdCardImageToNorFlash();
    }
    case(ImageHandlerStates::COPY_IMG_SDC_TO_SDC): {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    case(ImageHandlerStates::COPY_IMG_HAMMING_SDC_TO_FRAM): {
        return copyImgHammingSdcToFram();
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    case(ImageHandlerStates::COPY_IMG_FLASH_TO_SDC): {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    case(ImageHandlerStates::COPY_BL_FRAM_TO_FLASH): {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    case(ImageHandlerStates::COPY_BL_SDC_TO_FLASH): {
        return copySdCardImageToNorFlash();
    }
    case(ImageHandlerStates::COPY_BL_SDC_TO_FRAM): {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    case(ImageHandlerStates::COPY_BL_HAMMING_SDC_TO_FRAM): {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    }
    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t ImageCopyingEngine::startBootloaderToFlashOperation(bool fromFRAM)
{
    bootloader = true;
    if(fromFRAM) {
        imageHandlerState = ImageHandlerStates::COPY_BL_FRAM_TO_FLASH;
    }
    else {
        imageHandlerState = ImageHandlerStates::COPY_BL_SDC_TO_FLASH;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t ImageCopyingEngine::startHammingCodeToFramOperation(ImageSlot respectiveSlot,
        bool bootloader) {
    if(respectiveSlot == ImageSlot::NONE) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    hammingCode = true;
    this->bootloader = bootloader;
    if(not bootloader) {
        sourceSlot = respectiveSlot;
    }

    if(bootloader) {
        imageHandlerState = ImageHandlerStates::COPY_BL_HAMMING_SDC_TO_FRAM;
    }
    else {
        imageHandlerState = ImageHandlerStates::COPY_IMG_HAMMING_SDC_TO_FRAM;
    }
    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t ImageCopyingEngine::copySdCardImageToNorFlash() {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    if(internalState == GenericInternalState::IDLE) {
        internalState = GenericInternalState::STEP_1;
    }

    if(internalState == GenericInternalState::STEP_1) {
        result = handleNorflashErasure();
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
        if(result == SoftwareImageHandler::TASK_PERIOD_OVER_SOON) {
            return result;
        }
        else if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }
    }

    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t ImageCopyingEngine::copyImgHammingSdcToFram() {
    if(internalState == GenericInternalState::IDLE) {
        internalState = GenericInternalState::STEP_1;
    }
    if(internalState == GenericInternalState::STEP_1) {
        if(sourceSlot == ImageSlot::NORFLASH) {

        }
        SDCardAccess access;
        F_FILE* file = nullptr;
        prepareGenericFileInformation(access.currentVolumeId, &file);

        while(currentByteIdx < currentFileSize) {
            size_t sizeToRead = 0;
            if(currentFileSize > sizeToRead) {
                sizeToRead = imgBuffer->size();
            }
            else {
                currentFileSize = sizeToRead;
            }
            size_t sizeRead = f_read(imgBuffer->data(), 1, sizeToRead, file);
            if(sizeRead != sizeToRead) {
                // Should not happen!
                sif::printWarning("ImageCopyingEngine::copyImgHammingSdcToFram: "
                        "Not all bytes read!");
                return HasReturnvaluesIF::RETURN_FAILED;
            }

            int result = write_nor_flash_hamming_code(imgBuffer->data(), currentByteIdx, sizeRead);
            if(result != 0) {
                return HasReturnvaluesIF::RETURN_FAILED;
            }
            currentByteIdx += sizeRead;

            if(countdown->hasTimedOut()) {
                return SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
            }
        }
        reset();
#if OBSW_VERBOSE_LEVEL >= 1
        const char* message = nullptr;
        if(sourceSlot == ImageSlot::NORFLASH) {
            message = "NOR-Flash hamming code";
        }
        else if(sourceSlot == ImageSlot::SDC_SLOT_0) {
            message = "SD Card slot 0 hamming code";
        }
        else if(sourceSlot == ImageSlot::SDC_SLOT_1) {
            message = "SD Card slot 1 hamming code";
        }
        sif::printInfo("Copied %s successfully to storage (FRAM)\n\r", message);
#endif
    }
    return HasReturnvaluesIF::RETURN_OK;

}

ReturnValue_t ImageCopyingEngine::handleNorflashErasure() {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    if(bootloader) {
        // we only want to print this once.
        if(not helperFlag1) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "ImageCopyingEngine::handleNorflashErasure: Deleting old "
                    "bootloader!" << std::endl;
#else
            sif::printInfo("ImageCopyingEngine::handleNorflashErasure: Deleting old"
                    " bootloader!\n");
#endif
            helperFlag1 = true;
        }

        while(stepCounter < RESERVED_BL_SECTORS) {
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
        if(stepCounter == RESERVED_BL_SECTORS) {
            stepCounter = 0;
            helperFlag1 = false;
        }
    }
    else {
        result = handleObswErasure();
    }

    return result;
}

ReturnValue_t ImageCopyingEngine::handleObswErasure() {
    if(not helperFlag1) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "ImageCopyingEngine::handleNorflashErasure: Deleting old binary!" << std::endl;
#else
        sif::printInfo("ImageCopyingEngine::handleNorflashErasure: Deleting old binary!\n");
#endif
        SDCardAccess sdCardAccess;
        int result = change_directory(config::SW_REPOSITORY, true);
        if(result != F_NO_ERROR) {
            // The hardcoded repository does not exist. Exit!
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "ImageCopyingHelper::handleErasingForObsw: Software repository does "
                    "not exist. Cancelling erase operation!" << std::endl;
#else
            sif::printError("ImageCopyingHelper::handleErasingForObsw: Software repository does "
                    "not exist. Cancelling erase operation!\n");
#endif
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        if(sourceSlot == ImageSlot::SDC_SLOT_0) {
            currentFileSize = f_filelength(config::SW_SLOT_0_NAME);
        }
        else if(sourceSlot == ImageSlot::SDC_SLOT_1) {
            currentFileSize = f_filelength(config::SW_SLOT_1_NAME);
        }

        helperFlag1 = true;
        uint8_t requiredBlocks = RESERVED_OBSW_SECTORS;
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

    if(currentFileSize == 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 0
        sif::printWarning("ImageCopyingEngine::performNorCopyOperation: Current file size is 0!\n");
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }

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
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SoftwareImageHandler::performNorCopyOperation:"
                    << " Bytes read smaller than size to read!" << std::endl;
#else
            sif::printError("SoftwareImageHandler::performNorCopyOperation:"
                    " Bytes read smaller than size to read!\n");
#endif
            return HasReturnvaluesIF::RETURN_FAILED;
        }
    }

    helperFlag1 = true;
    /* we should consider a critical section here and extracting this function to a special task
    with the highest priority so it can not be interrupted. */
    size_t offset = 0;
    size_t baseAddress = getBaseAddress(stepCounter, &offset);
    // sif::debug << "Base Address: " << baseAddress << ", Offset: "
    //        << offset << std::endl;
    int retval = NORFLASH_WriteData(&NORFlash,  baseAddress + offset,
            imgBuffer->data(), sizeToRead);
    if(retval != 0) {
        errorCount++;
        if(errorCount >= 3) {
            // if writing to NAND failed 5 times, exit.
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SoftwareImageHandler::copyBootloaderToNor"
                    << "Flash: Write error!" << std::endl;
#else
            sif::printError("SoftwareImageHandler::copyBootloaderToNor"
                    "Flash: Write error!\n");
#endif
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        // Maybe SD card is busy, so try in next cycle..
        return SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
    }
    else {
        result = HasReturnvaluesIF::RETURN_OK;
    }

    // bucket write success
    currentByteIdx += sizeToRead;
    //sif::debug << "ImageCopyingEngine::performNorCopyOperation:
    //      << "Current Byte Index: " << currentByteIdx << std::endl;
    stepCounter ++;
    // Set this flag to false so that the next bucket can be read from the
    // SD card.
    helperFlag1 = false;

    if(currentByteIdx >= currentFileSize) {
        // operation finished.
        handleFinishPrintout();

        if(bootloader) {
            writeBootloaderSizeAndCrc();
        }

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

void ImageCopyingEngine::writeBootloaderSizeAndCrc() {
    int retval = NORFLASH_WriteData(&NORFlash, NORFLASH_BL_SIZE_START,
            reinterpret_cast<unsigned char *>(&currentFileSize), 4);
    if(retval != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "Writing bootloader size failed!" << std::endl;
#else
        sif::printError("Writing bootloader size failed!\n");
#endif
    }
    // calculate and write CRC to designated NOR-Flash address
    // This will be used by the bootloader to determine SEUs in the
    // bootloader.
    uint16_t crc16 = CRC::crc16ccitt(reinterpret_cast<const uint8_t*>(NORFLASH_BASE_ADDRESS_READ),
            currentFileSize);
    retval = NORFLASH_WriteData(&NORFlash, NORFLASH_BL_CRC16_START,
            reinterpret_cast<unsigned char *>(&crc16), sizeof(crc16));
    if(retval != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "Writing bootloader CRC16 failed!" << std::endl;
#else
        sif::printError("Writing bootloader CRC16 failed!\n");
#endif
    }
#if OBSW_VERBOSE_LEVEL >= 1
    else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << std::setfill('0') << std::setw(2) << std::hex
                << "Bootloader CRC16: " << "0x" << (crc16 >> 8 & 0xff) << ", "
                << "0x" << (crc16 & 0xff) << " written to address " << std::setw(8)
                << "0x" << NORFLASH_BL_CRC16_START_READ << std::setfill(' ')
                << std::dec << std::endl;
#else
        sif::printInfo("Bootloader CRC16: 0x%02x,0x%02x written to address 0x%08x\n",
                (crc16 >> 8) & 0xff, crc16 & 0xff, NORFLASH_BL_CRC16_START_READ);
#endif
    }
#endif
}

/**
 * This is the core function to translate algorithmic steps to the memory locations
 * read, erased or written. Perform changes with care!
 * Currently, the algorithm assumes that all small sectors are reserved for the bootloader.
 */
#if IOBC_BOOTLOADER_SIZE == IOBC_SMALL_BOOTLOADER_65KB

uint32_t ImageCopyingEngine::getBaseAddress(uint8_t stepCounter,
        size_t* offset) {
    if(bootloader) {
        // deletion steps, performed per-sector
        if(internalState == GenericInternalState::STEP_1) {
            switch(stepCounter) {
            case(0): return NORFLASH_SA0_ADDRESS;
            case(1): return NORFLASH_SA1_ADDRESS;
            case(2): return NORFLASH_SA2_ADDRESS;
            case(3): return NORFLASH_SA3_ADDRESS;
            case(4): return NORFLASH_SA4_ADDRESS;
            case(5): return NORFLASH_SA5_ADDRESS;
            case(6): return NORFLASH_SA6_ADDRESS;
            case(7): return NORFLASH_SA7_ADDRESS;
            }
        }
        // now we write in small sector buckets, so the offset is actually important.
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
            case(5): return NORFLASH_SA5_ADDRESS;
            case(6): return NORFLASH_SA6_ADDRESS;
            case(7): return NORFLASH_SA7_ADDRESS;
            }
        }
    }
    else {
        if(internalState == GenericInternalState::STEP_1) {
            switch(stepCounter) {
            case(0): return NORFLASH_SA8_ADDRESS;
            case(1): return NORFLASH_SA9_ADDRESS;
            case(2): return NORFLASH_SA10_ADDRESS;
            case(3): return NORFLASH_SA11_ADDRESS;
            case(4): return NORFLASH_SA12_ADDRESS;
            case(5): return NORFLASH_SA13_ADDRESS;
            case(6): return NORFLASH_SA14_ADDRESS;
            case(7): return NORFLASH_SA15_ADDRESS;
            case(8): return NORFLASH_SA16_ADDRESS;
            case(9): return NORFLASH_SA17_ADDRESS;
            case(10): return NORFLASH_SA18_ADDRESS;
            case(11): return NORFLASH_SA19_ADDRESS;
            case(12): return NORFLASH_SA20_ADDRESS;
            case(13): return NORFLASH_SA21_ADDRESS;
            case(14): return NORFLASH_SA22_ADDRESS;

            default: return NORFLASH_SA22_ADDRESS;
            }
        }
        else if(internalState == GenericInternalState::STEP_2) {
            switch(stepCounter) {
            case(0): return NORFLASH_SA8_ADDRESS;
            case(8): return NORFLASH_SA9_ADDRESS;
            case(16): return NORFLASH_SA10_ADDRESS;
            case(24): return NORFLASH_SA11_ADDRESS;
            case(32): return NORFLASH_SA12_ADDRESS;
            case(40): return NORFLASH_SA13_ADDRESS;
            case(48): return NORFLASH_SA14_ADDRESS;
            case(56): return NORFLASH_SA15_ADDRESS;
            case(64): return NORFLASH_SA16_ADDRESS;
            case(72): return NORFLASH_SA17_ADDRESS;
            case(80): return NORFLASH_SA18_ADDRESS;
            case(88): return NORFLASH_SA19_ADDRESS;
            case(96): return NORFLASH_SA20_ADDRESS;
            case(104): return NORFLASH_SA21_ADDRESS;
            case(112): return NORFLASH_SA22_ADDRESS;
            default:
                if(stepCounter < 112 + 8) {
                    *offset = (stepCounter % 8) * NORFLASH_SMALL_SECTOR_SIZE;
                }
                if(stepCounter < 8) return NORFLASH_SA8_ADDRESS;
                if(stepCounter < 16) return NORFLASH_SA9_ADDRESS;
                if(stepCounter < 24) return NORFLASH_SA10_ADDRESS;
                if(stepCounter < 32) return NORFLASH_SA11_ADDRESS;
                if(stepCounter < 40) return NORFLASH_SA12_ADDRESS;
                if(stepCounter < 48) return NORFLASH_SA13_ADDRESS;
                if(stepCounter < 56) return NORFLASH_SA14_ADDRESS;
                if(stepCounter < 64) return NORFLASH_SA15_ADDRESS;
                if(stepCounter < 72) return NORFLASH_SA16_ADDRESS;
                if(stepCounter < 80) return NORFLASH_SA17_ADDRESS;
                if(stepCounter < 88) return NORFLASH_SA18_ADDRESS;
                if(stepCounter < 96) return NORFLASH_SA19_ADDRESS;
                if(stepCounter < 104) return NORFLASH_SA20_ADDRESS;
                if(stepCounter < 112) return NORFLASH_SA21_ADDRESS;
                if(stepCounter < 120) return NORFLASH_SA22_ADDRESS;
            }
        }
    }
    return 0xffffffff;
}

#elif IOBC_BOOTLOADER_SIZE == IOBC_LARGE_BOOTLOADER_128KB

uint32_t ImageCopyingEngine::getBaseAddress(uint8_t stepCounter,
        size_t* offset) {
    if(bootloader) {
        // deletion steps, performed per-sector
        if(internalState == GenericInternalState::STEP_1) {
            switch(stepCounter) {
            case(0): return NORFLASH_SA0_ADDRESS;
            case(1): return NORFLASH_SA1_ADDRESS;
            case(2): return NORFLASH_SA2_ADDRESS;
            case(3): return NORFLASH_SA3_ADDRESS;
            case(4): return NORFLASH_SA4_ADDRESS;
            case(5): return NORFLASH_SA5_ADDRESS;
            case(6): return NORFLASH_SA6_ADDRESS;
            case(7): return NORFLASH_SA7_ADDRESS;
            case(8): return NORFLASH_SA8_ADDRESS;
            }
        }
        // now we write in small sector buckets, so the offset is actually important.
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
            case(5): return NORFLASH_SA5_ADDRESS;
            case(6): return NORFLASH_SA6_ADDRESS;
            case(7): return NORFLASH_SA7_ADDRESS;
            case(8): return NORFLASH_SA8_ADDRESS;
            // This sector is a large sector, so we need to set the offset here.
            case(9): {
                *offset = NORFLASH_SMALL_SECTOR_SIZE;
                return NORFLASH_SA8_ADDRESS;
            }
            case(10): {
                *offset = NORFLASH_SMALL_SECTOR_SIZE * 2;
                return NORFLASH_SA8_ADDRESS;
            }
            case(11): {
                *offset = NORFLASH_SMALL_SECTOR_SIZE * 3;
                return NORFLASH_SA8_ADDRESS;
            }
            case(12): {
                *offset = NORFLASH_SMALL_SECTOR_SIZE * 4;
                return NORFLASH_SA8_ADDRESS;
            }
            case(13): {
                *offset = NORFLASH_SMALL_SECTOR_SIZE * 5;
                return NORFLASH_SA8_ADDRESS;
            }
            case(14): {
                *offset = NORFLASH_SMALL_SECTOR_SIZE * 6;
                return NORFLASH_SA8_ADDRESS;
            }
            case(15): {
                *offset = NORFLASH_SMALL_SECTOR_SIZE * 7;
                return NORFLASH_SA8_ADDRESS;
            }
            }
        }
    }
    else {
        if(internalState == GenericInternalState::STEP_1) {
            switch(stepCounter) {
            case(0): return NORFLASH_SA9_ADDRESS;
            case(1): return NORFLASH_SA10_ADDRESS;
            case(2): return NORFLASH_SA11_ADDRESS;
            case(3): return NORFLASH_SA12_ADDRESS;
            case(4): return NORFLASH_SA13_ADDRESS;
            case(5): return NORFLASH_SA14_ADDRESS;
            case(6): return NORFLASH_SA15_ADDRESS;
            case(7): return NORFLASH_SA16_ADDRESS;
            case(8): return NORFLASH_SA17_ADDRESS;
            case(9): return NORFLASH_SA18_ADDRESS;
            case(10): return NORFLASH_SA19_ADDRESS;
            case(11): return NORFLASH_SA20_ADDRESS;
            case(12): return NORFLASH_SA21_ADDRESS;
            case(13): return NORFLASH_SA21_ADDRESS;
            case(14): return NORFLASH_SA22_ADDRESS;
            default: return NORFLASH_SA22_ADDRESS;
            }
        }
        else if(internalState == GenericInternalState::STEP_2) {
            switch(stepCounter) {
            case(0): return NORFLASH_SA9_ADDRESS;
            case(8): return NORFLASH_SA10_ADDRESS;
            case(16): return NORFLASH_SA11_ADDRESS;
            case(24): return NORFLASH_SA12_ADDRESS;
            case(32): return NORFLASH_SA13_ADDRESS;
            case(40): return NORFLASH_SA14_ADDRESS;
            case(48): return NORFLASH_SA15_ADDRESS;
            case(56): return NORFLASH_SA16_ADDRESS;
            case(64): return NORFLASH_SA17_ADDRESS;
            case(72): return NORFLASH_SA18_ADDRESS;
            case(80): return NORFLASH_SA19_ADDRESS;
            case(88): return NORFLASH_SA20_ADDRESS;
            case(96): return NORFLASH_SA21_ADDRESS;
            case(104): return NORFLASH_SA22_ADDRESS;
            default:
                if(stepCounter < 104 + 8) {
                    *offset = (stepCounter % 8) * NORFLASH_SMALL_SECTOR_SIZE;
                }
                if(stepCounter < 8) return NORFLASH_SA9_ADDRESS;
                if(stepCounter < 16) return NORFLASH_SA10_ADDRESS;
                if(stepCounter < 24) return NORFLASH_SA11_ADDRESS;
                if(stepCounter < 32) return NORFLASH_SA12_ADDRESS;
                if(stepCounter < 40) return NORFLASH_SA13_ADDRESS;
                if(stepCounter < 48) return NORFLASH_SA14_ADDRESS;
                if(stepCounter < 56) return NORFLASH_SA15_ADDRESS;
                if(stepCounter < 64) return NORFLASH_SA16_ADDRESS;
                if(stepCounter < 72) return NORFLASH_SA17_ADDRESS;
                if(stepCounter < 80) return NORFLASH_SA18_ADDRESS;
                if(stepCounter < 88) return NORFLASH_SA19_ADDRESS;
                if(stepCounter < 96) return NORFLASH_SA20_ADDRESS;
                if(stepCounter < 104) return NORFLASH_SA21_ADDRESS;
                if(stepCounter < 112) return NORFLASH_SA22_ADDRESS;
            }
        }
    }
    return 0xffffffff;
}

#endif /* IOBC_BOOTLOADER_SIZE == IOBC_LARGE_BOOTLOADER_128KB */

void ImageCopyingEngine::handleFinishPrintout() {
#if OBSW_VERBOSE_LEVEL >= 1
    if(bootloader) {

#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "Copying bootloader to NOR-Flash finished with " << stepCounter <<
                " steps!" << std::endl;
#else
        sif::printInfo("Copying bootloader to NOR-Flash finished with %hu steps!\n", stepCounter);
#endif

        // Print the ARM vectors for the bootloader.
#if OBSW_VERBOSE_LEVEL >= 2
        std::array<uint8_t, 7 * 4> armVectors;
        uint32_t currentArmVector = 0;
        NORFLASH_ReadData(&NORFlash, NORFLASH_SA0_ADDRESS,
                armVectors.data(), armVectors.size());
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << std::hex << std::setfill('0') << std::setw(8);
        sif::debug << "Written ARM vectors: " << std::endl;
#else
        sif::printDebug("Written ARM vectors: \n");
#endif
        std::memcpy(&currentArmVector, armVectors.data(), 4);
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "1: 0x" << currentArmVector << std::endl;
#else
        sif::printDebug("1: 0x%08x\n", currentArmVector);
#endif
        std::memcpy(&currentArmVector, armVectors.data() + 4, 4);
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "2: 0x" << currentArmVector << std::endl;
#else
        sif::printDebug("2: 0x%08x\n", currentArmVector);
#endif
        std::memcpy(&currentArmVector, armVectors.data() + 8, 4);
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "3: 0x" << currentArmVector << std::endl;
#else
        sif::printDebug("3: 0x%08x\n", currentArmVector);
#endif
        std::memcpy(&currentArmVector, armVectors.data() + 12, 4);
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "4: 0x" << currentArmVector << std::endl;
#else
        sif::printDebug("4: 0x%08x\n", currentArmVector);
#endif
        std::memcpy(&currentArmVector, armVectors.data() + 16, 4);
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "5: 0x" << currentArmVector << std::endl;
#else
        sif::printDebug("5: 0x%08x\n", currentArmVector);
#endif
        std::memcpy(&currentArmVector, armVectors.data() + 20, 4);
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "6: 0x" << currentArmVector << std::endl;
#else
        sif::printDebug("6: 0x%08x\n", currentArmVector);
#endif
        std::memcpy(&currentArmVector, armVectors.data() + 24, 4);
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "7: 0x" << currentArmVector << std::endl;
        sif::debug << std::dec << std::setfill(' ');
#else
        sif::printDebug("7: 0x%08x\n", currentArmVector);
#endif
#endif /* OBSW_VERBOSE_LEVEL >= 2 */

    }
    else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "Copying OBSW image to NOR-Flash finished with " << stepCounter <<
                " steps!" << std::endl;
#else
        sif::printInfo("Copying OBSW image to NOR-Flash finished with %hu steps!\n",
                stepCounter);
#endif
    }
#endif /* OBSW_VERBOSE_LEVEL >= 1 */

}

