#include "../ImageCopyingEngine.h"
#include <fsfw/timemanager/Countdown.h>
#include <fsfw/serviceinterface/ServiceInterface.h>

extern "C" {
// include nand flash stuff here
#include <at91/boards/at91sam9g20-ek/board.h>
#include <at91/boards/at91sam9g20-ek/board_memories.h>
#include <at91/peripherals/pio/pio.h>
#include <at91/utility/trace.h>
#include <at91/utility/hamming.h>
#include <at91/memories/nandflash/SkipBlockNandFlash.h>
}
#include <cinttypes>
#include <cmath>
#include <cstring>

ReturnValue_t ImageCopyingEngine::continueCurrentOperation() {
    switch(imageHandlerState) {
    case(ImageHandlerStates::IDLE): {
        return HasReturnvaluesIF::RETURN_OK;
    }
    case(ImageHandlerStates::COPY_IMG_SDC_TO_FLASH): {
        if(not nandConfigured) {
            ReturnValue_t result = configureNand(true);
            if(result != HasReturnvaluesIF::RETURN_OK) {
                return result;
            }
            nandConfigured = true;
        }
        return copySdCardImageToNandFlash();
    }
    case(ImageHandlerStates::COPY_IMG_SDC_TO_SDC): {
        return copySdcImgToSdc();
    }
    case(ImageHandlerStates::COPY_IMG_FLASH_TO_SDC): {
        break;
    }
    case(ImageHandlerStates::COPY_BL_FRAM_TO_FLASH): {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    case(ImageHandlerStates::COPY_BL_SDC_TO_FLASH): {
        if(not nandConfigured) {
            ReturnValue_t result = configureNand(true);
            if(result != HasReturnvaluesIF::RETURN_OK) {
                return result;
            }
            nandConfigured = true;
        }
        return copySdCardImageToNandFlash();
        break;
    }
    case(ImageHandlerStates::COPY_BL_SDC_TO_FRAM): {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    case(ImageHandlerStates::COPY_BL_HAMMING_SDC_TO_FRAM): {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    case(ImageHandlerStates::COPY_IMG_HAMMING_SDC_TO_FRAM): {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    }
    return HasReturnvaluesIF::RETURN_OK;
}

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

#if BOOTLOADER_TYPE == BOOTLOADER_TWO_STAGE
ReturnValue_t ImageCopyingEngine::startBootloaderToFlashOperation(bool fromFram,
        bool secondStageBootloader)
#else
ReturnValue_t ImageCopyingEngine::startBootloaderToFlashOperation(bool fromFram)
#endif
{
    bootloader = true;
    /* Not implemented yet */
    if(fromFram) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    else {
        imageHandlerState = ImageHandlerStates::COPY_BL_SDC_TO_FLASH;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t ImageCopyingEngine::copySdCardImageToNandFlash() {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;

    if(internalState == GenericInternalState::IDLE) {
        internalState = GenericInternalState::STEP_1;
    }

    if(internalState == GenericInternalState::STEP_1) {
        result = handleNandErasure();
        if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }
    }

    if(internalState == GenericInternalState::STEP_2) {
        result = handleSdToNandCopyOperation();
        if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }
    }

    return HasReturnvaluesIF::RETURN_OK;
}



ReturnValue_t ImageCopyingEngine::configureNand(bool disableDebugOutput) {
    if(disableDebugOutput) {
        setTrace(TRACE_LEVEL_WARNING);
    }
    ReturnValue_t result = nandFlashInit();
    if(result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "SoftwareImageHandler::copyBootloaderToNandFlash: "
                "Error initializing NAND-Flash." << std::endl;
#else
        sif::printError("SoftwareImageHandler::copyBootloaderToNandFlash: "
                "Error initializing NAND-Flash.\n");
#endif
    }
    if(disableDebugOutput) {
        setTrace(TRACE_LEVEL_DEBUG);
    }
    return result;
}

ReturnValue_t ImageCopyingEngine::handleNandErasure(bool disableAt91Output) {
    if(disableAt91Output) {
        setTrace(TRACE_LEVEL_WARNING);
    }

    if(countdown->hasTimedOut()) {
        return SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
    }

    if(bootloader) {
        /* First block will be used for bootloader, so we erase it first. */
        int retval = SkipBlockNandFlash_EraseBlock(&skipBlockNf, 0,
                NORMAL_ERASE);
        if(retval != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SoftwareImageHandler::copyBootloaderToNandFlash: "
                    << "Error erasing first block." << std::endl;
#else
            sif::printError("SoftwareImageHandler::copyBootloaderToNandFlash: "
                    "Error erasing first block.\n");
#endif
            /* If this happens, this won't work anyway */
            return HasReturnvaluesIF::RETURN_FAILED;
        }
    }
#if BOOTLOADER_TYPE == BOOTLOADER_TWO_STAGE
    else if(secondBootloader) {
        /* First block will be used for bootloader, so we erase it first. */
        int retval = SkipBlockNandFlash_EraseBlock(&skipBlockNf, 1,
                NORMAL_ERASE);
        if(retval != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SoftwareImageHandler::copyBootloaderToNandFlash: "
                    << "Error erasing second block." << std::endl;
#else
            sif::printError("SoftwareImageHandler::copyBootloaderToNandFlash: "
                    "Error erasing second block.\n");
#endif
            /* If this happens, this won't work anyway */
            return HasReturnvaluesIF::RETURN_FAILED;
        }
    }
#endif
    else {
        ReturnValue_t result = handleErasingForObsw();
        if(result == SoftwareImageHandler::TASK_PERIOD_OVER_SOON) {
            return SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
        }
        else if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }
    }

    if(disableAt91Output) {
        setTrace(TRACE_LEVEL_DEBUG);
    }

    internalState = GenericInternalState::STEP_2;

    if(countdown->hasTimedOut()) {
        return SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t ImageCopyingEngine::handleErasingForObsw() {
    // Make sure that the current file size and required blocks are only
    // cached once by using the helperFlag1.
    if(not helperFlag1) {
        SDCardAccess sdCardAccess;
        int result = change_directory(config::SW_REPOSITORY, true);
        if(result != F_NO_ERROR) {
            // The hardcoded repository does not exist. Exit!
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "ImageCopyingHelper::handleErasingForObsw: Software repository does "
                    "not exist. Cancelling erase operation!" << std::endl;
#else
            sif::printError("ImageCopyingHelper::handleErasingForObsw: Software repository does "
                    "not exist. Cancelling erase operation!\n");
#endif
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        if(sourceSlot == ImageSlot::SDC_SLOT_0) {
            currentFileSize = f_filelength(config::SW_SLOT_0_NAME);
        }
        else if(sourceSlot == ImageSlot::SDC_SLOT_1) {
            currentFileSize = f_filelength(config::SW_SLOT_1_NAME);
        }

        helperFlag1 = true;
        helperCounter1 = 0;
        uint8_t requiredBlocks = std::ceil(
                static_cast<float>(currentFileSize) /
                (PAGES_PER_BLOCK * NAND_PAGE_SIZE));
        /* We start counting at one to skip the bootloader, so we need to add one here. */
#if BOOTLOADER_TYPE == BOOTLOADER_ONE_STAGE
        helperCounter2 = requiredBlocks + 1;
#else
        helperCounter2 = requiredBlocks + 2;
#endif /* BOOTLOADER_TYPE == BOOTLOADER_ONE_STAGE */
    }

    while(helperCounter1 < helperCounter2) {
        // erase multiple blocks for required binary size.
        // Don't erase first block, is reserved for bootloader.
#if BOOTLOADER_TYPE == BOOTLOADER_ONE_STAGE
        int retval = SkipBlockNandFlash_EraseBlock(&skipBlockNf,
                helperCounter1 + 1, NORMAL_ERASE);
#else
        int retval = SkipBlockNandFlash_EraseBlock(&skipBlockNf,
                helperCounter1 + 2, NORMAL_ERASE);
#endif /* BOOTLOADER_TYPE == BOOTLOADER_ONE_STAGE */
        if(retval != 0) {
            // skip the block.
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SoftwareImageHandler::copyBootloaderToNandFlash: Faulty block " <<
                    helperCounter1 << " detected!" << std::endl;
#else
            sif::printError("SoftwareImageHandler::copyBootloaderToNandFlash: Faulty "
                    "block %hu detected!\n", helperCounter1);
#endif
        }
        helperCounter1++;
        if(countdown->hasTimedOut()) {
            return SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
        }
    }
    stepCounter = 0;
    helperFlag1 = false;
    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t ImageCopyingEngine::handleSdToNandCopyOperation(
        bool disableAt91Output) {
    SDCardAccess sdCardAccess;

    F_FILE* binaryFile = nullptr;

    // Get file information like binary size, open the file, seek correct
    // position etc.
    ReturnValue_t result = prepareGenericFileInformation(
            sdCardAccess.currentVolumeId, &binaryFile);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    if(disableAt91Output) {
        setTrace(TRACE_LEVEL_WARNING);
    }

    // All necessary informations have been gathered, start writing from
    // SD-Card to NAND-Flash
    while(true) {
        result = performNandCopyAlgorithm(&binaryFile);
        // Operation finished, timeout occured or operation failed.
        if(result != HasReturnvaluesIF::RETURN_OK) {
            if(disableAt91Output) {
                setTrace(TRACE_LEVEL_DEBUG);
            }
            return result;
        }
    }
}

ReturnValue_t ImageCopyingEngine::performNandCopyAlgorithm(
        F_FILE** binaryFile) {
    // If reading or writing failed, the loop might be restarted
    // to have multiple attempts, so we need to check for a timeout
    // at the start as well.
    if(countdown->hasTimedOut()) {
        return SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
    }

    size_t sizeToRead = NAND_PAGE_SIZE;
    if(currentFileSize - currentByteIdx < NAND_PAGE_SIZE) {
        sizeToRead = currentFileSize - currentByteIdx;
        // set the rest of the buffer which will not be overwritten
        // to 0.
        std::memset(imgBuffer->data() + sizeToRead, 0,
                NAND_PAGE_SIZE - sizeToRead);
    }



    // If data has been read but still needs to be copied, don't read.
    if(not helperFlag1) {
        size_t bytesRead = 0;
        ReturnValue_t result = readFile(imgBuffer->data(), sizeToRead,
                &bytesRead, binaryFile);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }
        if(bytesRead < sizeToRead) {
            // should not happen..
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SoftwareImageHandler::copyBootloaderToNandFlash: "
                    "Bytes read smaller than size to read!" << std::endl;
#else
            sif::printError("SoftwareImageHandler::copyBootloaderToNandFlash: "
                    "Bytes read smaller than size to read!\n");
#endif
            return HasReturnvaluesIF::RETURN_FAILED;
        }
    }

    errorCount = 0;
    helperFlag1 = true;

    if(stepCounter == 0) {
        if(bootloader) {
            // We will write the size of the binary to the
            // sixth ARM vector (see p.72 SAM9G20 datasheet)
            // Don't do this for anything else! Messing with the ARM
            // vectors can make optimized applications unstable.
            std::memcpy(imgBuffer->data() + 0x14, &currentFileSize,
                    sizeof(uint32_t));
            helperCounter1 = 0;
        }
        else {
            // This counter will be used to specify written block, and first
            // block is reserved for bootloader.
            helperCounter1 = 1;
        }

        helperCounter2 = 0;
        currentByteIdx = 0;
    }

    if(stepCounter == 0) {
#if OBSW_VERBOSE_LEVEL >= 2
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "ARM Vectors: " << std::endl;
        uint32_t armVector;
        sif::debug << std::hex << std::setfill('0') << std::setw(8);
        std::memcpy(&armVector, imgBuffer->data(), 4);
        sif::debug << "1: 0x" << armVector << std::endl;
        std::memcpy(&armVector, imgBuffer->data() + 4, 4);
        sif::debug << "2: 0x" << armVector << std::endl;
        std::memcpy(&armVector, imgBuffer->data() + 8, 4);
        sif::debug << "3: 0x" << armVector << std::endl;
        std::memcpy(&armVector, imgBuffer->data() + 12, 4);
        sif::debug << "4: 0x" << armVector << std::endl;
        std::memcpy(&armVector, imgBuffer->data() + 16, 4);
        sif::debug << "5: 0x" << armVector << std::endl;
        std::memcpy(&armVector, imgBuffer->data() + 20, 4);
        sif::debug << "6: 0x" << armVector << std::endl;
        std::memcpy(&armVector, imgBuffer->data() + 24, 4);
        sif::debug << "7: 0x" << armVector << std::endl;
        sif::debug << std::dec << std::setfill(' ');
#else

#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* OBSW_VERBOSE_LEVEL >= 2 */
    }

    // This is the logic necessary so that the block is incremented when
    // all 64 pages of the current block have been written.
    if((stepCounter > 0) and (helperCounter2 == PAGES_PER_BLOCK)) {
        helperCounter1++;
        helperCounter2 = 0;
    }

    int retval = SkipBlockNandFlash_WritePage(&skipBlockNf, helperCounter1,
            helperCounter2, imgBuffer->data(), NULL);
    if(retval != 0) {
        errorCount++;
        if(errorCount >= 3) {
            // if writing to NAND failed 5 times, exit.
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SoftwareImageHandler::copyBootloaderToNandFlash: Write error!"
                    << std::endl;
#else
            sif::printError("SoftwareImageHandler::copyBootloaderToNandFlash: Write error!\n");
#endif
            return HasReturnvaluesIF::RETURN_FAILED;
        }
        // Try in next cycle..
        return SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
    }

#if OBSW_VERBOSE_LEVEL >= 2
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "Written " << NAND_PAGE_SIZE << " bytes to NAND-Flash Block " <<
            helperCounter1 << " & Page " << helperCounter2 << std::endl;
#else
    sif::printDebug("Written %lu bytes to NAND-Flash block %hu & Page %hu\n",
            static_cast<unsigned long>(helperCounter1), helperCounter2);
#endif
#endif


    // Increment to write to next page.
    helperCounter2++;
    // Set this flag to false, so that the next page will be read
    // from the SD card.
    helperFlag1 = false;
    // reset error counter.
    errorCount = 0;

    // Page written successfully.
    stepCounter++;
    currentByteIdx += NAND_PAGE_SIZE;

    if(currentByteIdx >= currentFileSize) {
        // operation finished.
#if OBSW_VERBOSE_LEVEL >= 1
        if(bootloader) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "Copying bootloader to NAND-Flash finished with "
                    << stepCounter << " cycles!" << std::endl;
#else
            sif::printInfo("Copying bootloader to NAND-Flash finished with %hu cycles!\n",
                    stepCounter);
#endif
        }
        else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "Copying OBSW image to NAND-Flash finished with "
                    << stepCounter << " cycles!" << std::endl;
#else
            sif::printInfo("Copying OBSW image to NAND-Flash finished with %hu cycles!\n",
                    stepCounter);
#endif
        }
#endif

        // cache last finished state.
        lastFinishedState = imageHandlerState;
        reset();
        return SoftwareImageHandler::OPERATION_FINISHED;
    }
    else if(countdown->hasTimedOut()) {
        return  SoftwareImageHandler::TASK_PERIOD_OVER_SOON;
    }
    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t ImageCopyingEngine::nandFlashInit()
{
    // Configure SMC for Nandflash accesses
    // (done each time because of old ROM codes)
    BOARD_ConfigureNandFlash(nfBusWidth);
    PIO_Configure(pPinsNf, PIO_LISTSIZE(pPinsNf));

    //memset(&skipBlockNf, 0, sizeof(skipBlockNf));

    if (SkipBlockNandFlash_Initialize(&skipBlockNf, 0, cmdBytesAddr,
            addrBytesAddr, dataBytesAddr, nfCePin, nfRbPin)) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    // Check the data bus width of the NandFlash
    nfBusWidth = NandFlashModel_GetDataBusWidth(
            (struct NandFlashModel *)&skipBlockNf);

    // Reconfigure bus width
    BOARD_ConfigureNandFlash(nfBusWidth);

    // Get device parameters
    memSize = NandFlashModel_GetDeviceSizeInBytes(&skipBlockNf.ecc.raw.model);
    blockSize = NandFlashModel_GetBlockSizeInBytes(&skipBlockNf.ecc.raw.model);
    numBlocks = NandFlashModel_GetDeviceSizeInBlocks(
            &skipBlockNf.ecc.raw.model);
    pageSize = NandFlashModel_GetPageDataSize(&skipBlockNf.ecc.raw.model);
    numPagesPerBlock = NandFlashModel_GetBlockSizeInPages(
            &skipBlockNf.ecc.raw.model);

#if OBSW_VERBOSE_LEVEL >= 2
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
#endif

    return HasReturnvaluesIF::RETURN_OK;
}

void ImageCopyingEngine::handleFinishPrintout() {
#if OBSW_VERBOSE_LEVEL >= 1
    if(bootloader) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "Copying bootloader to NAND-Flash finished with "
                << stepCounter << " cycles!" << std::endl;
#else
        sif::printInfo("Copying bootloader to NAND-Flash finished with %hu cycles!\n",
                stepCounter);
#endif
    }
    else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "Copying OBSW image to NAND-Flash finished with "
                << stepCounter << " cycles!" << std::endl;
#else
        sif::printInfo("Copying OBSW image to NAND-Flash finished with %hu cycles!\n",
                stepCounter);
#endif
    }
#endif
}
