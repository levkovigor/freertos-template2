#include "PvchWorker.h"

PvchWorker::PvchWorker(object_id_t objectId): SystemObject(objectId) {
}

ReturnValue_t PvchWorker::performOperation(uint8_t opCode) {
    // Acquire the semaphore for the first time
    semaphore.acquire();
    while(true) {
        switch(state) {
        case(InternalStates::IDLE): {
            // block and wait for external command.
            semaphore.acquire();
            break;
        }
        }
    }

    // Should never be reached
    return HasReturnvaluesIF::RETURN_FAILED;
}
