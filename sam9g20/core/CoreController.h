#ifndef SAM9G20_CORE_CORECONTROLLER_H_
#define SAM9G20_CORE_CORECONTROLLER_H_

#include "SystemStateTask.h"
#include <fsfw/action/HasActionsIF.h>
#include <fsfw/controller/ControllerBase.h>
#include <fsfw/datapoollocal/LocalDataPoolManager.h>

#ifdef ISIS_OBC_G20
#include <sam9g20/memory/FRAMHandler.h>
extern "C" {
#include <hal/supervisor.h>
}
#endif

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <vector>

/**
 * @brief   Core Controller responsible for monitoring the OBC itself
 * @details
 * @author  R. Mueller
 *
 */
class CoreController: public ControllerBase,
        public HasActionsIF,
        public HasLocalDataPoolIF {
public:
    static constexpr uint8_t SUPERVISOR_INDEX = -1;
    static constexpr float RTC_RTT_SYNC_INTERVAL = 0.5;
	static constexpr uint32_t DAY_IN_SECONDS = 60 * 60 * 24;

	CoreController(object_id_t objectId, object_id_t systemStateTaskId);

	MessageQueueId_t getCommandQueue() const override;
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
    LocalDataPoolManager* getHkManagerHandle() override;
    dur_millis_t getPeriodicOperationFrequency() const override;
    LocalPoolDataSetBase* getDataSetHandle(sid_t sid) override;
    object_id_t getObjectId() const override;

	ReturnValue_t initialize() override;
	ReturnValue_t initializeAfterTaskCreation() override;

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
	ActionHelper actionHelper;
	LocalDataPoolManager poolManager;

#ifdef ISIS_OBC_G20
	FRAMHandler* framHandler = nullptr;
	supervisor_housekeeping_t supervisorHk;
	int16_t adcValues[SUPERVISOR_NUMBER_OF_ADC_CHANNELS] = {0};
#endif

	object_id_t systemStateTaskId;

	uint32_t lastDumpSecond = 0;

	uint32_t lastCounterUpdateSeconds = 0;
	static uint32_t counterOverflows;
	static uint32_t idleCounterOverflows;
	uint32_t last32bitCounterValue = 0;
	uint32_t last32bitIdleCounterValue = 0;

	SystemStateTask* systemStateTask = nullptr;

	void update64bitCounter();
	ReturnValue_t setUpSystemStateTask();
	ReturnValue_t initializeIsisTimerDrivers();
	void generateStatsCsvAndCheckStack();
	void writePaddedName(uint8_t* buffer, const char *pcTaskName);
};



#endif /* MISSION_DEVICES_COREHANDLER_H_ */
