#include "RS485Controller.h"
#include <OBSWConfig.h>
#include <fsfw/tasks/TaskFactory.h>


extern "C" {
#include <hal/Drivers/UART.h>
}
RS485Controller::RS485Controller(object_id_t objectId):
        SystemObject(objectId) {
}

ReturnValue_t RS485Controller::performOperation(uint8_t opCode) {
    RS485Steps step = static_cast<RS485Steps>(opCode);

    // check state of UART driver first, should be idle!
    // Returnvalue is ignored for now
    checkDriverState(&retryCount);

    switch(step) {
    case(SYRLINKS_ACTIVE): {
        // Activate transceiver and notify RS485 polling task by releasing
        // a semaphore so it can start sending packets.
        break;
    }
    case(PCDU_VORAGO_ACTIVE): {
        // Activate transceiver and notify RS485 polling task by releasing
        // a semaphore so it can start sending packets.
        break;
    }
    case(PL_VORAGO_ACTIVE): {
        // Activate transceiver and notify RS485 polling task by releasing
        // a semaphore so it can start sending packets.
        break;
    }
    case(PL_PIC24_ACTIVE): {
        // Activate transceiver and notify RS485 polling task by releasing
        // a semaphore so it can start sending packets.
        break;
    }
    default: {
        // should not happen
        break;
    }
    }

    // printout and event.
    if(retryCount > 0) {
#if OBSW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "RS485Controller::performOperation: RS485Controller"
                << " driver was busy for " << (uint16_t) retryCount
                << " attempts!" << std::endl;
#else
#endif
#endif
        retryCount = 0;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485Controller::initialize() {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485Controller::checkDriverState(uint8_t* retryCount) {
    UARTdriverState readState = UART_getDriverState(bus2_uart,
            read_uartDir);
    UARTdriverState writeState = UART_getDriverState(bus2_uart,
            read_uartDir);
    if(readState != 0x00 or writeState != 0x00) {
        if(readState == 0x33 or writeState == 0x33) {
            // errorneous state!
#if OBSW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "RS485Controller::performOperation: RS485 driver "
                    "in invalid state!" << std::endl;
#else
#endif
#endif
        }
        // config error, wait 1 ms and try again up to 10 times.
        for(uint8_t idx = 0; idx < RETRY_COUNTER; idx++) {
            TaskFactory::delayTask(1);
            readState = UART_getDriverState(bus2_uart,
                    read_uartDir);
            writeState = UART_getDriverState(bus2_uart,
                    read_uartDir);
            if(readState == 0x00 and writeState == 0x00) {
                return HasReturnvaluesIF::RETURN_OK;
            }
        }
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}
