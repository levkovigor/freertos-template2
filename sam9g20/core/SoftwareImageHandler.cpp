#include "SoftwareImageHandler.h"
#include <fsfw/tasks/PeriodicTaskIF.h>
#include <fsfw/timemanager/Stopwatch.h>
#include <sam9g20/memory/SDCardHandler.h>

extern "C" {

#ifdef ISIS_OBC_G20
#include <sam9g20/common/FRAMApi.h>
#include <hal/Storage/NORflash.h>
#endif

#ifdef AT91SAM9G20_EK
// include nand flash stuff here
#include <at91/boards/at91sam9g20-ek/board.h>
#include <at91/boards/at91sam9g20-ek/board_memories.h>
#include <at91/peripherals/pio/pio.h>
#include <at91/utility/trace.h>
#include <at91/utility/hamming.h>
#include <at91/memories/nandflash/SkipBlockNandFlash.h>
#endif
}

#include <config/OBSWConfig.h>
#include <sam9g20/memory/SDCardAccess.h>


SoftwareImageHandler::SoftwareImageHandler(object_id_t objectId):
        SystemObject(objectId), actionHelper(this, nullptr) {

}

ReturnValue_t SoftwareImageHandler::performOperation(uint8_t opCode) {
	//Stopwatch stopwatch;
    if(not operationOngoing and oneShot) {
#if defined(AT91SAM9G20_EK)
        copyBootloaderToNandFlash(false, false);
#endif
    }


    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SoftwareImageHandler::initialize() {
    // set action helper queue here.
#ifdef ISIS_OBC_G20
    int result = NORflash_start();

    if(result != 0) {
        // should never happen ! we should power cycle if this happens.
        return result;
    }
#endif

    return HasReturnvaluesIF::RETURN_OK;
}

void SoftwareImageHandler::copySdCardImageToNorFlash(SdCard sdCard,
        ImageSlot imageSlot, bool performHammingCheck) {
}

void SoftwareImageHandler::copyNorFlashImageToSdCards(SdCard sdCard,
        ImageSlot imageSlot, bool performHammingCheck) {
}

void SoftwareImageHandler::checkNorFlashImage() {
}

ReturnValue_t SoftwareImageHandler::initializeAfterTaskCreation() {
	countdown = new Countdown(
			static_cast<float>(this->executingTask->getPeriodMs()) * 0.8);
	return HasReturnvaluesIF::RETURN_OK;
}

void SoftwareImageHandler::setTaskIF(PeriodicTaskIF *executingTask) {
	this->executingTask = executingTask;
}

void SoftwareImageHandler::checkSdCardImage(SdCard sdCard,
        ImageSlot imageSlot) {
}

MessageQueueId_t SoftwareImageHandler::getCommandQueue() const {
    return 0;
}

ReturnValue_t SoftwareImageHandler::executeAction(ActionId_t actionId,
        MessageQueueId_t commandedBy, const uint8_t *data, size_t size) {
    return HasReturnvaluesIF::RETURN_OK;
}



#ifdef ISIS_OBC_G20
ReturnValue_t SoftwareImageHandler::copySdBootloaderToNorFlash(
        bool performHammingCheck) {
	countdown->resetTimer();
    // 1. step: Find out bootloader location and name
	//read_nor_flash_reboot_counter()
	// we should start a countdown and suspend the operation when coming
	// close to the periodic operation frequency. That way, other low prio
	// tasks get the chance to do stuff to.

    // NOR-Flash:
    // 4 small NOR-Flash sectors will be reserved for now: 8192 * 4 = 32768

	// Erase the 4 small sectors first. measure how long that takes.


	if(internalState == InternalState::CLEARING_MEMORY) {
		int result = 0;
		if(stepCounter == 0) {

			result = NORFLASH_EraseSector(&NORFlash, NORFLASH_SA0_ADDRESS);
			if(result != 0) {
				return HasReturnvaluesIF::RETURN_FAILED;
			}
		}

		if(countdown->hasTimedOut()) {
			return HasReturnvaluesIF::RETURN_OK;
		}

		if(stepCounter == 1) {
			result = NORFLASH_EraseSector(&NORFlash, NORFLASH_SA1_ADDRESS);
			if(result != 0) {
				return HasReturnvaluesIF::RETURN_FAILED;
			}
		}

		if(countdown->hasTimedOut()) {
			return HasReturnvaluesIF::RETURN_OK;
		}

		if(stepCounter == 2) {
			result = NORFLASH_EraseSector(&NORFlash, NORFLASH_SA2_ADDRESS);
			if(result != 0) {
				return HasReturnvaluesIF::RETURN_FAILED;
			}
		}

		if(countdown->hasTimedOut()) {
			return HasReturnvaluesIF::RETURN_OK;
		}

		if(stepCounter == 3) {
			result = NORFLASH_EraseSector(&NORFlash, NORFLASH_SA3_ADDRESS);
			if(result != 0) {
				return HasReturnvaluesIF::RETURN_FAILED;
			}
		}

		if(countdown->hasTimedOut()) {
			return HasReturnvaluesIF::RETURN_OK;
		}

		if(stepCounter == 4) {
			internalState = InternalState::COPYING;
		}
	}


//    stopwatch.stop(true);
//    stopwatch.start();

	// lets ignore that for now and just write the SD card bootloader to
	// the nor flash. lets assume the name is
	// "bl.bin" and the repository is "BIN"

	SDCardAccess sdCardAccess;


//	result = change_directory("BIN", true);

	currentFileSize = f_filelength("bl.bin");
	F_FILE* bootloader = f_open("bl.bin", "r");

	while(true) {
		// read length of NOR-Flash small section
		f_read(readArray.data(), sizeof(uint8_t), 8192, bootloader);

		// we should consider a critical section here and extracting this function
		// to a special task with the highest priority so it can not be interrupted.

//		result = NORFLASH_WriteData(&NORFlash, NORFLASH_SA0_ADDRESS,
//				readArray.data(), NORFLASH_SMALL_SECTOR_SIZE);
//		if(result != 0) {
//			return HasReturnvaluesIF::RETURN_FAILED;
//		}
	}

    return HasReturnvaluesIF::RETURN_OK;
}

#endif

#ifdef AT91SAM9G20_EK

/// Nandflash memory size.
static unsigned int memSize;
/// Size of one block in the nandflash, in bytes.
static unsigned int blockSize;
/// Number of blocks in nandflash.
static unsigned short numBlocks;
/// Size of one page in the nandflash, in bytes.
static unsigned short pageSize;
/// Number of page per block
static unsigned short numPagesPerBlock;
// Nandflash bus width
static unsigned char nfBusWidth = 16;

/// Pins used to access to nandflash.
static const Pin pPinsNf[] = {PINS_NANDFLASH};
/// Nandflash device structure.
static struct SkipBlockNandFlash skipBlockNf;
/// Address for transferring command bytes to the nandflash.
static unsigned int cmdBytesAddr = BOARD_NF_COMMAND_ADDR;
/// Address for transferring address bytes to the nandflash.
static unsigned int addrBytesAddr = BOARD_NF_ADDRESS_ADDR;
/// Address for transferring data bytes to the nandflash.
static unsigned int dataBytesAddr = BOARD_NF_DATA_ADDR;
/// Nandflash chip enable pin.
static const Pin nfCePin = BOARD_NF_CE_PIN;
/// Nandflash ready/busy pin.
static const Pin nfRbPin = BOARD_NF_RB_PIN;

ReturnValue_t SoftwareImageHandler::copyBootloaderToNandFlash(
        bool performHammingCheck, bool displayInfo) {
    if(not displayInfo) {
        setTrace(TRACE_LEVEL_WARNING);
    }

    if(internalState == InternalState::IDLE) {
    	internalState = InternalState::STEP_1;
    }

    if(internalState == InternalState::STEP_1) {
    	if(stepCounter == 0) {
            BOARD_ConfigureNandFlash(nfBusWidth);
            PIO_Configure(pPinsNf, PIO_LISTSIZE(pPinsNf));
            ReturnValue_t result = nandFlashInit(false);
            if(result != HasReturnvaluesIF::RETURN_OK) {
                sif::error << "SoftwareImageHandler::copyBootloaderToNand"
                		<< "Flash: Error initializing NAND-Flash." << std::endl;
                return HasReturnvaluesIF::RETURN_FAILED;
            }
            stepCounter ++;
    	}

        if(countdown->hasTimedOut()) {
        	return HasReturnvaluesIF::RETURN_OK;
        }

    	if(stepCounter == 1) {
            // First block will be used for bootloader, so we erase it first.
            uint8_t retval = SkipBlockNandFlash_EraseBlock(&skipBlockNf, 0,
                    NORMAL_ERASE);
            if(retval != 0) {
                sif::error << "SoftwareImageHandler::copyBootloaderToNandFlash: "
                         << "Error erasing first block." << std::endl;
                return HasReturnvaluesIF::RETURN_FAILED;
            }
            stepCounter = 0;
    	}

        internalState = InternalState::STEP_2;

        if(countdown->hasTimedOut()) {
        	return HasReturnvaluesIF::RETURN_OK;
        }
    }

    if(internalState == InternalState::STEP_2) {
    	SDCardAccess sdCardAccess;
    	if(not miscFlag) {
    		//SDCardHandler::printSdCard();
    		miscFlag = true;
    		if(countdown->hasTimedOut()) {
    			return HasReturnvaluesIF::RETURN_OK;
    		}
    	}

    	int result = change_directory(config::BOOTLOADER_REPOSITORY, true);
    	if(result != F_NO_ERROR) {
    		// changing directory failed!
    		return HasReturnvaluesIF::RETURN_FAILED;
    	}

    	currentFileSize = f_filelength("BL.BIN");
    	sif::info << "Copying AT91 bootloader on SD card "
    			<< sdCardAccess.currentVolumeId << " to AT91 NAND-Flash.."
				<< std::endl;
    	sif::info << "Bootloader size: " <<  currentFileSize
    			<< " bytes." << std::endl;
    	F_FILE* bootloader = f_open("BL.BIN", "r");
    	if(f_getlasterror() != F_NO_ERROR) {
    		// Opening file failed!
    		return HasReturnvaluesIF::RETURN_FAILED;
    	}
    	size_t sizeToRead = NAND_PAGE_SIZE;

    	while(true) {
    		// If reading or writing failed, the loop will be restarted
    		// to have multiple attempts, so we need to check for a timeout
    		// at the start as well.
    		if(countdown->hasTimedOut()) {
            	return HasReturnvaluesIF::RETURN_OK;
            }

    		if(currentFileSize - currentIdx < NAND_PAGE_SIZE) {
    			sizeToRead = currentFileSize - currentIdx;
    			// set the rest of the buffer which will not be overwritten
    			// to 0.
    			std::memset(readArray.data() + sizeToRead, 0,
    					NAND_PAGE_SIZE - sizeToRead);
    		}

    		ssize_t bytesRead = 0;

    		// If data has been read but still needs to be copied, don't read.
    		if(not dataRead) {
    			bytesRead = f_read(readArray.data(), sizeof(uint8_t),
    					sizeToRead, bootloader);
    			if(bytesRead < 0) {
    				errorCount++;
    				// if reading a file failed 5 times, exit.
    				if(errorCount >= 5) {
    					sif::error << "SoftwareImageHandler::copyBootloader"
    							<< "ToNandFlash: Reading file failed!"
								<< std::endl;
    					return HasReturnvaluesIF::RETURN_FAILED;
    				}
    				// reading file failed.
					continue;
    			}
    		}

    		errorCount = 0;
    		dataRead = true;

    		if(static_cast<size_t>(bytesRead) < sizeToRead) {
    			// should not happen..
    			sif::error << "SoftwareImageHandler::copyBootloaderToNandFlash:"
    					<< " Bytes read smaller than size to read!"
						<< std::endl;
    			return HasReturnvaluesIF::RETURN_FAILED;
    		}

    		if(stepCounter == 0) {
    			// We need to write the size of the binary to the
    			// sixth ARM vector (see p.72 SAM9G20 datasheet)
    			std::memcpy(readArray.data() + 0x14, &currentFileSize,
    					sizeof(uint32_t));
    		}
    		result = SkipBlockNandFlash_WritePage(&skipBlockNf, 0,
    				stepCounter, readArray.data(), NULL);
    		if(result != 0) {
    			errorCount++;
    			if(errorCount >= 5) {
    				// if writing to NAND failed 5 times, exit.
    				sif::error << "SoftwareImageHandler::copyBootloaderToNand"
    						<< "Flash: " << "Error writing to first block."
							<< std::endl;
    				return HasReturnvaluesIF::RETURN_FAILED;
    			}
    			continue;
    		}
    		else {
#ifdef DEBUG
    			sif::debug << "Written " << NAND_PAGE_SIZE << " bytes to "
    					<< "NAND-Flash Block 0 & Page " << stepCounter
						<< std::endl;
#endif
    			dataRead = false;
    			errorCount = 0;
    		}

    		stepCounter++;
    		currentIdx += NAND_PAGE_SIZE;

    		if(currentIdx >= currentFileSize) {
    			// operation finished.
    			sif::info << "Copying bootloader to NAND-Flash finished with "
    					<< stepCounter << " cycles!" << std::endl;
    			operationOngoing = false;
    			stepCounter = 0;
    	        oneShot = false;
    			internalState = InternalState::IDLE;
    			return HasReturnvaluesIF::RETURN_OK;
    		}
    		else if(countdown->hasTimedOut()) {
            	return HasReturnvaluesIF::RETURN_OK;
            }
    	}
    }

    if(not displayInfo) {
        setTrace(TRACE_LEVEL_DEBUG);
    }

    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SoftwareImageHandler::nandFlashInit(bool displayInfo)
{
    // Configure SMC for Nandflash accesses (done each time because of old ROM codes)
    BOARD_ConfigureNandFlash(nfBusWidth);
    PIO_Configure(pPinsNf, PIO_LISTSIZE(pPinsNf));

    //memset(&skipBlockNf, 0, sizeof(skipBlockNf));

    if (SkipBlockNandFlash_Initialize(&skipBlockNf, 0, cmdBytesAddr,
             addrBytesAddr, dataBytesAddr, nfCePin, nfRbPin)) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    // Check the data bus width of the NandFlash
    nfBusWidth = NandFlashModel_GetDataBusWidth((struct NandFlashModel *)&skipBlockNf);

    // Reconfigure bus width
    BOARD_ConfigureNandFlash(nfBusWidth);

    // Get device parameters
    memSize = NandFlashModel_GetDeviceSizeInBytes(&skipBlockNf.ecc.raw.model);
    blockSize = NandFlashModel_GetBlockSizeInBytes(&skipBlockNf.ecc.raw.model);
    numBlocks = NandFlashModel_GetDeviceSizeInBlocks(&skipBlockNf.ecc.raw.model);
    pageSize = NandFlashModel_GetPageDataSize(&skipBlockNf.ecc.raw.model);
    numPagesPerBlock = NandFlashModel_GetBlockSizeInPages(&skipBlockNf.ecc.raw.model);

    if(displayInfo) {
        TRACE_INFO("Size of the whole device in bytes : 0x%x \n\r",
                memSize);
        TRACE_INFO("Size in bytes of one single block of a device : 0x%x \n\r",
                blockSize);
        TRACE_INFO("Number of blocks in the entire device : 0x%x \n\r",
                numBlocks);
        TRACE_INFO("Size of the data area of a page in bytes : 0x%x \n\r",
                pageSize);
        TRACE_INFO("Number of pages in the entire device : 0x%x \n\r",
                numPagesPerBlock);
        TRACE_INFO("Bus width : %d \n\r",nfBusWidth);

    }

    return HasReturnvaluesIF::RETURN_OK;
}
#endif

