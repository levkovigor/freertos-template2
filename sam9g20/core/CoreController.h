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
private:
	ActionHelper actionHelper;
	uint16_t numberOfTasks = 0;
	bool oneShot = true;


	supervisor_housekeeping_t supervisorHk;
	std::vector<TaskStatus_t> taskStatArray;
	SystemStateTask* systemStateTask = nullptr;
	int16_t adcValues[SUPERVISOR_NUMBER_OF_ADC_CHANNELS] = {0};
};



#endif /* MISSION_DEVICES_COREHANDLER_H_ */
