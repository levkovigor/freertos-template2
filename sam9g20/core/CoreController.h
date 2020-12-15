#ifndef SAM9G20_CORE_CORECONTROLLER_H_
#define SAM9G20_CORE_CORECONTROLLER_H_

#include <fsfw/controller/ExtendedControllerBase.h>

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
    static constexpr uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::CORE_CONTROLLER;

    //! Triggered on startup. P1 Boot counter.
    static constexpr Event BOOT_EVENT = MAKE_EVENT(0, severity::INFO);

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
            LocalDataPool& localDataPoolMap,
            LocalDataPoolManager& poolManager) override;
    LocalPoolDataSetBase* getDataSetHandle(sid_t sid) override;

	ReturnValue_t initialize() override;
	ReturnValue_t initializeAfterTaskCreation() override;

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

	static constexpr ActionId_t REQUEST_CPU_STATS_CHECK_STACK = 0;
	static constexpr ActionId_t RESET_OBC = 10;
	static constexpr ActionId_t POWERCYCLE_OBC = 11;

private:

	//! Uptime second counter which will also be checked for overflows.
	static uint32_t uptimeSeconds;
	static uint32_t counterOverflows;
	static uint32_t idleCounterOverflows;

	SystemStateTask* systemStateTask = nullptr;
	static MutexIF* timeMutex;

#ifdef ISIS_OBC_G20
	FRAMHandler* framHandler = nullptr;
	supervisor_housekeeping_t supervisorHk;
	int16_t adcValues[SUPERVISOR_NUMBER_OF_ADC_CHANNELS] = {0};

	uint16_t msOverflowCounter = 0;
	uint32_t lastUptimeMs = 0;
#endif

	object_id_t systemStateTaskId = objects::NO_OBJECT;

	void performSupervisorHandling();
	void performPeriodicTimeHandling();
	uint32_t updateSecondsCounter();

	uint32_t lastFastCounterUpdateSeconds = 0;
	uint32_t last32bitCounterValue = 0;
	uint32_t last32bitIdleCounterValue = 0;

	void update64bit10kHzCounter();
	ReturnValue_t setUpSystemStateTask();
	ReturnValue_t initializeIsisTimerDrivers();
	void generateStatsCsvAndCheckStack();
	void writePaddedName(uint8_t* buffer, const char *pcTaskName);
};



#endif /* MISSION_DEVICES_COREHANDLER_H_ */
