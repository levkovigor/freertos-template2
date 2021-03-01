#include "SDCHStateMachine.h"
#include "SDCardHandlerDefinitions.h"

#include <fsfw/action/HasActionsIF.h>
#include <sam9g20/memory/SDCAccessManager.h>


SDCHStateMachine::SDCHStateMachine() {
}

ReturnValue_t SDCHStateMachine::performStateMachineStep() {
    switch(internalState) {
    case(this->States::IDLE): {
        break;
    }
    case(this->States::CHANGING_SD_CARD): {
        bool success = SDCardAccessManager::instance()->tryActiveSdCardChange();
        if(not success) {
            /* Reattempt next cycle */
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        this->resetAndSetToIdle();
        return sdchandler::EXECUTION_COMPLETE;
    }
    case(this->States::SPLITTING_FILE): {
        break;
    }
    case(this->States::COPY_FILE): {
        break;
    }
    case(this->States::MOVE_FILE): {
        break;
    }
    default: {
        break;
    }
    }

    return HasReturnvaluesIF::RETURN_OK;
}

bool SDCHStateMachine::setToAttemptSdCardChange() {
    if(internalState != this->States::IDLE) {
        if(internalState == this->States::CHANGING_SD_CARD) {
            return false;
        }
        else {
            /* Cancel ongoing operation */
            this->resetAndSetToIdle();
        }
    }
    this->internalState = this->States::CHANGING_SD_CARD;
    return true;
}

void SDCHStateMachine::resetAndSetToIdle() {
    internalState = this->States::IDLE;
    this->reset();
}

void SDCHStateMachine::reset() {
}

SDCHStateMachine::States SDCHStateMachine::getInternalState() const {
    return internalState;
}
