#ifndef SAM9G20_TMTCBRIDGE_TMTCSERIALBRIDGE_H_
#define SAM9G20_TMTCBRIDGE_TMTCSERIALBRIDGE_H_

#include <fsfw/tmtcservices/TmTcBridge.h>
#include <fsfw/tmtcservices/AcceptsTelecommandsIF.h>

extern "C" {
	#include "board.h"
	#include "AT91SAM9G20.h"
	#include <hal/Drivers/UART.h>
}

/**
 * @brief 	Handles  TM downlink via the serial interface, using the ISIS UART
 * 			drivers
 * @author 	R. Mueller
 */
class TmTcSerialBridge : public TmTcBridge {
    friend class TcSerialPollingTask;
public:
    static constexpr uint16_t SERIAL_FRAME_LEN = 256;

	TmTcSerialBridge(object_id_t objectId_, object_id_t tcDistributor,
			object_id_t tmStoreId, object_id_t tcStoreId);
	virtual ~TmTcSerialBridge();

	ReturnValue_t initialize() override;

	/**
	 * @param operationCode
	 * @return
	 */
	ReturnValue_t performOperation(uint8_t operationCode = 0) override;

	ReturnValue_t handleTmQueue() override;
	ReturnValue_t handleStoredTm() override;

	/**
	 * TM Send implementation uses ISIS UART driver
	 * @param data
	 * @param dataLen
	 * @return
	 */
	ReturnValue_t sendTm(const uint8_t * data, size_t dataLen) override;
private:

};

#endif /* SAM9G20_TMTCBRIDGE_TMTCSERIALBRIDGE_H_ */
