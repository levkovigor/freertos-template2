#ifndef SAM9G20_TMTCBRIDGE_SERIALPOLLINGTASK_H_
#define SAM9G20_TMTCBRIDGE_SERIALPOLLINGTASK_H_

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/storagemanager/StorageManagerIF.h>
#include <fsfw/container/SharedRingBuffer.h>
#include <fsfw/osal/FreeRTOS/BinarySemaphore.h>


extern "C" {
#include <board.h>
#include <AT91SAM9G20.h>
#include <hal/Drivers/UART.h>
}

#include <config/tmtc/tmtcSize.h>
#include <utility>
#include <vector>

/**
 * @brief   Separate task to poll UART with ISIS drivers. The provided read()
 *          call blocks the task !
 * @details
 * Polled data is sent to the serial bridge for further processing.
 * If the ctor frameSize parameter is supplied, it will be passed to the driver
 * read() call to stop reading after the specified frame size.
 * Otherwise, a default frame size will be used. In any case, a timeout
 * can be specified.
 * @author 	R. Mueller
 */
class TcSerialPollingTask:  public SystemObject,
        public ExecutableObjectIF,
        public HasReturnvaluesIF {
    friend class TmTcSerialBridge;
public:
    static constexpr uint32_t BAUD_RATE = 230400;
    static constexpr uint8_t TC_RECEPTION_QUEUE_DEPTH = 10;
    /** The frame size will be set to this value if no other value is
     *  supplied in the constructor. */
    static constexpr uint16_t SERIAL_FRAME_MAX_SIZE =
    		tmtcsize::MAX_SERIAL_FRAME_SIZE;
    static constexpr float DEFAULT_SERIAL_TIMEOUT_BAUDTICKS = 5;

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
	TcSerialPollingTask(object_id_t objectId, object_id_t tcBridge,
			object_id_t sharedRingBufferId,
			uint16_t serialTimeoutBaudticks = DEFAULT_SERIAL_TIMEOUT_BAUDTICKS,
			size_t frameSize = 0);

	virtual ~TcSerialPollingTask();

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

	bool started = false;

	UARTgenericTransfer uartTransfer1;
	static volatile uint8_t transfer1bytesReceived;
	BinarySemaphore uartSemaphore1;
	UARTtransferStatus transfer1Status = done_uart;
	std::array<uint8_t, SERIAL_FRAME_MAX_SIZE> readBuffer1;

	BinarySemaphore uartSemaphore2;
	static volatile uint8_t transfer2bytesReceived;
	UARTgenericTransfer uartTransfer2;
	UARTtransferStatus transfer2Status = done_uart;
	std::array<uint8_t, SERIAL_FRAME_MAX_SIZE> readBuffer2;


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
};


#endif /* SAM9G20_TMTCBRIDGE_SERIALPOLLINGTASK_H_ */
