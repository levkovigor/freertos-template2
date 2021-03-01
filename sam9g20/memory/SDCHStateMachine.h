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
        /* Attempt to change the active SD card */
        CHANGING_SD_CARD,
        /* A file is being split into PUS packets for downlinking */
        SPLITTING_FILE,
        COPY_FILE,
        MOVE_FILE
    };

    bool setToAttemptSdCardChange();

    void resetAndSetToIdle();

    SDCHStateMachine();

    /**
     * Perform a step. Return HasActionIF::EXECUTION_COMPLETE is an operation was completed
     * successfully, RETURN_FAILED if the algorithm should be cancelled and RETURN_OK
     * if the step was completed but the algorithm is not finished yet.
     * @return
     */
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

    void reset();
};


#endif /* SAM9G20_MEMORY_SDCHSTATEMACHINE_H_ */
