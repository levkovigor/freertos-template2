#ifndef SAM9G20_COMIF_RS485DEVICECOMIF_H_
#define SAM9G20_COMIF_RS485DEVICECOMIF_H_

#include <fsfw/devicehandlers/DeviceCommunicationIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfwconfig/OBSWConfig.h>

extern "C" {

#include <hal/Drivers/UART.h>
}
class RS485DeviceComIF: public DeviceCommunicationIF, public SystemObject {
public:
    static constexpr uint8_t INTERFACE_ID = CLASS_ID::RS485_COM_IF;

    static constexpr ReturnValue_t RS485_INACTIVE = MAKE_RETURN_CODE(0x00);
	RS485DeviceComIF(object_id_t objectId);

    virtual ReturnValue_t initializeInterface(CookieIF * cookie) override;
    virtual ReturnValue_t sendMessage(CookieIF *cookie,
            const uint8_t * sendData, size_t sendLen) override;
    virtual ReturnValue_t getSendSuccess(CookieIF *cookie) override;
    virtual ReturnValue_t requestReceiveMessage(CookieIF *cookie,
            size_t requestLen) override;
    virtual ReturnValue_t readReceivedMessage(CookieIF *cookie,
            uint8_t **buffer, size_t *size) override;

	virtual ~RS485DeviceComIF();
private:

};

#endif /* BSP_COMIF_RS485DEVICECOMIF_H_ */
