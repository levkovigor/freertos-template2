#ifndef SAM9G20_MEMORY_SDCHSTATEMACHINE_H_
#define SAM9G20_MEMORY_SDCHSTATEMACHINE_H_

#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <sam9g20/memory/SDCardDefinitions.h>


/**
 * @brief   Encapsulates the state machine used by the SD card handler, used to keep
 *          the complexity under control.
 */
class SDCHStateMachine {
public:
    /* Internal states of the state machine */
    enum class States {
        /* Nothing to do */
        IDLE,
        /* The active SD card is being changed, so no operation possible */
        CHANGING_ACTIVE_SD_CARD,
        /* A file is being split into PUS packets */
        SPLITTING_FILE,
        /* A file is copied or moved */
        COPYING_MOVING_FILE,
    };

    SDCHStateMachine();

    ReturnValue_t performStateMachineStep();
    SDCHStateMachine::States getInternalState() const;

private:
    SDCHStateMachine::States internalState = SDCHStateMachine::States::IDLE;
    /* Can be used for various purposes, e.g. as source path and source
    filename for copy operations */
    RepositoryPath path1;
    FileName fileName1;
    /* Can be used for various purposes, e.g. as target path and target filename
    for copy operations */
    RepositoryPath path2;
    FileName fileName2;
};


#endif /* SAM9G20_MEMORY_SDCHSTATEMACHINE_H_ */
