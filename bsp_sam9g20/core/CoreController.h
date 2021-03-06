#ifndef SAM9G20_CORE_CORECONTROLLER_H_
#define SAM9G20_CORE_CORECONTROLLER_H_

#include <fsfw/controller/ExtendedControllerBase.h>
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <bsp_sam9g20/common/fram/CommonFRAM.h>
#include <OBSWConfig.h>

#ifdef ISIS_OBC_G20
extern "C" {
#include <hal/supervisor.h>
}

class FRAMHandler;
#endif
class SystemStateTask;

/**
 * @brief   Core Controller responsible for monitoring the OBC itself
 * @details
 * @author  R. Mueller
 *
 */
class CoreController: public ExtendedControllerBase {
public:
    enum Stores: uint8_t{
        TM_STORE = 0,
        TC_STORE = 1,
        IPC_STORE = 2
    };

    static constexpr uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::CORE_CONTROLLER;

    //! Triggered on startup. P1 Boot counter.
    static constexpr Event BOOT_EVENT = MAKE_EVENT(0, severity::INFO);
    static constexpr Event FRAM_FAILURE = event::makeEvent(SUBSYSTEM_ID, 1, severity::MEDIUM);
    static constexpr Event SUPERVISOR_FAILURE = event::makeEvent(SUBSYSTEM_ID, 2, severity::MEDIUM);
    static constexpr Event RTT_RTC_FAILURE = event::makeEvent(SUBSYSTEM_ID, 3, severity::MEDIUM);
    static constexpr Event FSFW_CLOCK_SYNC = event::makeEvent(SUBSYSTEM_ID, 4, severity::INFO);

    static constexpr Event BOOT_SINGLEBIT_ERROR_CORRECTED = event::makeEvent(SUBSYSTEM_ID, 5,
            severity::MEDIUM);
    static constexpr Event BOOT_MULTIBIT_ERROR_DETECTED = event::makeEvent(SUBSYSTEM_ID, 6,
            severity::MEDIUM);
    static constexpr Event BOOT_ECC_ERROR_DETECTED = event::makeEvent(SUBSYSTEM_ID, 7,
            severity::MEDIUM);

    static constexpr uint8_t SUPERVISOR_INDEX = -1;

    static constexpr float RTC_RTT_SYNC_INTERVAL = 0.5;
	static constexpr uint32_t DAY_IN_SECONDS = 60 * 60 * 24;
	static constexpr float SECONDS_ON_MS_OVERFLOW = 4294967.296;

	CoreController(object_id_t objectId, object_id_t systemStateTaskId);

	ReturnValue_t handleCommandMessage(CommandMessage *message) override;
	void performControlOperation() override;
	ReturnValue_t checkModeCommand(Mode_t mode, Submode_t submode,
	            uint32_t *msToReachTheMode) override;

	/** HasActionsIF override */
	ReturnValue_t executeAction(ActionId_t actionId,
	            MessageQueueId_t commandedBy, const uint8_t* data,
	            size_t size) override;

	/** HasLocalDataPoolIF overrides */
    ReturnValue_t initializeLocalDataPool(
            localpool::DataPool& localDataPoolMap,
            LocalDataPoolManager& poolManager) override;
    LocalPoolDataSetBase* getDataSetHandle(sid_t sid) override;

	ReturnValue_t initialize() override;
	ReturnValue_t initializeAfterTaskCreation() override;

	ReturnValue_t storeSelect(StorageManagerIF** store, Stores storeType);
	ReturnValue_t getFillCountCommand(Stores storeType, MessageQueueId_t commandedBy,
	            ActionId_t replyId);
	ReturnValue_t handleClearStoreCommand(Stores storeType, ActionId_t pageOrWholeStore,
	            StorageManagerIF::max_subpools_t pageIndex);

	/**
	 * This function can be used by other software components as well
	 * to get a second uptime counter which will not overflow.
	 * @return
	 */
	static uint32_t getUptimeSeconds();

	/**
	 * This returns the 64bit value of a 10kHz counter.
	 * @return
	 */
	static uint64_t getTotalRunTimeCounter();
	static uint64_t getTotalIdleRunTimeCounter();

	static constexpr ActionId_t REQUEST_CPU_STATS_CSV = 0;
	static constexpr ActionId_t REQUEST_CPU_STATS_PRINT = 1;

	static constexpr ActionId_t ENABLE_GLOBAL_HAMMING_CODE_CHECKS = 2;
	static constexpr ActionId_t DISABLE_GLOBAL_HAMMING_CODE_CHECKS = 3;

	static constexpr ActionId_t ENABLE_LOCAL_HAMMING_CODE_CHECKS = 4;
	static constexpr ActionId_t DISABLE_LOCAL_HAMMING_CODE_CHECKS = 5;

	static constexpr ActionId_t RESET_REBOOT_COUNTER = 6;

	static constexpr ActionId_t RESET_OBC = 10;
	static constexpr ActionId_t POWERCYCLE_OBC = 11;

	static constexpr ActionId_t CLEAR_STORE_PAGE = 12;
	static constexpr ActionId_t CLEAR_WHOLE_STORE = 13;
	static constexpr ActionId_t GET_FILL_COUNT = 14;

	static constexpr ActionId_t ARM_DEPLOYMENT_TIMER = 16;

    /**
     * Dump the whole bootloader block which also contains information about the
     * hamming code checks
     */
    static constexpr ActionId_t DUMP_BOOTLOADER_BLOCK = 28;
	static constexpr ActionId_t PRINT_FRAM_BL_BLOCK = 29;
    static constexpr ActionId_t PRINT_FRAM_CRIT_BLOCK = 30;
    /* Careful with this. Might be deactivated at a later project stage so it can not
    be accidentely called during flight */
    static constexpr ActionId_t ZERO_OUT_FRAM_DEFAULT_ZERO_FIELD = 35;

private:

	//! Uptime second counter which will also be checked for overflows.
	static uint32_t uptimeSeconds;
	static uint32_t counterOverflows;
	static uint32_t idleCounterOverflows;

	SystemStateTask* systemStateTask = nullptr;
	static MutexIF* timeMutex;

    /* If the FSFW clock seconds and the ISIS clock seconds difference is higher than this value,
    synchronize the FSFW clock. Will be a tweakable parameter */
    uint8_t clockSecDiffSyncTrigger = 2;

    uint32_t epochTime = 0;
#ifdef ISIS_OBC_G20
	FRAMHandler* framHandler = nullptr;
	supervisor_housekeeping_t supervisorHk;
	int16_t adcValues[SUPERVISOR_NUMBER_OF_ADC_CHANNELS] = {0};

	uint16_t msOverflowCounter = 0;
	uint32_t lastUptimeMs = 0;
#elif defined(AT91SAM9G20_EK)
    bool deploymentTimerArmed = false;
	uint32_t lastRttSecondCount = 0;
	uint32_t lastSdCardUpdate = 0;
	//! Increment for uptime will be cached for periodic SD card updates
    uint32_t uptimeIncrement = 0;
    //! Increment for deployment timer will be cached for periodic SD card updates
    uint32_t deploymentTimerIncrement = 0;
#endif

    uint32_t lastDeploymentTimerIncrement = 0;

	object_id_t systemStateTaskId = objects::NO_OBJECT;

	void performSupervisorHandling();
	void performPeriodicTimeHandling();
	uint32_t updateSecondsCounter();

	uint32_t lastFastCounterUpdateSeconds = 0;
	uint32_t last32bitCounterValue = 0;
	uint32_t last32bitIdleCounterValue = 0;

	void update64bit10kHzCounter();
	ReturnValue_t setUpSystemStateTask();
	void generateStatsCsvAndCheckStack();
	void writePaddedName(uint8_t* buffer, const char *pcTaskName);
    enum DataIdx {
        STORE_TYPE,
        PAGE_INDEX
    };

    ReturnValue_t manipulateGlobalHammingFlag(bool set, ActionId_t actionId,
            MessageQueueId_t commandedBy, const uint8_t *data, size_t size);
    ReturnValue_t manipulateLocalHammingFlag(bool set, ActionId_t actionId,
            MessageQueueId_t commandedBy, const uint8_t *data, size_t size);
    ReturnValue_t resetRebootCounter(ActionId_t actionId, MessageQueueId_t commandedBy,
            const uint8_t *data, size_t size);
#if OBSW_VERBOSE_LEVEL >= 1
    void determinePrintoutType(SlotType slotType, char* printoutType, size_t printBuffLen);
#endif

#if OBSW_MONITOR_ALLOCATION == 1
    bool swInitCompleteFlagFlipped = false;
#endif

    ReturnValue_t initializeTimerDrivers();
#ifdef ISIS_OBC_G20
    ReturnValue_t initializeIobcTimerDrivers();
#elif defined(AT91SAM9G20_EK)
    ReturnValue_t initializeAt91TimerDrivers();
#endif
};



#endif /* MISSION_DEVICES_COREHANDLER_H_ */
