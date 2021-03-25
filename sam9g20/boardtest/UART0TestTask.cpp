#include "UART0TestTask.h"

#include <fsfw/serviceinterface/ServiceInterface.h>

extern "C" {
#include <board.h>
#include <hal/Drivers/UART.h>
}

/* this is a test for the UART0 driver from ISIS
 * at the AT91SAM9G20:
 * bus0_uart TX = PB4
 * bus0_uart RX = PB5
 * This test will receive 4 characters over UART0, capitalize them and send them back.
 * If you send \"12ab\", you will receive back \"12AB\" on the same bus.
 */

UART0TestTask::UART0TestTask(const char * printName, object_id_t objectId) :
		        SystemObject(objectId), printName(printName), i(0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "UART0TestTask object created!" << std::endl;
#else
#endif

    retValInt = UART_start(bus0_uart, configBus0);
    if (retValInt != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "UARTtest: UART_start returned " << retValInt << "! bus 0" << std::endl;
#else
#endif
    }
}

ReturnValue_t UART0TestTask::performOperation(uint8_t operationCode){

    retValInt = UART_read(bus0_uart, readData, readSize);
    if (retValInt != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "\n\r taskUARTtest: UART_read returned: " << retValInt
                << " for bus 0" << std::endl;
#else
#endif
    }
    for (i = 0; i < readSize; i++) {
        if (readData[i] >= 'a' && readData[i] <= 'z') {
            writeData[i] = readData[i] - 'a' + 'A';
        } else {
            writeData[i] = readData[i];
        }
    }
    writeData[i] = '\n';
    writeData[i + 1] = '\r';

    // Write 2 bytes more than we received for \n\r
    retValInt = UART_write(bus0_uart, writeData, readSize + 2);
    if (retValInt != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "taskUARTtest: UART_write returned: " << retValInt << " for bus 0"
                << std::endl;
#else
#endif
    }
    return HasReturnvaluesIF::RETURN_OK;
}


UART0TestTask::~UART0TestTask() {
    // TODO Auto-generated destructor stub
}
