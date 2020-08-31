#ifndef SAM9G20_SYSTEMSTATETASK_H_
#define SAM9G20_SYSTEMSTATETASK_H_
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/osal/FreeRTOS/BinarySemaphore.h>
#include <fsfw/osal/FreeRTOS/BinSemaphUsingTask.h>
#include <fsfw/tasks/ExecutableObjectIF.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <vector>
#include <array>

class CoreController;

/**
 * Low priority task used to get system stats.
 */
class SystemStateTask: public SystemObject,
        public ExecutableObjectIF {
public:
    SystemStateTask(object_id_t objectId, object_id_t coreControllerId);

    ReturnValue_t performOperation(uint8_t opCode) override;
    void readSystemState();
    bool getSystemStateWasRead() const;

    void generateStatsAndCheckStack();
    bool getSystemStateStatsWereGenerated() const;

    void readAndGenerateStats();
    ReturnValue_t initializeAfterTaskCreation() override;
private:
    enum class InternalState {
        READING_STATS,
        STATS_READ,
        GENERATING_STATS,
        STATS_GENERATED
    };
    InternalState internalState;

    object_id_t coreControllerId;
    CoreController* coreController = nullptr;
    // TODO: make size scale with number of tasks
    std::array<uint8_t, 2048> statsArray;
    std::vector<TaskStatus_t> taskStatArray;
    BinarySemaphoreUsingTask* semaphore = nullptr;

    TaskStatus_t* taskStatusWritePtr = nullptr;
    uint16_t numberOfTasks = 0;

    void generateStatsCsvAndCheckStack();
    void writePaddedName(uint8_t* buffer,
            const char *pcTaskName);
};


#endif /* SAM9G20_SYSTEMSTATETASK_H_ */
