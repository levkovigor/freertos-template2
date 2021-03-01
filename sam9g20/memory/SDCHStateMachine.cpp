#include "SDCHStateMachine.h"

SDCHStateMachine::SDCHStateMachine() {
}

ReturnValue_t SDCHStateMachine::performStateMachineStep() {
    return HasReturnvaluesIF::RETURN_OK;
}

SDCHStateMachine::States SDCHStateMachine::getInternalState() const {
    return internalState;
}
