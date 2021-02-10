/**
 * @file    RS485TmTcTarget.h
 * @brief   This file defines the RS485TmTcTarget class.
 * @date    22.12.2020
 * @author  L. Rajer
 */
#include "RS485TmTcTarget.h"
#include <fsfw/ipc/QueueFactory.h>
#include <fsfwconfig/OBSWConfig.h>
#include "../../mission/utility/uslpDataLinkLayer/USLPTransferFrame.h"
#include <mission/utility/uslpDataLinkLayer/UslpDataLinkLayer.h>

RS485TmTcTarget::RS485TmTcTarget(object_id_t objectId, object_id_t tcDestination,
        object_id_t tmStoreId, object_id_t tcStoreId, object_id_t sharedRingBufferId,
        object_id_t UslpDataLinkLayerId) :
        SystemObject(objectId), tcDestination(tcDestination), tmStoreId(tmStoreId), tcStoreId(
                tcStoreId), sharedRingBufferId(sharedRingBufferId), UslpDataLinkLayerId(UslpDataLinkLayerId) {
    tmTcReceptionQueue = QueueFactory::instance()->createMessageQueue(TMTC_RECEPTION_QUEUE_DEPTH);
}

RS485TmTcTarget::~RS485TmTcTarget() {
}

ReturnValue_t RS485TmTcTarget::initialize() {

    tcStore = objectManager->get<StorageManagerIF>(tcStoreId);
    if (tcStore == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "RS485TmTcTarget::initialize: TC store invalid. Make sure"
                "it is created and set up properly." << std::endl;
#endif
        return ObjectManagerIF::CHILD_INIT_FAILED;
    }

    AcceptsTelecommandsIF *tcDistributor = objectManager->get<AcceptsTelecommandsIF>(tcDestination);
    if (tcDistributor == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "RS485TmTcTarget::initialize: TC Distributor invalid" << std::endl;
#endif
        return ObjectManagerIF::CHILD_INIT_FAILED;
    }

    tmFifo = new DynamicFIFO<store_address_t>(maxNumberOfPacketsStored);

    tmTcReceptionQueue->setDefaultDestination(tcDistributor->getRequestQueue());

    tmStore = objectManager->get<StorageManagerIF>(tmStoreId);
    if (tmStore == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "RS485TmTcTarget::initialize: TM store invalid. Make sure"
                "it is created and set up properly." << std::endl;
#endif
        return ObjectManagerIF::CHILD_INIT_FAILED;
    }

    linkLayer = objectManager->get<UslpDataLinkLayer>(UslpDataLinkLayerId);
    if (linkLayer == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "RS485TmTcTarget::initialize: Data Link Layer invalid. Make sure"
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
    analyzerTask = new RingBufferAnalyzer(ringBuffer, virtualChannelFrameSizes,
            AnalyzerModes::USLP_FRAMES);

    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485TmTcTarget::performOperation(uint8_t opCode) {
    handleReceiveBuffer();
    return HasReturnvaluesIF::RETURN_OK;
}

MessageQueueId_t RS485TmTcTarget::getReportReceptionQueue(uint8_t virtualChannel) {
    return tmTcReceptionQueue->getId();
}

ReturnValue_t RS485TmTcTarget::fillSendFrameBuffer(USLPTransferFrame *frame) {
    TmTcMessage message;
    const uint8_t *data = nullptr;
    size_t size = 0;
    uint8_t bytesPackedCounter = 0;
    ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;

    // Handle Overhang first
    if (overhangMessage != nullptr) {

        result = tmStore->getData(overhangMessage->getStorageId(), &data, &size);

        if (size - overhangMessageSentBytes <= config::RS485_COM_FPGA_TFDZ_SIZE) {
            (void) std::memcpy(frame->getDataZone(), data + overhangMessageSentBytes, size);
            bytesPackedCounter += size;
            overhangMessage = nullptr;
            overhangMessageSentBytes = 0;
            tmStore->deleteData(message.getStorageId());
        } else {
            (void) std::memcpy(frame->getDataZone(), data + overhangMessageSentBytes,
                    config::RS485_COM_FPGA_TFDZ_SIZE);
            // Set first header pointer to 0xFFFF as no packet header is present
            frame->setFirstHeaderOffset(0xFFFF);
            overhangMessageSentBytes += config::RS485_COM_FPGA_TFDZ_SIZE;
            return HasReturnvaluesIF::RETURN_OK;
        }
    }

    // Set first header pointer to position of first packet header
    frame->setFirstHeaderOffset(bytesPackedCounter);

    while (tmTcReceptionQueue->receiveMessage(&message) == HasReturnvaluesIF::RETURN_OK) {

        result = tmStore->getData(message.getStorageId(), &data, &size);
        if (result != HasReturnvaluesIF::RETURN_OK) {
            continue;
        }

        if (size + bytesPackedCounter <= config::RS485_COM_FPGA_TFDZ_SIZE) {
            (void) std::memcpy(frame->getDataZone() + bytesPackedCounter, data, size);
            tmStore->deleteData(message.getStorageId());
            bytesPackedCounter += size;
        } else {
            (void) std::memcpy(frame->getDataZone() + bytesPackedCounter, data,
                    config::RS485_COM_FPGA_TFDZ_SIZE - bytesPackedCounter);
            // Storage for next frame
            overhangMessage = &message;
            overhangMessageSentBytes += config::RS485_COM_FPGA_TFDZ_SIZE - bytesPackedCounter;
            return HasReturnvaluesIF::RETURN_OK;
        }

    }

    return result;
}

ReturnValue_t RS485TmTcTarget::setvirtualChannelFrameSizes(
        std::map<uint8_t, size_t> *virtualChannelFrameSizes) {

    if (virtualChannelFrameSizes == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    } else if (virtualChannelFrameSizes->empty()) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    this->virtualChannelFrameSizes = virtualChannelFrameSizes;
    return HasReturnvaluesIF::RETURN_OK;

}

ReturnValue_t RS485TmTcTarget::handleReceiveBuffer() {
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


uint16_t RS485TmTcTarget::getIdentifier() {
    // This is no PUS service, so we just return 0
    return 0;
}

MessageQueueId_t RS485TmTcTarget::getRequestQueue() {
    // Default implementation: Relay TC messages to TC distributor directly.
    return tmTcReceptionQueue->getDefaultDestination();
}
