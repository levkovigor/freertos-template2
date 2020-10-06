#include "SoftwareImageHandler.h"
#include <sam9g20/memory/SDCardHandler.h>
#include <fsfw/tasks/PeriodicTaskIF.h>
#include <fsfw/timemanager/Countdown.h>
#include <fsfw/timemanager/Stopwatch.h>
#include <fsfw/ipc/QueueFactory.h>
#include <sam9g20/core/ImageCopyingEngine.h>

#ifdef ISIS_OBC_G20
extern "C" {
#include <hal/Storage/NORflash.h>
}
#endif

SoftwareImageHandler::SoftwareImageHandler(object_id_t objectId):
        SystemObject(objectId), actionHelper(this, nullptr) {
    //oneShot = true;
#ifdef AT91SAM9G20_EK
    imgCpHelper->configureNand(true);
#endif
    receptionQueue = QueueFactory::instance()->createMessageQueue(
            SW_IMG_HANDLER_MQ_DEPTH);
}

ReturnValue_t SoftwareImageHandler::performOperation(uint8_t opCode) {
    countdown->resetTimer();
    CommandMessage message;
    ReturnValue_t result = receptionQueue->receiveMessage(&message);
    if(result == HasReturnvaluesIF::RETURN_OK) {
        result = actionHelper.handleActionMessage(&message);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            sif::debug << "SoftwareImageHandler::performOperation: Unknown"
                    << "  command with command ID " << message.getCommand()
                    << " received!" << std::endl;
        }
    }
    while(countdown->isBusy()) {
        switch(handlerState) {
        case(HandlerState::IDLE): {
            if(oneShot) {
                imgCpHelper->startBootloaderToFlashOperation(false);
                handlerState = HandlerState::COPYING;
                oneShot = false;
            }
            else {
                return HasReturnvaluesIF::RETURN_OK;
            }
            // check for messages or whether periodic scrubbing is necessary
            break;
        }
        case(HandlerState::COPYING): {
            // continue current copy operation.

            result = imgCpHelper->continueCurrentOperation();
            // timeout or failure.
            if(result == TASK_PERIOD_OVER_SOON) {
                return HasReturnvaluesIF::RETURN_OK;
            }
            else if(result == HasReturnvaluesIF::RETURN_FAILED) {
                handlerState = HandlerState::IDLE;
            }
            else {
                // copy op finished
                if(imgCpHelper->getLastFinishedState() == ImageCopyingEngine::
                        ImageHandlerStates::COPY_SDC_BL_TO_FLASH) {
                    imgCpHelper->startSdcToFlashOperation(SdCard::SD_CARD_0,
                            ImageSlot::IMAGE_0);
                }
                else {
                    handlerState = HandlerState::IDLE;
                }
            }
            break;
        }
        case(HandlerState::SCRUBBING): {
            // continue current scrubbing operation.
            break;
        }

        default: {
            break;
        }
        }

    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SoftwareImageHandler::initialize() {
    // set action helper queue here.
    ReturnValue_t result = actionHelper.initialize(receptionQueue);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
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
	countdown = new Countdown(static_cast<float>(
	        this->executingTask->getPeriodMs()) * 0.85);
	imgCpHelper = new ImageCopyingEngine(this, countdown, &imgBuffer);
	return HasReturnvaluesIF::RETURN_OK;
}

void SoftwareImageHandler::setTaskIF(PeriodicTaskIF *executingTask) {
	this->executingTask = executingTask;
}



void SoftwareImageHandler::checkSdCardImage(SdCard sdCard,
        ImageSlot imageSlot) {
}

MessageQueueId_t SoftwareImageHandler::getCommandQueue() const {
    return receptionQueue->getId();
}

ReturnValue_t SoftwareImageHandler::executeAction(ActionId_t actionId,
        MessageQueueId_t commandedBy, const uint8_t *data, size_t size) {
    return HasReturnvaluesIF::RETURN_OK;
}



#ifdef ISIS_OBC_G20

ReturnValue_t SoftwareImageHandler::copySdCardImageToNorFlash(SdCard sdCard,
        ImageSlot imageSlot) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SoftwareImageHandler::copyNorFlashImageToSdCards(SdCard sdCard,
        ImageSlot imageSlot) {
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



