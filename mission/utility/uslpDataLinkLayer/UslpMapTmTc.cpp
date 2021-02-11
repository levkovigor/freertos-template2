#include "UslpMapTmTc.h"
#include "USLPTransferFrame.h"
#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/storagemanager/StorageManagerIF.h>
#include <fsfw/tmtcpacket/SpacePacketBase.h>
#include <fsfw/tmtcservices/AcceptsTelecommandsIF.h>
#include <fsfw/tmtcservices/TmTcMessage.h>
#include <cstring>

UslpMapTmTc::UslpMapTmTc(object_id_t objectId, uint8_t mapId, object_id_t tcDestination,
        object_id_t tmStoreId, object_id_t tcStoreId) :
        SystemObject(objectId), mapId(mapId), bufferPosition(packetBuffer), tcDestination(
                tcDestination), tmStoreId(tmStoreId), tcStoreId(tcStoreId) {
    tmTcReceptionQueue = QueueFactory::instance()->createMessageQueue(TMTC_RECEPTION_QUEUE_DEPTH);
    std::memset(packetBuffer, 0, sizeof(packetBuffer));
    outputFrame = new USLPTransferFrame();
}

ReturnValue_t UslpMapTmTc::initialize() {
    tcStore = objectManager->get<StorageManagerIF>(tcStoreId);
    if (tcStore == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "UslpMapTmTc::initialize: TC store invalid. Make sure"
                "it is created and set up properly." << std::endl;
#endif
        return ObjectManagerIF::CHILD_INIT_FAILED;
    }

    AcceptsTelecommandsIF *tcDistributor = objectManager->get<AcceptsTelecommandsIF>(tcDestination);
    if (tcDistributor == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "UslpMapTmTc::initialize: TC Distributor invalid" << std::endl;
#endif
        return ObjectManagerIF::CHILD_INIT_FAILED;
    }

    tcQueueId = tcDistributor->getRequestQueue();
    tmTcReceptionQueue->setDefaultDestination(tcQueueId);

    tmStore = objectManager->get<StorageManagerIF>(tmStoreId);
    if (tmStore == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "RS485TmTcTarget::initialize: TM store invalid. Make sure"
                "it is created and set up properly." << std::endl;
#endif
        return ObjectManagerIF::CHILD_INIT_FAILED;
    }
    return RETURN_OK;
}

ReturnValue_t UslpMapTmTc::extractPackets(USLPTransferFrame *frame) {
    ReturnValue_t status = RETURN_OK;
    uint16_t headerOffset = frame->getFirstHeaderOffset();

    // End of packet at start of frame
    if (headerOffset > 0) {
        // Make sure there is enough space in buffer
        if (headerOffset <= MAX_PACKET_SIZE - (bufferPosition - packetBuffer)) {
            memcpy(bufferPosition, frame->getDataZone(), headerOffset);
            status = sendCompletePacket(packetBuffer, packetLength);
        } else {
            status = CONTENT_TOO_LARGE;
        }
        clearBuffers();
        // Middle of packet with no header
        if (headerOffset == 0xFFFF) {
            // TODO: Write this or make sure it does not happen
            return CONTENT_TOO_LARGE;
        }
    }
    if (status != RETURN_OK) {
        return status;
    }
    status = handleWholePackets(frame);

    return status;
}

ReturnValue_t UslpMapTmTc::handleWholePackets(USLPTransferFrame *frame) {
    ReturnValue_t status = TOO_SHORT_BLOCKED_PACKET;
    uint16_t totalLength = frame->getDataZoneSize() - frame->getFirstHeaderOffset();
    if (totalLength > MAX_PACKET_SIZE)
        return CONTENT_TOO_LARGE;
    uint8_t *position = frame->getFirstHeader();
    while (totalLength > SpacePacketBase::MINIMUM_SIZE) {
        // Check if idle filler packet
        // TODO: Better idle condition
        if (*position == 0) {
            break;
        }
        SpacePacketBase packet(position);
        uint32_t packetSize = packet.getFullSize();
        if (packetSize <= totalLength) {
            status = sendCompletePacket(packet.getWholeData(), packet.getFullSize());
            totalLength -= packet.getFullSize();
            position += packet.getFullSize();
            status = RETURN_OK;
        } else {
            break;
        }
    }
    // Start of another packet left or idle frames
    if (totalLength > 0) {
        // Check if idle filler packet
        // TODO: Better idle condition
        if (*position == 0) {
            return status;
        } else if (totalLength <= MAX_PACKET_SIZE) {
            memcpy(bufferPosition, position, totalLength);
            bufferPosition = &packetBuffer[totalLength];
        }
    }

    return status;
}
ReturnValue_t UslpMapTmTc::packFrame(uint8_t *inputBuffer, size_t inputSize, uint8_t *outputBuffer,
        size_t outputSize, size_t tfdzSize) {
#if FSFW_VERBOSE_LEVEL >= 1
    if (inputBuffer != nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "UslpMapTmTc::packFrame: Provided input buffer will be ignored"
                << std::endl;
#endif
    }
#endif
    outputFrame->setFrameLocation(outputBuffer, tfdzSize);
    TmTcMessage message;
    const uint8_t *data = nullptr;
    size_t size = 0;
    uint8_t bytesPackedCounter = 0;
    ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;

    // Handle Overhang first
    if (overhangMessage != nullptr) {

        result = tmStore->getData(overhangMessage->getStorageId(), &data, &size);

        if (size - overhangMessageSentBytes <= tfdzSize) {
            (void) std::memcpy(outputFrame->getDataZone(), data + overhangMessageSentBytes, size);
            bytesPackedCounter += size;
            overhangMessage = nullptr;
            overhangMessageSentBytes = 0;
            tmStore->deleteData(message.getStorageId());
        } else {
            (void) std::memcpy(outputFrame->getDataZone(), data + overhangMessageSentBytes,
                    tfdzSize);
            // Set first header pointer to 0xFFFF as no packet header is present
            outputFrame->setFirstHeaderOffset(0xFFFF);
            overhangMessageSentBytes += tfdzSize;
            return HasReturnvaluesIF::RETURN_OK;
        }
    }

    // Set first header pointer to position of first packet header
    outputFrame->setFirstHeaderOffset(bytesPackedCounter);

    while (tmTcReceptionQueue->receiveMessage(&message) == HasReturnvaluesIF::RETURN_OK) {

        result = tmStore->getData(message.getStorageId(), &data, &size);
        if (result != HasReturnvaluesIF::RETURN_OK) {
            continue;
        }

        if (size + bytesPackedCounter <= tfdzSize) {
            (void) std::memcpy(outputFrame->getDataZone() + bytesPackedCounter, data, size);
            tmStore->deleteData(message.getStorageId());
            bytesPackedCounter += size;
        } else {
            (void) std::memcpy(outputFrame->getDataZone() + bytesPackedCounter, data,
                    tfdzSize - bytesPackedCounter);
            // Storage for next frame
            overhangMessage = &message;
            overhangMessageSentBytes += tfdzSize - bytesPackedCounter;
            return HasReturnvaluesIF::RETURN_OK;
        }

    }

    return result;
}

ReturnValue_t UslpMapTmTc::sendCompletePacket(uint8_t *data, uint32_t size) {
    store_address_t store_id;
    ReturnValue_t status = this->tcStore->addData(&store_id, data, size);
    if (status == RETURN_OK) {
        TmTcMessage message(store_id);
        // Default implementation: Relay TC messages to TC distributor directly.
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

MessageQueueId_t UslpMapTmTc::getReportReceptionQueue(uint8_t virtualChannel) {
    return tmTcReceptionQueue->getId();
}

