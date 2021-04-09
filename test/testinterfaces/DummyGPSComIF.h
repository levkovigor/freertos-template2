#ifndef TEST_TESTDEVICES_DUMMYGPSCOMIF_H_
#define TEST_TESTDEVICES_DUMMYGPSCOMIF_H_

#include <fsfw/devicehandlers/DeviceCommunicationIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/tmtcservices/AcceptsTelemetryIF.h>
#include <fsfw/container/FIFO.h>
#include <fsfw/container/SimpleRingBuffer.h>
#include <fsfw/serialize/SerialLinkedListAdapter.h>
#include <test/testinterfaces/DummyCookie.h>

/**
 * @brief 	This file contains all GPS direct commands and
 * 			the structure of received GPS data.
 *
 */
class OwnGpsNavigationMessage: public SerialLinkedListAdapter<SerializeIF> {
public:
    OwnGpsNavigationMessage() {
        setLinks();
    }
    SerializeElement<uint16_t> startOfSequence; //!< GPSHandler::START_OF_SEQUENCE
    SerializeElement<uint16_t> payloadSize;
    SerializeElement<uint8_t> messageId; //!< DeviceCommandId_t GPSHandler::NAVIGATION_DATA_MESSAGE
    SerializeElement<uint8_t> fixMode; //!< enum GPSHandler::fixMode
    SerializeElement<uint8_t> numberOfSvInFix;
    SerializeElement<uint16_t> gnssWeek;
    SerializeElement<uint32_t> timeOfWeek;
    SerializeElement<int32_t> latitude;
    SerializeElement<int32_t> longitude;
    SerializeElement<uint32_t> ellipsoidAltitude;
    SerializeElement<uint32_t> meanSeaLevelAltitude;
    SerializeElement<uint16_t> gdop;
    SerializeElement<uint16_t> pdop;
    SerializeElement<uint16_t> hdop;
    SerializeElement<uint16_t> vdop;
    SerializeElement<uint16_t> tdop;
    SerializeElement<int32_t> ecefX;
    SerializeElement<int32_t> ecefY;
    SerializeElement<int32_t> ecefZ;
    SerializeElement<int32_t> ecefVx;
    SerializeElement<int32_t> ecefVy;
    SerializeElement<int32_t> ecefVz;
    SerializeElement<uint8_t> checksum;
    SerializeElement<uint16_t> endOfSequence; //!< GPSHandler::END_OF_SEQUENCE
private:
    //Forbid copying because of next pointer to member
    OwnGpsNavigationMessage(const OwnGpsNavigationMessage &message);
    void setLinks() {
        setStart(&startOfSequence);
        startOfSequence.setNext(&payloadSize);
        payloadSize.setNext(&messageId);
        messageId.setNext(&fixMode);
        fixMode.setNext(&numberOfSvInFix);
        numberOfSvInFix.setNext(&gnssWeek);
        gnssWeek.setNext(&timeOfWeek);
        timeOfWeek.setNext(&latitude);
        latitude.setNext(&longitude);
        longitude.setNext(&ellipsoidAltitude);
        ellipsoidAltitude.setNext(&meanSeaLevelAltitude);
        meanSeaLevelAltitude.setNext(&gdop);
        gdop.setNext(&pdop);
        pdop.setNext(&hdop);
        hdop.setNext(&vdop);
        vdop.setNext(&tdop);
        tdop.setNext(&ecefX);
        ecefX.setNext(&ecefY);
        ecefY.setNext(&ecefZ);
        ecefZ.setNext(&ecefVx);
        ecefVx.setNext(&ecefVy);
        ecefVy.setNext(&ecefVz);
        ecefVz.setNext(&checksum);
        checksum.setNext(&endOfSequence);
    }
};

/**
 * @brief Used to simulate GPS Device.
 * @details Analyzes sent data by GPS Device and generates replies
 * 	        If there is no reply to send, send generated GPS packet.
 * @ingroup test
 */
class DummyGPSComIF: public DeviceCommunicationIF, public SystemObject{
public:
    static constexpr uint8_t BINARY_HEADER_SIZE = 4;
    static constexpr uint8_t BINARY_HEADER_AND_TAIL_SIZE = 7;

    DummyGPSComIF(object_id_t object_id_);
    virtual ~DummyGPSComIF();

    virtual ReturnValue_t initializeInterface(CookieIF * cookie);
    /**
     * Called by DHB in the SEND_WRITE doSendWrite().
     * This function is used to send data to the physical device
     * by implementing and calling related drivers or wrapper functions.
     * @param cookie
     * @param data
     * @param len
     * @return @c RETURN_OK for successfull send
     *         Everything else triggers sending failed event with returnvalue as parameter 1
     */
    virtual ReturnValue_t sendMessage(CookieIF *cookie, const uint8_t * sendData,
            size_t sendLen) ;

    virtual ReturnValue_t getSendSuccess(CookieIF *cookie) ;

    virtual ReturnValue_t requestReceiveMessage(CookieIF *cookie, size_t requestLen) ;

    /**
     * Called by DHB in the GET_WIRTE doGetRead().
     * This function is used to receive data from the physical device
     * by implementing and calling related drivers or wrapper functions.
     * @param cookie
     * @param data
     * @param len
     * @return @c RETURN_OK for successfull receive
     *         Everything else triggers receiving failed with returnvalue as parameter 1
     */
    virtual ReturnValue_t readReceivedMessage(CookieIF *cookie, uint8_t **buffer,
            size_t *size) ;

private:
    /**
     * Send TM packet which contains received data as TM[17,130]. Wiretapping will do the same.
     * @param data
     * @param len
     */
    void sendTmPacket(const uint8_t * data,uint32_t len);

    /**
     * Send fake acknowledgement for GPS binary command to test gps device handler.
     * @param data
     * @param len
     * @param AckType 0 for ACK reply(0x83) or 1 for NACK reply(0x84)
     */
    void prepareAckOrNackReply(const uint8_t * data, size_t * len,uint8_t AckType);


    /**
     * Generate a fake GPS packet to test GPS Device Handler.
     * For device protocol, refer to Application Note AN0028 p.134 of Venus838 GPS receiver
     * @param gpsPacket
     * @param packetSize
     */
    void generateGpsPacket(CookieIF * cookie, uint8_t ** gpsPacket, size_t * packetSize);
    void generateGps0Packet();
    void generateGps1Packet();

    static uint8_t calcChecksum(const uint8_t * payload,uint16_t payloadSize);

    AcceptsTelemetryIF* funnel;
    MessageQueueIF* tmQueue;

    uint32_t maxReplyLen;

    uint8_t testMode;
    enum testModes {
        NORMAL,
        FAULTY
    };
    // We need separate buffers because sometimes, the reply is sent after a number of cycles and could
    // be overwritten. Additionally, we might have to store multiple replies
    // for now, use circular buffer for replies and fifo to store gpsReplySizes.
    // we can do it like this only if gpsReplyCountTrigger is constant, otherwise we would have to use a map !
    uint16_t gpsReplyBufferSize;  //!< Size of data to be returned
    SimpleRingBuffer gpsReplyRingBuffer;
    FIFO<uint16_t, 10> gpsDataSizes;
    uint8_t gpsReplyBuffer[128];

    uint8_t gpsDataBuffer[128];
    uint16_t gpsDataBufferSize;

    uint8_t gpsReplyCounter; //!< Used to fake the reponse time of device and to allow the sending of gps data
    uint8_t gpsReplyCountTrigger = 2; //!< At this value, the reply to a request will be sent back

    enum AckType {
        ACK = 0x83,//!< ACK
        NACK = 0x84//!< NACK
    };

    uint16_t packetSubCounter;

    OwnGpsNavigationMessage navMessage;
};

#endif /* TEST_TESTDEVICES_DUMMYDEVICECOMIF_H_ */
