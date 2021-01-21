/**
 * @file    RS485TmTcTarget.h
 * @brief   This file defines the RS485TmTcTarget class.
 * @date    22.12.2020
 * @author  L. Rajer
 */
#include "RS485TmTcTarget.h"
#include <fsfw/ipc/QueueFactory.h>
#include <mission/utility/USLPTransferFrame.h>
#include <fsfwconfig/OBSWConfig.h>

RS485TmTcTarget::RS485TmTcTarget(object_id_t objectId, object_id_t tmStoreId) :
        SystemObject(objectId), tmStoreId(tmStoreId) {
    tmTcReceptionQueue = QueueFactory::instance()->createMessageQueue(TMTC_RECEPTION_QUEUE_DEPTH);
}

RS485TmTcTarget::~RS485TmTcTarget() {
}

ReturnValue_t RS485TmTcTarget::initialize() {

    tmStore = objectManager->get<StorageManagerIF>(tmStoreId);
    if (tmStore == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TmTcBridge::initialize: TM store invalid. Make sure"
                "it is created and set up properly." << std::endl;
#endif
        return ObjectManagerIF::CHILD_INIT_FAILED;
    }

    tmFifo = new DynamicFIFO<store_address_t>(maxNumberOfPacketsStored);

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
            overhangMessageSentBytes += config::RS485_COM_FPGA_TFDZ_SIZE;
            return HasReturnvaluesIF::RETURN_OK;
        }
    }

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

