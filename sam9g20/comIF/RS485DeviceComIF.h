#ifndef SAM9G20_COMIF_RS485DEVICECOMIF_H_
#define SAM9G20_COMIF_RS485DEVICECOMIF_H_

#include <fsfw/devicehandlers/DeviceCommunicationIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <sam9g20/comIF/cookies/RS485Cookie.h>
#include <fsfw/osal/FreeRTOS/BinarySemaphore.h>
#include <fsfw/osal/FreeRTOS/TaskManagement.h>
#include <fsfwconfig/OBSWConfig.h>
#include <fsfw/container/SharedRingBuffer.h>
#include <sam9g20/core/RingBufferAnalyzer.h>


extern "C" {

#include <hal/Drivers/UART.h>
}

struct RS485WriteTransfer{
	unsigned char * writeData;
	size_t 	sendLen;
	int status;
};


class RS485DeviceComIF: public DeviceCommunicationIF,
public SystemObject,
public ExecutableObjectIF{
public:

	static constexpr size_t TMTC_FRAME_MAX_LEN =
	    		config::RS485_MAX_SERIAL_FRAME_SIZE;
	    static constexpr uint8_t MAX_TC_PACKETS_HANDLED = 5;

	RS485DeviceComIF(object_id_t objectId, object_id_t sharedRingBufferId);
	virtual ~RS485DeviceComIF();


 /** ExecutableObjectIF overrides */
	ReturnValue_t performOperation(uint8_t opCode) override;
	ReturnValue_t initialize() override;

 /** ExecutableObjectIF overrides */
    virtual ReturnValue_t initializeInterface(CookieIF * cookie) override;
    virtual ReturnValue_t sendMessage(CookieIF *cookie,
            const uint8_t * sendData, size_t sendLen) override;
    virtual ReturnValue_t getSendSuccess(CookieIF *cookie) override;
    virtual ReturnValue_t requestReceiveMessage(CookieIF *cookie,
            size_t requestLen) override;
    virtual ReturnValue_t readReceivedMessage(CookieIF *cookie,
            uint8_t **buffer, size_t *size) override;

    static constexpr uint8_t INTERFACE_ID = CLASS_ID::RS485_COM_IF;

    static constexpr ReturnValue_t RS485_INACTIVE = MAKE_RETURN_CODE(0x00);


private:


    uint8_t retryCount = 0;
    ReturnValue_t checkDriverState(uint8_t* retryCount);


    std::array<RS485WriteTransfer, RS485Devices::DEVICE_COUNT_RS485> sendArray;

    object_id_t sharedRingBufferId;
    SharedRingBuffer* sharedRingBuffer = nullptr;
    RingBufferAnalyzer* analyzerTask = nullptr;

    std::array<uint8_t, TMTC_FRAME_MAX_LEN + 5> receiveArray;

    ReturnValue_t handleReceiveBuffer();
    ReturnValue_t handlePacketReception(size_t foundLen);
    static void genericUartCallback(SystemContext context,
            xSemaphoreHandle sem);

};

#endif /* BSP_COMIF_RS485DEVICECOMIF_H_ */
