#include <sam9g20/utility/FreeRTOSStackMonitor.h>

#include <fsfw/ipc/MutexHelper.h>
#include <fsfw/osal/FreeRTOS/FixedTimeslotTask.h>
#include <fsfw/osal/FreeRTOS/PeriodicTask.h>
#include <fsfw/osal/FreeRTOS/TaskManagement.h>
#include <fsfw/timemanager/Clock.h>


FreeRTOSStackMonitor::FreeRTOSStackMonitor(object_id_t objectId,
        uint8_t timePointFifoSize):
        TaskMonitor(objectId, timePointFifoSize) {}

FreeRTOSStackMonitor::~FreeRTOSStackMonitor() {}

ReturnValue_t FreeRTOSStackMonitor::performOperation(uint8_t opCode) {
	MutexHelper(monitorMutex, MutexIF::TimeoutType::WAITING, 10);
	if(internalMode == InternalMode::NONE_OP) {
		return HasReturnvaluesIF::RETURN_OK;
	}

	return TaskMonitor::performOperation(opCode);
}

void FreeRTOSStackMonitor::performActionForEachTask() {
	if(internalMode == InternalMode::PRINT_MODE or
			internalMode == InternalMode::PRINT_CHECK_MODE) {
		sif::info << "FreeRTOSTaskMonitor: Stack High Watermark Information:"
				<< std::endl;
		sif::info << "Numbers are lowest remaining stack since task start."
				<< std::endl;
	}

	performOperationOnEachFixedTimeslotTask();
	performOperationOnEachPeriodicTask();

    if(internalMode == InternalMode::PRINT_MODE or
    			internalMode == InternalMode::PRINT_CHECK_MODE) {
    	printf("\n");
    }
}

void FreeRTOSStackMonitor::performOperationOnPeriodicTask(object_id_t objectId,
		PeriodicTaskIF* periodicTask) {
	FreeRTOSTaskIF* freeRTOSTask =
			dynamic_cast<FreeRTOSTaskIF*>(periodicTask);
	if(freeRTOSTask == nullptr) {
		return;
	}
	performStackCheck(objectId, freeRTOSTask);
}

void FreeRTOSStackMonitor::performOperationOnFixedTimeslotTask(object_id_t objectId,
		FixedTimeslotTaskIF* periodicTask) {
	FreeRTOSTaskIF* freeRTOSTask =
			dynamic_cast<FreeRTOSTaskIF*>(periodicTask);
	if(freeRTOSTask == nullptr) {
		return;
	}
	performStackCheck(objectId, freeRTOSTask);
}

void FreeRTOSStackMonitor::performStackCheck(object_id_t objectId,
		FreeRTOSTaskIF* task) {
	size_t remainingStack = TaskManagement::getTaskStackHighWatermark(
			task->getTaskHandle());

	if(internalMode == InternalMode::PRINT_MODE or
			internalMode == InternalMode::PRINT_CHECK_MODE) {
		sif::info << "Task " << pcTaskGetName(task->getTaskHandle())
				    		  << ": " << remainingStack  << " bytes" << std::endl;
	}

	if(internalMode == InternalMode::CHECK_MODE or
			internalMode == InternalMode::PRINT_CHECK_MODE) {
		if(remainingStack < this->remainingStackThreshold) {
			char* taskName = pcTaskGetName(task->getTaskHandle());
			sif::warning << "FreeRTOSTaskMonitor: Task " << taskName
					<< ": Low remaining stack, "
					<< remainingStack << " bytes remaining." << std::endl;
			// there should be an option to only generate this event and debug
			// output once. Either keep an internal list of object IDs
			// or make the underlying map more complicated.
			triggerEvent(HIGH_STACK_WATERMARK, objectId, 0);
		}
	}
}

void FreeRTOSStackMonitor::performOneShotStackUsageOperation(
		dur_seconds_t timePointInFuture) {
	MutexHelper(monitorMutex, MutexIF::TimeoutType::WAITING, 10);
	if(timePointInFuture == 0) {
		performOneShotOperation = true;
		return;
	}

	// Insert as check performed in the future.
	timeval timeInFuture = timevalOperations::toTimeval(timePointInFuture) +
			Clock::getUptime();
	infoGenerationTimepoints.insert(timeInFuture);

}

void FreeRTOSStackMonitor::setPrintMode() {
	MutexHelper(monitorMutex, MutexIF::TimeoutType::WAITING, 10);
	switch(internalMode) {
	case(InternalMode::NONE_OP): {
		internalMode = InternalMode::PRINT_MODE;
		return;
	}
	case(InternalMode::CHECK_MODE): {
		internalMode = InternalMode::PRINT_CHECK_MODE;
		return;
	}
	case(InternalMode::PRINT_MODE):
	case(InternalMode::PRINT_CHECK_MODE):
	default:
		return;
	}
}

void FreeRTOSStackMonitor::clearPrintMode() {
	MutexHelper(monitorMutex, MutexIF::TimeoutType::WAITING, 10);
	switch(internalMode) {
	case(InternalMode::PRINT_MODE): {
		internalMode = InternalMode::NONE_OP;
		return;
	}
	case(InternalMode::PRINT_CHECK_MODE): {

		internalMode = InternalMode::CHECK_MODE;
		return;
	}
	case(InternalMode::NONE_OP):
	case(InternalMode::CHECK_MODE):
	default:
		return;
	}
}

void FreeRTOSStackMonitor::setCheckMode(size_t remainingStackThreshold) {
	MutexHelper(monitorMutex, MutexIF::TimeoutType::WAITING, 10);
	switch(internalMode) {
	case(InternalMode::NONE_OP): {
		setStackThreshold(remainingStackThreshold);
		internalMode = InternalMode::CHECK_MODE;
		return;
	}
	case(InternalMode::PRINT_MODE): {
		setStackThreshold(remainingStackThreshold);
		internalMode = InternalMode::PRINT_CHECK_MODE;
		return;
	}
	case(InternalMode::PRINT_CHECK_MODE):
	case(InternalMode::CHECK_MODE):
	default:
		setStackThreshold(remainingStackThreshold);
		return;
	}
}

void FreeRTOSStackMonitor::clearCheckMode() {
	MutexHelper(monitorMutex, MutexIF::TimeoutType::WAITING, 10);
	switch(internalMode) {
	case(InternalMode::PRINT_CHECK_MODE): {
		internalMode = InternalMode::PRINT_MODE;
		return;
	}
	case(InternalMode::CHECK_MODE): {
		internalMode = InternalMode::NONE_OP;
		return;
	}
	case(InternalMode::NONE_OP):
	case(InternalMode::PRINT_MODE):
	default:
		return;

	}
}

ReturnValue_t FreeRTOSStackMonitor::executeOneShotOperation() {
	performActionForEachTask();
	return TaskMonitor::OPERATION_DONE;
}

ReturnValue_t FreeRTOSStackMonitor::executePeriodicOperation() {
	performActionForEachTask();
	return TaskMonitor::OPERATION_DONE;
}

ReturnValue_t FreeRTOSStackMonitor::executeTimepointOperation(
		timeval timepoint) {
	performActionForEachTask();
	return TaskMonitor::OPERATION_DONE;
}

void FreeRTOSStackMonitor::setStackThreshold(size_t remainingStackThreshold) {
	this->remainingStackThreshold = remainingStackThreshold;
}
