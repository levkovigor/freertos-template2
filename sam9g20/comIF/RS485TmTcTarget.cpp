/**
 * @file    RS485TmTcTarget.h
 * @brief   This file defines the RS485TmTcTarget class.
 * @date    22.12.2020
 * @author  L. Rajer
 */
#include "RS485TmTcTarget.h"
#include <fsfw/ipc/QueueFactory.h>

RS485TmTcTarget::RS485TmTcTarget(object_id_t objectId, object_id_t tmStoreId) :
        SystemObject(objectId), tmStoreId(tmStoreId) {
    tmTcReceptionQueue = QueueFactory::instance()->
            createMessageQueue(TMTC_RECEPTION_QUEUE_DEPTH);
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

ReturnValue_t RS485TmTcTarget::fillSendFrameBuffer() {
    TmTcMessage message;
    const uint8_t *data = nullptr;
    size_t size = 0;
    ReturnValue_t result = tmTcReceptionQueue->receiveMessage(&message);
    result = tmStore->getData(message.getStorageId(), &data, &size);
    return result;
}

//ReturnValue_t RS485TmTcTarget::handleTmQueue() {
//    TmTcMessage message;
//    const uint8_t *data = nullptr;
//    size_t size = 0;
//    ReturnValue_t status = HasReturnvaluesIF::RETURN_OK;
//    for (ReturnValue_t result = tmTcReceptionQueue->receiveMessage(&message);
//            result == HasReturnvaluesIF::RETURN_OK;
//            result = tmTcReceptionQueue->receiveMessage(&message)) {
//
//        if (packetSentCounter >= sentPacketsPerCycle) {
//            storeDownlinkData(&message);
//            continue;
//        }
//
//        result = tmStore->getData(message.getStorageId(), &data, &size);
//        if (result != HasReturnvaluesIF::RETURN_OK) {
//            status = result;
//            continue;
//        }
//
//        result = sendTm(data, size);
//        if (result != HasReturnvaluesIF::RETURN_OK) {
//            status = result;
//        } else {
//            tmStore->deleteData(message.getStorageId());
//            packetSentCounter++;
//        }
//    }
//    return status;
//}
