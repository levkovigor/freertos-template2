#ifndef SAM9G20_TMTCBRIDGE_TMTCSERIALBRIDGE_H_
#define SAM9G20_TMTCBRIDGE_TMTCSERIALBRIDGE_H_

#include <fsfw/tmtcservices/TmTcBridge.h>
#include <fsfw/tmtcservices/AcceptsTelecommandsIF.h>
#include <sam9g20/comIF/SerialAnalyzerTask.h>

extern "C" {
	#include <sam9g20/at91/include/at91/boards/ISIS_OBC_G20/board.h>
	#include <sam9g20/at91/include/at91/boards/ISIS_OBC_G20/at91sam9g20/AT91SAM9G20.h>
	#include <hal/Drivers/UART.h>
}

#include <config/tmtc/tmtcSize.h>
#include <array>

/**
 * @brief 	Handles  TM downlink via the serial interface, using the ISIS UART
 * 			drivers
 * @author 	R. Mueller
 */
class TmTcSerialBridge : public TmTcBridge {
    friend class TcSerialPollingTask;
public:
    static constexpr uint16_t SERIAL_FRAME_LEN = 256;
    static constexpr size_t TMTC_FRAME_MAX_LEN = tmtcsize::MAX_SERIAL_FRAME_SIZE;
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
	std::array<uint8_t, TMTC_FRAME_MAX_LEN + 300> tcArray;
	std::array<uint8_t, TMTC_FRAME_MAX_LEN + 300> tmArray;
	object_id_t sharedRingBufferId;
	SerialAnalyzerTask* analyzerTask = nullptr;

	ReturnValue_t handleTcReception(size_t foundLen);

};

#endif /* SAM9G20_TMTCBRIDGE_TMTCSERIALBRIDGE_H_ */
