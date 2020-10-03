#include "SoftwareImageHandler.h"
#include "ImageCopyingHelper.h"

#include <sam9g20/memory/SDCardHandler.h>
#include <fsfw/tasks/PeriodicTaskIF.h>
#include <fsfw/timemanager/Countdown.h>
#include <fsfw/timemanager/Stopwatch.h>

extern "C" {
#include <hal/Storage/NORflash.h>
}

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


ReturnValue_t SoftwareImageHandler::copySdCardImageToNorFlash(SdCard sdCard,
        ImageSlot imageSlot, bool performHammingCheck) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SoftwareImageHandler::copyNorFlashImageToSdCards(SdCard sdCard,
        ImageSlot imageSlot, bool performHammingCheck) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SoftwareImageHandler::checkNorFlashImage() {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t copySdBootloaderToNorFlash() {
    return HasReturnvaluesIF::RETURN_OK;
}

#else

#endif



