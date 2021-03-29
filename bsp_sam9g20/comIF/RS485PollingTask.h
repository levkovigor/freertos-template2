#ifndef SAM9G20_TMTCBRIDGE_RS485POLLINGTASK_H_
#define SAM9G20_TMTCBRIDGE_RS485POLLINGTASK_H_

#include <fsfw/devicehandlers/DeviceCommunicationIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/osal/FreeRTOS/BinarySemaphore.h>
#include <bsp_sam9g20/comIF/UartPollingBase.h>

extern "C" {
#include <AT91SAM9G20.h>
#include <hal/Drivers/UART.h>
}

#include <OBSWConfig.h>

/**
 * @brief   Separate task to poll RS485 with ISIS drivers. Reads all data,
 *          by keeping up two queue transfers which use DMA and callbacks.
 * @details
 * This polling task does not handle the data and simply reads all of it into
 * a shared ring buffer. Analyzing the ring buffer needs to be done by
 * another task.
 * @author  R. Mueller
 */
class RS485PollingTask: public UartPollingBase {
public:
    static constexpr uint32_t RS485_REGULARD_BAUD = config::RS485_REGULAR_BAUD;
    static constexpr uint32_t RS485_FAST_BAUD = config::RS485_FAST_BAUD;

    /** The frame size will be set to this value if no other value is
     *  supplied in the constructor. */
    static constexpr size_t RS485_MAX_SERIAL_FRAME_SIZE =
            config::RS485_MAX_SERIAL_FRAME_SIZE;
    static constexpr float RS485_SERIAL_TIMEOUT_BAUDTICKS =
            config::RS485_SERIAL_TIMEOUT_BAUDTICKS;

    RS485PollingTask(object_id_t objectId,
            object_id_t sharedRingBufferId);

    ReturnValue_t performOperation(uint8_t opCode) override;
    ReturnValue_t initialize() override;

private:

    UARTconfig configBus2 = { .mode = AT91C_US_USMODE_NORMAL |
            AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE |
            AT91C_US_OVER_16 | AT91C_US_NBSTOP_1_BIT, .baudrate = 115200,
            .timeGuard = 0, .busType = rs422_noTermination_uart,
            .rxtimeout = 0xFFFF };

    static bool uart2Started;

    UARTgenericTransfer uartTransfer1;
    static volatile size_t transfer1bytesReceived;
    BinarySemaphore uartSemaphore1;
    UARTtransferStatus transfer1Status = done_uart;
    std::array<uint8_t, config::RS485_MAX_SERIAL_FRAME_SIZE> readBuffer1;

    BinarySemaphore uartSemaphore2;
    static volatile size_t transfer2bytesReceived;
    UARTgenericTransfer uartTransfer2;
    UARTtransferStatus transfer2Status = done_uart;
    std::array<uint8_t, config::RS485_MAX_SERIAL_FRAME_SIZE> readBuffer2;
    UARTgenericTransfer writeStruct;
    BinarySemaphore writeSemaphore;
    UARTtransferStatus writeResult = UARTtransferStatus::done_uart;

    void initiateUartTransfers();
    void pollUart();

    static void uart1Callback(SystemContext context, xSemaphoreHandle sem);
    static void uart2Callback(SystemContext context, xSemaphoreHandle sem);
};



#endif /* SAM9G20_TMTCBRIDGE_RS485POLLINGTASK_H_ */
