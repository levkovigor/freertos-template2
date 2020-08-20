#ifndef SAM9G20_TMTCBRIDGE_SERIALPOLLINGTASK_H_
#define SAM9G20_TMTCBRIDGE_SERIALPOLLINGTASK_H_

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/storagemanager/StorageManagerIF.h>
#include <fsfw/container/SimpleRingBuffer.h>
#include <fsfw/container/FIFO.h>
#include <fsfw/container/SharedRingBuffer.h>
#include <sam9g20/tmtcbridge/PusParser.h>

extern "C" {
#include <board.h>
#include <AT91SAM9G20.h>
#include <hal/Drivers/UART.h>
}

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
    static constexpr uint16_t MAX_RECEPTION_BUFFER_SIZE = 2048;
    static constexpr float DEFAULT_SERIAL_TIMEOUT_BAUDTICKS = 20;
    //! 12 bytes is minimal size of a PUS packet, which is used in the ctor
    //! to calculate this value.
    size_t maxNumberOfStoredPackets;

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
			size_t frameSize = 0,
			object_id_t sharedRingBufferId = objects::NO_OBJECT,
			uint16_t serialTimeoutBaudticks = DEFAULT_SERIAL_TIMEOUT_BAUDTICKS);

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

	//! Variable sized container to have flexible frame sizes.
	std::vector<uint8_t> recvBuffer;
	/** Will be used in read call to driver to determine how many bytes
	 * are read. Do not change after initialization! */
	size_t frameSize;

	UARTconfig configBus0 = { .mode = AT91C_US_USMODE_NORMAL |
			AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE |
			AT91C_US_OVER_16 | AT91C_US_NBSTOP_1_BIT, .baudrate = 115200,
			.timeGuard = 1, .busType = rs232_uart, .rxtimeout = 0xFFFF };

	PusParser* pusParser = nullptr;

	MessageQueueId_t targetTcDestination = MessageQueueIF::NO_QUEUE;

	/**
	 * @brief 	This function handles reading UART data.
	 * @details
	 * Calls the UART_read() function which blocks the task until the
	 * specified transfer is done.
	 *
	 * If TC packets are found, they are  forwarded to the internal software bus
	 * and the next UART_read() call is called immediately, so there
	 * should be a break large enough between telecommands so the software can
	 * call the next UART_read() in time.
	 */
	void pollUart();
	ReturnValue_t pollTc();
	void handleSuccessfulTcRead();


	/**
	 * Overrun error. Packet was still read. Maybe implement reading,
	 * not done for now.
	 * @return
	 */
	ReturnValue_t handleOverrunError();

	/**
	 * This function forwards the found PUS packets to the FSFW software bus.
	 * Critical section because the next read function should be called ASAP.
	 */
	void handleTc();
	void transferPusToSoftwareBus(uint16_t recvBufferIndex,
			uint16_t packetSize);
	void printTelecommand(uint8_t* tcPacket, uint16_t packetSize);

	void ringBufferPrototypePoll();
	object_id_t sharedRingBufferId;
	SharedRingBuffer* sharedRingBuffer = nullptr;
};

#endif /* SAM9G20_TMTCBRIDGE_SERIALPOLLINGTASK_H_ */
