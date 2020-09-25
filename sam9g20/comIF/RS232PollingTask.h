#ifndef SAM9G20_TMTCBRIDGE_SERIALPOLLINGTASK_H_
#define SAM9G20_TMTCBRIDGE_SERIALPOLLINGTASK_H_

#include "ComConstants.h"

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/storagemanager/StorageManagerIF.h>
#include <fsfw/container/SharedRingBuffer.h>
#include <fsfw/objectmanager/SystemObjectIF.h>
#include <fsfw/objectmanager/frameworkObjects.h>
#include <fsfw/osal/FreeRTOS/BinarySemaphore.h>

extern "C" {
#include <board.h>
#include <AT91SAM9G20.h>
#include <hal/Drivers/UART.h>
}

#include <config/constants.h>

#include <array>

/**
 * @brief   Separate task to poll UART with ISIS drivers. Reads all data,
 * 			by keeping up two queue transfers which use DMA and callbacks.
 * @details
 * This polling task is agnostics to the data and simply reads all of it into
 * a shared ring buffer. Analyzing the ring buffer needs to be done by
 * another task.
 * @author 	R. Mueller
 */
class RS232PollingTask:  public SystemObject,
        public ExecutableObjectIF,
        public HasReturnvaluesIF {
    friend class TmTcSerialBridge;
    friend class RS232DeviceComIF;
public:

    static constexpr uint32_t RS232_BAUD_RATE = config::RS232_BAUDRATE;
    /** The frame size will be set to this value if no other value is
     *  supplied in the constructor. */
    static constexpr uint16_t RS232_MAX_SERIAL_FRAME_SIZE =
    		config::RS232_MAX_SERIAL_FRAME_SIZE;
    static constexpr float RS232_SERIAL_TIMEOUT_BAUDTICKS =
    		config::RS232_SERIAL_TIMEOUT_BAUDTICKS;

    /**
     * Default ctor.
     * @param objectId
     * @param tcBridge
     * Bridge object which just relays TC messages.
     * @param frameSize
     * Optional frame size which can be used if fixed TC
     * packet frames are used
     * @param serialTimeoutBaudticks Timeout in baudticks. Maxiumum uint16_t
     * value is reserved for the dafault timeout
     */
	RS232PollingTask(object_id_t objectId, object_id_t tcBridge,
			object_id_t sharedRingBufferId);

	virtual ~RS232PollingTask();

	virtual ReturnValue_t initialize();

	/**
	 * Executed periodically (set task priority and periodicity in init_mission)
	 * @param operationCode
	 * @return
	 */
	virtual ReturnValue_t performOperation(uint8_t operationCode = 0);

protected:
	StorageManagerIF* tcStore = nullptr;

private:
	object_id_t tcBridge = objects::NO_OBJECT;


	UARTconfig configBus0 = { .mode = AT91C_US_USMODE_NORMAL |
			AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE |
			AT91C_US_OVER_16 | AT91C_US_NBSTOP_1_BIT, .baudrate = 115200,
			.timeGuard = 0, .busType = rs232_uart, .rxtimeout = 0xFFFF };

	static bool uart0Started;

	UARTgenericTransfer uartTransfer1;
	static volatile uint8_t transfer1bytesReceived;
	BinarySemaphore uartSemaphore1;
	UARTtransferStatus transfer1Status = done_uart;
	std::array<uint8_t, RS232_MAX_SERIAL_FRAME_SIZE> readBuffer1;

	BinarySemaphore uartSemaphore2;
	static volatile uint8_t transfer2bytesReceived;
	UARTgenericTransfer uartTransfer2;
	UARTtransferStatus transfer2Status = done_uart;
	std::array<uint8_t, RS232_MAX_SERIAL_FRAME_SIZE> readBuffer2;


	/**
	 * @brief 	This function handles reading UART data in a permanent loop
	 * @details
	 */
	void initiateUartTransfers();
	void pollUart();
	ReturnValue_t pollTc();


	void ringBufferPrototypePoll();
	object_id_t sharedRingBufferId;
	SharedRingBuffer* sharedRingBuffer = nullptr;

	/**
	 * Overrun error. Packet was still read. Maybe implement reading,
	 * not done for now.
	 * @return
	 */
	ReturnValue_t handleOverrunError();

	static void uart1Callback(SystemContext context, xSemaphoreHandle sem);
	static void uart2Callback(SystemContext context, xSemaphoreHandle sem);
	static void genericUartCallback(SystemContext context,
			xSemaphoreHandle sem);

	uint32_t errorCounter = 0;
	ReturnValue_t lastError = HasReturnvaluesIF::RETURN_OK;
};


#endif /* SAM9G20_TMTCBRIDGE_SERIALPOLLINGTASK_H_ */
