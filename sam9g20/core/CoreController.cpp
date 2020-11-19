#include "CoreController.h"
#include "SystemStateTask.h"

#include <fsfwconfig/OBSWConfig.h>

#include <FreeRTOSConfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/tasks/TaskFactory.h>
#include <fsfw/timemanager/Clock.h>
#include <fsfw/timemanager/Stopwatch.h>
#include <fsfwconfig/objects/systemObjectList.h>
#include <fsfwconfig/OBSWVersion.h>

extern "C" {
#ifdef ISIS_OBC_G20
#include <sam9g20/memory/FRAMHandler.h>
#include <sam9g20/common/FRAMApi.h>
#include <hal/Timing/Time.h>
#else
#include <hal/Timing/RTT.h>
#endif
}

#include <utility/exithandler.h>
#include <portwrapper.h>
#include <utility/compile_time.h>
#include <cinttypes>


uint32_t CoreController::counterOverflows = 0;
uint32_t CoreController::idleCounterOverflows = 0;
uint32_t CoreController::uptimeSeconds = 0;
MutexIF* CoreController::timeMutex = nullptr;

CoreController::CoreController(object_id_t objectId,
        object_id_t systemStateTaskId):
        ExtendedControllerBase(objectId, objects::NO_OBJECT),
        systemStateTaskId(systemStateTaskId) {
	timeMutex = MutexFactory::instance()->createMutex();
#ifdef ISIS_OBC_G20
    sif::info << "CoreController: Starting Supervisor component." << std::endl;
    Supervisor_start(nullptr, 0);
#endif
}

uint32_t CoreController::getUptimeSeconds() {
	MutexHelper(timeMutex, MutexIF::TimeoutType::WAITING, 20);
	return uptimeSeconds;
}

void CoreController::performControlOperation() {
    // First task: Supervisor handling.
	performSupervisorHandling();
	// Second task: All time related handling.
    performPeriodicTimeHandling();
}

ReturnValue_t CoreController::handleCommandMessage(CommandMessage *message) {
    return CommandMessageIF::UNKNOWN_COMMAND;
}


void CoreController::performSupervisorHandling() {
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
}

void CoreController::performPeriodicTimeHandling() {
    timeMutex->lockMutex(MutexIF::TimeoutType::WAITING, 20);
    uint32_t currentUptimeSeconds = updateSecondsCounter();
    timeMutex->unlockMutex();

    /* Dynamic memory allocation is only allowed at software startup */
#if OBSW_MONITOR_ALLOCATION == 1
    if(currentUptimeSeconds > 2 and not
    		config::softwareInitializationComplete) {
    	config::softwareInitializationComplete = true;
    }
#endif

    /* Check for overflows of 10kHz 32bit counter regularly
    (currently every day). */
    if(currentUptimeSeconds - lastFastCounterUpdateSeconds >= DAY_IN_SECONDS) {
        update64bit10kHzCounter();
        lastFastCounterUpdateSeconds = currentUptimeSeconds;
    }
#ifdef ISIS_OBC_G20
    // Store current uptime in seconds in FRAM, using the FRAM handler.
    int result = update_seconds_since_epoch(currentUptimeSeconds);
    if(result != 0) {
        // should not happen!
    }

    uint32_t epoch = 0;
    result = Time_getUnixEpoch(reinterpret_cast<unsigned int*>(&epoch));
    // todo: compare FSFW clock with RTT clock and sync FSFW clock to RTT
    // clock if drift is too high.
#endif
}

uint32_t CoreController::updateSecondsCounter() {
#ifdef AT91SAM9G20_EK
    // We can only use RTT on the AT91, on the iOBC it will be reset
    // constantly.
    uptimeSeconds = RTT_GetTime();
#else
    uint32_t currentUptimeSeconds = 0;
    /* Millisecond count can overflow regularly (around every 50 days) */
    uint32_t uptimeMs = 0;
    Clock::getUptime(&uptimeMs);

    // I am just going to assume that the first uptime encountered is going
    // to be larger than 0 milliseconds.
    if(uptimeMs <= lastUptimeMs) {
    	msOverflowCounter++;
    }
    currentUptimeSeconds /= configTICK_RATE_HZ;

    lastUptimeMs = uptimeMs;
    uptimeSeconds = msOverflowCounter * SECONDS_ON_MS_OVERFLOW +
    		currentUptimeSeconds;
#endif
    return uptimeSeconds;
}

ReturnValue_t CoreController::checkModeCommand(Mode_t mode, Submode_t submode,
        uint32_t *msToReachTheMode) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t CoreController::executeAction(ActionId_t actionId,
        MessageQueueId_t commandedBy, const uint8_t *data, size_t size) {
    switch(actionId) {
    case(REQUEST_CPU_STATS_CHECK_STACK): {
        if(not systemStateTask->readAndGenerateStats()) {
            return HasActionsIF::IS_BUSY;
        }
        actionHelper.finish(commandedBy, actionId,
                HasReturnvaluesIF::RETURN_OK);
        return HasReturnvaluesIF::RETURN_OK;
    }
    case(RESET_OBC): {
#ifdef AT91SAM9G20_EK
        restart();
#else
        int retval = increment_reboot_counter(true, true);
        if(retval != 0) {
            sif::error << "CoreController::executeAction: "
                    << "Incrementing reboot counter failed!" << std::endl;
        }
        supervisor_generic_reply_t reply;
        Supervisor_reset(&reply, SUPERVISOR_INDEX);
#endif
        return HasReturnvaluesIF::RETURN_OK;
    }
    case(POWERCYCLE_OBC): {
#ifdef AT91SAM9G20_EK
        restart();
#else

        int retval = increment_reboot_counter(true, true);
        if(retval != 0) {
            sif::error << "CoreController::executeAction: "
                    << "Incrementing reboot counter failed!" << std::endl;
        }
        supervisor_generic_reply_t reply;
        Supervisor_powerCycleIobc(&reply, SUPERVISOR_INDEX);
#endif
        return HasReturnvaluesIF::RETURN_OK;
    }
    default:
        return HasActionsIF::INVALID_ACTION_ID;
    }
}

uint64_t CoreController::getTotalRunTimeCounter() {
#if configGENERATE_RUN_TIME_STATS == 1
    return static_cast<uint64_t>(counterOverflows) << 32 |
            vGetCurrentTimerCounterValue();
#else
    // return 1 for safety (avoid division by zero)
    return 1;
#endif
}

uint64_t CoreController::getTotalIdleRunTimeCounter() {
#if configGENERATE_RUN_TIME_STATS == 1
    return static_cast<uint64_t>(idleCounterOverflows) << 32 |
            ulTaskGetIdleRunTimeCounter();
#else
    // return 1 for safety (avoid division by zero)
    return 1;
#endif
}

void CoreController::update64bit10kHzCounter() {
#if configGENERATE_RUN_TIME_STATS == 1
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
#endif
}


ReturnValue_t CoreController::initializeAfterTaskCreation() {
    ReturnValue_t result = ExtendedControllerBase::initializeAfterTaskCreation();
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    result = setUpSystemStateTask();
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

#ifdef ISIS_OBC_G20
    bool bootloaderIncremented = false;
    int retval = verify_reboot_flag(&bootloaderIncremented);
    if(retval != 0) {
        sif::error << "CoreController::initialize: Error verifying the boot"
                << " counter!" << std::endl;
    }
#endif
    return initializeIsisTimerDrivers();
}

ReturnValue_t CoreController::initialize() {
#ifdef ISIS_OBC_G20
    framHandler = objectManager->get<FRAMHandler>(objects::FRAM_HANDLER);
    if(framHandler == nullptr) {
        sif::error << "CoreController::initialize: No FRAM handler found!"
                << std::endl;
    }
#endif
    return ExtendedControllerBase::initialize();
}


ReturnValue_t CoreController::setUpSystemStateTask() {
    systemStateTask = objectManager->
            get<SystemStateTask>(systemStateTaskId);
    if(systemStateTask == nullptr) {
        sif::error << "CoreController::performControlOperation:"
                "System state task invalid!" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
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

    uint32_t secSinceEpoch = 0;
    retval = read_seconds_since_epoch(&secSinceEpoch);
    if(retval != 0) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    // if stored time is 0, set the compile time of the binary
    if(secSinceEpoch == 0) {
        secSinceEpoch = UNIX_TIMESTAMP;
        retval = update_seconds_since_epoch(secSinceEpoch);
    }

    //sif::info << "CoreController: Setting initial clock." << std::endl;

    timeval currentTime;

    // Setting ISIS clock.
    Time_setUnixEpoch(secSinceEpoch);

    // Setting FSFW clock.
    currentTime.tv_sec = secSinceEpoch;
    Clock::setClock(&currentTime);

    sif::info << "CoreController: Clock set." << std::endl;
#else
    RTT_start();
    timeval currentTime;
    uint32_t secSinceEpoch = UNIX_TIMESTAMP;
    currentTime.tv_sec = secSinceEpoch;
    Clock::setClock(&currentTime);
#endif



    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t CoreController::initializeLocalDataPool(
        LocalDataPool &localDataPoolMap, LocalDataPoolManager &poolManager) {
    return HasReturnvaluesIF::RETURN_OK;
}

LocalPoolDataSetBase* CoreController::getDataSetHandle(sid_t sid) {
    return nullptr;
}
