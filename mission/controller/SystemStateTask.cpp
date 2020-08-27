#include "SystemStateTask.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <fsfw/ipc/MutexHelper.h>
#include <fsfw/timemanager/Stopwatch.h>

SystemStateTask::SystemStateTask(object_id_t objectId): SystemObject(objectId) {
}

ReturnValue_t SystemStateTask::performOperation(uint8_t opCode) {
    semaphore->acquire();
    while(true) {
        semaphore->acquire();
        if(taskStatusWritePtr != nullptr and numberOfTasks > 0) {
            uxTaskGetSystemState(taskStatusWritePtr, numberOfTasks, nullptr);
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
}

void SystemStateTask::assignStatusWritePtr(TaskStatus_t *writePtr,
        uint16_t numberOfTasks) {
    this->taskStatusWritePtr = writePtr;
    this->numberOfTasks = numberOfTasks;
}

void SystemStateTask::readSystemState() {
    if(semaphore->getSemaphoreCounter() == 0) {
        semaphore->release();
    }
}

bool SystemStateTask::getSystemStateWasRead() const {
    if(semaphore->getSemaphoreCounter() == 0) {
        return true;
    }
    return false;
}

ReturnValue_t SystemStateTask::initializeAfterTaskCreation() {
    semaphore = new BinarySemaphoreUsingTask();
    return HasReturnvaluesIF::RETURN_OK;
}
