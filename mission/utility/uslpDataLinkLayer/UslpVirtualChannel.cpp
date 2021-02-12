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
    // Check if VC datazone length matches datazone lenght of frame
    if (frame->getDataZoneSize() != tfdzSize) {
        return DATA_CORRUPTED;
    }
    // Check if First Header Offset is too large, if larger than tfdz size it should be 0xFFFF
    else if (frame->getFirstHeaderOffset() >= tfdzSize && frame->getFirstHeaderOffset() != 0xFFFF) {
        return DATA_CORRUPTED;
    }
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
        return VC_NOT_FOUND;
    } else {

        return (iter->second)->extractPackets(frame);
    }
}

ReturnValue_t UslpVirtualChannel::multiplexFrameMap(const uint8_t *inputBuffer, size_t inputSize,
        uint8_t *outputBuffer, size_t outputSize, uint8_t mapId, USLPTransferFrame *&returnFrame) {
    mapChannelIterator iter = mapChannels.find(mapId);
    if (iter == mapChannels.end()) {
        return VC_NOT_FOUND;
    }
    if (inputSize > tfdzSize) {
        return CONTENT_TOO_LARGE;
    }

    ReturnValue_t result = (iter->second)->packFrame(inputBuffer, inputSize, outputBuffer, outputSize, tfdzSize,
            returnFrame);
    if (result == RETURN_OK){
        returnFrame->setVirtualChannelId(channelId);
    }
    return result;
}
