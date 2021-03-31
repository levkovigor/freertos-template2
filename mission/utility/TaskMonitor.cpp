#include <fsfw/ipc/MutexGuard.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <mission/utility/TaskMonitor.h>
#include <fsfw/serviceinterface/ServiceInterface.h>

std::map<object_id_t, PeriodicTaskIF*> TaskMonitor::periodicTaskMap;
std::map<object_id_t, FixedTimeslotTaskIF*> TaskMonitor::fixedTimeslotTaskMap;

TaskMonitor::TaskMonitor(object_id_t objectId, uint8_t maxNumOfTimepoints):
		SystemObject(objectId), infoGenerationTimepoints(maxNumOfTimepoints)  {
	monitorMutex = MutexFactory::instance()->createMutex();
}

TaskMonitor::~TaskMonitor() {
	// I assume the task monitor will have program lifetime. Otherwise,
	// we have to track the subclasses and their destructions.
	MutexFactory::instance()->deleteMutex(monitorMutex);
}

void TaskMonitor::setPeriodicOperation(float periodSeconds) {
	MutexGuard(monitorMutex, MutexIF::TimeoutType::WAITING, 10);
	performPeriodicOperation = true;
    this->periodicCounter = 0;
    this->periodicInterval = periodSeconds * 1000.0;
}

void TaskMonitor::clearPeriodicOperation() {
    MutexGuard(monitorMutex, MutexIF::TimeoutType::WAITING, 10);
    performPeriodicOperation = false;
}

ReturnValue_t TaskMonitor::insertPeriodicTask(object_id_t objectId,
        PeriodicTaskIF *periodicTask) {
    auto insertPair = TaskMonitor::periodicTaskMap.emplace(objectId,
    		periodicTask);
    if(insertPair.second) {
        return HasReturnvaluesIF::RETURN_OK;
    }
    return HasReturnvaluesIF::RETURN_FAILED;
}

ReturnValue_t TaskMonitor::insertFixedTimeslotTask(object_id_t objectId,
        FixedTimeslotTaskIF *fixedTimeslotTask) {
    auto insertPair = TaskMonitor::fixedTimeslotTaskMap.emplace(objectId,
    		fixedTimeslotTask);
    if(insertPair.second) {
        return HasReturnvaluesIF::RETURN_OK;
    }
    return HasReturnvaluesIF::RETURN_FAILED;
}

PeriodicTaskIF* TaskMonitor::getPeriodicTaskHandle(object_id_t objectId) {
    auto iter = TaskMonitor::periodicTaskMap.find(objectId);
    if(iter == TaskMonitor::periodicTaskMap.end()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TaskMonitor::getPeriodicTaskHandle: Object ID does"
                " not exists in PeriodicTask map." << std::endl;
#else
        sif::printWarning("TaskMonitor::getPeriodicTaskHandle: Object ID does"
                " not exists in PeriodicTask map.\n");
#endif
        return nullptr;
    }
    return iter->second;
}

FixedTimeslotTaskIF* TaskMonitor::getFixedTimeslotTaskHandle(
        object_id_t objectId) {
    auto iter = TaskMonitor::fixedTimeslotTaskMap.find(objectId);
    if(iter == TaskMonitor::fixedTimeslotTaskMap.end()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TaskMonitor::getPeriodicTaskHandle: Object ID does"
                " not exists in PeriodicTask map." << std::endl;
#else
        sif::printWarning("TaskMonitor::getPeriodicTaskHandle: Object ID does"
                " not exists in PeriodicTask map.\n");
#endif
        return nullptr;
    }
    return iter->second;
}

void TaskMonitor::performOperationOnEachPeriodicTask() {
    for(auto const& periodicTask: TaskMonitor::periodicTaskMap) {
        performOperationOnPeriodicTask(periodicTask.first, periodicTask.second);
    }
}

ReturnValue_t TaskMonitor::performOperation(uint8_t opCode) {
	// One shot report or check was requested.
	if(performOneShotOperation) {
		ReturnValue_t result = executeOneShotOperation();
		if(result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		performOneShotOperation = false;
	}

	// Periodic handling is on.
	if(performPeriodicOperation) {
		periodicCounter++;
		if(periodicCounter * taskInterval > periodicInterval) {
			ReturnValue_t result = executePeriodicOperation();
			periodicCounter = 0;
			if(result != HasReturnvaluesIF::RETURN_OK) {
				return result;
			}
		}
	}

	// Timepoint handling.
	if(not infoGenerationTimepoints.empty()) {
		timeval timeToCheck;
		infoGenerationTimepoints.peek(&timeToCheck);
		if(Clock::getUptime() > timeToCheck) {
			ReturnValue_t result = executeTimepointOperation(timeToCheck);
			infoGenerationTimepoints.pop();
			if(result != HasReturnvaluesIF::RETURN_OK) {
				return result;
			}
		}
	}
	return HasReturnvaluesIF::RETURN_OK;
}

void TaskMonitor::performOperationOnEachFixedTimeslotTask() {
    for(auto const& fixedTimeslotTask: TaskMonitor::fixedTimeslotTaskMap) {
        performOperationOnFixedTimeslotTask(fixedTimeslotTask.first,
        		fixedTimeslotTask.second);
    }
}

ReturnValue_t TaskMonitor::initialize() {
	return HasReturnvaluesIF::RETURN_OK;
}

void TaskMonitor::setTaskIF(PeriodicTaskIF *executingTask) {
    this->executingTask = executingTask;
}

ReturnValue_t TaskMonitor::initializeAfterTaskCreation() {
    this->taskInterval = executingTask->getPeriodMs();
    return HasReturnvaluesIF::RETURN_OK;
}
