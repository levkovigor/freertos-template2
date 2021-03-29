#include <fsfw/memory/HasFileSystemIF.h>
#include "SystemStateTask.h"

#include "SystemStateTask.h"
#include <OBSWConfig.h>
#include <FreeRTOSConfig.h>
#include <FSFWConfig.h>

#include <fsfw/serviceinterface/ServiceInterfacePrinter.h>
#include <fsfw/serviceinterface/serviceInterfaceDefintions.h>
#include <fsfw/objectmanager/ObjectManagerIF.h>
#include <fsfw/tasks/TaskFactory.h>
<<<<<<< HEAD:sam9g20/core/SystemStateTask.cpp
#include <sam9g20/core/CoreController.h>
=======
#include <bsp_sam9g20/core/CoreController.h>
>>>>>>> mueller/master:bsp_sam9g20/core/SystemStateTask.cpp
#include <fsfw/storagemanager/StorageManagerIF.h>


#include <inttypes.h>
#include <mission/memory/FileSystemMessage.h>
<<<<<<< HEAD:sam9g20/core/SystemStateTask.cpp
#include <sam9g20/memory/SDCardDefinitions.h>
#include <sam9g20/memory/SDCardHandlerPackets.h>
=======
#include <bsp_sam9g20/memory/sdcardDefinitions.h>
#include <bsp_sam9g20/memory/SDCardHandlerPackets.h>
>>>>>>> mueller/master:bsp_sam9g20/core/SystemStateTask.cpp


SystemStateTask::SystemStateTask(object_id_t objectId,
        object_id_t coreControllerId): SystemObject(objectId),
        internalState(InternalState::IDLE), coreControllerId(coreControllerId),
        statsVector(0), taskStatArray(0)  {
}

ReturnValue_t SystemStateTask::performOperation(uint8_t opCode) {
    semaphore->acquire();
    while(true) {
        switch(internalState) {
        case(InternalState::IDLE): {
            // block and wait for external command.
            semaphore->acquire();
            break;
        }
        case(InternalState::GENERATING_STATS_CSV):
        case(InternalState::GENERATING_STATS_PRINT): {

            if(numberOfTasks > 0) {
#if configGENERATE_RUN_TIME_STATS == 1
                // liest Daten um Runtime Stats zu generieren
                uxTaskGetSystemState(taskStatArray.data(),
                        numberOfTasks, nullptr);
#else
                sif::info << "SystemStateTask::Generation of run time stats "
                        << "disabled!" << std::endl;
                internalState = InternalState::IDLE;
#endif
            }
            else {
                internalState = InternalState::IDLE;
                sif::printWarning("SystemStateTask::performOperation: Number of tasks is 0!\n");
                break;
            }

            performStatsGeneration(internalState);
            internalState = InternalState::IDLE;

            break;
        }
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
}


bool SystemStateTask::generateStatsCsv() {
    if(internalState != InternalState::IDLE) {
        return false;
    }

    internalState = InternalState::GENERATING_STATS_CSV;
    semaphore->release();
    return true;
}

bool SystemStateTask::generateStatsPrint() {
    if(internalState != InternalState::IDLE) {
        return false;
    }

    internalState = InternalState::GENERATING_STATS_PRINT;
    semaphore->release();
    return true;
}

ReturnValue_t SystemStateTask::initializeAfterTaskCreation() {
    ipcStore = objectManager->get<StorageManagerIF>(objects::IPC_STORE);
    if (ipcStore == nullptr) {
        sif::printWarning("SystemStateTask::initializeAfterTaskCreation: No IPC store found\n");
    }
    semaphore = new BinarySemaphoreUsingTask();
    numberOfTasks = uxTaskGetNumberOfTasks();
    taskStatArray.reserve(numberOfTasks + 3);
    taskStatArray.resize(numberOfTasks + 3);


    size_t sizeToReserve = (numberOfTasks + 3) * 75;
    statsVector.reserve(sizeToReserve);
    statsVector.resize(sizeToReserve);

    HasFileSystemIF* sdCardHandler = objectManager->get<HasFileSystemIF>(objects::SD_CARD_HANDLER);
    if (sdCardHandler == nullptr) {
        sif::printError("SystemStateTask::initializeAfterTaskCreation: "
                "SD Card Handler does not exist\n");
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    queueId = sdCardHandler->getCommandQueue();

    coreController = objectManager->get<CoreController>(coreControllerId);
    if(coreController == nullptr) {
        sif::printError("SystemStateTask::initializeAfterTaskCreation: "
                "Core Controller does not exist\n");
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    /* To prevent mangled output */
    TaskFactory::delayTask(5);
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "SystemStateTask: " << numberOfTasks << " tasks counted." << std::endl;
#else
    sif::printInfo("SystemStateTask: %hu tasks counted.\n", numberOfTasks);
#endif
    return HasReturnvaluesIF::RETURN_OK;
}

void SystemStateTask::performStatsGeneration(InternalState csvOrPrint) {
    if (statsVector.size() == 0) {
        sif::printWarning("SystemStateTask::performStatsGeneration size of statsVector is 0");
        return;
    }
    uint64_t uptimeTicks = coreController->getTotalRunTimeCounter();
    uint64_t idleTicks = coreController->getTotalIdleRunTimeCounter();

    Clock::TimeOfDay_t loggerTime;
    Clock::getDateAndTime(&loggerTime);
    size_t statsIdx = 0;
    statsIdx += std::sprintf(reinterpret_cast<char*>(statsVector.data()),
            "Time: %02" SCNu32 ".%02" SCNu32  ".%02" SCNu32 " %02" SCNu32
            ":%02" SCNu32 ":%02" SCNu32 "\n\r",
            loggerTime.day,
            loggerTime.month,
            loggerTime.year,
            loggerTime.hour,
            loggerTime.minute,
            loggerTime.second);
    const char* const infoColumn = "10 kHz time base used for CPU statistics\n\r";
    size_t infoSize = strlen(infoColumn);
    if(infoSize > 50) {
        infoSize = 50;
    }
    const char* const headerColumn = "Task Name\tAbsTime [Ticks]\tRelTime [%]\t"
            "LstRemStack [bytes]\n\r";
    size_t headerSize = strlen(headerColumn);
    if(headerSize > 70) {
        headerSize = 70;
    }

    std::memcpy(statsVector.data() + statsIdx, infoColumn, infoSize);
    statsIdx += infoSize;
    std::memcpy(statsVector.data() + statsIdx, headerColumn, headerSize);
    statsIdx += headerSize;

    /* For percentage calculations. */
    uptimeTicks /= 100UL;

    for(const auto& task: taskStatArray) {
        if(task.xHandle == nullptr) {
            continue;
        }
        if(task.usStackHighWaterMark * 4 < STACK_THRESHOLD and
                std::strcmp(task.pcTaskName, "IDLE") != 0) {
            uint32_t firstFourLetters = 0;
            uint32_t secondFourLetters = 0;
            std::memcpy(&firstFourLetters, task.pcTaskName,
                    sizeof(firstFourLetters));
            std::memcpy(&secondFourLetters, task.pcTaskName + 4,
                    sizeof(secondFourLetters));
            triggerEvent(LOW_REM_STACK, firstFourLetters, secondFourLetters);
        }

        if(task.pcTaskName != nullptr) {

            if (csvOrPrint == InternalState::GENERATING_STATS_CSV) {
                writeCsvStatLine(task, statsIdx, idleTicks, uptimeTicks);
            }
            else {
                writeDebugStatLine(task, statsIdx, idleTicks, uptimeTicks);
            }
            statsVector[statsIdx] = '\n';
            statsIdx ++;
            statsVector[statsIdx] = '\r';
            statsIdx ++;
        }
    }
    statsVector[statsIdx] = '\0';
    statsIdx ++;

    if (csvOrPrint == InternalState::GENERATING_STATS_PRINT) {
#if OBSW_VERBOSE_LEVEL >= 1
        printf("%s%s\r\n", sif::ANSI_COLOR_RESET, statsVector.data());
        printf("Number of bytes written: %d\r\n", statsIdx);
#endif
        return;
    }

    /* It is assumed that the datasize doesnt exceed 4096 bytes */
    if (ipcStore == nullptr) {
        return;
    }
    size_t sizeToAddToStore = 0;
    store_address_t storeId;
    size_t placeholderStatsIdx = statsIdx;
    size_t maxFileSizePerBucket = config::STORE_VERY_LARGE_BUCKET_SIZE - 40;
    uint8_t* storeDestination = nullptr;
    RepositoryPath repositoryPath = "MISC";
    FileName fileName = "stats";
    char str[4];
    sprintf(str, "%d", csvCounter);

    fileName.append(str, 3);
    fileName += ".csv";
    sif::printInfo("%s\n", fileName);
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    bool breakOnFinish = false;
    auto writeType = WriteCommand::WriteType::NEW_FILE;

    for(uint8_t idx = 0; idx < 3; idx++) {
        if (placeholderStatsIdx % maxFileSizePerBucket == placeholderStatsIdx) {
            breakOnFinish = true;
            sizeToAddToStore = placeholderStatsIdx;
        }
        else {
            sizeToAddToStore = maxFileSizePerBucket;
        }

        if (idx > 0) {
            writeType = WriteCommand::WriteType::APPEND_TO_FILE;
        }
        result = ipcStore->getFreeElement(&storeId, sizeToAddToStore,
                &storeDestination);

        if (result != HasReturnvaluesIF::RETURN_OK) {
            sif::printWarning("SystemStateTask::performStatsGeneration: "
                    "Could not get free element from store\n");
            break;
        }
        WriteCommand writeCommand(repositoryPath, fileName,
                statsVector.data() + idx * maxFileSizePerBucket, sizeToAddToStore, writeType);

        size_t serializedSize = 0;
        result = writeCommand.serialize(&storeDestination, &serializedSize,
                config::STORE_VERY_LARGE_BUCKET_SIZE, SerializeIF::Endianness::MACHINE);

        if (result != HasReturnvaluesIF::RETURN_OK) {
            sif::printWarning("SystemStateTask::performStatsGeneration: "
                    "Could not serialize\n");
            ipcStore->deleteData(storeId);
            break;
        }
        CommandMessage fileCommand;
        FileSystemMessage::setCreateFileCommand(&fileCommand, storeId);
        result = MessageQueueSenderIF::sendMessage(queueId, &fileCommand);
        if (result != HasReturnvaluesIF::RETURN_OK) {
            sif::printWarning("SystemStateTask::performStatsGeneration: "
                    "Could not send CSV message\n");
            ipcStore->deleteData(storeId);
            break;
        }

        if (breakOnFinish) {
            break;
        }
        placeholderStatsIdx = placeholderStatsIdx - sizeToAddToStore;
    }

}

void SystemStateTask::writeDebugStatLine(const TaskStatus_t& task,
        size_t& statsIdx, uint64_t idleTicks, uint64_t uptimeTicks) {
    writePaddedName(static_cast<uint8_t*>(statsVector.data() + statsIdx),
            static_cast<const char*>(task.pcTaskName));
    statsIdx += configMAX_TASK_NAME_LEN;
    // newlib nano does not support 64 bit print, so eventually the printout
    // will become invalid.
    if(std::strcmp(task.pcTaskName, "IDLE") == 0) {
        statsIdx += std::snprintf((char*)(statsVector.data() + statsIdx),
                configMAX_TASK_NAME_LEN + 64,
                "%lu\t\t\%lu\t\t%lu", static_cast<uint32_t>(idleTicks & 0xFFFF),
                static_cast<uint32_t>(idleTicks / uptimeTicks),
                task.usStackHighWaterMark * sizeof(configSTACK_DEPTH_TYPE));
    }
    else {
        statsIdx += std::snprintf((char*)(statsVector.data() + statsIdx),
                configMAX_TASK_NAME_LEN + 64,
                "%lu\t\t%lu\t\t%lu", task.ulRunTimeCounter,
                static_cast<uint32_t>(task.ulRunTimeCounter / uptimeTicks),
                task.usStackHighWaterMark * sizeof(configSTACK_DEPTH_TYPE));
    }
}

void SystemStateTask::writeCsvStatLine(const TaskStatus_t& task,
        size_t& statsIdx, uint64_t idleTicks, uint64_t uptimeTicks) {

    if(std::strcmp(task.pcTaskName, "IDLE") == 0) {
        statsIdx += std::snprintf((char*)(statsVector.data() + statsIdx),
                configMAX_TASK_NAME_LEN + 64,
                "%s,%lu,%lu,%lu", task.pcTaskName,
                static_cast<uint32_t>(idleTicks & 0xFFFF),
                static_cast<uint32_t>(idleTicks / uptimeTicks),
                task.usStackHighWaterMark * sizeof(configSTACK_DEPTH_TYPE));
    }
    else {
        statsIdx += std::snprintf((char*)(statsVector.data() + statsIdx),
                configMAX_TASK_NAME_LEN + 64,
                "%s,%lu,%lu,%lu", task.pcTaskName, task.ulRunTimeCounter,
                static_cast<uint32_t>(task.ulRunTimeCounter / uptimeTicks),
                task.usStackHighWaterMark * sizeof(configSTACK_DEPTH_TYPE));
    }
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

