#include "UslpMapDevice.h"
#include "USLPTransferFrame.h"
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <cstring>

UslpMapDevice::UslpMapDevice(uint8_t mapId, uint8_t* receiveBuffer, size_t receiveBufferSize) :
        mapId(mapId), receiveBuffer(receiveBuffer), receiveBufferSize(receiveBufferSize) {

    outputFrame = new USLPTransferFrame();
}

ReturnValue_t UslpMapDevice::initialize() {
    return RETURN_OK;
}

ReturnValue_t UslpMapDevice::extractPackets(USLPTransferFrame *frame) {
    // TODO: Mutex
    if(frame->getDataZoneSize() <= receiveBufferSize){
        (void) memcpy(receiveBuffer, frame->getDataZone(), frame->getDataZoneSize());
        return HasReturnvaluesIF::RETURN_OK;
    }
    return HasReturnvaluesIF::RETURN_FAILED;

}


ReturnValue_t UslpMapDevice::packFrame(const uint8_t *inputBuffer, size_t inputSize,
        uint8_t *outputBuffer, size_t outputSize, size_t tfdzSize,
        USLPTransferFrame *&returnFrame) {

    // Output Checks
    if (outputSize < tfdzSize + USLPTransferFrame::FRAME_OVERHEAD || outputBuffer == nullptr){
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    // Input checks
    if (inputSize > tfdzSize || inputBuffer == nullptr){
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    outputFrame->setFrameLocation(outputBuffer, tfdzSize);
    // The partially filled frame is passed back up
    returnFrame = outputFrame;
    (void) memcpy(outputFrame->getDataZone(), inputBuffer, inputSize);
    setFrameInfo(outputFrame);

    return HasReturnvaluesIF::RETURN_OK;
}


void UslpMapDevice::setFrameInfo(USLPTransferFrame *frame) {
    frame->setMapId(mapId);
    frame->setProtocolIdentifier(USLP_PROTOCOL_ID);
    frame->setTFDZConstructionRules(USLP_TFDZ_CONSTRUCTION_RULES);

}
uint8_t UslpMapDevice::getMapId() const {
    return mapId;
}



