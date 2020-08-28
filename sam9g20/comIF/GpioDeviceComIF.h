#ifndef BSP_COMIF_GPIO_DEVICECOMIF_H_
#define BSP_COMIF_GPIO_DEVICECOMIF_H_
#include <fsfw/devicehandlers/DeviceCommunicationIF.h>
#include <fsfw/objectmanager/SystemObject.h>

extern "C" {
#include "pio.h"
}

/**
 * @brief Encapsulates access to the GPIO pins of the iOBC
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
	 * @brief Device specific initialization, using the cookie.
	 * @details
	 * The cookie is already prepared in the factory. If the communication
	 * interface needs to be set up in some way and requires cookie information,
	 * this can be performed in this function, which is called on device handler
	 * initialization.
	 * @param cookie
	 * @return -@c RETURN_OK if initialization was successfull
	 * 		   - Everything else triggers failure event with returnvalue as parameter 1
	 */
	virtual ReturnValue_t initializeInterface(CookieIF * cookie);

	/**
	 * Called by DHB in the SEND_WRITE doSendWrite().
	 * This function is used to send data to the physical device
	 * by implementing and calling related drivers or wrapper functions.
	 * @param cookie
	 * @param data
	 * @param len
	 * @return -@c RETURN_OK for successfull send
	 *         - Everything else triggers failure event with returnvalue as parameter 1
	 */
	virtual ReturnValue_t sendMessage(CookieIF *cookie, const uint8_t * sendData,
			size_t sendLen);

	/**
	 * Called by DHB in the GET_WRITE doGetWrite().
	 * Get send confirmation that the data in sendMessage() was sent successfully.
	 * @param cookie
	 * @return -@c RETURN_OK if data was sent successfull
	 * 		   - Everything else triggers falure event with returnvalue as parameter 1
	 */
	virtual ReturnValue_t getSendSuccess(CookieIF *cookie);

	/**
	 * Called by DHB in the SEND_WRITE doSendRead().
	 * Request a reply.
	 * @param cookie
	 * @return -@c RETURN_OK to confirm the request for data has been sent.
	 *         -@c NO_READ_REQUEST if no request shall be made. readReceivedMessage()
	 *         	   will not be called in the respective communication cycle.
	 *         - Everything else triggers failure event with returnvalue as parameter 1
	 */
	virtual ReturnValue_t requestReceiveMessage(CookieIF *cookie, size_t requestLen);

	/**
	 * Called by DHB in the GET_WRITE doGetRead().
	 * This function is used to receive data from the physical device
	 * by implementing and calling related drivers or wrapper functions.
	 * @param cookie
	 * @param data
	 * @param len
	 * @return @c RETURN_OK for successfull receive
	 *         - Everything else triggers failure event with returnvalue as parameter 1
	 */
	virtual ReturnValue_t readReceivedMessage(CookieIF *cookie, uint8_t **buffer,
			size_t *size);

private:

};



#endif /* BSP_COMIF_GPIO_DEVICECOMIF_H_ */
