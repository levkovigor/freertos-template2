#include "USLPTransferFrame.h"
#include "UslpVirtualChannel.h"
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>

UslpVirtualChannel::UslpVirtualChannel(uint8_t setChannelId, size_t tfdzSize) :
        channelId(setChannelId), tfdzSize(tfdzSize) {
    // Nothing to do here anymore
}

ReturnValue_t UslpVirtualChannel::initialize() {
    ReturnValue_t returnValue = RETURN_FAILED;

    for (mapChannelIterator iterator = mapChannels.begin(); iterator != mapChannels.end();
            iterator++) {
        returnValue = iterator->second->initialize();
        if (returnValue != RETURN_OK)
            break;
    }
    return returnValue;
}

ReturnValue_t UslpVirtualChannel::frameAcceptanceAndReportingMechanism(USLPTransferFrame *frame) {
    // TODO: Check if lengths of given frame and assigned length match
    return mapDemultiplexing(frame);
}

uint8_t UslpVirtualChannel::getChannelId() const {
    return channelId;
}

ReturnValue_t UslpVirtualChannel::addMapChannel(uint8_t mapId, UslpMapIF *object) {
    std::pair<mapChannelIterator, bool> returnValue = mapChannels.insert(
            std::pair<uint8_t, UslpMapIF*>(mapId, object));
    if (returnValue.second == true) {
        return RETURN_OK;
    } else {
        return RETURN_FAILED;
    }
}

ReturnValue_t UslpVirtualChannel::mapDemultiplexing(USLPTransferFrame *frame) {
    uint8_t mapId = frame->getMapId();
    mapChannelIterator iter = mapChannels.find(mapId);
    if (iter == mapChannels.end()) {
//      error << "VirtualChannelReception::mapDemultiplexing on VC " << std::hex << (int) channelId
//              << ": MapChannel " << (int) mapId << std::dec << " not found." << std::endl;
        return VC_NOT_FOUND;
    } else {
        return (iter->second)->extractPackets(frame);
    }
}

