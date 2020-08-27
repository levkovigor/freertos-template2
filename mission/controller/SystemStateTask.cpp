#include "SystemStateTask.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <fsfw/ipc/MutexHelper.h>
#include <fsfw/timemanager/Stopwatch.h>

SystemStateTask::SystemStateTask(object_id_t objectId): SystemObject(objectId) {
    //mutex = MutexFactory::instance()->createMutex();
}

ReturnValue_t SystemStateTask::performOperation(uint8_t opCode) {
    //MutexHelper(mutex, MutexIF::WAITING, 5);
    //semaphore.acquire();
    if(semaphore == nullptr) {
        semaphore = new BinarySemaphoreUsingTask();
    }
    semaphore->acquire();
    while(true) {
        semaphore->acquire();
        if(taskStatusWritePtr != nullptr and numberOfTasks > 0
                and xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
            //Stopwatch stopwatch;
            uxTaskGetSystemState(taskStatusWritePtr, numberOfTasks, nullptr);
            if(not filledOnce) {
                filledOnce = true;
            }
        }
    }

    return HasReturnvaluesIF::RETURN_OK;
}

void SystemStateTask::assignStatusWritePtr(TaskStatus_t *writePtr,
        uint16_t numberOfTasks) {
    //MutexHelper(mutex, MutexIF::WAITING, 5);
    this->taskStatusWritePtr = writePtr;
    this->numberOfTasks = numberOfTasks;
}

void SystemStateTask::readSystemState() {
    uint8_t test = semaphore->getSemaphoreCounter();
    if(test == 0) {
        semaphore->release();
    }

}

MutexIF* SystemStateTask::getMutexHandle() {
    //return mutex;
}

ReturnValue_t SystemStateTask::initializeAfterTaskCreation() {

    return HasReturnvaluesIF::RETURN_OK;
}
