#ifndef SAM9G20_CORE_IMAGECOPYINGHELPER_H_
#define SAM9G20_CORE_IMAGECOPYINGHELPER_H_

#include <sam9g20/core/SoftwareImageHandler.h>

extern "C" {
#include <at91/memories/nandflash/NandCommon.h>
}

class ImageCopyingHelper {
public:
    ImageCopyingHelper(SoftwareImageHandler* owner, Countdown* countdown,
            SoftwareImageHandler::ImageBuffer* imgBuffer);

    void reset() {
        internalState = GenericInternalState::IDLE;
        stepCounter = 0;
        currentByteIdx = 0;
        currentFileSize = 0;
        errorCount = 0;
        helperFlag1 = false;
        helperFlag2 = false;
        helperCounter1 = 0;
        helperCounter2 = 0;
    }


private:
    SoftwareImageHandler* owner;
    Countdown* countdown;
    SoftwareImageHandler::ImageBuffer* imgBuffer;

    GenericInternalState internalState = GenericInternalState::IDLE;
    bool displayInfo = false;
    uint16_t stepCounter = 0;
    size_t currentByteIdx = 0;
    size_t currentFileSize = 0;
    uint8_t errorCount = 0;
    bool helperFlag1 = false;
    bool helperFlag2 = false;
    uint16_t helperCounter1 = 0;
    uint16_t helperCounter2 = 0;


#ifdef AT91SAM9G20_EK
    static constexpr size_t NAND_PAGE_SIZE = NandCommon_MAXPAGEDATASIZE;
    static constexpr uint8_t PAGES_PER_BLOCK = NandCommon_MAXNUMPAGESPERBLOCK;

    /**
     * @param bootloader    Set to true if bootloader shall be copied
     * @param performHammingCheck
     * @param configureNandFlash NAND flash configuration only needs to be
     * done once.
     * @param obswSlot If bootloader is set to false, set to false for slot 0
     * and to true for slot 1. Slot 0 by default.
     * @return
     */
    ReturnValue_t copySdCardImageToNandFlash(bool bootloader,
            bool performHammingCheck,
            bool configureNandFlash,
            bool obswSlot = false);
    ReturnValue_t nandFlashInit();
    /**
     * Common function to set up NAND and also erase blocks for either
     * the bootloader or the software image.
     * @param configureNand
     * @param bootloader
     * @param softwareSlot  false for slot 0, true for slot 1
     * @return
     */
    ReturnValue_t handleNandInitAndErasure(bool configureNand,
            bool bootloader, bool softwareSlot = false);
    ReturnValue_t handleErasingForObsw(bool softwareSlot);
    ReturnValue_t handleSdToNandCopyOperation(bool bootloader, bool obswSlot,
            bool displayExtendedPrintout = false);
#endif

};




#endif /* SAM9G20_CORE_IMAGECOPYINGHELPER_H_ */
