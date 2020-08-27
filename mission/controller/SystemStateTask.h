#ifndef MISSION_CONTROLLER_SYSTEMSTATETASK_H_
#define MISSION_CONTROLLER_SYSTEMSTATETASK_H_
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/osal/FreeRTOS/BinarySemaphore.h>
#include <fsfw/osal/FreeRTOS/BinSemaphUsingTask.h>
#include <fsfw/tasks/ExecutableObjectIF.h>

/**
 * Low priority task used to get system stats.
 */
class SystemStateTask: public SystemObject,
        public ExecutableObjectIF {
public:
    SystemStateTask(object_id_t objectId);

    void assignStatusWritePtr(TaskStatus_t* writePtr, uint16_t numberOfTasks);
    ReturnValue_t performOperation(uint8_t opCode) override;
    void readSystemState();

    ReturnValue_t initializeAfterTaskCreation() override;
private:
    BinarySemaphoreUsingTask* semaphore = nullptr;

    TaskStatus_t* taskStatusWritePtr = nullptr;
    uint16_t numberOfTasks = 0;
};


#endif /* MISSION_CONTROLLER_SYSTEMSTATETASK_H_ */
