#include "UslpReception.h"
#include "../globalfunctions/CRC.h"
#include "../serviceinterface/ServiceInterfaceStream.h"

UslpReception::UslpReception(uint8_t *set_frame_buffer, uint16_t set_scid) :
        spacecraftId(set_scid), frameBuffer(set_frame_buffer), receivedDataLength(0), currentFrame(
        NULL) {
    //Nothing to do except from setting the values above.
}

DataLinkLayer::~DataLinkLayer() {

}

ReturnValue_t UslpReception::frameValidationCheck() {
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

    //Compare detected frame length with the one in the header
    uint16_t length = currentFrame.getFullSize();
    if (length > receivedDataLength) {
        //Frame is too long or just right
//      error << "frameValidationCheck: Too short.";
//      currentFrame.print();
        return TOO_SHORT;
    }
    if (USE_CRC) {
        return this->frameCheckCRC();
    }
    return RETURN_OK;
}

ReturnValue_t UslpReception::frameCheckCRC() {
    uint16_t checkValue = CRC::crc16ccitt(this->currentFrame.getFullFrame(),
            this->currentFrame.getFullSize());
    if (checkValue == 0) {
        return RETURN_OK;
    } else {
        return CRC_FAILED;
    }

}

ReturnValue_t UslpReception::virtualChannelDemultiplexing() {
    uint8_t vcId = currentFrame.getVirtualChannelId();
    virtualChannelIterator iter = virtualChannels.find(vcId);
    if (iter == virtualChannels.end()) {
        return RETURN_FAILED;
    } else {
        return (iter->second)->frameAcceptanceAndReportingMechanism(&currentFrame, clcw);
    }
}

ReturnValue_t UslpReception::processFrame(uint16_t length) {
    receivedDataLength = length;
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

ReturnValue_t UslpReception::addVirtualChannel(uint8_t virtualChannelId,
        VirtualChannelReceptionIF *object) {
    std::pair<virtualChannelIterator, bool> returnValue = virtualChannels.insert(
            std::pair<uint8_t, VirtualChannelReceptionIF*>(virtualChannelId, object));
    if (returnValue.second == true) {
        return RETURN_OK;
    } else {
        return RETURN_FAILED;
    }
}

ReturnValue_t UslpReception::initialize() {
    ReturnValue_t returnValue = RETURN_FAILED;
    //Set Virtual Channel ID to first virtual channel instance in this DataLinkLayer instance to avoid faulty information (e.g. 0) in the VCID.
    if (virtualChannels.begin() == virtualChannels.end()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "DataLinkLayer::initialize: No VC assigned to this DLL instance! "
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
    return returnValue;

}

