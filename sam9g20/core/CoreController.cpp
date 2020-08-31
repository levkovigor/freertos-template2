#include <fsfw/timemanager/Clock.h>
#include "CoreController.h"

#include <fsfw/timemanager/Stopwatch.h>
#include <sam9g20/common/FRAMApi.h>
#include <systemObjectList.h>

extern "C" {
#include <hal/Timing/Time.h>
#include <hal/Timing/RTT.h>
}

#include <utility/portwrapper.h>
#include <FreeRTOSConfig.h>
#include <utility/compile_time.h>
#include <inttypes.h>

uint32_t CoreController::counterOverflows = 0;
uint32_t CoreController::idleCounterOverflows = 0;

CoreController::CoreController(object_id_t objectId): ControllerBase(objectId,
        objects::NO_OBJECT), actionHelper(this, commandQueue),
        taskStatArray(0) {
#ifdef ISIS_OBC_G20
    sif::info << "CoreController: Starting Supervisor component." << std::endl;
    Supervisor_start(nullptr, 0);
#endif
}

ReturnValue_t CoreController::handleCommandMessage(CommandMessage *message) {
    return actionHelper.handleActionMessage(message);
}

void CoreController::performControlOperation() {

    // First task: get supervisor state
#ifdef ISIS_OBC_G20
    int result = Supervisor_getHousekeeping(&supervisorHk, SUPERVISOR_INDEX);
    if(result != 0) {
        // should not happen!
    }
    supervisor_enable_status_t* temporaryEnable =
            &(supervisorHk.fields.enableStatus);
    if(temporaryEnable) {}
    Supervisor_calculateAdcValues(&supervisorHk, adcValues);
    // now store everything into a local pool. Also take action if any values
    // are out of order.
#endif

    /* Check for overflows of 32bit counter regularly (currently every day).
    the second counter will take 4-5 years to overflow which exceeds
    mission time. */
    uint32_t currentUptimeSeconds = RTT_GetTime();
    if(currentUptimeSeconds - lastCounterUpdateSeconds >= DAY_IN_SECONDS) {
    	update64bitCounter();
    	lastCounterUpdateSeconds = currentUptimeSeconds;
    }

    // Store current uptime in seconds in FRAM, using the FRAM handler.
#ifdef ISIS_OBC_G20
    result = update_seconds_since_epoch(currentUptimeSeconds);
    if( result != 0) {
        // should not happen!
    }
#endif

    if(currentUptimeSeconds - lastDumpSecond >= 20 and not cpuStatDumpPending) {
        cpuStatsDumpRequested = true;
        lastDumpSecond = currentUptimeSeconds;
    }

    if(cpuStatsDumpRequested == true and not cpuStatDumpPending) {
        systemStateTask->readSystemState();
        cpuStatDumpPending = true;
    }


    if(cpuStatDumpPending and systemStateTask->getSystemStateWasRead()) {
        // move this to low prio task, takes rather long..
        Stopwatch stopwatch;
        generateStatsCsvAndCheckStack();
        cpuStatDumpPending = false;
        cpuStatsDumpRequested = false;
    }
}

ReturnValue_t CoreController::checkModeCommand(Mode_t mode, Submode_t submode,
        uint32_t *msToReachTheMode) {
    return HasReturnvaluesIF::RETURN_OK;
}

MessageQueueId_t CoreController::getCommandQueue() const {
    return ControllerBase::getCommandQueue();
}

ReturnValue_t CoreController::executeAction(ActionId_t actionId,
        MessageQueueId_t commandedBy, const uint8_t *data, size_t size) {
	if(actionId == REQUEST_CPU_STATS_CHECK_STACK) {
		if(cpuStatDumpPending or cpuStatsDumpRequested) {
			// that command is still pending.
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		// perform necessary steps to generate CPU stats and dump them.
	    // do this in a low priority task which is unblocked for that purpose.
	    cpuStatsDumpRequested = true;
	}
    return HasReturnvaluesIF::RETURN_OK;
}



uint64_t CoreController::getTotalRunTimeCounter() {
	return static_cast<uint64_t>(counterOverflows) << 32 |
			vGetCurrentTimerCounterValue();
}

uint64_t CoreController::getTotalIdleRunTimeCounter() {
	return static_cast<uint64_t>(idleCounterOverflows) << 32 |
			ulTaskGetIdleRunTimeCounter();
}

void CoreController::update64bitCounter() {
    uint32_t currentCounter = vGetCurrentTimerCounterValue();
    uint32_t currentIdleCounter = ulTaskGetIdleRunTimeCounter();
    if(currentCounter < last32bitCounterValue) {
    	// overflow occured.
    	counterOverflows ++;
    }

    if(currentIdleCounter < last32bitIdleCounterValue) {
    	// overflow occured.
    	idleCounterOverflows ++;
    }

    last32bitCounterValue = currentCounter;
    last32bitIdleCounterValue = currentIdleCounter;
}

void CoreController::generateStatsCsvAndCheckStack() {
    // vTaskGetRunTimeStats();
    // Special 10kHz tick values for total run time and idle run time.
    uint64_t upTimeTicks = getTotalRunTimeCounter();
    uint64_t idleTaskTicks = getTotalIdleRunTimeCounter();

    // we will write to file directly later, write to buffer for now

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
    upTimeTicks /= 100UL;
    // newlib nano does not support 64 bit print, so eventually the printout
    // will become invalid.
    uint32_t idlePrintout = idleTaskTicks & 0xFFFF;
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
                        static_cast<uint32_t>(idleTaskTicks / upTimeTicks),
                        task.usStackHighWaterMark * sizeof(configSTACK_DEPTH_TYPE));
            }
            else {
                statsIdx += std::snprintf((char*)(statsArray.data() + statsIdx),
                        configMAX_TASK_NAME_LEN + 64,
                        "%lu\t\t%lu\t\t%lu", task.ulRunTimeCounter,
                        static_cast<uint32_t>(task.ulRunTimeCounter / upTimeTicks),
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

void CoreController::writePaddedName(uint8_t* buffer,
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

ReturnValue_t CoreController::initializeAfterTaskCreation() {
    setUpSystemStateTask();
    return initializeIsisTimerDrivers();
}

void CoreController::setUpSystemStateTask() {
    numberOfTasks = uxTaskGetNumberOfTasks();
    taskStatArray.reserve(numberOfTasks);
    taskStatArray.resize(numberOfTasks);
    systemStateTask = objectManager->
            get<SystemStateTask>(objects::SYSTEM_STATE_TASK);
    if(systemStateTask != nullptr) {
        systemStateTask->assignStatusWritePtr(taskStatArray.data(),
                numberOfTasks);
    }
    else {
        sif::error << "CoreController::performControlOperation:"
                "System state task invalid!" << std::endl;
    }
}

ReturnValue_t CoreController::initializeIsisTimerDrivers() {
#ifdef ISIS_OBC_G20
    sif::info << "CoreController: Starting RTC and RTT." << std::endl;

    // Time will be set later, this just starts the synchronization task
    // with a frequence of 0.5 seconds.
    int retval = Time_start(NULL, RTC_RTT_SYNC_INTERVAL);
    if(retval >> 8 == 0xFF) {
        sif::error << "CoreController::initializeAfterTaskCreation:"
                "ISIS RTC start failure!" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    retval = retval & 0xFF;
    if(retval == 1) {
        // RTT not ticking. Should not happen! Trigger event.
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    else if(retval == 2) {
        // Should not happen, we did not specify time..
        sif::error << "CoreController::initializeAfterTaskCreation: Config"
                << " error!" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    else if(retval == 3) {
        // Should not happen, the scheduler is already running
        sif::error << "CoreController::initializeAfterTaskCreation: Config"
                << " error, FreeRTOS scheduler not running!" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    uint64_t secSinceEpoch = 0;
    retval = read_seconds_since_epoch(&secSinceEpoch);
    if(retval != 0) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    // if stored time is 0, set the compile time of the binary
    if(secSinceEpoch == 0) {
        secSinceEpoch = __TIME_UNIX__;
        retval = update_seconds_since_epoch(secSinceEpoch);
    }

    sif::info << "CoreController: Setting initial clock." << std::endl;

    timeval currentTime;

//    Time_setUnixEpoch(__TIME_UNIX__);
//    currentTime.tv_sec = __TIME_UNIX__;
//    Clock::setClock(&currentTime);
//    update_seconds_since_epoch(secSinceEpoch);

    // Setting ISIS clock.
    Time_setUnixEpoch(secSinceEpoch);

    // Setting FSFW clock.
    currentTime.tv_sec = secSinceEpoch;
    Clock::setClock(&currentTime);

    sif::info << "CoreController: Clock set." << std::endl;
#else
    RTT_start();
#endif



    return HasReturnvaluesIF::RETURN_OK;
}
