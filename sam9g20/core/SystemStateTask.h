#ifndef SAM9G20_SYSTEMSTATETASK_H_
#define SAM9G20_SYSTEMSTATETASK_H_

#include <fsfw/osal/FreeRTOS/BinSemaphUsingTask.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/events/Event.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <vector>
#include <array>

class CoreController;

/**
 * @brief       Low priority task used to get system stats.
 * @author      R. Mueller
 */
class SystemStateTask: public SystemObject,
        public ExecutableObjectIF {
public:
	static constexpr size_t STACK_THRESHOLD = 1000;

	static constexpr uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::SYSTEM_STATE_TASK;
	static constexpr Event LOW_REM_STACK = MAKE_EVENT(0, SEVERITY::LOW); //!< P1: First four letter of task name, P2: Second four letters of task name.

    SystemStateTask(object_id_t objectId, object_id_t coreControllerId);

    ReturnValue_t performOperation(uint8_t opCode) override;

    // Read system state into the task status array.
    bool readSystemState();
    bool generateStatsAndCheckStack();
    // Perform both operation at once
    bool readAndGenerateStats();

    /**
     * Called manually.
     * @return
     */
    ReturnValue_t initializeAfterTaskCreationNoOverride();

    uint16_t numberOfTasks = 0;
private:
    enum class InternalState {
    	IDLE,
        READING_STATS,
        GENERATING_STATS,
    };
    InternalState internalState;
    bool readOnce = false;
    bool doubleOperationRequested = false;

    object_id_t coreControllerId;
    CoreController* coreController = nullptr;

    std::vector<uint8_t> statsVector;
    std::vector<TaskStatus_t> taskStatArray;
    BinarySemaphoreUsingTask* semaphore = nullptr;

    TaskStatus_t* taskStatusWritePtr = nullptr;


    void generateStatsCsvAndCheckStack();
    void writePaddedName(uint8_t* buffer,
            const char *pcTaskName);
    void writeDebugStatLine(const TaskStatus_t& task,
    		size_t& statsIdx, uint64_t idleTicks, uint64_t uptimeTicks);
    void writeCsvStatLine(const TaskStatus_t& task,
    		size_t& statsIdx, uint64_t idleTicks, uint64_t uptimeTicks);
};


#endif /* SAM9G20_SYSTEMSTATETASK_H_ */
