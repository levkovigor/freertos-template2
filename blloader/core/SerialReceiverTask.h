#ifndef BLLOADER_CORE_RECEIVER_H_
#define BLLOADER_CORE_RECEIVER_H_

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/serialize/SerialLinkedListAdapter.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <AT91SAM9G20.h>

extern "C" {
#include <hal/Drivers/UART.h>
}


enum class BinaryType: uint8_t {
    BOOTLOADER,
    NOR_FLASH,
    SD_CARD_1_SLOT1,
    SD_CARD_1_SLOT2,
    SD_CARD_2_SLOT1,
    SD_CARD_2_SLOT2
};

class LeadPacketData: public SerialLinkedListAdapter<SerializeIF> {
	SerializeElement<uint8_t> binaryType;
	SerializeElement<uint16_t> numberOfPackets;
	SerializeElement<uint32_t> binarySize;
	SerializeElement<uint8_t> hammingCodeIncluded;
};

class SerialReceiverTask: public SystemObject, public ExecutableObjectIF {
public:
	static constexpr uint16_t DEFAULT_TIMEOUT_BAUDTICKS = 20;
	static constexpr uint32_t DEFAULT_BAUDRATE = 115200;

	SerialReceiverTask(object_id_t objectId,
			uint32_t baudRate = DEFAULT_BAUDRATE,
			uint16_t timeoutBaudTicks = DEFAULT_TIMEOUT_BAUDTICKS);

	virtual ReturnValue_t performOperation(uint8_t opCode) override;
	virtual ReturnValue_t initialize() override;
private:
	void handleBinaryReception();

	unsigned int mode = AT91C_US_USMODE_NORMAL
			| AT91C_US_CLKS_CLOCK
			| AT91C_US_CHRL_8_BITS
			| AT91C_US_PAR_NONE
			| AT91C_US_NBSTOP_1_BIT
			| AT91C_US_CHMODE_NORMAL;
	UARTconfig uartConfig;
};

//void handle_binary_reception(void);
//void configure_usart0(uint32_t baud_rate, uint16_t timeout);
//void uart_polling_task();


#endif /* BLLOADER_CORE_RECEIVER_H_ */
