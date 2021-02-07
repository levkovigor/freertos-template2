#include "RS485PollingTask.h"

bool RS485PollingTask::uart2Started = false;

volatile size_t RS485PollingTask::transfer1bytesReceived = 0;
volatile size_t RS485PollingTask::transfer2bytesReceived = 0;

RS485PollingTask::RS485PollingTask(object_id_t objectId,
        object_id_t sharedRingBufferId): UartPollingBase(objectId,
                sharedRingBufferId, bus2_uart, &configBus2) {
#if RS485_WITH_TERMINATION == 1
    configBus2.busType = rs422_withTermination_uart;
#endif
    configBus2.rxtimeout = RS485_SERIAL_TIMEOUT_BAUDTICKS;
    configBus2.baudrate = RS485_REGULARD_BAUD;
}

ReturnValue_t RS485PollingTask::performOperation(uint8_t opCode) {
    initiateUartTransfers();
    while(true) {
        pollUart();
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485PollingTask::initialize() {
    ReturnValue_t result = UartPollingBase::initialize();
    if(result == HasReturnvaluesIF::RETURN_OK) {
        uart2Started = true;
    }
    return result;
}

void RS485PollingTask::initiateUartTransfers() {
    uartTransfer1.bus = bus2_uart;
    uartTransfer1.callback = uart1Callback;
    uartTransfer1.direction = read_uartDir;
    uartTransfer1.postTransferDelay = 0;
    uartTransfer1.readData = readBuffer1.data();
    uartTransfer1.readSize = RS485_MAX_SERIAL_FRAME_SIZE;
    uartTransfer1.result = &transfer1Status;
    uartSemaphore1.acquire();
    uartTransfer1.semaphore = uartSemaphore1.getSemaphore();

    uartTransfer2.bus = bus2_uart;
    uartTransfer2.callback = uart2Callback;
    uartTransfer2.direction = read_uartDir;
    uartTransfer2.postTransferDelay = 0;
    uartTransfer2.readData = readBuffer2.data();
    uartTransfer2.readSize = RS485_MAX_SERIAL_FRAME_SIZE;
    uartTransfer2.result = &transfer2Status;
    uartSemaphore2.acquire();
    uartTransfer2.semaphore = uartSemaphore2.getSemaphore();

    int result = UART_queueTransfer(&uartTransfer1);
    if(result != 0) {
        // config error
        sif::error << "RS485PollingTask::initiateUartTransfers: Config error"
                << std::endl;
    }
    result = UART_queueTransfer(&uartTransfer2);
    if(result != 0) {
        // config error
        sif::error << "RS485PollingTask::initiateUartTransfers: Config error"
                << std::endl;
    }
}

void RS485PollingTask::pollUart() {
    ReturnValue_t result = uartSemaphore1.acquire();
    if(result == HasReturnvaluesIF::RETURN_OK) {
        handleTransferCompletion(readBuffer1.data(), transfer1bytesReceived,
                transfer1Status, config::RS485_MUTEX_TIMEOUT);
        int retval = UART_queueTransfer(&uartTransfer1);
        if(retval != 0) {
            otherErrorCount++;
        }
    }

    result = uartSemaphore2.acquire();
    if(result == HasReturnvaluesIF::RETURN_OK) {
        handleTransferCompletion(readBuffer2.data(), transfer2bytesReceived,
                transfer2Status, config::RS485_MUTEX_TIMEOUT);
        int retval = UART_queueTransfer(&uartTransfer2);
        if(retval != 0) {
            otherErrorCount++;
        }
    }

    if((parityErrorCount > 0) or (overrunErrorCount > 0) or
            (framingErrorCount > 0) or (otherErrorCount > 0)) {
        generateErrorEventResetCounters();

    }
}

void RS485PollingTask::uart1Callback(SystemContext context,
        xSemaphoreHandle sem) {
    transfer1bytesReceived = UART_getPrevBytesRead(bus2_uart);
    genericUartCallback(context, sem);
}

void RS485PollingTask::uart2Callback(SystemContext context,
        xSemaphoreHandle sem) {
    transfer2bytesReceived = UART_getPrevBytesRead(bus2_uart);
    genericUartCallback(context, sem);
}
