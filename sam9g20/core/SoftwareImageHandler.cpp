#include <sam9g20/core/SoftwareImageHandler.h>

SoftwareImageHandler::SoftwareImageHandler(object_id_t objectId):
        SystemObject(objectId), actionHelper(this, nullptr) {
}

ReturnValue_t SoftwareImageHandler::performOperation(uint8_t opCode) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SoftwareImageHandler::initialize() {
    // set action helper queue here.
    return HasReturnvaluesIF::RETURN_OK;
}

void SoftwareImageHandler::copySdCardImageToNorFlash(SdCard sdCard,
        ImageSlot imageSlot) {
}

void SoftwareImageHandler::copyNorFlashImageToSdCards(SdCard sdCard,
        ImageSlot imageSlot) {
}

void SoftwareImageHandler::checkNorFlashImage() {
}

void SoftwareImageHandler::checkSdCardImage(SdCard sdCard,
        ImageSlot imageSlot) {
}
