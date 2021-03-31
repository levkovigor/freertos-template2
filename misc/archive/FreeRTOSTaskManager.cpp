#include <bsp_sam9g20/utility/FreeRTOSTaskManager.h>

FreeRTOSTaskManager::FreeRTOSTaskManager(object_id_t objectId):
		TaskMonitor(objectId) {
}

ReturnValue_t FreeRTOSTaskManager::performOperation(uint8_t opCode) {
	return HasReturnvaluesIF::RETURN_OK;
}

uint32_t FreeRTOSTaskManager::updateCurrentIdleCounter() {
	uint32_t newIdleCounter = ulTaskGetIdleRunTimeCounter();
	if(newIdleCounter < lastIdleTaskCounterSnapshot) {
		idleTaskCounterOverflows++;
	}
	lastIdleTaskCounterSnapshot = newIdleCounter;
	return newIdleCounter;
}


uint64_t FreeRTOSTaskManager::getCurrentIdleCounter() {
	uint32_t idleTaskCounter = updateCurrentIdleCounter();
	return static_cast<uint64_t>(idleTaskCounterOverflows) << 32 |
			idleTaskCounter;
}

