#ifndef SAM9G20_TMTCBRIDGE_TMTCSERIALBRIDGE_H_
#define SAM9G20_TMTCBRIDGE_TMTCSERIALBRIDGE_H_

#include <fsfw/tmtcservices/TmTcBridge.h>
#include <fsfw/tmtcservices/AcceptsTelecommandsIF.h>
#include <sam9g20/utility/RingBufferAnalyzer.h>

extern "C" {
	#include <board.h>
	#include <AT91SAM9G20.h>
	#include <hal/Drivers/UART.h>
}

#include <config/constants.h>
#include <array>

/**
 * @brief 	Handles TM downlink via the serial interface, using the ISIS UART
 * 			drivers
 * @author 	R. Mueller
 */
class TmTcSerialBridge : public TmTcBridge {
    friend class RS232PollingTask;
public:
    static constexpr size_t TMTC_FRAME_MAX_LEN =
    		config::RS232_MAX_SERIAL_FRAME_SIZE;
    static constexpr uint8_t MAX_TC_PACKETS_HANDLED = 5;

	TmTcSerialBridge(object_id_t objectId_, object_id_t tcDistributor,
			object_id_t tmStoreId, object_id_t tcStoreId,
			object_id_t sharedRingBufferId);
	virtual ~TmTcSerialBridge();

	ReturnValue_t initialize() override;

	/**
	 * @param operationCode
	 * @return
	 */
	ReturnValue_t performOperation(uint8_t operationCode = 0) override;

	ReturnValue_t handleTc() override;

	/**
	 * TM Send implementation uses ISIS UART driver
	 * @param data
	 * @param dataLen
	 * @return
	 */
	ReturnValue_t sendTm(const uint8_t * data, size_t dataLen) override;
private:
	std::array<uint8_t, TMTC_FRAME_MAX_LEN + 5> tcArray;
	std::array<uint8_t, TMTC_FRAME_MAX_LEN + 5> tmArray;
	object_id_t sharedRingBufferId;
	RingBufferAnalyzer* analyzerTask = nullptr;

	ReturnValue_t handleTcReception(size_t foundLen);

};

#endif /* SAM9G20_TMTCBRIDGE_TMTCSERIALBRIDGE_H_ */
