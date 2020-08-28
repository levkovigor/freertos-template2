#ifndef MISSION_DEVICES_COREHANDLER_H_
#define MISSION_DEVICES_COREHANDLER_H_

#include "SystemStateTask.h"
#include <fsfw/action/HasActionsIF.h>
#include <fsfw/controller/ControllerBase.h>


extern "C" {
#include <hal/supervisor.h>
}

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <vector>

/**
 * @brief   Core Controller responsible for monitoring the OBC itself
 * TODO:    Propably needs HasLocalDataPoolIF too
 */
class CoreController: public ControllerBase,
        public HasActionsIF {
public:
    static constexpr uint8_t SUPERVISOR_INDEX = -1;
    static constexpr float RTC_RTT_SYNC_INTERVAL = 0.5;
	static constexpr uint32_t DAY_IN_SECONDS = 60 * 60 * 24;

	CoreController(object_id_t objectId);

	MessageQueueId_t getCommandQueue() const override;
	ReturnValue_t handleCommandMessage(CommandMessage *message) override;
	void performControlOperation() override;
	ReturnValue_t checkModeCommand(Mode_t mode, Submode_t submode,
	            uint32_t *msToReachTheMode) override;
	ReturnValue_t executeAction(ActionId_t actionId,
	            MessageQueueId_t commandedBy, const uint8_t* data,
	            size_t size) override;
	ReturnValue_t initializeAfterTaskCreation() override;

	static uint64_t getTotalRunTimeCounter();
	static uint64_t getTotalIdleRunTimeCounter();

	ActionId_t REQUEST_CPU_STATS_CHECK_STACK = 0;
private:
	ActionHelper actionHelper;
	uint16_t numberOfTasks = 0;
	bool cpuStatsDumpRequested = true;

	uint32_t lastCounterUpdateSeconds = 0;
	static uint32_t counterOverflows;
	static uint32_t idleCounterOverflows;
	uint32_t last32bitCounterValue = 0;
	uint32_t last32bitIdleCounterValue = 0;

	supervisor_housekeeping_t supervisorHk;
	std::vector<TaskStatus_t> taskStatArray;
	SystemStateTask* systemStateTask = nullptr;
	int16_t adcValues[SUPERVISOR_NUMBER_OF_ADC_CHANNELS] = {0};

	void update64bitCounter();
	void setUpSystemStateTask();
	void initializeIsisTimerDrivers();
};



#endif /* MISSION_DEVICES_COREHANDLER_H_ */
