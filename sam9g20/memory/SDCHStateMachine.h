#ifndef SAM9G20_MEMORY_SDCHSTATEMACHINE_H_
#define SAM9G20_MEMORY_SDCHSTATEMACHINE_H_

#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <privlib/hcc/include/hcc/api_fat.h>
#include <sam9g20/memory/SDCardDefinitions.h>

class Countdown;

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
        /* A file is being split into PUS packets for downlinking */
        SPLITTING_FILE,
        COPY_FILE,
        MOVE_FILE
    };

    SDCHStateMachine(Countdown* ownerCountdown);

    /**
     * Continue current operation of state machine. Return HasActionIF::EXECUTION_COMPLETE is an
     * operation was completed successfully, RETURN_FAILED if the algorithm should be cancelled
     * and RETURN_OK if the step was completed but the algorithm is not finished yet.
     * @return
     */
    ReturnValue_t continueCurrentOperation();

    bool setCopyFileOperation(RepositoryPath& sourceRepo, FileName& sourceName,
            RepositoryPath& targetRepo, FileName& targetName);

    void resetAndSetToIdle();

    SDCHStateMachine::States getInternalState() const;

private:
    SDCHStateMachine::States internalState = SDCHStateMachine::States::IDLE;
    std::array<uint8_t, 8 * 1024> fileBuffer;
    Countdown* ownerCountdown = nullptr;

    /* Can be used for various purposes, e.g. as source path and source
    filename for copy operations */
    RepositoryPath path1;
    FileName fileName1;
    /* Can be used for various purposes, e.g. as target path and target filename
    for copy operations */
    RepositoryPath path2;
    FileName fileName2;

    size_t currentByteIdx = 0;
    size_t currentFileSize = 0;
    uint32_t stepCounter = 0;

    void reset();

    ReturnValue_t prepareCopyFileInformation(F_FILE** filePtr);
};


#endif /* SAM9G20_MEMORY_SDCHSTATEMACHINE_H_ */
