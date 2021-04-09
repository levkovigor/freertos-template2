#include "DummyGPSComIF.h"

#include <fsfw/serialize/SerializeAdapter.h>
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/tmtcservices/CommandingServiceBase.h>
#include <fsfw/tmtcpacket/pus/TmPacketStored.h>
#include <fsfw/ipc/QueueFactory.h>
#include <devices/logicalAddresses.h>
#include <tmtc/apid.h>
#include <tmtc/pusIds.h>

#include <cstring>

DummyGPSComIF::DummyGPSComIF(object_id_t object_id_):
SystemObject(object_id_),maxReplyLen(0),testMode(testModes::NORMAL),
gpsReplyBufferSize(0), gpsReplyRingBuffer(1024,true),gpsDataBufferSize(0),
gpsReplyCounter(0), packetSubCounter(0)
{
    tmQueue = QueueFactory::instance()->createMessageQueue();
    funnel = objectManager->get<AcceptsTelemetryIF>(objects::TM_FUNNEL);
    tmQueue->setDefaultDestination(funnel->getReportReceptionQueue());
}

ReturnValue_t DummyGPSComIF::initializeInterface(CookieIF * cookie) {
    return RETURN_OK;
}

ReturnValue_t DummyGPSComIF::sendMessage(CookieIF *cookie, const uint8_t * sendData,
        size_t sendLen) {
    //DummyCookie * dummyCookie = dynamic_cast<DummyCookie*>(cookie);
    switch(testMode) {
    case(testModes::NORMAL): {
        gpsReplyCounter ++;
        prepareAckOrNackReply(sendData,&sendLen,AckType::ACK);
        //sendTmPacket(data,len);
        return RETURN_OK;
    }
    case(testModes::FAULTY): {
        gpsReplyCounter ++;
        prepareAckOrNackReply(sendData,&sendLen,AckType::NACK);
        //sendTmPacket(data,len);
        return RETURN_OK;
    }
    default:
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "DummyGPSComIF::sendMessage: Unknown Device Handler" << std::endl;
#else
        sif::printInfo("DummyGPSComIF::sendMessage: Unknown Device Handler\n");
#endif
        return RETURN_FAILED;
    }
}


void DummyGPSComIF::prepareAckOrNackReply(const uint8_t * data, size_t * len,uint8_t AckType) {
    if(*len < 7) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "DummyGPSComIF::prepareAckOrNackReply: Invalid packet length" << std::endl;
#else
        sif::printError("DummyGPSComIF::prepareAckOrNackReply: Invalid packet length\n");
#endif
        return;
    }
    uint8_t checksum = 0;
    uint8_t messageId = data[4];
    switch(messageId) {
    case(0x1): // restart gps device command
    case(0x4): // reset gps device command
    case(0x5): // set gps baud command
    case(0x9): // set message type command
    case(0xE): // set gps update rate command
    case(0x8): // set gps nmea output command
    {
        uint8_t reply[9] = {0xA0,0xA1,0x00,0x02,AckType,messageId,checksum,0x0D,0x0A};
        gpsReplyBufferSize = sizeof(reply);
        checksum = DummyGPSComIF::calcChecksum(
                reply + BINARY_HEADER_SIZE,
                gpsReplyBufferSize - BINARY_HEADER_AND_TAIL_SIZE);
        reply[gpsReplyBufferSize - 3] = checksum;
        if(gpsDataSizes.full()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "Dummy Com IF: Maximum number of allowed parallel commands "
                    "for GPS Dummy reached." << std::endl;
#else
            sif::printError("Dummy Com IF: Maximum number of allowed parallel commands "
                    "for GPS Dummy reached.\n");
#endif
            uint32_t oldestSize;
            gpsDataSizes.retrieve(reinterpret_cast<uint16_t *>(&oldestSize));
            gpsReplyRingBuffer.deleteData(oldestSize,false,NULL);
        }
        gpsDataSizes.insert(gpsReplyBufferSize);
        gpsReplyRingBuffer.writeData(reply,gpsReplyBufferSize);

        break;
    }
    default: {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "Dummy Com IF: Message Reply not implemented yet" << std::endl;
#else
        sif::printInfo("Dummy Com IF: Message Reply not implemented yet\n");
#endif
        break;
    }
    }
}


ReturnValue_t DummyGPSComIF::getSendSuccess(CookieIF *cookie) {
    return RETURN_OK;
}

ReturnValue_t DummyGPSComIF::requestReceiveMessage(CookieIF *cookie,
        size_t requestLen) {
    // debug << "ComIF Request Receive Message" << std::endl;
    return RETURN_OK;
}

ReturnValue_t DummyGPSComIF::readReceivedMessage(CookieIF *cookie,
        uint8_t **buffer, size_t *size) {
    // DummyCookie * dummyCookie = dynamic_cast<DummyCookie*>(cookie);
    // generated reply is sent back
    if(gpsReplyCounter == gpsReplyCountTrigger) {
        if(gpsDataSizes.empty()) {
            *size = 0;
            gpsReplyCounter = 0;
            return RETURN_OK;
        }
        //uint16_t dataLen;
        gpsDataSizes.retrieve(&gpsReplyBufferSize);
        *size = this -> gpsReplyBufferSize;
        gpsReplyRingBuffer.readData(gpsReplyBuffer,*size,false,NULL);
        gpsReplyRingBuffer.deleteData(*size,false,NULL);
        *buffer = this->gpsReplyBuffer;
        gpsReplyCounter = 0;
    }
    else {
        gpsReplyCounter ++;
        generateGpsPacket(cookie, buffer,size);
        this->gpsDataBufferSize = 0;
    }
    return RETURN_OK;
}


void DummyGPSComIF::generateGpsPacket(CookieIF * cookie, uint8_t ** gpsPacket,
        size_t * packetSize) {
    TestCookie * dummyCookie = dynamic_cast<TestCookie*>(cookie);
    if(dummyCookie == nullptr) {
        return;
    }
    //do this so the buffer persists when the function is exited. not very efficient
    //but I like to do a braced encloded intialization  for this huge buffer.
    gpsDataBufferSize = navMessage.getSerializedSize();
    if(dummyCookie->getAddress() == addresses::DUMMY_GPS0) {
        generateGps0Packet();
    }
    else if(dummyCookie->getAddress() == addresses::DUMMY_GPS1) {
        generateGps1Packet();
    }
    else {
        generateGps0Packet();
    }

    size_t navMessageSize = navMessage.getSerializedSize();
    if(navMessageSize > sizeof(gpsDataBuffer) or navMessageSize < 7) {
        return;
    }
    // calculate checksum of payload
    uint8_t checksum = DummyGPSComIF::calcChecksum(gpsDataBuffer + BINARY_HEADER_SIZE,
            navMessageSize - BINARY_HEADER_AND_TAIL_SIZE);
    gpsDataBuffer[navMessage.getSerializedSize()-3] = checksum;
    gpsDataBufferSize = navMessage.getSerializedSize();
    *gpsPacket = this->gpsDataBuffer;
    *packetSize = gpsDataBufferSize;
}

void DummyGPSComIF::generateGps0Packet() {
    uint8_t gpsData[gpsDataBufferSize] = {
            0xA0, 0xA1,
            0x00, 0x3b,
            0xA8,
            0x00, // fix mode
            0x02, // numbers of sv in fix
            0x06,0x04,
            0x02, 0x32, 0x18, 0x18,
            0x00, 0x00, 0x00, 0x01, // latitude
            0x48, 0x20, 0x78, 0xED,
            0x00, 0x00, 0x2E, 0x3B,
            0x00, 0x00, 0x26, 0x93,
            0x00, 0x93,
            0x00, 0x93,
            0x00, 0x93,
            0x00, 0x93,
            0x00, 0x93,
            0xEE, 0x35, 0x4D, 0x30,
            0x1D, 0x99, 0xAA, 0x37,
            0x0F, 0xD7, 0x0B, 0x47,
            0x00, 0x00, 0x00, 0x01, // velcoity ECEF x
            0x00, 0x00, 0x00, 0x02, // velocity ECEF y
            0x00, 0x00, 0x00, 0x03, // velocity ECEF z
            0xF5,
            0x0A, 0x0D
    };
    memcpy(gpsDataBuffer,gpsData,gpsDataBufferSize);
}

void DummyGPSComIF::generateGps1Packet() {
    uint8_t gpsData[gpsDataBufferSize] = {
            0xA0, 0xA1,
            0x00, 0x3b,
            0xA8,
            0x01, // fix mode
            0x08, // numbers of SV in fix
            0x06,0x04,
            0x02, 0x32, 0x18, 0x18,
            0x00, 0x00, 0x00, 0x01, // latitude
            0x48, 0x20, 0x78, 0xED,
            0x00, 0x00, 0x2E, 0x3B,
            0x00, 0x00, 0x26, 0x93,
            0x00, 0x93,
            0x00, 0x93,
            0x00, 0x93,
            0x00, 0x93,
            0x00, 0x93,
            0xEE, 0x35, 0x4D, 0x30,
            0x1D, 0x99, 0xAA, 0x37,
            0x0F, 0xD7, 0x0B, 0x47,
            0x00, 0x00, 0x00, 0x04, // velcoity ECEF x
            0x00, 0x00, 0x00, 0x05, // velocity ECEF y
            0x00, 0x00, 0x00, 0x06, // velocity ECEF z
            0xF5,
            0x0A, 0x0D
    };
    memcpy(gpsDataBuffer,gpsData,gpsDataBufferSize);
}

uint8_t DummyGPSComIF::calcChecksum(const uint8_t * payload,uint16_t payloadSize) {
    uint8_t checksum = 0;
    for(int i = 0; i < payloadSize; i++) {
        checksum = checksum ^ payload[i];
    }
    return checksum;
}

DummyGPSComIF::~DummyGPSComIF() {
}

