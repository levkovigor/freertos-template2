#include "CoreController.h"

#include <fsfw/timemanager/Stopwatch.h>
#include <systemObjectList.h>

CoreController::CoreController(object_id_t objectId): ControllerBase(objectId,
        objects::NO_OBJECT), actionHelper(this, commandQueue), taskStatArray(0) {
#ifdef ISIS_OBC_G20
    Supervisor_start(NULL, 0);
#endif
}

ReturnValue_t CoreController::handleCommandMessage(CommandMessage *message) {
    return actionHelper.handleActionMessage(message);
}

void CoreController::performControlOperation() {

    // First task: get supervisor state
#ifdef ISIS_OBC_G20
    int result = Supervisor_getHousekeeping(&supervisorHk, SUPERVISOR_INDEX);
    // should not happen.
    if(result != 0) {}
    supervisor_enable_status_t* temporaryEnable =
            &(supervisorHk.fields.enableStatus);
    if(temporaryEnable) {}
    Supervisor_calculateAdcValues(&supervisorHk, adcValues);
    // now store everything into a local pool. Also take action if any values
    // are out of order.
#endif

    // Second task: get system task state by using FreeRTOS API
    // total run time counter will be a calculated seperately, using
    // a 48 or 64bit counter.

    // do this in low priority task which is unblocked.
    //systemStateTask->readSystemState();

//    if(systemStateTask -> filledOnce) {
//        for(auto &task: taskStatArray) {
//            sif::info << task.pcTaskName << std::endl;
//        }
//    }
}

ReturnValue_t CoreController::checkModeCommand(Mode_t mode, Submode_t submode,
        uint32_t *msToReachTheMode) {
    return HasReturnvaluesIF::RETURN_OK;
}

MessageQueueId_t CoreController::getCommandQueue() const {
    return ControllerBase::getCommandQueue();
}

ReturnValue_t CoreController::executeAction(ActionId_t actionId,
        MessageQueueId_t commandedBy, const uint8_t *data, size_t size) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t CoreController::initializeAfterTaskCreation() {
    numberOfTasks = uxTaskGetNumberOfTasks();
    taskStatArray.reserve(numberOfTasks);
    taskStatArray.resize(numberOfTasks);
    systemStateTask = objectManager->
            get<SystemStateTask>(objects::SYSTEM_STATE_TASK);
    if(systemStateTask != nullptr) {
        systemStateTask->assignStatusWritePtr(taskStatArray.data(),
                numberOfTasks);
    }
    else {
        sif::error << "CoreController::performControlOperation:"
                "System state task invalid!" << std::endl;
    }
    return HasReturnvaluesIF::RETURN_OK;
}
