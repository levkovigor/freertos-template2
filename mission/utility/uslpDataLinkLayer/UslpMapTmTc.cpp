#include "UslpMapTmTc.h"
#include "USLPTransferFrame.h"
#include "../ipc/QueueFactory.h"
#include "../serviceinterface/ServiceInterfaceStream.h"
#include "../storagemanager/StorageManagerIF.h"
#include "../tmtcpacket/SpacePacketBase.h"
#include "../tmtcservices/AcceptsTelecommandsIF.h"
#include "../tmtcservices/TmTcMessage.h"
#include <cstring>

UslpMapTmTc::UslpMapTmTc(uint8_t mapId, object_id_t tmSource, object_id_t tcDestination) :
        mapId(mapId), bufferPosition(packetBuffer), packetDestination(setPacketDestination), tcQueueId(
                MessageQueueIF::NO_QUEUE) {
    std::memset(packetBuffer, 0, sizeof(packetBuffer));
}

ReturnValue_t UslpMapTmTc::initialize() {
    packetStore = objectManager->get<StorageManagerIF>(objects::TC_STORE);
    AcceptsTelecommandsIF *distributor = objectManager->get<AcceptsTelecommandsIF>(
            packetDestination);
    if ((packetStore != NULL) && (distributor != NULL)) {
        tcQueueId = distributor->getRequestQueue();
        return RETURN_OK;
    } else {
        return RETURN_FAILED;
    }
}

ReturnValue_t UslpMapTmTc::extractPackets(USLPTransferFrame *frame) {
    ReturnValue_t status = TOO_SHORT_MAP_EXTRACTION;
    switch (segmentationFlag) {
    case NO_SEGMENTATION:
        status = unpackBlockingPackets(frame);
        break;
    case FIRST_PORTION:
        packetLength = frame->getDataLength();
        if (packetLength <= MAX_PACKET_SIZE) {
            memcpy(packetBuffer, frame->getDataField(), packetLength);
            bufferPosition = &packetBuffer[packetLength];
            status = RETURN_OK;
        } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "MapPacketExtraction::extractPackets. Packet too large! Size: "
                    << packetLength << std::endl;
#endif
            clearBuffers();
            status = CONTENT_TOO_LARGE;
        }
        break;
    case CONTINUING_PORTION:
    case LAST_PORTION:
        if (lastSegmentationFlag == FIRST_PORTION || lastSegmentationFlag == CONTINUING_PORTION) {
            packetLength += frame->getDataLength();
            if (packetLength <= MAX_PACKET_SIZE) {
                memcpy(bufferPosition, frame->getDataField(), frame->getDataLength());
                bufferPosition = &packetBuffer[packetLength];
                if (segmentationFlag == LAST_PORTION) {
                    status = sendCompletePacket(packetBuffer, packetLength);
                    clearBuffers();
                }
                status = RETURN_OK;
            } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
                sif::error << "MapPacketExtraction::extractPackets. Packet too large! Size: "
                        << packetLength << std::endl;
#endif
                clearBuffers();
                status = CONTENT_TOO_LARGE;
            }
        } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "MapPacketExtraction::extractPackets. Illegal segment! Last flag: "
                    << (int) lastSegmentationFlag << std::endl;
#endif
            clearBuffers();
            status = ILLEGAL_SEGMENTATION_FLAG;
        }
        break;
    default:
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "MapPacketExtraction::extractPackets. Illegal segmentationFlag: "
                << (int) segmentationFlag << std::endl;
#endif
        clearBuffers();
        status = DATA_CORRUPTED;
        break;
    }
    lastSegmentationFlag = segmentationFlag;
    return status;
}

ReturnValue_t MapPacketExtraction::unpackBlockingPackets(TcTransferFrame *frame) {
    ReturnValue_t status = TOO_SHORT_BLOCKED_PACKET;
    uint32_t totalLength = frame->getDataLength();
    if (totalLength > MAX_PACKET_SIZE)
        return CONTENT_TOO_LARGE;
    uint8_t *position = frame->getDataField();
    while ((totalLength > SpacePacketBase::MINIMUM_SIZE)) {
        SpacePacketBase packet(position);
        uint32_t packetSize = packet.getFullSize();
        if (packetSize <= totalLength) {
            status = sendCompletePacket(packet.getWholeData(), packet.getFullSize());
            totalLength -= packet.getFullSize();
            position += packet.getFullSize();
            status = RETURN_OK;
        } else {
            status = DATA_CORRUPTED;
            totalLength = 0;
        }
    }
    if (totalLength > 0) {
        status = RESIDUAL_DATA;
    }
    return status;
}

ReturnValue_t UslpMapTmTc::sendCompletePacket(uint8_t *data, uint32_t size) {
    store_address_t store_id;
    ReturnValue_t status = this->packetStore->addData(&store_id, data, size);
    if (status == RETURN_OK) {
        TmTcMessage message(store_id);
        status = MessageQueueSenderIF::sendMessage(tcQueueId, &message);
    }
    return status;
}

void UslpMapTmTc::clearBuffers() {
    memset(packetBuffer, 0, sizeof(packetBuffer));
    bufferPosition = packetBuffer;
    packetLength = 0;
}

void UslpMapTmTc::printPacketBuffer(void) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "DLL: packet_buffer contains: " << std::endl;
#endif
    for (uint32_t i = 0; i < this->packetLength; ++i) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "packet_buffer[" << std::dec << i << "]: 0x" << std::hex
                << (uint16_t) this->packetBuffer[i] << std::endl;
#endif
    }
}

uint8_t UslpMapTmTc::getMapId() const {
    return mapId;
}

