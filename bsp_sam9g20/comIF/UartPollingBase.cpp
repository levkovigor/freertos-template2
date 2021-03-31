#include "UartPollingBase.h"
#include "ComConstants.h"

#include <fsfw/objectmanager/ObjectManagerIF.h>
#include <fsfw/osal/FreeRTOS/BinarySemaphore.h>
#include <fsfw/osal/FreeRTOS/TaskManagement.h>
#include <fsfw/serviceinterface/ServiceInterface.h>

UartPollingBase::UartPollingBase(object_id_t objectId,
        object_id_t sharedRingBufferId, UARTbus uartBus,
        UARTconfig* uartConfiguration):
        SystemObject(objectId), uartBus(uartBus),
        sharedRingBufferId(sharedRingBufferId),
        uartConfiguration(uartConfiguration) {
}

ReturnValue_t UartPollingBase::initialize() {
    ReturnValue_t result = UART_start(uartBus,*uartConfiguration);

    if (result != RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "Serial Polling: UART_start init error with code " <<
                static_cast<int>(result) << std::endl;
#else
        sif::printError("Serial Polling: UART_start init error with code %hu\n", result);
#endif
    }

    sharedRingBuffer = objectManager->get<SharedRingBuffer>(sharedRingBufferId);
    if(sharedRingBuffer == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TcSerialPollingTask::initialize: Passed ring buffer invalid!" << std::endl;
#else
        sif::printError("TcSerialPollingTask::initialize: Passed ring buffer invalid\n!");
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return result;
}


void UartPollingBase::handleTransferCompletion(uint8_t *data,
        volatile size_t &bytesReceived, UARTtransferStatus &transferStatus,
        uint16_t mutexTimeoutMs) {
    ReturnValue_t result = sharedRingBuffer->lockRingBufferMutex(
            MutexIF::TimeoutType::WAITING, mutexTimeoutMs);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        lastError = result;
        otherErrorCount++;
    }

    // check for erroneous operations first
    result = handleTransferResult(transferStatus);
    if(result == HasReturnvaluesIF::RETURN_OK) {
        sharedRingBuffer->writeData(data, bytesReceived);
        bytesReceived = 0;
    }

    sharedRingBuffer->unlockRingBufferMutex();
}


ReturnValue_t UartPollingBase::handleTransferResult(
        UARTtransferStatus status) {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
    switch(status) {
    case(UARTtransferStatus::done_uart): {
        result = HasReturnvaluesIF::RETURN_OK;
        break;
    }
    case(UARTtransferStatus::pending_uart): {
        result = HasReturnvaluesIF::RETURN_OK;
        break;
    }
    case(UARTtransferStatus::parityError_uart): {
        parityErrorCount++;
        break;
    }
    case(UARTtransferStatus::framingError_uart): {
        framingErrorCount++;
        break;
    }
    case(UARTtransferStatus::framingAndParityError_uart): {
        framingErrorCount++;
        parityErrorCount++;
        break;
    }
    case(UARTtransferStatus::overrunError_uart): {
        overrunErrorCount++;
        break;
    }
    case(UARTtransferStatus::error_uart): {
        otherErrorCount++;
        break;
    }

    default: {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "RS232PollingTask::handleTransferResult: Unknown error with code "
                << status << " occured!" << std::endl;
#else
        sif::printError("RS232PollingTask::handleTransferResult: Unknown error with code "
                "%d occured!\n", status);
#endif
        break;
    }

    }
    return result;
}

void UartPollingBase::setEventReporting(bool enable) {
    eventReportingEnabled = enable;
}

void UartPollingBase::generateErrorEventResetCounters() {
    // generate an event to notify possible communication issues.
    uint32_t parameter1 = (parityErrorCount << 24) +
            (overrunErrorCount << 16) + (framingErrorCount << 8) +
            otherErrorCount;
    uint32_t parameter2 = lastError;
    if(eventReportingEnabled) {
        triggerEvent(comconstants::RS232_POLLING_ERROR, parameter1, parameter2);
    }

    // reset error count.
    parityErrorCount = 0;
    overrunErrorCount = 0;
    framingErrorCount = 0;
    otherErrorCount = 0;
}

void UartPollingBase::genericUartCallback(SystemContext context,
        xSemaphoreHandle sem) {
    BaseType_t higherPriorityTaskAwoken = pdFALSE;
    if(context == SystemContext::task_context) {
        BinarySemaphore::release(sem);
    }
    else {
        BinarySemaphore::releaseFromISR(sem,
                &higherPriorityTaskAwoken);
    }
    if(context == SystemContext::isr_context and
            higherPriorityTaskAwoken == pdPASS) {
        // Request a context switch before exiting ISR, as recommended
        // by FreeRTOS.
        TaskManagement::requestContextSwitch(CallContext::ISR);
    }
}
