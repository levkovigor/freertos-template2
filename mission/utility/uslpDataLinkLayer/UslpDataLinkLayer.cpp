#include "UslpDataLinkLayer.h"

#include <fsfw/globalfunctions/CRC.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>

#include "USLPTransferFrame.h"

UslpDataLinkLayer::UslpDataLinkLayer(object_id_t objectId, uint16_t set_scid) :
        SystemObject(objectId), spacecraftId(set_scid), frameBuffer(nullptr), receivedDataLength(0) {
    //Nothing to do except from setting the values above.
}

UslpDataLinkLayer::~UslpDataLinkLayer() {
}
ReturnValue_t UslpDataLinkLayer::initialize() {
    return RETURN_OK;
}

ReturnValue_t UslpDataLinkLayer::frameValidationCheck() {
    //Check TF_version number
    if (this->currentFrame.getVersionNumber() != FRAME_VERSION_NUMBER_DEFAULT) {
        return WRONG_TF_VERSION;
    }
    //Check SpaceCraft ID
    if (this->currentFrame.getSpacecraftId() != this->spacecraftId) {
        return WRONG_SPACECRAFT_ID;
    }
    //Check other header limitations:
    if (!this->currentFrame.truncatedFlagSet() && !this->currentFrame.sourceFlagSet()) {
        return NO_VALID_FRAME_TYPE;
    }

    if (USE_CRC) {
        return this->frameCheckCRC();
    }
    return RETURN_OK;
}

ReturnValue_t UslpDataLinkLayer::frameCheckCRC() {
    uint16_t checkValue = CRC::crc16ccitt(this->currentFrame.getFullFrame(),
            this->currentFrame.getFullFrameSize());
    if (checkValue == 0) {
        return RETURN_OK;
    } else {
        return CRC_FAILED;
    }

}

ReturnValue_t UslpDataLinkLayer::virtualChannelDemultiplexing() {
    uint8_t vcId = currentFrame.getVirtualChannelId();
    virtualChannelIterator iter = virtualChannels.find(vcId);
    if (iter == virtualChannels.end()) {
        return RETURN_FAILED;
    } else {
        return (iter->second)->frameAcceptanceAndReportingMechanism(&currentFrame);
    }
}

ReturnValue_t UslpDataLinkLayer::processFrame(uint16_t length) {
    receivedDataLength = length;
    USLPTransferFrame frame_candidate(frameBuffer, length - USLPTransferFrame::FRAME_OVERHEAD);
    this->currentFrame = frame_candidate;
    ReturnValue_t status = frameValidationCheck();
    if (status != RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "DataLinkLayer::processFrame: frame reception failed. "
                "Error code: " << std::hex << status << std::dec << std::endl;
#endif
//      currentFrame.print();
        return status;
    } else {
        return virtualChannelDemultiplexing();
    }
}

ReturnValue_t UslpDataLinkLayer::packFrame(const uint8_t *inputBuffer, size_t inputSize,
        uint8_t *outputBuffer, size_t outputSize, uint8_t vcId, uint8_t mapId) {
    virtualChannelIterator iter = virtualChannels.find(vcId);
    ReturnValue_t result = RETURN_FAILED;
    if (iter == virtualChannels.end()) {
        return VC_NOT_FOUND;
    }
    USLPTransferFrame *returnFrame = nullptr;
    result = (iter->second)->multiplexFrameMap(inputBuffer, inputSize, outputBuffer, outputSize,
            mapId, returnFrame);
    if (result == RETURN_OK) {
        finalizeFrame(returnFrame);
    }

    return result;
}

ReturnValue_t UslpDataLinkLayer::addVirtualChannel(uint8_t virtualChannelId,
        UslpVirtualChannelIF *object) {
    std::pair<virtualChannelIterator, bool> returnValue = virtualChannels.insert(
            std::pair<uint8_t, UslpVirtualChannelIF*>(virtualChannelId, object));
    if (returnValue.second == true) {
        return RETURN_OK;
    } else {
        return RETURN_FAILED;
    }
}

ReturnValue_t UslpDataLinkLayer::initializeBuffer(uint8_t *frameBuffer) {
    ReturnValue_t returnValue = RETURN_FAILED;
    //Set Virtual Channel ID to first virtual channel instance in this DataLinkLayer instance to avoid faulty information (e.g. 0) in the VCID.
    if (virtualChannels.begin() == virtualChannels.end()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "DataLinkLayer::initializeBuffer: No VC assigned to this DLL instance! "
                << std::endl;
#endif
        return RETURN_FAILED;
    }

    for (virtualChannelIterator iterator = virtualChannels.begin();
            iterator != virtualChannels.end(); iterator++) {
        returnValue = iterator->second->initialize();
        if (returnValue != RETURN_OK)
            break;
    }
    if (frameBuffer == nullptr) {
        returnValue = RETURN_FAILED;
    } else {
        this->frameBuffer = frameBuffer;
    }
    return returnValue;

}

void UslpDataLinkLayer::finalizeFrame(USLPTransferFrame *frame) {
    frame->setVersionNumber(FRAME_VERSION_NUMBER_DEFAULT);
    frame->setSpacecraftId(spacecraftId);
    frame->setSourceFlag(true);
    frame->setTruncatedFlag(true);

    // CRC
    uint16_t crc = CRC::crc16ccitt(frame->getFullFrame(),
            frame->getFullFrameSize() - frame->FRAME_CRC_SIZE);
    *(frame->getFullFrame() + frame->getFullFrameSize() - 2) = (crc & 0XFF00) >> 8; // CRCH
    *(frame->getFullFrame() + frame->getFullFrameSize() - 1) = (crc) & 0X00FF;      // CRCL
}



