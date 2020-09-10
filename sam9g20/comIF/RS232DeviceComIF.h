/**
 * @file RS232DeviceComIF.h
 *
 * @date 30.30.2020
 */

#ifndef MISSION_COMIF_RS232DEVICECOMIF_H_
#define MISSION_COMIF_RS232DEVICECOMIF_H_

#include <fsfw/devicehandlers/DeviceCommunicationIF.h>
#include <fsfw/objectmanager/SystemObject.h>

// UART Driver, pre-compiled with C (libHal.a)
extern "C" {
#include <sam9g20/at91/include/at91/boards/ISIS_OBC_G20/at91sam9g20/AT91SAM9G20.h>
#include <hal/Drivers/UART.h>
}



class RS232DeviceComIF: public DeviceCommunicationIF, public SystemObject {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::RS232_CHANNEL;

	static const ReturnValue_t OPENING_ERROR = MAKE_RETURN_CODE(0x00); // could not configure and start RS232 connection

	RS232DeviceComIF(object_id_t object_id_);
	virtual ~RS232DeviceComIF();

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

	uint8_t buffer[2048];
	uint16_t len;
	UARTconfig setRS232Config(UARTconfig RS232Config);

};

#endif /* MISSION_COMMIF_DUMMYDEVICECOMIF_H_ */
