#ifndef SAM9G20_UTILITY_FREERTOSCPUMONITOR_H_
#define SAM9G20_UTILITY_FREERTOSCPUMONITOR_H_
#include <mission/utility/TaskMonitor.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <map>
/**
 * This class could also take care of stack tracing because if the info
 * is queried, the stack high watermark will be queried as well.
 * Question is: There will be propably a uint64_t idle task timer which will
 * detect overflows of the uint32_t idle timer counter which will overflow
 * every few days (which in turn detect overflows of the uint16_t timer
 * peripheral which overflows every few seconds..) by snapshotting
 * the current value occasionally and checlwhether the new value is smaller.
 * We could do that for every task, but then we would need a map of every task
 * handle  to the current uint64_t timer. This could be used to get run time
 * stats indefinitely (well, propably a few years depending on frequency of timer),
 * and also with higher accuracy. This map could also be used
 * to keep a boolean to prevent stack warning events to occur all the time.
 *
 * In any case, for this class, the CPU stats functonality of this class
 * should be preprocessor protected by checking configGENERATE_RUN_TIME_STATS
 * because if that parameter is not one, the stats won't be generated anyway.
 *
 * I called this class Task Manager because by having access to the task
 * classes, the FreeRTOS system calls to suspend, resume or set properties
 * of tasks during run-time become available, but the first thing which
 * is going to be implemented is monitoring.
 *
 * TODO: Map which maps Task Handle to a struct containing the last uint32_t
 * 	     timer snapshot and overflow counter (can be uint16_t for non-idle tasks
 * 	     I guess), a boolean which specifies whether a stack overflow event
 * 	     was already sent.
 *
 * There are two ways to check tasks: Use uxTaskGetSystemState to query
 * all tasks or use vTaskGetInfo to only query tasks in the static map
 * of task monitor (objectID -> Task Class pointer). The best way would
 * be to have two modes: a mode using the first function to populate
 * the map with ALL tasks and another mode to just populate it with
 * tasks of the static map. Both ways would make the task monitor map redundant,
 * but then again it is like a bridge from objectID -> task handle -> freeRTOS
 * information.
 * Then the FreeRTOS uxTaskGetSystemState should be used to
 */
class FreeRTOSTaskManager: public TaskMonitor {
public:
	FreeRTOSTaskManager(object_id_t objectId);

	ReturnValue_t performOperation(uint8_t opCode) override;
private:
	struct TaskInformation {
		bool stackOverflowEvent = false;
		// Could also be a uint16_t, but a 48 bit counter will last
		// 90 years when used with a 100 kHz timer base so this should be ok.
		uint16_t timerOverflowCounter;
		uint32_t lastTimerCounterSnapshot;
	};

	using taskInfoMap = std::map<TaskHandle_t, struct TaskInformation>;

	uint32_t updateCurrentIdleCounter();
	uint64_t getCurrentIdleCounter();
	uint32_t idleTaskCounterOverflows = 0;
	uint32_t lastIdleTaskCounterSnapshot = 0;

};



#endif /* SAM9G20_UTILITY_FREERTOSCPUMONITOR_H_ */
