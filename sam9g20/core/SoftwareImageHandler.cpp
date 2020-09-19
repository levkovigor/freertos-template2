#include <fsfw/timemanager/Stopwatch.h>


#include <sam9g20/core/SoftwareImageHandler.h>

extern "C" {
#ifdef ISIS_OBC_G20
#include <hal/Storage/NORflash.h>
#elif defined(AT91SAM9G20_EK)
// include nand flash stuff here
#include <at91/boards/at91sam9g20-ek/board.h>
#include <at91/boards/at91sam9g20-ek/board_memories.h>
#include <at91/peripherals/pio/pio.h>
#include <at91/utility/trace.h>
#include <at91/utility/hamming.h>
#include <at91/memories/nandflash/SkipBlockNandFlash.h>
#endif

}


#ifdef AT91SAM9G20_EK
//#define OP_BOOTSTRAP_on
/// Nandflash memory size.
static unsigned int memSize;
/// Size of one block in the nandflash, in bytes.
static unsigned int blockSize;
/// Number of blocks in nandflash.
static unsigned short numBlocks;
/// Size of one page in the nandflash, in bytes.
static unsigned short pageSize;
/// Number of page per block
static unsigned short numPagesPerBlock;
// Nandflash bus width
static unsigned char nfBusWidth = 16;

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
    if(oneShot) {
        Stopwatch stopwatch;
#if defined(AT91SAM9G20_EK)
        copyBootloaderToNandFlash(false, true);
        stopwatch.stop(true);
#endif
        oneShot = false;
    }

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



//#ifdef ISIS_OBC_G20
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

//#elif defined(AT91SAM9G20_EK)

ReturnValue_t SoftwareImageHandler::copyBootloaderToNandFlash(
        bool performHammingCheck, bool displayInfo) {
    if(not displayInfo) {
        setTrace(TRACE_LEVEL_WARNING);
    }
    BOARD_ConfigureNandFlash(nfBusWidth);
    PIO_Configure(pPinsNf, PIO_LISTSIZE(pPinsNf));
    ReturnValue_t result = nandFlashInit(displayInfo);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "SoftwareImageHandler::copyBootloaderToNandFlash: "
                << "Error initializing NAND-Flash." << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    // First block will be used for bootloader, so we erase it first.
    uint8_t retval = SkipBlockNandFlash_EraseBlock(&skipBlockNf, 0,
            NORMAL_ERASE);
    if(retval != 0) {
        sif::error << "SoftwareImageHandler::copyBootloaderToNandFlash: "
                 << "Error erasing first block." << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    uint8_t buffer[2048];
    std::memset(buffer, 1, 1024);
    std::memset(buffer + 1024, 5, 1024);
    retval = SkipBlockNandFlash_WritePage(&skipBlockNf, 0, 0, buffer, NULL);
    if(retval != 0) {
        sif::error << "SoftwareImageHandler::copyBootloaderToNandFlash: "
                << "Error writing to first block." << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    retval = SkipBlockNandFlash_ReadPage(&skipBlockNf, 0, 0, buffer, NULL);
    if(retval != 0) {
        sif::error << "SoftwareImageHandler::copyBootloaderToNandFlash: "
                << "Error reading from first block." << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    if(not displayInfo) {
        setTrace(TRACE_LEVEL_DEBUG);
    }

    sif::info << "Should be all 1: " << (int) buffer[0] << ", "
            << (int) buffer[1023] << std::endl;
    sif::info << "Should be all 5: " << (int) buffer[1025] << ", "
            << (int) buffer[2047] << std::endl;

    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SoftwareImageHandler::nandFlashInit(bool displayInfo)
{
    // Configure SMC for Nandflash accesses (done each time because of old ROM codes)
    BOARD_ConfigureNandFlash(nfBusWidth);
    PIO_Configure(pPinsNf, PIO_LISTSIZE(pPinsNf));

    //memset(&skipBlockNf, 0, sizeof(skipBlockNf));

    if (SkipBlockNandFlash_Initialize(&skipBlockNf, 0, cmdBytesAddr,
             addrBytesAddr, dataBytesAddr, nfCePin, nfRbPin)) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    // Check the data bus width of the NandFlash
    nfBusWidth = NandFlashModel_GetDataBusWidth((struct NandFlashModel *)&skipBlockNf);

    // Reconfigure bus width
    BOARD_ConfigureNandFlash(nfBusWidth);

    // Get device parameters
    memSize = NandFlashModel_GetDeviceSizeInBytes(&skipBlockNf.ecc.raw.model);
    blockSize = NandFlashModel_GetBlockSizeInBytes(&skipBlockNf.ecc.raw.model);
    numBlocks = NandFlashModel_GetDeviceSizeInBlocks(&skipBlockNf.ecc.raw.model);
    pageSize = NandFlashModel_GetPageDataSize(&skipBlockNf.ecc.raw.model);
    numPagesPerBlock = NandFlashModel_GetBlockSizeInPages(&skipBlockNf.ecc.raw.model);

    if(displayInfo) {
        TRACE_INFO("Size of the whole device in bytes : 0x%x \n\r",
                memSize);
        TRACE_INFO("Size in bytes of one single block of a device : 0x%x \n\r",
                blockSize);
        TRACE_INFO("Number of blocks in the entire device : 0x%x \n\r",
                numBlocks);
        TRACE_INFO("Size of the data area of a page in bytes : 0x%x \n\r",
                pageSize);
        TRACE_INFO("Number of pages in the entire device : 0x%x \n\r",
                numPagesPerBlock);
        TRACE_INFO("Bus width : %d \n\r",nfBusWidth);

    }

    return HasReturnvaluesIF::RETURN_OK;
}
//#endif

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
