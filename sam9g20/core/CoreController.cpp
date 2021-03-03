#include "CoreController.h"
#include "SystemStateTask.h"

#include <OBSWConfig.h>
#include <OBSWVersion.h>
#include <objects/systemObjectList.h>

#include <FreeRTOSConfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/tasks/TaskFactory.h>
#include <fsfw/timemanager/Clock.h>
#include <fsfw/timemanager/Stopwatch.h>

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
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "CoreController: Starting Supervisor component." << std::endl;
#else
    sif::printInfo("CoreController: Starting Supervisor component.\n");
#endif
    Supervisor_start(nullptr, 0);
#endif
}

void CoreController::performControlOperation() {
    /* First task: Supervisor handling. */
    performSupervisorHandling();
    /* Second task: All time related handling. */
    performPeriodicTimeHandling();
}

ReturnValue_t CoreController::handleCommandMessage(CommandMessage *message) {
    return CommandMessageIF::UNKNOWN_COMMAND;
}


void CoreController::performSupervisorHandling() {
#ifdef ISIS_OBC_G20
    int result = Supervisor_getHousekeeping(&supervisorHk, SUPERVISOR_INDEX);
    if(result != 0) {
        /* should not happen! */
        triggerEvent(SUPERVISOR_FAILURE);
    }
    supervisor_enable_status_t* temporaryEnable =
            &(supervisorHk.fields.enableStatus);
    if(temporaryEnable) {}
    Supervisor_calculateAdcValues(&supervisorHk, adcValues);
    /* now store everything into a local pool. Also take action if any values are out of order. */
#endif
}

ReturnValue_t CoreController::checkModeCommand(Mode_t mode, Submode_t submode,
        uint32_t *msToReachTheMode) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t CoreController::executeAction(ActionId_t actionId,
        MessageQueueId_t commandedBy, const uint8_t *data, size_t size) {
    switch(actionId) {
    case(REQUEST_CPU_STATS_CSV): {
            if(not systemStateTask->generateStatsCsv()) {
                return HasActionsIF::IS_BUSY;
            }

            actionHelper.finish(true, commandedBy, actionId, HasReturnvaluesIF::RETURN_OK);
            return HasReturnvaluesIF::RETURN_OK;
        }
    case(REQUEST_CPU_STATS_PRINT): {
        if(not systemStateTask->generateStatsPrint()) {
            return HasActionsIF::IS_BUSY;
        }
        actionHelper.finish(true, commandedBy, actionId, HasReturnvaluesIF::RETURN_OK);
        return HasReturnvaluesIF::RETURN_OK;
    }
    case(RESET_OBC): {
#ifdef AT91SAM9G20_EK
        restart();
#else
        supervisor_generic_reply_t reply;
        Supervisor_reset(&reply, SUPERVISOR_INDEX);
#endif
        return HasReturnvaluesIF::RETURN_OK;
    }
    case(POWERCYCLE_OBC): {
#ifdef AT91SAM9G20_EK
        restart();
#else
        supervisor_generic_reply_t reply;
        Supervisor_powerCycleIobc(&reply, SUPERVISOR_INDEX);
#endif
        return HasReturnvaluesIF::RETURN_OK;
    }
    case(CLEAR_STORE_PAGE): {
        if (size > 2 or size < 1) {
            return HasActionsIF::INVALID_PARAMETERS;
        }
        return handleClearStoreCommand(static_cast<Stores>(data[STORE_TYPE]), actionId,
                data[PAGE_INDEX]);
    }
    case(CLEAR_WHOLE_STORE): {
        if (size > 2 or size < 1) {
            return HasActionsIF::INVALID_PARAMETERS;
        }
        return handleClearStoreCommand(static_cast<Stores>(data[STORE_TYPE]), actionId, 0);
    }
    case(GET_FILL_COUNT): {
        if (size != 1) {
            return HasActionsIF::INVALID_PARAMETERS;
        }
        return getFillCountCommand(static_cast<Stores>(data[STORE_TYPE]), commandedBy, actionId);
    }
    case(PRINT_FRAM_CRIT_BLOCK): {
        FRAMHandler::printCriticalBlock();
        return HasActionsIF::EXECUTION_FINISHED;
    }
    case(ZERO_OUT_FRAM_DEFAULT_ZERO_FIELD): {
        int errorVal = 0;
        ReturnValue_t result = FRAMHandler::zeroOutDefaultZeroFields(&errorVal);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            actionHelper.finish(false, commandedBy, actionId, errorVal);
        }
        return HasActionsIF::EXECUTION_FINISHED;
    }
    default:
        return HasActionsIF::INVALID_ACTION_ID;
    }
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
    uint32_t new_reboot_counter = 0;
    int retval = fram_increment_reboot_counter(&new_reboot_counter);
    if(retval != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "CoreController::initialize: Error incrementing the boot"
                << " counter!" << std::endl;
#else
        sif::printError("CoreController::initialize: Error incrementing the boot"
                " counter!\n");
#endif
    }
    triggerEvent(BOOT_EVENT, new_reboot_counter, 0);
#else
    triggerEvent(BOOT_EVENT, 0, 0);
#endif

    return initializeIsisTimerDrivers();
}

ReturnValue_t CoreController::initialize() {
#ifdef ISIS_OBC_G20
    framHandler = objectManager->get<FRAMHandler>(objects::FRAM_HANDLER);
    if(framHandler == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "CoreController::initialize: No FRAM handler found!"
                << std::endl;
#else
        sif::printError("CoreController::initialize: No FRAM handler found!\n");
#endif
    }
#endif
    return ExtendedControllerBase::initialize();
}


ReturnValue_t CoreController::setUpSystemStateTask() {
    systemStateTask = objectManager->
            get<SystemStateTask>(systemStateTaskId);
    if(systemStateTask == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "CoreController::performControlOperation: "
                "System state task invalid!" << std::endl;
#else
        sif::printError("CoreController::performControlOperation: "
                "System state task invalid!\n");
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t CoreController::initializeIsisTimerDrivers() {
#ifdef ISIS_OBC_G20
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "CoreController: Starting RTC and RTT." << std::endl;
#else
    sif::printInfo("CoreController: Starting RTC and RTT.\n");
#endif

    /* Time will be set later, this just starts the synchronization task
    with a frequency of 0.5 seconds. */
    int retval = Time_start(NULL, RTC_RTT_SYNC_INTERVAL);
    if(retval >> 8 == 0xFF) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "CoreController::initializeAfterTaskCreation:"
                "ISIS RTC start failure!" << std::endl;
#else
        sif::printError("CoreController::initializeAfterTaskCreation:"
                "ISIS RTC start failure!\n");
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    retval = retval & 0xFF;
    if(retval == 1) {
        /* RTT not ticking. Should not happen! Trigger event. */
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    else if(retval == 2) {
        /* Should not happen, we did not specify time.. */
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "CoreController::initializeAfterTaskCreation: Config"
                << " error!" << std::endl;
#else
        sif::printError("CoreController::initializeAfterTaskCreation: Config"
                " error!\n");
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    else if(retval == 3) {
        /* Should not happen, the scheduler is already running */
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "CoreController::initializeAfterTaskCreation: Config"
                << " error, FreeRTOS scheduler not running!" << std::endl;
#else
        sif::printError("CoreController::initializeAfterTaskCreation: Config"
                " error, FreeRTOS scheduler not running!\n");
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    uint32_t secSinceEpoch = 0;
    retval = fram_read_seconds_since_epoch(&secSinceEpoch);
    if(retval != 0) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    /* If stored time is 0 or all ones, set the compile time of the binary */
    if(secSinceEpoch == 0 or secSinceEpoch == 0xffffffff) {
        secSinceEpoch = UNIX_TIMESTAMP;
        retval = fram_update_seconds_since_epoch(secSinceEpoch);
        if(retval != 0) {
            /* FRAM issues */
        }
    }

    timeval currentTime;
    currentTime.tv_sec = secSinceEpoch;

    /* Setting ISIS clock. */
    Time_setUnixEpoch(secSinceEpoch);

    /* Setting FSFW clock. */
    Clock::setClock(&currentTime);

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "CoreController: Clock set." << std::endl;
#else
    sif::printInfo("CoreController: Clock set.\n");
#endif

#else
    RTT_start();
    timeval currentTime;
    uint32_t secSinceEpoch = UNIX_TIMESTAMP;
    currentTime.tv_sec = secSinceEpoch;
    Clock::setClock(&currentTime);
#endif

    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t CoreController::storeSelect(StorageManagerIF** store, Stores storeType) {

        if (store == nullptr) {
            return HasReturnvaluesIF::RETURN_FAILED;
        }

        switch(storeType) {
        case(TM_STORE): {
            *store = objectManager->get<StorageManagerIF>(objects::TM_STORE);
            break;
        }
        case(TC_STORE): {
            *store = objectManager->get<StorageManagerIF>(objects::TC_STORE);
            break;
        }
        case(IPC_STORE): {
            *store = objectManager->get<StorageManagerIF>(objects::IPC_STORE);
            break;
        }
        default: {
            sif::printWarning("CoreController::storeSelect: Invalid Store!\n\r");
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        }
        return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t CoreController::getFillCountCommand(Stores storeType, MessageQueueId_t commandedBy,
        ActionId_t replyId) {

    StorageManagerIF* store = nullptr;
    ReturnValue_t result = storeSelect(&store, storeType);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    uint8_t numberOfPools = store->getNumberOfSubPools();
    uint8_t buffer[numberOfPools + 2];
    uint8_t bytesWritten;
    buffer[0] = storeType;
    store->getFillCount(&buffer[1], &bytesWritten);
    result = actionHelper.reportData(commandedBy, replyId, buffer, bytesWritten);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    return HasActionsIF::EXECUTION_FINISHED;
}

ReturnValue_t CoreController::handleClearStoreCommand(Stores storeType, ActionId_t pageOrWholeStore,
                StorageManagerIF::max_subpools_t poolIndex) {

    StorageManagerIF* store = nullptr;
    ReturnValue_t result = storeSelect(&store, storeType);
    if (result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    if(store == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    if (pageOrWholeStore == CLEAR_STORE_PAGE) {
        if(poolIndex >= store->getNumberOfSubPools()) {
            return HasActionsIF::INVALID_PARAMETERS;
        }
        store->clearSubPool(poolIndex);
    }
    else if (pageOrWholeStore == CLEAR_WHOLE_STORE){
        store->clearStore();
    }

    return HasActionsIF::EXECUTION_FINISHED;
}

ReturnValue_t CoreController::initializeLocalDataPool(localpool::DataPool &localDataPoolMap,
        LocalDataPoolManager &poolManager) {
    return HasReturnvaluesIF::RETURN_OK;
}

LocalPoolDataSetBase* CoreController::getDataSetHandle(sid_t sid) {
    return nullptr;
}


void CoreController::performPeriodicTimeHandling() {
    /* Update uptime second counter which is not subject to regular overflowing */
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
    /* Store current time since epoch in seconds in FRAM, using the FRAM handler. */
    unsigned int epochTime = 0;
    Time_getUnixEpoch(&epochTime);
    int result = fram_update_seconds_since_epoch(static_cast<uint32_t>(epochTime));
    if(result != 0) {
        /* Should not happen! */
        triggerEvent(FRAM_FAILURE, result);
    }

    uint32_t epoch = 0;
    result = Time_getUnixEpoch(reinterpret_cast<unsigned int*>(&epoch));

    /* Correct clock drift of FSFW clock (FreeRTOS/oscillator based) based on ISIS clock */
    timeval currentFsfwTime;
    Clock::getClock_timeval(&currentFsfwTime);
    if(std::abs(currentFsfwTime.tv_sec - epochTime) > clockSecDiffSyncTrigger) {
        currentFsfwTime.tv_sec = epochTime;
        currentFsfwTime.tv_usec = 0;
        Clock::setClock(&currentFsfwTime);
    }
#endif /* ISIS_OBC_G20 */
}

uint32_t CoreController::updateSecondsCounter() {
#ifdef AT91SAM9G20_EK
    /* We can only use RTT on the AT91, on the iOBC it will be reset constantly. */
    uptimeSeconds = RTT_GetTime();
#else
    uint32_t currentUptimeSeconds = 0;
    /* Millisecond count can overflow regularly (around every 50 days) */
    uint32_t uptimeMs = 0;
    Clock::getUptime(&uptimeMs);

    /* I am just going to assume that the first uptime encountered is going
    to be larger than 0 milliseconds. */
    if(uptimeMs <= lastUptimeMs) {
        msOverflowCounter++;
    }
    currentUptimeSeconds = uptimeMs / configTICK_RATE_HZ;

    lastUptimeMs = uptimeMs;
    uptimeSeconds = msOverflowCounter * SECONDS_ON_MS_OVERFLOW + currentUptimeSeconds;
#endif
    return uptimeSeconds;
}

uint32_t CoreController::getUptimeSeconds() {
    MutexHelper(timeMutex, MutexIF::TimeoutType::WAITING, 20);
    return uptimeSeconds;
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


