#ifndef SAM9G20_TMTCBRIDGE_RS485POLLINGTASK_H_
#define SAM9G20_TMTCBRIDGE_RS485POLLINGTASK_H_

#include <fsfw/devicehandlers/DeviceCommunicationIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/osal/FreeRTOS/BinarySemaphore.h>

extern "C" {
#include <hal/Drivers/UART.h>
}


class RS485PollingTask: public DeviceCommunicationIF, public SystemObject {
public:


    RS485PollingTask(object_id_t objectId);

    virtual ReturnValue_t initializeInterface(CookieIF * cookie) override;
    virtual ReturnValue_t sendMessage(CookieIF *cookie,
            const uint8_t * sendData, size_t sendLen) override;
    virtual ReturnValue_t getSendSuccess(CookieIF *cookie) override;
    virtual ReturnValue_t requestReceiveMessage(CookieIF *cookie,
            size_t requestLen) override;
    virtual ReturnValue_t readReceivedMessage(CookieIF *cookie,
            uint8_t **buffer, size_t *size) override;
private:
    UARTgenericTransfer writeStruct;
    BinarySemaphore writeSemaphore;
    UARTtransferStatus writeResult = UARTtransferStatus::done_uart;
};



#endif /* SAM9G20_TMTCBRIDGE_RS485POLLINGTASK_H_ */
