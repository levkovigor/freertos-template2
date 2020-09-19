#include <fsfw/timemanager/Stopwatch.h>


#include <sam9g20/core/SoftwareImageHandler.h>

extern "C" {
#include <board.h>
#ifdef ISIS_OBC_G20
#include <hal/Storage/NORflash.h>
#elif defined(AT91SAM9G20_EK)
// include nand flash stuff here
#include <board_memories.h>
#include <at91/peripherals/pio/pio.h>
#include <at91/memories/nandflash/SkipBlockNandFlash.h>
#endif

}


#ifdef AT91SAM9G20_EK
/// Pins used to access to nandflash.
static const Pin pPinsNf[] = {PINS_NANDFLASH};
/// Nandflash device structure.
static struct SkipBlockNandFlash skipBlockNf;
/// Address for transferring command bytes to the nandflash.
static unsigned int cmdBytesAddr = BOARD_NF_COMMAND_ADDR;
/// Address for transferring address bytes to the nandflash.
static unsigned int addrBytesAddr = BOARD_NF_ADDRESS_ADDR;
/// Address for transferring data bytes to the nandflash.
static unsigned int dataBytesAddr = BOARD_NF_DATA_ADDR;
/// Nandflash chip enable pin.
static const Pin nfCePin = BOARD_NF_CE_PIN;
/// Nandflash ready/busy pin.
static const Pin nfRbPin = BOARD_NF_RB_PIN;
#endif

SoftwareImageHandler::SoftwareImageHandler(object_id_t objectId):
        SystemObject(objectId), actionHelper(this, nullptr) {

}

ReturnValue_t SoftwareImageHandler::performOperation(uint8_t opCode) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SoftwareImageHandler::initialize() {
    // set action helper queue here.
#ifdef ISIS_OBC_G20
    int result = NORflash_start();

    if(result != 0) {
        // should never happen ! we should power cycle if this happens.
        return result;
    }
#endif

    return HasReturnvaluesIF::RETURN_OK;
}

#ifdef ISIS_OBC_G20
//ReturnValue_t SoftwareImageHandler::copyBootloaderToNorFlash(
//        bool performHammingCheck) {
//    // The bootloader will be written to the NOR-Flash or NAND-Flash.
//
//    // NOR-Flash:
//    // 4 small NOR-Flash sectors will be reserved for now: 8192 * 4 = 32768
//
//    // Erase the 4 small sectors first. measure how long that takes.
//    Stopwatch stopwatch;
//    int result = NORFLASH_EraseSector(&NORFlash, NORFLASH_SA0_ADDRESS);
//    if(result != 0) {
//        return HasReturnvaluesIF::RETURN_FAILED;
//    }
//
//    result = NORFLASH_EraseSector(&NORFlash, NORFLASH_SA1_ADDRESS);
//    if(result != 0) {
//        return HasReturnvaluesIF::RETURN_FAILED;
//    }
//
//    result = NORFLASH_EraseSector(&NORFlash, NORFLASH_SA2_ADDRESS);
//    if(result != 0) {
//        return HasReturnvaluesIF::RETURN_FAILED;
//    }
//
//    result = NORFLASH_EraseSector(&NORFlash, NORFLASH_SA3_ADDRESS);
//    if(result != 0) {
//        return HasReturnvaluesIF::RETURN_FAILED;
//    }
//
//    stopwatch.stop(true);
//    stopwatch.start();
//
//    // we should consider a critical section here and extracting this function
//    // to a special task with the highest priority so it can not be interrupted.
//
//    // now we read the bootloader in 3-4 steps and write it to NOR-Flash
//
//    // readSdCardMagic1()
//    uint8_t* magicBuffer;
//    result = NORFLASH_WriteData(&NORFlash, NORFLASH_SA0_ADDRESS,
//            magicBuffer, NORFLASH_SMALL_SECTOR_SIZE);
//    if(result != 0) {
//        return HasReturnvaluesIF::RETURN_FAILED;
//    }
//
//    // readSdCardMagic2()
//    uint8_t* magicBuffer2;
//    result = NORFLASH_WriteData(&NORFlash, NORFLASH_SA1_ADDRESS,
//            magicBuffer2, NORFLASH_SMALL_SECTOR_SIZE);
//    if(result != 0) {
//        return HasReturnvaluesIF::RETURN_FAILED;
//    }
//
//    // readSdCardMagic3()
//    uint8_t* magicBuffer3;
//    size_t someRemainingSize = 0;
//    result = NORFLASH_WriteData(&NORFlash, NORFLASH_SA2_ADDRESS,
//            magicBuffer3, someRemainingSize);
//    if(result != 0) {
//        return HasReturnvaluesIF::RETURN_FAILED;
//    }
//
//    // readSdCardMagic4()
//    uint8_t* magicBuffer4;
//    someRemainingSize = 1;
//    if(someRemainingSize > 0) {
//        result = NORFLASH_WriteData(&NORFlash, NORFLASH_SA3_ADDRESS,
//                magicBuffer4, someRemainingSize);
//        if(result != 0) {
//            return HasReturnvaluesIF::RETURN_FAILED;
//        }
//    }
//
//    someRemainingSize = 2;
//
//    if(someRemainingSize > 0) {
//        // bootloader too large
//        return HasReturnvaluesIF::RETURN_FAILED;
//    }
//
//    return HasReturnvaluesIF::RETURN_OK;
//}

#elif defined(AT91SAM9G20_EK)

ReturnValue_t SoftwareImageHandler::copyBootloaderToNandFlash(
        bool performHammingCheck) {
    BOARD_ConfigureNandFlash(nfBusWidth);
    PIO_Configure(pPinsNf, PIO_LISTSIZE(pPinsNf));
    return HasReturnvaluesIF::RETURN_OK;
}
#endif

void SoftwareImageHandler::copySdCardImageToNorFlash(SdCard sdCard,
        ImageSlot imageSlot, bool performHammingCheck) {
}

void SoftwareImageHandler::copyNorFlashImageToSdCards(SdCard sdCard,
        ImageSlot imageSlot, bool performHammingCheck) {
}

void SoftwareImageHandler::checkNorFlashImage() {
}

void SoftwareImageHandler::checkSdCardImage(SdCard sdCard,
        ImageSlot imageSlot) {
}

MessageQueueId_t SoftwareImageHandler::getCommandQueue() const {
    return 0;
}

ReturnValue_t SoftwareImageHandler::executeAction(ActionId_t actionId,
        MessageQueueId_t commandedBy, const uint8_t *data, size_t size) {
    return HasReturnvaluesIF::RETURN_OK;
}
