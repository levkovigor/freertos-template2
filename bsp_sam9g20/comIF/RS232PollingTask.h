#ifndef SAM9G20_TMTCBRIDGE_SERIALPOLLINGTASK_H_
#define SAM9G20_TMTCBRIDGE_SERIALPOLLINGTASK_H_

#include "ComConstants.h"
#include "UartPollingBase.h"
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/container/SharedRingBuffer.h>
#include <fsfw/objectmanager/SystemObjectIF.h>
#include <fsfw/objectmanager/frameworkObjects.h>
#include <fsfw/osal/freertos/BinarySemaphore.h>
#include <OBSWConfig.h>

extern "C" {
#include <board.h>
#include <AT91SAM9G20.h>
#include <hal/Drivers/UART.h>
}

#include <array>

/**
 * @brief   Separate task to poll RS232 with ISIS drivers. Reads all data,
 * 			by keeping up two queue transfers which use DMA and callbacks.
 * @details
 * This polling task does not handle the data and simply reads all of it into
 * a shared ring buffer. Analyzing the ring buffer needs to be done by
 * another task.
 * @author 	R. Mueller
 */
class RS232PollingTask: public UartPollingBase {
    friend class TmTcSerialBridge;
    friend class RS232DeviceComIF;
public:

    static constexpr uint32_t RS232_BAUD_RATE = config::RS232_BAUDRATE;
    /** The frame size will be set to this value if no other value is
     *  supplied in the constructor. */
    static constexpr size_t RS232_MAX_SERIAL_FRAME_SIZE =
    		config::RS232_MAX_SERIAL_FRAME_SIZE +
			config::TRANSPORT_LAYER_ADDITION;
    static constexpr float RS232_SERIAL_TIMEOUT_BAUDTICKS =
    		config::RS232_SERIAL_TIMEOUT_BAUDTICKS;

    /**
     * Default ctor.
     * @param objectId
     * @param sharedRingBufferId
     * Data will be written into this ring buffer.
     */
	RS232PollingTask(object_id_t objectId, object_id_t sharedRingBufferId);

	ReturnValue_t performOperation(uint8_t opCode) override;
	ReturnValue_t initialize() override;

	virtual ~RS232PollingTask();

private:

	UARTconfig configBus0 = { .mode = AT91C_US_USMODE_NORMAL |
			AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE |
			AT91C_US_OVER_16 | AT91C_US_NBSTOP_1_BIT, .baudrate = 115200,
			.timeGuard = 0, .busType = rs232_uart, .rxtimeout = 0xFFFF };

	static volatile bool uart0Started;

	UARTgenericTransfer uartTransfer1 = {};
	static volatile size_t transfer1bytesReceived;
	BinarySemaphore uartSemaphore1;
	UARTtransferStatus transfer1Status = done_uart;
	std::array<uint8_t, RS232_MAX_SERIAL_FRAME_SIZE> readBuffer1;

	BinarySemaphore uartSemaphore2;
	static volatile size_t transfer2bytesReceived;
	UARTgenericTransfer uartTransfer2 = {};
	UARTtransferStatus transfer2Status = done_uart;
	std::array<uint8_t, RS232_MAX_SERIAL_FRAME_SIZE> readBuffer2;


	/**
	 * @brief 	This function handles reading UART data in a permanent loop
	 * @details
	 */
	void initiateUartTransfers();
	void pollUart();

	static void uart1Callback(SystemContext context, xSemaphoreHandle sem);
	static void uart2Callback(SystemContext context, xSemaphoreHandle sem);

};


#endif /* SAM9G20_TMTCBRIDGE_SERIALPOLLINGTASK_H_ */
