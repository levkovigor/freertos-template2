#ifndef SAM9G20_UTILITY_FREERTOSTASKMONITOR_H_
#define SAM9G20_UTILITY_FREERTOSTASKMONITOR_H_

#include <mission/utility/TaskMonitor.h>
#include <fsfw/container/DynamicFIFO.h>
#include <fsfw/events/Event.h>
#include <fsfw/osal/FreeRTOS/FreeRTOSTaskIF.h>

/**
 * @brief 	This task monitor can be used to monitor FreeRTOS task stack
 * @details
 * The current implementation can be used to monitor the stack high watermark.
 * It features different action modes, which can also be combined.
 *
 * 1. Check Operation Mode:
 * Checker mode which checks the lowest remaing stack of all
 * tasks which have been added to the monitoring map agains a threshold
 * and triggers an event if that threshold is exceeded.
 *
 * 2. Print Operation Mode:
 * Print mode which prints the remaining stack of all tasks in the monitoring
 * map to the standard output.
 *
 *
 * It also has different criteria when to perform these actions, see
 * the generic TaskMonitor documentation for the different criteria.
 *
 * The class will not do anything in the default constructed mode, so the
 * desired mode needs to be set with the respective interface functions.
 *
 */
class FreeRTOSStackMonitor: public TaskMonitor {
public:
	static constexpr uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::TASK_MONITOR;
	static constexpr Event HIGH_STACK_WATERMARK = MAKE_EVENT(SUBSYSTEM_ID,
			SEVERITY::MEDIUM);
	/**
	 * Initialize the monitoring object.
	 * @param objectId
	 * @param timePointFifoSize Size of the FIFO to store timepoints
	 */
    FreeRTOSStackMonitor(object_id_t objectId, uint8_t timePointFifoSize = 5);
    virtual~ FreeRTOSStackMonitor();

    ReturnValue_t executeOneShotOperation() override;
    ReturnValue_t executePeriodicOperation() override;
    ReturnValue_t executeTimepointOperation(timeval timepoint) override;

    /** Mode-Setting functions */
    void setCheckMode(size_t remainingStackThreshold);
    void clearCheckMode();

    void setPrintMode();
    void clearPrintMode();

    void performOneShotStackUsageOperation(dur_seconds_t timePointInFuture = 0);

    /**
     * Override base performOp to implement mutex protection and to
     * return prematurely if internal mode is NONE_OP.
     * @param opCode
     * @return
     */
    virtual ReturnValue_t performOperation(uint8_t opCode) override;

    /**
     * Actions to perform for each task. Can be overriden for custom
     * implementation.
     * Default implementation will use the internal mode to print stack usage
     * and/or perform checks against threshold.
     * @param objectId Object ID of respective task.
     * @param periodicTask Pointer to task class
     */
    virtual void performOperationOnPeriodicTask(object_id_t objectId,
    		PeriodicTaskIF* periodicTask) override;
    virtual void performOperationOnFixedTimeslotTask(object_id_t objectId,
    		FixedTimeslotTaskIF* periodicTask) override;

private:

    enum class InternalMode: uint8_t {
    	//! No operation performed
    	NONE_OP = 0,
    	//! This flag is used to print information.
    	PRINT_MODE = 1,
    	//! This flag is used to check remaining stack against threshold and
    	//! trigger events instead of printing when values becomes too low.
    	CHECK_MODE = 2,
		//! Perform both checks
		PRINT_CHECK_MODE = 3
    };

    InternalMode internalMode = InternalMode::NONE_OP;

    size_t remainingStackThreshold = 0;

    void performActionForEachTask();
    void performStackCheck(object_id_t objectId, FreeRTOSTaskIF* task);
    void setStackThreshold(size_t remainingStackThreshold);
};

#endif /* SAM9G20_UTILITY_FREERTOSTASKMONITOR_H_ */
