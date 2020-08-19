/**
 * @file 	AtmelArduinoHandler.cpp
 *
 * @date	13.04.2020
 * @author	R. Mueller
 */


#ifndef SAM9G20_BOARDTEST_ATMELARDUINOHANDLER_H_
#define SAM9G20_BOARDTEST_ATMELARDUINOHANDLER_H_

#include <sam9g20/comIF/cookies/I2cCookie.h>
#include <test/testdevices/ArduinoDeviceHandler.h>
#include <string>

class AtmelArduinoHandler: public ArduinoHandler {
public:
	AtmelArduinoHandler(object_id_t object_id, object_id_t com_if,
			CookieIF * cookie, uint8_t device_switch, std::string id_string);
	virtual ~AtmelArduinoHandler();

	void doStartUp() override;
	ReturnValue_t buildNormalDeviceCommand(DeviceCommandId_t* id) override;
	void printReply(uint8_t* reply, size_t reply_size) override;

	ReturnValue_t initialize() override;
private:
	inline static bool wait = true;
	enum ComInterfaceType: uint8_t {
		UNKNOWN,
		I2C,
		SPI
	};
	ComInterfaceType comType = UNKNOWN;

	void setI2cComType(I2cCommunicationType i2cComType);
	I2cCommunicationType getI2cComType() const;
	void setSpiMessages();
	static inline bool spiMessageAdapted = false;

	/**
	 * For consecutive write and read calls, call this when building a command.
	 * Reply length needs to be known
	 * @param receiveDataSize
	 */
	void setI2cReceiveDataSize(size_t receiveDataSize);
};


#endif /* SAM9G20_BOARDTEST_ATMELARDUINOHANDLER_H_ */
