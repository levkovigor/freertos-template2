#ifndef SAM9G20_COMIF_GPIODEVICECOMIF_H_
#define SAM9G20_COMIF_GPIODEVICECOMIF_H_

#include <fsfw/devicehandlers/DeviceCommunicationIF.h>
#include <fsfw/objectmanager/SystemObject.h>

extern "C" {
#include <at91/peripherals/pio/pio.h>
}

/**
 * @brief   Encapsulates access to the GPIO pins of the iOBC
 */
class GpioDeviceComIF: public DeviceCommunicationIF,
public SystemObject
{
public:
	GpioDeviceComIF(object_id_t objectId_);
	virtual ~GpioDeviceComIF();

	static void setGPIO(uint32_t address);
	static void clearGPIO(uint32_t address);
	static bool getGPIO(uint32_t address, uint8_t outputPin);

	static Pin pinSelect(uint32_t address);

	/**
	 * Used to SPI slave select decoder. See truth table of Housekeeping Board.
	 */
	static void enableDecoder1();
	static void enableDecoder2();
	static void enableDecoder3();
	static void enableDecoder4();
	static void disableDecoders();

	/**
	 * Used to select the decoder outputs. See SN74LVC138A-EP function table
	 */
	static void enableDecoderOutput1();
	static void enableDecoderOutput2();
	static void enableDecoderOutput3();
	static void enableDecoderOutput4();
	static void enableDecoderOutput5();
	static void enableDecoderOutput6();
	static void enableDecoderOutput7();
	static void enableDecoderOutput8();

	/**
	 * Used to select the RS485 Transceivers
	 */
	static void enableTransceiverFPGA1();
	static void enableTransceiverFPGA2();
	static void enableTransceiverPCDU();
	static void enableTransceiverVorago();
	static void enableTransceiverPIC24();

	/** DeviceCommunicationIF abstract function implementation */
	virtual ReturnValue_t initializeInterface(CookieIF * cookie) override;
	virtual ReturnValue_t sendMessage(CookieIF *cookie,
	        const uint8_t * sendData, size_t sendLen) override;
	virtual ReturnValue_t getSendSuccess(CookieIF *cookie) override;
	virtual ReturnValue_t requestReceiveMessage(CookieIF *cookie,
	        size_t requestLen) override;
	virtual ReturnValue_t readReceivedMessage(CookieIF *cookie,
	        uint8_t **buffer, size_t *size) override;

private:

};

#endif /* SAM9G20_COMIF_GPIODEVICECOMIF_H_ */
