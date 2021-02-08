#include "UslpMapTmTc.h"
#include "USLPTransferFrame.h"
#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/storagemanager/StorageManagerIF.h>
#include <fsfw/tmtcpacket/SpacePacketBase.h>
#include <fsfw/tmtcservices/AcceptsTelecommandsIF.h>
#include <fsfw/tmtcservices/TmTcMessage.h>
#include <cstring>

UslpMapTmTc::UslpMapTmTc(uint8_t mapId, object_id_t tcDestination, object_id_t tmStoreId,
        object_id_t tcStoreId) :
        mapId(mapId), bufferPosition(packetBuffer), tcDestination(tcDestination), tmStoreId(
                tmStoreId), tcStoreId(tcStoreId) {
    std::memset(packetBuffer, 0, sizeof(packetBuffer));
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

    tmTcReceptionQueue->setDefaultDestination(tcDistributor->getRequestQueue());

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
    ReturnValue_t status = TOO_SHORT_MAP_EXTRACTION;
    return status;
}

ReturnValue_t UslpMapTmTc::packFrameMapa() {
    return RETURN_OK;
}

ReturnValue_t UslpMapTmTc::packFrameMapp() {
    return RETURN_OK;
}

ReturnValue_t UslpMapTmTc::sendCompletePacket(uint8_t *data, uint32_t size) {
    store_address_t store_id;
    ReturnValue_t status = this->tcStore->addData(&store_id, data, size);
    if (status == RETURN_OK) {
        TmTcMessage message(store_id);
        // Default implementation: Relay TC messages to TC distributor directly.
        status = MessageQueueSenderIF::sendMessage(tmTcReceptionQueue->getDefaultDestination(),
                &message);
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

