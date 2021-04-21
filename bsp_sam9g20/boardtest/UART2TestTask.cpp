#include "UART2TestTask.h"
#include <fsfw/osal/FreeRTOS/TaskManagement.h>

#include <fsfw/serviceinterface/ServiceInterface.h>

extern "C" {
#include <AT91SAM9G20.h>
#include <board.h>
#ifdef AT91SAM9G20_EK
#include "led_ek.h"
#endif
#include <at91/utility/trace.h>
}

/* this is a test for the UART2 driver from ISIS
 * at the AT91SAM9G20:
 * bus2_uart TX = PB8
 * bus2_uart RX = PB9
 * This test will receive 4 characters over UART2,
 * capitalize them and send them back.
 * If you send \"12ab\", you will receive back \"12AB\" on the same bus.
 */

UART2TestTask::UART2TestTask(const char * printName, object_id_t objectId):
        SystemObject(objectId), printName(printName), uartMode(SEND_TEST),
        uartState(WRITE) {
    configBus2.mode = AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK |
            AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE | AT91C_US_OVER_16 |
            AT91C_US_NBSTOP_1_BIT;
    configBus2.baudrate = 115200;
    configBus2.timeGuard = 1;
    configBus2.busType = rs422_withTermination_uart;
    configBus2.rxtimeout = 0xFFFF;
    retValInt = UART_start(uartBus2 , configBus2);
    if (retValInt != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "UARTtest: UART_start returned " << retValInt << " for bus 2" << std::endl;
#else
        sif::printWarning("UARTtest: UART_start returned %d for bus2\n", retValInt);
#endif
    }
}

void UART2TestTask::setRxTimeout(uint32_t timeoutMs) {
    configBus2.rxtimeout = (float) timeoutMs / 1000.0 * configBus2.baudrate;
}

UART2TestTask::~UART2TestTask() {}

ReturnValue_t UART2TestTask::performOperation(uint8_t operationCode){
    switch(uartMode) {
    case(SEND_TEST): {
        performSendTest();
        break;
    }
    case(READ_SEND_TEST): {
        performReadSendTest();
        break;
    }
    case(NON_BLOCKING): {
        performNonBlockingOperation();
        break;
    }
    }
    return HasReturnvaluesIF::RETURN_OK;
}

void UART2TestTask::performSendTest() {
    char data[] = "Hallo!";
    size_t dataLen = sizeof(data);

    retValInt = UART_write(bus2_uart,
            reinterpret_cast<unsigned char*>(data), dataLen);
    if (retValInt != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "taskUARTtest: UART_read returned: " << retValInt << " for bus 2" << std::endl;
#else
#endif
    }
}

void UART2TestTask::performReadSendTest() {
    retValInt = UART_read(bus2_uart, readData1, readSize1);
    if (retValInt != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "taskUARTtest: UART_read returned: " << retValInt << " for bus 2" << std::endl;
#else
#endif
    }
    unsigned int counter = 0;
    for (counter = 0; counter < readSize1; counter++) {
        if (readData1[counter] >= 'a' && readData1[counter] <= 'z') {
            writeData1[counter] = readData1[counter] - 'a' + 'A';
        } else {
            writeData1[counter] = readData1[counter];
        }
    }
    writeData1[counter] = '\n';
    writeData1[counter + 1] = '\r';

    // Write 2 bytes more than we received for \n\r
    retValInt = UART_write(bus2_uart, writeData1, readSize1 + 2);
    if (retValInt != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "taskUARTtest: UART_write returned: " << retValInt
                << " for bus 2" << std::endl;
#else
#endif
    }
}

void UART2TestTask::performNonBlockingOperation() {
    if(uartState == UartState::WRITE) {
        performNonBlockingWriteRead();
    }
    else if(uartState == UartState::WAIT_REPLY) {
        checkComStatus();
    }
}

void UART2TestTask::performNonBlockingWriteRead() {
    UARTgenericTransfer writeStruct;
    writeStruct.bus = uartBus2;
    writeStruct.direction = UARTdirection::write_uartDir;
    writeStruct.callback = UartCallback;
    writeStruct.postTransferDelay = 5;
    writeStruct.writeData = writeData.data();
    writeStruct.writeSize = writeData.size();
    writeStruct.result = &currentWriteStatus;
    writeStruct.semaphore = writeSemaphore.getSemaphore();

    UARTgenericTransfer readStruct;
    setRxTimeout(RX_TIMEOUT_MS);
    readStruct.bus = uartBus2;
    readStruct.direction = UARTdirection::read_uartDir;
    readStruct.callback = UartCallback;
    readStruct.postTransferDelay = 5;
    readData.reserve(MAX_REPLY_LEN);
    readStruct.readData = readData.data();
    readStruct.readSize = MAX_REPLY_LEN;
    readStruct.result = &currentReadStatus;
    readStruct.semaphore = readSemaphore.getSemaphore();


    ReturnValue_t result = writeSemaphore.acquire(SemaphoreIF::BLOCKING);
    if(result != RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "UART 2 Test Task: Could not take binary semaphore" << std::endl;
#else
#endif
    }

    int driverResult = UART_queueTransfer(&writeStruct);
    if(driverResult != 0)  {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "UART 2 Test Task: Transfer init failed with code "
                << driverResult << std::endl;
#else
#endif
    }

    result = readSemaphore.acquire(SemaphoreIF::BLOCKING);
    if(result != RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "UART 2 Test Task: Could not take binary semaphore" << std::endl;
#else
#endif
    }
    driverResult = UART_queueTransfer(&readStruct);
    if(driverResult != 0)  {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "UART 2 Test Task: Transfer init failed with code "
                << driverResult << std::endl;
#else
#endif
    }

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "UART 2 Test Task sent: [";
#else
#endif
    for (unsigned int count = 0;count != writeData.size(); count ++) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << std::hex << "0x" <<(int) writeData[count];

#else
#endif
        if(count < writeData.size() - 1) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << ", ";
#else
#endif
        }
    }

#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << std::dec << "]" << std::endl;
#else
#endif
    uartState = UartState::WAIT_REPLY;
}


void UART2TestTask::checkComStatus() {
    // try to take semaphore to check whether the write was successfull
    checkWriteStatus();
    checkReadStatus();

}

void UART2TestTask::checkWriteStatus() {
    ReturnValue_t result = writeSemaphore.acquire();
    if(result == BinarySemaphore::SEMAPHORE_TIMEOUT) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "UART 2 Test Task: Binary semaphore blocked when waiting write "
                "reply " << std::endl;
#else
#endif
    }
    else if(result != RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "UART Test Task: Configuration Error." << std::endl;
#else
#endif
    }
    else {
        uartState = UartState::WRITE;
        writeSemaphore.release();
    }
}

void UART2TestTask::checkReadStatus() {
    // try to take semaphore to check whether the write was successfull
    ReturnValue_t result = readSemaphore.acquire();
    if(result == BinarySemaphore::SEMAPHORE_TIMEOUT) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "UART 2 Test Task: Binary semaphore blocked when waiting read "
                "reply " << std::endl;
#else
#endif
    }
    else if(result != RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "I2C Test Task: Configuration Error." << std::endl;
#else
#endif
    }
    else {
        replySize = UART_getPrevBytesRead(uartBus2);
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "UART 2 Test Task received: [" ;
#else
#endif
        for (auto count = 0;count != replySize; count ++) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << std::hex << "0x" <<(int) readData[count];
#else
#endif
            if(count < replySize - 1) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
                sif::info << ", ";
#else
#endif
            }
        }
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << std::dec << "]" << std::endl;
#else
#endif
        readSemaphore.release();
    }
}

void UART2TestTask::UartCallback(SystemContext context,
        xSemaphoreHandle semaphore) {
    BaseType_t higherPriorityTaskAwoken = pdFALSE;
    ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
    //TRACE_INFO("SPI Callback reached\n\r");
    if(context == SystemContext::task_context) {
        result = BinarySemaphore::release(semaphore);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            TRACE_INFO("I2C Callback error !");
        }
    }
    else {
        result = BinarySemaphore::releaseFromISR(semaphore,
                &higherPriorityTaskAwoken);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            TRACE_INFO("I2C Callback error !");
        }
        if(higherPriorityTaskAwoken == pdPASS) {
            TaskManagement::requestContextSwitch(CallContext::ISR);
            TRACE_INFO("SPI Test: Higher Priority Task awoken !");
        }
    }
}
