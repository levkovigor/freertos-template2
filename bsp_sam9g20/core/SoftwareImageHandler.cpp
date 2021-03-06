#include "SoftwareImageHandler.h"
#include "ImageCopyingEngine.h"
#include "ScrubbingEngine.h"

#include <bsp_sam9g20/memory/SDCardHandler.h>
#include <bsp_sam9g20/memory/SDCardAccess.h>

#include <fsfw/tasks/PeriodicTaskIF.h>
#include <fsfw/timemanager/Countdown.h>
#include <fsfw/timemanager/Stopwatch.h>
#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/serviceinterface/ServiceInterface.h>

#ifdef ISIS_OBC_G20
#include <bsp_sam9g20/common/fram/FRAMApi.h>
#else
#include <bsp_sam9g20/common/fram/VirtualFRAMApi.h>
#endif

SoftwareImageHandler::SoftwareImageHandler(object_id_t objectId): SystemObject(objectId),
        receptionQueue(QueueFactory::instance()->createMessageQueue(SW_IMG_HANDLER_MQ_DEPTH)),
        actionHelper(this, receptionQueue), parameterHelper(this) {
}

ReturnValue_t SoftwareImageHandler::performOperation(uint8_t opCode) {
    countdown->resetTimer();
    handleMessages();
    performStateMachine();
    return HasReturnvaluesIF::RETURN_OK;
}


void SoftwareImageHandler::handleMessages() {
    CommandMessage message;
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    for(uint8_t idx = 0; idx < MAX_MESSAGES_HANDLED; idx ++) {
        result = receptionQueue->receiveMessage(&message);
        if(result == MessageQueueIF::EMPTY) {
            break;
        }
        else if(result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::warning << "SoftwareImageHandler::performOperation: "
                    "Error receiving message!" << std::endl;
#else
            sif::printWarning("SoftwareImageHandler::performOperation: "
                    "Error receiving message!\n");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
        }

        result = actionHelper.handleActionMessage(&message);
        if(result == HasReturnvaluesIF::RETURN_OK) {
            continue;
        }

        result = parameterHelper.handleParameterMessage(&message);
        if(result == HasReturnvaluesIF::RETURN_OK) {
            continue;
        }
    }
}

void SoftwareImageHandler::performStateMachine() {
    while(countdown->isBusy()) {
        switch(handlerState) {
        case(HandlerState::IDLE): {
            /* Check whether periodic scrubbing is necessary otherwise, return.
            Periodic scrubbing not implemented yet. */
            return;
        }
        case(HandlerState::COPYING): {
            /* Continue current copy operation. */
            ReturnValue_t result = imgCpHelper->continueCurrentOperation();
            /* Timeout or failure. */
            if(result == image::TASK_PERIOD_OVER_SOON) {
                return;
            }
            else if(result == HasReturnvaluesIF::RETURN_FAILED) {
                imgCpHelper->reset();
                handlerState = HandlerState::IDLE;
            }
            else {
                actionHelper.finish(true, recipient, currentAction, HasReturnvaluesIF::RETURN_OK);
                currentAction = 0xffffffff;
                recipient = MessageQueueIF::NO_QUEUE;
                imgCpHelper->reset();
                handlerState = HandlerState::IDLE;
            }
            break;
        }
        case(HandlerState::SCRUBBING): {
            /* Continue current scrubbing operation. */
            break;
        }

        default: {
            break;
        }
        }

    }
}


ReturnValue_t SoftwareImageHandler::executeAction(ActionId_t actionId,
        MessageQueueId_t commandedBy, const uint8_t *data, size_t size) {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    switch(actionId) {
    case(COPY_BOOTLOADER_SDC_TO_FLASH): {
        result = handleCopyingSdcBlToFlash(actionId, commandedBy, data, size);
        break;
    }
    case(COPY_OBSW_SDC_TO_FLASH): {
        result = handleCopyingSdcImgToFlash(actionId, commandedBy, data, size);
        break;
    }
    case(COPY_OBSW_SDC_TO_SDC): {
        result = handleCopyingSdcToSdc(actionId, commandedBy, data, size);
        break;
    }
    case(ENABLE_HAMMING_CODE_CHECK_FOR_COPYING): {
        SDCardAccess access;
        result = fram_set_ham_check_flag();
        if(result != 0) {
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        break;
    }
    case(COPY_HAMMING_SDC_TO_STORAGE): {
        result = handleCopyingHammingToStorage(actionId, commandedBy, data, size);
        break;
    }
    default: {
        return HasActionsIF::INVALID_ACTION_ID;
    }
    }
    return result;
}


ReturnValue_t SoftwareImageHandler::initialize() {
    /* Set action helper queue here. */
    ReturnValue_t result = actionHelper.initialize(receptionQueue);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    result = parameterHelper.initialize();
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

#ifdef ISIS_OBC_G20
    int retval = NORflash_start();

    if(retval != 0) {
        /* Should never happen ! Maybe power cycle if this happens? In any case, trigger event */
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "SoftwareImageHandler::initialize: NOR-Flash start failed"
                << std::endl;
#else
        sif::printError("SoftwareImageHandler::initialize: NOR-Flash start failed\n");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
        return result;
    }
#endif

    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t SoftwareImageHandler::initializeAfterTaskCreation() {
    countdown = new Countdown(static_cast<float>(
            this->executingTask->getPeriodMs()) * 0.75);
    imgCpHelper = new ImageCopyingEngine(this, countdown, &imgBuffer);
    if(imgCpHelper == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    scrubbingEngine = new ScrubbingEngine(this);
    if(scrubbingEngine == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

void SoftwareImageHandler::setTaskIF(PeriodicTaskIF *executingTask) {
    this->executingTask = executingTask;
}

ReturnValue_t SoftwareImageHandler::handleCopyingSdcBlToFlash(ActionId_t actionId,
        MessageQueueId_t commandedBy, const uint8_t *data, size_t size) {
    if(handlerState == HandlerState::COPYING) {
        return HasActionsIF::IS_BUSY;
    }
#if defined(AT91SAM9G20_EK)
    /* No FRAM support yet */
    if(size < 1) {
        return HasActionsIF::INVALID_PARAMETERS;
    }
    image::ImageSlot sourceSlot = image::ImageSlot::NONE;
    if(data[0] == 0) {
        sourceSlot = image::ImageSlot::BOOTLOADER_0;
    }
#if BOOTLOADER_TYPE == BOOTLOADER_TWO_STAGE
    else if(data[0] == 1) {
        sourceSlot = image::ImageSlot::BOOTLOADER_1;

    }
#endif
    else {
        return HasActionsIF::INVALID_PARAMETERS;
    }

    imgCpHelper->startBootloaderToFlashOperation(sourceSlot, false);
#else /* iOBC */

    if(size != 1) {
        return HasActionsIF::INVALID_PARAMETERS;
    }

    bool fromFram = data[0];
    imgCpHelper->startBootloaderToFlashOperation(image::ImageSlot::BOOTLOADER_0, fromFram);
#endif

    currentAction = actionId;
    recipient = commandedBy;
    handlerState = HandlerState::COPYING;
    actionHelper.step(1, commandedBy, actionId, HasReturnvaluesIF::RETURN_OK);
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SoftwareImageHandler::handleCopyingSdcImgToFlash(ActionId_t actionId,
        MessageQueueId_t commandedBy, const uint8_t *data, size_t size) {
    if(handlerState == HandlerState::COPYING) {
        actionHelper.finish(false, commandedBy, actionId, image::BUSY);
    }
    if(size != 1) {
        return HasActionsIF::INVALID_PARAMETERS;
    }

    uint8_t targetBinary = data[0];

    if(targetBinary == 0) {
        imgCpHelper->startSdcToFlashOperation(image::ImageSlot::SDC_SLOT_0);
    }
    else if(targetBinary == 1) {
        imgCpHelper->startSdcToFlashOperation(image::ImageSlot::SDC_SLOT_1);
    }
    else {
        return HasActionsIF::INVALID_PARAMETERS;
    }

    currentAction = actionId;
    recipient = commandedBy;
    handlerState = HandlerState::COPYING;
    actionHelper.step(1, commandedBy, actionId, HasReturnvaluesIF::RETURN_OK);
    return HasReturnvaluesIF::RETURN_OK;
}

void SoftwareImageHandler::checkSdCardImage(SdCard sdCard,
        image::ImageSlot imageSlot) {
}

MessageQueueId_t SoftwareImageHandler::getCommandQueue() const {
    return receptionQueue->getId();
}


ReturnValue_t SoftwareImageHandler::getParameter(uint8_t domainId,
        uint8_t uniqueIdentifier, ParameterWrapper *parameterWrapper,
        const ParameterWrapper *newValues, uint16_t startAtIndex) {
    switch(uniqueIdentifier) {
    case(ParameterIds::HAMMING_CODE_FROM_SDC): {
        /* Actually, we can just virtualize the FRAM on the SD-Card, so I am not sure
        whether we still need this. I will still keep it forn now. */
        parameterWrapper->set(scrubbingEngine->hammingCodeOnSdCard);
        return HasReturnvaluesIF::RETURN_OK;
    }
    default:
        return HasParametersIF::INVALID_IDENTIFIER_ID;
    }
}

#ifdef ISIS_OBC_G20

ReturnValue_t SoftwareImageHandler::copySdCardImageToNorFlash(SdCard sdCard,
        image::ImageSlot imageSlot) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SoftwareImageHandler::copyNorFlashImageToSdCards(SdCard sdCard,
        image::ImageSlot imageSlot) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SoftwareImageHandler::checkNorFlashImage() {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t copySdBootloaderToNorFlash() {
    return HasReturnvaluesIF::RETURN_OK;
}

#else

#endif /* ISIS_OBC_G20 */

ReturnValue_t SoftwareImageHandler::handleCopyingHammingToStorage(ActionId_t actionId,
        MessageQueueId_t commandedBy, const uint8_t *data, size_t size) {
    if(handlerState == HandlerState::COPYING) {
        actionHelper.finish(false, commandedBy, actionId, image::BUSY);
    }

    if(size != 1) {
        return HasActionsIF::INVALID_PARAMETERS;
    }

    /* Each hamming code belongs to an image in a certain slot, and we determine this
    from the raw parameters */
    image::ImageSlot respectiveSlot = image::ImageSlot::NONE;
    uint8_t rawParam = data[0];
#if OBSW_VERBOSE_LEVEL >= 1
    const char* targetPrintout = nullptr;
#endif

    if(rawParam == 0) {
#if OBSW_VERBOSE_LEVEL >= 1
        targetPrintout = "flash memory";
#endif
        respectiveSlot = image::ImageSlot::FLASH;
    }
    else if(rawParam == 1) {
#if OBSW_VERBOSE_LEVEL >= 1
        targetPrintout = "SD-Card slot 0";
#endif
        respectiveSlot = image::ImageSlot::SDC_SLOT_0;
    }
    else if(rawParam == 2) {
#if OBSW_VERBOSE_LEVEL >= 1
        targetPrintout = "SD-Card slot 1";
#endif
        respectiveSlot = image::ImageSlot::SDC_SLOT_1;
    }
    else if(rawParam == 3) {
#if OBSW_VERBOSE_LEVEL >= 1
        targetPrintout = "bootloader";
#endif
        respectiveSlot = image::ImageSlot::BOOTLOADER_0;
    }
    else if(rawParam == 4) {
#if OBSW_VERBOSE_LEVEL >= 1
        targetPrintout = "bootloader 2";
#endif
        respectiveSlot = image::ImageSlot::BOOTLOADER_1;
    }
    else {
        return HasActionsIF::INVALID_PARAMETERS;
    }

#if OBSW_VERBOSE_LEVEL >= 1
    if(targetPrintout != nullptr) {
        sif::printInfo("Received command to copy hamming code of %s to FRAM\n", targetPrintout);
    }
#endif
    ReturnValue_t result = imgCpHelper->startHammingCodeToFramOperation(respectiveSlot);
    handlerState = HandlerState::COPYING;
    recipient = commandedBy;
    actionHelper.step(1, commandedBy, actionId, result);
    return result;
}

ReturnValue_t SoftwareImageHandler::handleCopyingSdcToSdc(ActionId_t actionId,
        MessageQueueId_t commandedBy, const uint8_t *data, size_t size) {
    if(handlerState == HandlerState::COPYING) {
        actionHelper.finish(commandedBy, actionId, image::BUSY);
    }if(size != 1) {
        return HasActionsIF::INVALID_PARAMETERS;
    }

    uint8_t targetBinary = data[0];

    if(targetBinary == 0) {
        imgCpHelper->startSdcToSdcOperation(image::ImageSlot::SDC_SLOT_0);
    }
    else if(targetBinary == 1) {
        imgCpHelper->startSdcToSdcOperation(image::ImageSlot::SDC_SLOT_1);
    }
    else {
        return HasActionsIF::INVALID_PARAMETERS;
    }

    currentAction = actionId;
    recipient = commandedBy;
    handlerState = HandlerState::COPYING;
    actionHelper.step(1, commandedBy, actionId, HasReturnvaluesIF::RETURN_OK);
    return HasReturnvaluesIF::RETURN_OK;
}
