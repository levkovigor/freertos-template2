#ifndef BSP_SAM9G20_DEVICES_PVCH_PVCHWORKER_H_
#define BSP_SAM9G20_DEVICES_PVCH_PVCHWORKER_H_

#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/osal/freertos/BinSemaphUsingTask.h"
#include "fsfw/tasks/ExecutableObjectIF.h"

/**
 * @brief   Worker thread which will handle performing the PVCH experiment and managing involved
 *          sensors and devices
 * @details
 * This worker will be suspended most of the time and will be woken by the #PvchHandler device
 * handler on request.
 * @author  R. Mueller
 */
class PvchWorker: public SystemObject, public ExecutableObjectIF {
public:
    PvchWorker(object_id_t objectId);

    ReturnValue_t performOperation(uint8_t opCode) override;

private:

    enum class InternalStates {
        IDLE,
    };

    InternalStates state = InternalStates::IDLE;

    // This semaphore is used to drive the worker
    BinarySemaphoreUsingTask semaphore;
};

#endif /* BSP_SAM9G20_DEVICES_PVCH_PVCHWORKER_H_ */
