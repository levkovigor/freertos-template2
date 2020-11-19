#include "../ImageCopyingEngine.h"
#include <fsfw/timemanager/Countdown.h>

extern "C" {
#include <sam9g20/common/FRAMApi.h>
#include <hal/Storage/NORflash.h>
}

#include <fsfw/globalfunctions/CRC.h>

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

ReturnValue_t ImageCopyingEngine::handleNorflashErasure() {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    if(bootloader) {
        sif::info << "ImageCopyingEngine::handleNorflashErasure: Deleting old"
                << " bootloader!" << std::endl;
        while(stepCounter < RESERVED_BL_SMALL_SECTORS) {
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
        if(stepCounter == RESERVED_BL_SMALL_SECTORS) {
            stepCounter = 0;
        }
    }
    else {
        result = handleObswErasure();
    }

    return result;
}

ReturnValue_t ImageCopyingEngine::handleObswErasure() {
    if(not helperFlag1) {
        sif::info << "ImageCopyingEngine::handleNorflashErasure: Deleting old"
                << " binary!" << std::endl;
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
        if(currentFileSize <=  NORFLASH_TOTAL_SMALL_SECTOR_MEM_OBSW) {
        	requiredBlocks = std::ceil(
        			static_cast<float>(currentFileSize) /
					(NORFLASH_SMALL_SECTOR_SIZE));
        }
        else {
        	requiredBlocks = RESERVED_OBSW_SMALL_SECTORS;
        	requiredBlocks += std::ceil(
        			static_cast<float>(currentFileSize -
        					NORFLASH_TOTAL_SMALL_SECTOR_MEM_OBSW) /
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
    // sif::debug << "Base Address: " << baseAddress << ", Offset: "
    //        << offset << std::endl;
    int retval = NORFLASH_WriteData(&NORFlash,  baseAddress + offset,
            imgBuffer->data(), sizeToRead);
    if(retval != 0) {
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
#if OBSW_ENHANCED_PRINTOUT == 1
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
		sif::error << "Writing bootloader size failed!" << std::endl;
	}
	// calculate and write CRC to designated NOR-Flash address
	// This will be used by the bootloader to determine SEUs in the
	// bootloader.
	uint16_t crc16 = CRC::crc16ccitt(reinterpret_cast<const uint8_t*>(
			NORFLASH_BASE_ADDRESS_READ),
			currentFileSize);
	retval = NORFLASH_WriteData(&NORFlash, NORFLASH_BL_CRC16_START,
			reinterpret_cast<unsigned char *>(&crc16),
			sizeof(crc16));
	if(retval != 0) {
		sif::error << "Writing bootloader CRC16 failed!" << std::endl;
	}
#if OBSW_ENHANCED_PRINTOUT == 1
	else {
    	sif::info << std::setfill('0') << std::setw(2) << std::hex
    			<< "Bootloader CRC16: " << "0x" << (crc16 >> 8 & 0xff) << ", "
				<< "0x" << (crc16 & 0xff) << " written to address "
				<< "0x" << NORFLASH_BL_CRC16_START << std::setfill(' ')
				<< std::dec << std::endl;
	}
#endif
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
            case(11): return NORFLASH_SA9_ADDRESS;
            case(19): return NORFLASH_SA10_ADDRESS;
            case(27): return NORFLASH_SA11_ADDRESS;
            case(35): return NORFLASH_SA12_ADDRESS;
            case(43): return NORFLASH_SA13_ADDRESS;
            case(51): return NORFLASH_SA14_ADDRESS;
            case(59): return NORFLASH_SA15_ADDRESS;
            case(67): return NORFLASH_SA16_ADDRESS;
            case(75): return NORFLASH_SA17_ADDRESS;
            case(83): return NORFLASH_SA18_ADDRESS;
            case(91): return NORFLASH_SA19_ADDRESS;
            case(99): return NORFLASH_SA20_ADDRESS;
            case(107): return NORFLASH_SA21_ADDRESS;
            case(115): return NORFLASH_SA22_ADDRESS;
            default:
                if(stepCounter < 115 + 8) {
                    // We always write one small sector at a time, so we need
                    // to write big sectors in multiple steps. This required
                    // calculating an offset.
                    *offset = ((stepCounter - 3) % 8) *
                            NORFLASH_SMALL_SECTOR_SIZE;
                }
                if(stepCounter < 11) return NORFLASH_SA8_ADDRESS;
                if(stepCounter < 19) return NORFLASH_SA9_ADDRESS;
                if(stepCounter < 27) return NORFLASH_SA10_ADDRESS;
                if(stepCounter < 35) return NORFLASH_SA11_ADDRESS;
                if(stepCounter < 43) return NORFLASH_SA12_ADDRESS;
                if(stepCounter < 51) return NORFLASH_SA13_ADDRESS;
                if(stepCounter < 59) return NORFLASH_SA14_ADDRESS;
                if(stepCounter < 67) return NORFLASH_SA15_ADDRESS;
                if(stepCounter < 75) return NORFLASH_SA16_ADDRESS;
                if(stepCounter < 83) return NORFLASH_SA17_ADDRESS;
                if(stepCounter < 91) return NORFLASH_SA18_ADDRESS;
                if(stepCounter < 99) return NORFLASH_SA19_ADDRESS;
                if(stepCounter < 107) return NORFLASH_SA20_ADDRESS;
                if(stepCounter < 115) return NORFLASH_SA21_ADDRESS;
                if(stepCounter < 123) return NORFLASH_SA22_ADDRESS;
            }
        }
    }
    return 0xffffffff;
}


