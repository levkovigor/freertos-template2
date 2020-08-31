#include "SystemStateTask.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <fsfw/ipc/MutexHelper.h>
#include <fsfw/timemanager/Stopwatch.h>
#include <fsfw/objectmanager/ObjectManagerIF.h>
#include <sam9g20/core/CoreController.h>
#include <inttypes.h>

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
    	case(InternalState::READING_STATS): {
    		if(numberOfTasks > 0) {
    			uxTaskGetSystemState(taskStatArray.data(), numberOfTasks, nullptr);
    			if(not readOnce) {
    				readOnce = true;
    			}
    		}

    		if(doubleOperationRequested) {
    			internalState = InternalState::GENERATING_STATS;
    			doubleOperationRequested = false;
    		}
    		else {
    			internalState = InternalState::IDLE;
    		}

    		break;
    	}

    	case(InternalState::GENERATING_STATS): {
    		generateStatsCsvAndCheckStack();
    		internalState = InternalState::IDLE;
    		break;
    	}
    	}
    }
    return HasReturnvaluesIF::RETURN_OK;
}


bool SystemStateTask::readAndGenerateStats() {
	doubleOperationRequested = true;
	if(internalState != InternalState::IDLE) {
		return false;
	}

	internalState = InternalState::READING_STATS;
	semaphore->release();
	return true;
}

bool SystemStateTask::readSystemState() {
	if(internalState != InternalState::IDLE) {
		return false;
	}

	internalState = InternalState::READING_STATS;
	semaphore->release();
	return true;
}

bool SystemStateTask::generateStatsAndCheckStack() {
	if(not readOnce or internalState != InternalState::IDLE) {
		return false;
	}

	semaphore->release();
	return true;
}

ReturnValue_t SystemStateTask::initializeAfterTaskCreation() {
    semaphore = new BinarySemaphoreUsingTask();
    numberOfTasks = uxTaskGetNumberOfTasks();
    taskStatArray.reserve(numberOfTasks + 3);
    taskStatArray.resize(numberOfTasks + 3);

    size_t sizeToReserve = (numberOfTasks + 3) * 75;
    statsVector.reserve(sizeToReserve);
    statsVector.resize(sizeToReserve);

    sif::info << "SystemStateTask: " << numberOfTasks << " tasks counted."
    		<< std::endl;
    coreController = objectManager->get<CoreController>(coreControllerId);
    if(coreController == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

void SystemStateTask::generateStatsCsvAndCheckStack() {
    // TODO: write to file directly.
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
    std::string infoColumn = "10 kHz time base used for CPU statistics\n\r";
    std::string headerColumn = "Task Name\tAbsTime [Ticks]\tRelTime [%]\t"
            "LstRemStack [bytes]\n\r";
    std::memcpy(statsVector.data() + statsIdx, infoColumn.data(), infoColumn.size());
    statsIdx += infoColumn.size();
    std::memcpy(statsVector.data() + statsIdx, headerColumn.data(), headerColumn.size());
    statsIdx += headerColumn.size();

    /* For percentage calculations. */
    uptimeTicks /= 100UL;

    for(const auto& task: taskStatArray) {
    	if(task.xHandle == nullptr) {
    		continue;
    	}
    	if(task.usStackHighWaterMark * 4 < STACK_THRESHOLD) {
#ifdef DEBUG
    		sif::debug << "SystemStateTask: "<< task.pcTaskName << " least "
    				<< "remaining stack is " << task.usStackHighWaterMark * 4
					<< " bytes" << std::endl;
#endif
    		uint32_t firstFourLetters = 0;
    		uint32_t secondFourLetters = 0;
    		std::memcpy(&firstFourLetters, task.pcTaskName, sizeof(firstFourLetters));
    		std::memcpy(&secondFourLetters, task.pcTaskName + 4, sizeof(secondFourLetters));
    		triggerEvent(LOW_REM_STACK, firstFourLetters, secondFourLetters);
    	}
        if(task.pcTaskName != nullptr) {
#ifdef DEBUG
        	// human readable format here, tab seperator
        	writeDebugStatLine(task, statsIdx, idleTicks, uptimeTicks);
#else
        	// CSV format here, tab seperator
        	writeCsvStatLine(task, statsIdx, idleTicks, uptimeTicks);
#endif
            statsVector[statsIdx] = '\n';
            statsIdx ++;
            statsVector[statsIdx] = '\r';
            statsIdx ++;
        }
    }
    statsVector[statsIdx] = '\0';
#ifdef DEBUG
    printf("%s\r\n",statsVector.data());
    printf("Number of bytes written: %d\r\n", statsIdx);
#endif

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

