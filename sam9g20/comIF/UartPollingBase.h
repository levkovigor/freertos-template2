#ifndef SAM9G20_COMIF_UARTPOLLINGBASE_H_
#define SAM9G20_COMIF_UARTPOLLINGBASE_H_

#include <fsfw/container/SharedRingBuffer.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <fsfw/tasks/ExecutableObjectIF.h>

extern "C" {
#include <hal/Drivers/UART.h>
}

/**
 * @brief   Base class for common for for the RS232 and RS485 polling tasks.
 */
class UartPollingBase: public SystemObject,
        public ExecutableObjectIF,
        public HasReturnvaluesIF {
public:
    UartPollingBase(object_id_t objectId,
            object_id_t sharedRingBufferId, UARTbus uartBus,
            UARTconfig* uartConfiguration);

    virtual ReturnValue_t initialize();

    void setEventReporting(bool enable);
protected:

    bool eventReportingEnabled = true;

    UARTbus uartBus = bus0_uart;
    object_id_t sharedRingBufferId;
    UARTconfig* uartConfiguration;
    SharedRingBuffer* sharedRingBuffer = nullptr;

    uint8_t parityErrorCount = 0;
    uint8_t overrunErrorCount = 0;
    uint8_t framingErrorCount = 0;
    uint8_t otherErrorCount = 0;

    ReturnValue_t lastError = HasReturnvaluesIF::RETURN_OK;

    ReturnValue_t handleTransferResult(UARTtransferStatus status);
    void generateErrorEventResetCounters();
};



#endif /* SAM9G20_COMIF_UARTPOLLINGBASE_H_ */
