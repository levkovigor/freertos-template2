#ifndef MISSION_UTILITY_TASKMONITOR_H_
#define MISSION_UTILITY_TASKMONITOR_H_
#include <fsfw/container/DynamicFIFO.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/tasks/PeriodicTaskIF.h>
#include <map>

class FixedTimeslotTaskIF;

/**
 * @brief 	Generic task monitoring class
 * @brief
 * Provides a framework to monitor peridic tasks by keeping an internal map
 * of object IDs to the respecitve periodic task handles.
 * This map can be filled in the task factory for the mission code during
 * software initialization.
 * Custom implementations can implement the abstract functions
 * to perform operations on the task class handles.
 * The task map should be filled at initialization time only!
 *
 * There are different operations the periodic handler of this task can
 * perform:
 *
 * 1. One-Shot Request:  A task can issue a one-shot request to perform the
 * 	  operations immediately  or at a later timepoint
 *
 * 2. Periodic Handling: The operations will be performed periodically at a
 *    specified interval.
 *
 * 3. Timepoint Request: Operaton will be performed at specific timepoint.
 *
 * TODO: Expose map through getter function? I think it might be a good
 *       idea to make this function commandable by implementing HasActionsIF.
 * TODO: Actually, this could be turned into a generic handler task which
 *       can perform one-shot requests, periodic requests and timepoint
 *       requests to save boilerplate code. The HasActionsIF could be made
 *       optional. There are propably overlaps with ControllerBase.h
 *       But ControllerBase.h implies a specific purpose anyway. This
 *       class does not have health but it can have modes though.
 *
 */
class TaskMonitor: public SystemObject,
        public ExecutableObjectIF {
public:
	// Used for internal purposes, can be returned by abstract function
	// implementations.
	static constexpr ReturnValue_t OPERATION_DONE =
			HasReturnvaluesIF::makeReturnCode(0, 2);

    TaskMonitor(object_id_t objectId, uint8_t maxNumberOfTimepoints = 5);
    virtual~ TaskMonitor();

    /**
     * @brief 	Generic periodic handler.
     * @details
     * Handles one-shot operation requests, periodic operation and timepoint
     * operation. This function is not mutex protected.
     * If a child implementation requires mutex protection, it is expected
     * that the #monitorMutex is locked in the overriden function
     * and that that the base #performOperation is called by the child class.
     * @param opCode
     * @return
     */
    ReturnValue_t performOperation(uint8_t opCode) override;

    ReturnValue_t insertPeriodicTask(object_id_t objectId,
            PeriodicTaskIF* periodicTask);
    ReturnValue_t insertFixedTimeslotTask(object_id_t objectId,
            FixedTimeslotTaskIF* fixedTimeslotTask);

    PeriodicTaskIF* getPeriodicTaskHandle(object_id_t objectId);
    FixedTimeslotTaskIF* getFixedTimeslotTaskHandle(object_id_t objectId);

    virtual void setPeriodicOperation(dur_seconds_t period);
    virtual void clearPeriodicOperation();

    /**
     * Execute a one shot operation request. Implemented by child implementation
     * @return
     * - @c RETURN_OK Procceed generic operation
     * - @c Anything else: Stop generic operation
     */
    virtual ReturnValue_t executeOneShotOperation() = 0;
    virtual ReturnValue_t executePeriodicOperation() = 0;
    virtual ReturnValue_t executeTimepointOperation(timeval timepoint) = 0;

    void performOperationOnEachPeriodicTask();
    virtual void performOperationOnPeriodicTask(object_id_t objectId,
    		PeriodicTaskIF* periodicTask) = 0;

    void performOperationOnEachFixedTimeslotTask();
    virtual void performOperationOnFixedTimeslotTask(object_id_t objectId,
    		FixedTimeslotTaskIF* fixedTimeslotTask) = 0;


    ReturnValue_t initialize() override;

protected:
    // Every child class should use the same map which is only created once.
    static std::map<object_id_t, PeriodicTaskIF*> periodicTaskMap;
    static std::map<object_id_t, FixedTimeslotTaskIF*> fixedTimeslotTaskMap;
    MutexIF* monitorMutex = nullptr;
    PeriodicTaskIF* executingTask = nullptr;

    //! Parameter to instruction one shot operatipn
    bool performOneShotOperation = false;

    //! Parameter for periodic operation.
    bool performPeriodicOperation = false;
    uint32_t periodicCounter = 0;
    dur_millis_t periodicInterval = 20000;
    dur_millis_t taskInterval = 0;

    //! Info can be generated at various timepoints.
    //! TODO: maybe a pair of timeval and action ID to be more generic?
    DynamicFIFO<timeval> infoGenerationTimepoints;


    ReturnValue_t initializeAfterTaskCreation() override;
    void setTaskIF(PeriodicTaskIF* executingTask) override;
};


#endif /* MISSION_UTILITY_TASKMONITOR_H_ */
