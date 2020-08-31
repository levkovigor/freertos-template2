#include "SystemStateTask.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <fsfw/ipc/MutexHelper.h>
#include <fsfw/timemanager/Stopwatch.h>
#include <fsfw/objectmanager/ObjectManagerIF.h>
#include <sam9g20/core/CoreController.h>

SystemStateTask::SystemStateTask(object_id_t objectId,
        object_id_t coreControllerId): SystemObject(objectId),
        internalState(InternalState::STATS_GENERATED),
        coreControllerId(coreControllerId), taskStatArray(0) {
}

ReturnValue_t SystemStateTask::performOperation(uint8_t opCode) {
    semaphore->acquire();
    while(true) {
        // Task will be unblocked by other task
        semaphore->acquire();
        internalState = InternalState::READING_STATS;
        if(taskStatusWritePtr != nullptr and numberOfTasks > 0) {
            uxTaskGetSystemState(taskStatusWritePtr, numberOfTasks, nullptr);
        }
//        internalState = InternalState::STATS_READ;
//        // Task will be unblocked by other task
//        semaphore->acquire();
//        internalState = InternalState::GENERATING_STATS;
//        generateStatsCsvAndCheckStack();
//        internalState = InternalState::STATS_GENERATED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}


void SystemStateTask::readAndGenerateStats() {
}

void SystemStateTask::readSystemState() {
    if(internalState == InternalState::READING_STATS) {
        return;
    }

    if(internalState == InternalState::GENERATING_STATS) {
        internalState = InternalState::READING_STATS;
        return;
    }

    if(semaphore->getSemaphoreCounter() == 0) {
        semaphore->release();
    }
}

bool SystemStateTask::getSystemStateWasRead() const {
    if(internalState == InternalState::READING_STATS) {
        return false;
    }

    if(semaphore->getSemaphoreCounter() == 0) {
        return true;
    }
    return false;
}

ReturnValue_t SystemStateTask::initializeAfterTaskCreation() {
    semaphore = new BinarySemaphoreUsingTask();
    numberOfTasks = uxTaskGetNumberOfTasks();
    taskStatArray.reserve(numberOfTasks);
    taskStatArray.resize(numberOfTasks);
    coreController = objectManager->get<CoreController>(coreControllerId);
    if(coreController == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

void SystemStateTask::generateStatsAndCheckStack() {
}

bool SystemStateTask::getSystemStateStatsWereGenerated() const {
    return true;
}

void SystemStateTask::generateStatsCsvAndCheckStack() {
    // we will write to file directly later, write to buffer for now
    uint64_t uptimeTicks = coreController->getTotalRunTimeCounter();
    uint64_t idleTicks = coreController->getTotalIdleRunTimeCounter();

    std::string timestamp = "placeholder for timestamp\n\r";
    std::string infoColumn = "10 kHz time base used for CPU statistics\n\r";
    std::string headerColumn = "Task Name\tAbsTime [Ticks]\tRelTime [%]\t"
            "LstRemStack [bytes]\n\r";
    size_t statsIdx = 0;
    std::memcpy(statsArray.data() + statsIdx, timestamp.data(), timestamp.size());
    statsIdx += timestamp.size();
    std::memcpy(statsArray.data() + statsIdx, infoColumn.data(), infoColumn.size());
    statsIdx += infoColumn.size();
    std::memcpy(statsArray.data() + statsIdx, headerColumn.data(), headerColumn.size());
    statsIdx += headerColumn.size();

    /* For percentage calculations. */
    uptimeTicks /= 100UL;
    // newlib nano does not support 64 bit print, so eventually the printout
    // will become invalid.
    uint32_t idlePrintout = idleTicks & 0xFFFF;
    for(const auto& task: taskStatArray) {
        // TODO: check whether any stack is too close to overflowing.
        if(task.pcTaskName != nullptr) {
            // human readable format here, tab seperator
#ifdef DEBUG
            writePaddedName(static_cast<uint8_t*>(statsArray.data() + statsIdx),
                    static_cast<const char*>(task.pcTaskName));
            statsIdx += configMAX_TASK_NAME_LEN;
            if(std::strcmp(task.pcTaskName, "IDLE") == 0) {
                statsIdx += std::snprintf((char*)(statsArray.data() + statsIdx),
                        configMAX_TASK_NAME_LEN + 64,
                        "%lu\t\t\%lu\t\t%lu", idlePrintout,
                        static_cast<uint32_t>(idleTicks / uptimeTicks),
                        task.usStackHighWaterMark * sizeof(configSTACK_DEPTH_TYPE));
            }
            else {
                statsIdx += std::snprintf((char*)(statsArray.data() + statsIdx),
                        configMAX_TASK_NAME_LEN + 64,
                        "%lu\t\t%lu\t\t%lu", task.ulRunTimeCounter,
                        static_cast<uint32_t>(task.ulRunTimeCounter / uptimeTicks),
                        task.usStackHighWaterMark * sizeof(configSTACK_DEPTH_TYPE));
            }
#else
            // TODO: CSV format here, no padding, comma seperator
#endif
            statsArray[statsIdx] = '\n';
            statsIdx ++;
            statsArray[statsIdx] = '\r';
            statsIdx ++;
        }
    }
    statsArray[statsIdx] = '\0';
    printf("%s\r\n",statsArray.data());
    printf("Number of bytes written: %d\r\n", statsIdx);
}


void SystemStateTask::writePaddedName(uint8_t* buffer,
        const char *pcTaskName) {
    /* Start by copying the entire string. */
    size_t bytesWritten = std::snprintf((char*) buffer,
            configMAX_TASK_NAME_LEN, pcTaskName);

    //buffer[bytesWritten++] = ',';

    /* Pad the end of the string with spaces to ensure columns line up when
    printed out. */
    for(uint32_t x = bytesWritten; x < configMAX_TASK_NAME_LEN; x++ )
    {
        buffer[x] = ' ';
        bytesWritten ++;
    }

    /* Terminate. */
    buffer[bytesWritten++] = ( char ) 0x00;
}

