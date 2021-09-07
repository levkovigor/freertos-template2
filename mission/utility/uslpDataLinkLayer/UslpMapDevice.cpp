#include "UslpMapDevice.h"
#include "USLPTransferFrame.h"
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <fsfw/ipc/MutexHelper.h>
#include <sam9g20/comIF/cookies/RS485Cookie.h>
#include <cstring>

UslpMapDevice::UslpMapDevice(uint8_t mapId, uint8_t *receiveBuffer, size_t receiveBufferSize,
        RS485Cookie* rs485Cookie) :
        mapId(mapId), receiveBuffer(receiveBuffer), receiveBufferSize(receiveBufferSize), rs485Cookie(rs485Cookie) {

    outputFrame = new USLPTransferFrame();
}

ReturnValue_t UslpMapDevice::initialize() {
    return RETURN_OK;
}

ReturnValue_t UslpMapDevice::extractPackets(USLPTransferFrame *frame) {
    ReturnValue_t result = RETURN_FAILED;
    // TODO: Better timeout value
    MutexHelper mutexLock(rs485Cookie->getSendMutexHandle(), MutexIF::TimeoutType::WAITING, 20);
    if (rs485Cookie->getComStatusReceive() == ComStatusRS485::TRANSFER_INIT){
        if (frame->getDataZoneSize() <= receiveBufferSize) {
              (void) memcpy(receiveBuffer, frame->getDataZone(), frame->getDataZoneSize());
              rs485Cookie->setComStatusReceive(ComStatusRS485::TRANSFER_SUCCESS);
              result = RETURN_OK;
          }else{
              rs485Cookie->setComStatusReceive(ComStatusRS485::FAULTY);
          }
    }else{
        result = RETURN_OK;
    }


    return result;

}

ReturnValue_t UslpMapDevice::packFrame(const uint8_t *inputBuffer, size_t inputSize,
        uint8_t *outputBuffer, size_t outputSize, size_t tfdzSize,
        USLPTransferFrame *&returnFrame) {

    // Output Checks
    if (outputSize < tfdzSize + USLPTransferFrame::FRAME_OVERHEAD || outputBuffer == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    // Input checks
    if (inputSize > tfdzSize || inputBuffer == nullptr) {
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

