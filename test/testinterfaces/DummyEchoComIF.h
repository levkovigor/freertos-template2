#ifndef TEST_TESTDEVICES_DUMMYECHOCOMIF_H_
#define TEST_TESTDEVICES_DUMMYECHOCOMIF_H_

#include <test/testinterfaces/DummyCookie.h>

#include <fsfw/devicehandlers/DeviceCommunicationIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/tmtcservices/AcceptsTelemetryIF.h>

#include <vector>

/**
 * @brief Used to simply returned sent data from device handler
 * @details Assign this com IF in the factory when creating the device handler
 * @ingroup test
 */
class DummyEchoComIF: public DeviceCommunicationIF, public SystemObject {
public:
	DummyEchoComIF(object_id_t object_id_, bool initFunnel = false);
	virtual ~DummyEchoComIF();

	/**
	 * @brief Device specific initialization, using the cookie.
	 * @details
	 * The cookie is already prepared in the factory. If the communication
	 * interface needs to be set up in some way and requires cookie information,
	 * this can be performed in this function, which is called on device handler
	 * initialization.
	 * @param cookie
	 * @return -@c RETURN_OK if initialization was successfull
	 *         - Everything else triggers failure event with
	 *         returnvalue as parameter 1
	 */
	ReturnValue_t initializeInterface(CookieIF * cookie) override;

	/**
	 * Called by DHB in the SEND_WRITE doSendWrite().
	 * This function is used to send data to the physical device
	 * by implementing and calling related drivers or wrapper functions.
	 * @param cookie
	 * @param data
	 * @param len
	 * @return -@c RETURN_OK for successfull send
	 *         - Everything else triggers failure event with
	 *         returnvalue as parameter 1
	 */
	ReturnValue_t sendMessage(CookieIF *cookie, const uint8_t * sendData,
	        size_t sendLen) override;

	/**
	 * Called by DHB in the GET_WRITE doGetWrite().
	 * Get send confirmation that the data in sendMessage() was sent successfully.
	 * @param cookie
	 * @return -@c RETURN_OK if data was sent successfull
	 *         - Everything else triggers falure event with
	 *         returnvalue as parameter 1
	 */
	ReturnValue_t getSendSuccess(CookieIF *cookie) override;

	/**
	 * Called by DHB in the SEND_WRITE doSendRead().
	 * It is assumed that it is always possible to request a reply
	 * from a device. If a requestLen of 0 is supplied, no reply was enabled
	 * and communication specific action should be taken (e.g. read nothing
	 * or read everything).
	 *
	 * @param cookie
	 * @param requestLen Size of data to read
	 * @return -@c RETURN_OK to confirm the request for data has been sent.
	 *         - Everything else triggers failure event with
	 *         returnvalue as parameter 1
	 */
	ReturnValue_t requestReceiveMessage(CookieIF *cookie,
	        size_t requestLen) override;

	/**
	 * Called by DHB in the GET_WRITE doGetRead().
	 * This function is used to receive data from the physical device
	 * by implementing and calling related drivers or wrapper functions.
	 * @param cookie
	 * @param buffer [out] Set reply here (by using *buffer = ...)
	 * @param size [out] size pointer to set (by using *size = ...).
	 *             Set to 0 if no reply was received
	 * @return -@c RETURN_OK for successfull receive
	 *         -@c NO_REPLY_RECEIVED if not reply was received. Setting size to
	 *             0 has the same effect
	 *         - Everything else triggers failure event with
	 *           returnvalue as parameter 1
	 */
	ReturnValue_t readReceivedMessage(CookieIF *cookie, uint8_t **buffer,
	        size_t *size) override;

private:

	/**
	 * Send TM packet which contains received data as TM[17,130]. Wiretapping will do the same.
	 * @param data
	 * @param len
	 */
	void sendTmPacket(const uint8_t *data,uint32_t len);

	AcceptsTelemetryIF* funnel = nullptr;
	MessageQueueIF* tmQueue = nullptr;

	std::vector<uint8_t> replyBuffer;
	uint32_t dummyBufferSize = 0;

	uint8_t dummyReplyCounter = 0;

	uint16_t packetSubCounter = 0;
};

#endif /* TEST_TESTDEVICES_DUMMYDEVICECOMIF_H_ */
