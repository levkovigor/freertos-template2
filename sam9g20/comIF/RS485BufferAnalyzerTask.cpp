#include "RS485BufferAnalyzerTask.h"
#include <mission/utility/uslpDataLinkLayer/UslpDataLinkLayer.h>
#include <fsfw/objectmanager/ObjectManager.h>


RS485BufferAnalyzerTask::RS485BufferAnalyzerTask(object_id_t objectId, object_id_t sharedRingBufferId,
        object_id_t UslpDataLinkLayerId) :
        SystemObject(objectId), sharedRingBufferId(sharedRingBufferId), UslpDataLinkLayerId(
                UslpDataLinkLayerId) {
}

RS485BufferAnalyzerTask::~RS485BufferAnalyzerTask() {
}

ReturnValue_t RS485BufferAnalyzerTask::initialize() {
    linkLayer = objectManager->get<UslpDataLinkLayer>(UslpDataLinkLayerId);
    if (linkLayer == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "RS485BufferAnalyzerTask::initialize: Data Link Layer invalid. Make sure"
                "it is created and set up properly." << std::endl;
#endif
        return ObjectManagerIF::CHILD_INIT_FAILED;
    }
    // TODO: Check if this fails
    linkLayer->initializeBuffer(receiveArray.data());

    // The ring buffer analyzer will run in performOperation
    SharedRingBuffer *ringBuffer = objectManager->get<SharedRingBuffer>(sharedRingBufferId);
    if (ringBuffer == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    bufferAnalyzer = new RingBufferAnalyzer(ringBuffer, virtualChannelFrameSizes,
            AnalyzerModes::USLP_FRAMES);


    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485BufferAnalyzerTask::performOperation(uint8_t opCode) {
    handleReceiveBuffer();
    return HasReturnvaluesIF::RETURN_OK;
}



ReturnValue_t RS485BufferAnalyzerTask::setvirtualChannelFrameSizes(
        std::map<uint8_t, size_t> *virtualChannelFrameSizes) {

    if (virtualChannelFrameSizes == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    } else if (virtualChannelFrameSizes->empty()) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    this->virtualChannelFrameSizes = virtualChannelFrameSizes;
    return HasReturnvaluesIF::RETURN_OK;

}

ReturnValue_t RS485BufferAnalyzerTask::handleReceiveBuffer() {
    for (uint8_t tcPacketIdx = 0; tcPacketIdx < MAX_TC_PACKETS_HANDLED; tcPacketIdx++) {
        size_t packetFoundLen = 0;
        ReturnValue_t result = analyzerTask->checkForPackets(receiveArray.data(),
                receiveArray.size(), &packetFoundLen);
        if (result == HasReturnvaluesIF::RETURN_OK) {
            result = linkLayer->processFrame(packetFoundLen);
            if (result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
                sif::debug << "RS485DeviceComIF::handleReceiveBuffer: Handling Buffer" << " failed!"
                        << std::endl;
#endif
                return result;
            }
        } else if (result == RingBufferAnalyzer::POSSIBLE_PACKET_LOSS) {
            // trigger event?
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::debug << "RS485DeviceComIF::handleReceiveBuffer: Possible data loss" << std::endl;
#endif
            continue;
        } else if (result == RingBufferAnalyzer::NO_PACKET_FOUND) {
            return HasReturnvaluesIF::RETURN_OK;
        }
    }

    return HasReturnvaluesIF::RETURN_OK;
}


