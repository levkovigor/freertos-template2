#include "SoftwareImageHandler.h"
#include "ImageCopyingHelper.h"

#include <sam9g20/memory/SDCardHandler.h>
#include <fsfw/tasks/PeriodicTaskIF.h>
#include <fsfw/timemanager/Countdown.h>
#include <fsfw/timemanager/Stopwatch.h>


SoftwareImageHandler::SoftwareImageHandler(object_id_t objectId):
        SystemObject(objectId), actionHelper(this, nullptr) {
    handlerState = HandlerState::COPY_BL;
}

ReturnValue_t SoftwareImageHandler::performOperation(uint8_t opCode) {
	//Stopwatch stopwatch;
	//bool performingOne = false;
	countdown->resetTimer();
    switch(handlerState) {
    case(HandlerState::COPY_BL): {
        break;
    }
    case(HandlerState::COPY_OBSW): {
        break;
    }
    case(HandlerState::SCRUB_BL): {
        break;
    }
    case(HandlerState::SCRUB_OBSW_NOR_FLASH): {
        break;
    }
    default: {
        break;
    }
    }

//
//        if(not displayInfo) {
//            setTrace(TRACE_LEVEL_WARNING);
//        }
//
//        performingOne = true;
//        ReturnValue_t result = copySdCardImageToNandFlash(true, false, true);
//        if(result != HasReturnvaluesIF::RETURN_OK) {
//        	// major error, cancel operation
//        	blCopied = true;
//        }
//
//        if(not displayInfo) {
//            setTrace(TRACE_LEVEL_DEBUG);
//        }
//    }

//    if(not obswCopied and not performingOne) {
//        if(not displayInfo) {
//            setTrace(TRACE_LEVEL_WARNING);
//        }
//
//        ReturnValue_t result = copySdCardImageToNandFlash(false, false, false);
//        if(result != HasReturnvaluesIF::RETURN_OK) {
//        	// major error, cancel operation
//        	obswCopied = false;
//        }
//
//        if(not displayInfo) {
//            setTrace(TRACE_LEVEL_DEBUG);
//        }

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


ReturnValue_t SoftwareImageHandler::initializeAfterTaskCreation() {
	countdown = new Countdown(
			static_cast<float>(this->executingTask->getPeriodMs()) * 0.8);
	imgCpHelper = new ImageCopyingHelper(this, countdown, &imgBuffer);
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


void SoftwareImageHandler::copySdCardImageToNorFlash(SdCard sdCard,
        ImageSlot imageSlot, bool performHammingCheck) {
}

void SoftwareImageHandler::copyNorFlashImageToSdCards(SdCard sdCard,
        ImageSlot imageSlot, bool performHammingCheck) {
}

void SoftwareImageHandler::checkNorFlashImage() {
}

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


	if(internalState == GenericInternalState::CLEARING_MEMORY) {
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
			internalState = GenericInternalState::COPYING;
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



