#ifndef SAM9G20_CORE_IMAGECOPYINGENGINE_H_
#define SAM9G20_CORE_IMAGECOPYINGENGINE_H_

#include <sam9g20/common/SDCardApi.h>
#include <sam9g20/core/SoftwareImageHandler.h>
#include <sam9g20/memory/SDCardDefinitions.h>
#include <sam9g20/memory/SDCardAccess.h>

#ifdef AT91SAM9G20_EK
extern "C" {
#include <at91/memories/nandflash/NandCommon.h>
}
#else
#include <hal/Storage/NORflash.h>
#endif

/**
 * @brief   This class encapsulates all image copying operations required by
 *          the SoftwareImageHandler
 * @author  R. Mueller
 */
class ImageCopyingEngine {
public:

    enum class ImageHandlerStates {
        IDLE,
        // Copy SDC image either to NOR-Flash (iOBC) or NAND-Flash (AT91 EK)
        COPY_SDC_IMG_TO_FLASH,
        // Copy image on NOR-Flash or NAND-Flash to SD-Card
        COPY_FLASH_IMG_TO_SDC,
        // Copy bootloader on SD-Card to NOR-Flash or NAND-Flash
        COPY_SDC_BL_TO_FLASH,
        // Copy bootloader in FRAM to NOR-Flash or NAND-Flash
        COPY_FRAM_BL_TO_FLASH,
        // Replace image on SD-Card by another image on SD-Card
        REPLACE_SDC_IMG
    };

    ImageCopyingEngine(SoftwareImageHandler* owner, Countdown* countdown,
            SoftwareImageHandler::ImageBuffer* imgBuffer);

    /**
     * Specify whether a hamming code check will be performed for all
     * operations.
     * @param enableHammingCodeCheck
     */
    void setHammingCodeCheck(bool enableHammingCodeCheck);

    void setActiveSdCard(SdCard sdCard);

    void enableExtendedDebugOutput(bool enableMoreOutput);

#ifdef AT91SAM9G20_EK
    /**
     * Should be called once before using the NAND-Flash on the AT91!
     */
    ReturnValue_t configureNand(bool disableDebugOutput);
#endif

    ImageHandlerStates getImageHandlerState() const;
    ImageHandlerStates getLastFinishedState() const;

    /**
     * Can be used by SoftwareImageHandler to generate step replies.
     * @return
     */
    GenericInternalState getInternalState() const;

    /**
     * Can be used by SoftwareImageHandler to check whether the handler is
     * ready for a new operation.
     * @return
     */
    bool getIsOperationOngoing() const;

    /**
     * Start an operation to copy something from the SD card to the flash
     * memory. Don't forget to configure the NAND-Flash once
     * when calling this on the AT91 board.
     * @param sdCard        Select the SD card
     * @param imageSlot     Select the image slot (if OBSW is copied)
     * @return
     */
    ReturnValue_t startSdcToFlashOperation(ImageSlot imageSlot);

    /**
     * Starts to copy the bootloader to the flash. Use with care!
     * Parts of this operation might be performed in a special high
     * priority task which can not be preempted and interrupted to ensure
     * nothing goes wrong.
     * @param fromFRAM  Specify whether to copy the bootloader from the FRAM.
     * Only works on the iOBC.
     * @return
     */
    ReturnValue_t startBootloaderToFlashOperation(bool fromFRAM);

    /**
     * Copy the image on flash to the SD card. Don't forget to configure the
     * NAND-Flash once when calling this on the AT91 board.
     * @param sdCard
     * @param imageSlot
     * @return
     */
    ReturnValue_t startFlashToSdcOperation(ImageSlot imageSlot);

    /**
     * Continue the current operation.
     * @return
     * -@c RETURN_OK if the operation was finished
     * -@c TASK_PERIOD_OVER_SOON if the operation was continued but not finished
     *     and the task period is over soon.
     * -@c RETURN_FAILED if the operation has failed.
     */
    ReturnValue_t continueCurrentOperation();

    /**
     * Reset the state of the helper class.
     */
    void reset();
private:
    SoftwareImageHandler* owner;
    Countdown* countdown;
    SoftwareImageHandler::ImageBuffer* imgBuffer;

    ImageHandlerStates imageHandlerState = ImageHandlerStates::IDLE;
    SdCard activeSdCard = SdCard::SD_CARD_0;
    ImageSlot imageSlot = ImageSlot::IMAGE_0;
    GenericInternalState internalState = GenericInternalState::IDLE;
    bool performHammingCodeCheck = false;
    bool extendedDebugOutput = false;
    bool bootloader = false;
    uint16_t stepCounter = 0;
    size_t currentByteIdx = 0;
    size_t currentFileSize = 0;
    uint8_t errorCount = 0;
    bool helperFlag1 = false;
    bool helperFlag2 = false;
    uint16_t helperCounter1 = 0;
    uint16_t helperCounter2 = 0;

    ImageHandlerStates lastFinishedState = ImageHandlerStates::IDLE;


#ifdef AT91SAM9G20_EK
    bool nandConfigured = false;
    static constexpr size_t NAND_PAGE_SIZE = NandCommon_MAXPAGEDATASIZE;
    static constexpr uint8_t PAGES_PER_BLOCK = NandCommon_MAXNUMPAGESPERBLOCK;

    /**
     * This is the primary function to copy either the software image
     * or the bootloader from the SD card to the NAND-Flash. The function
     * behaviour will be determined by the class members which are set
     * by the #start...() functions.
     * @return
     */
    ReturnValue_t copySdCardImageToNandFlash();
    ReturnValue_t handleNandErasure(bool disableAt91Output = true);
    ReturnValue_t handleErasingForObsw();
    ReturnValue_t handleSdToNandCopyOperation(bool disableAt91Output = true);
    ReturnValue_t nandFlashInit();
    ReturnValue_t performNandCopyAlgorithm(F_FILE** binaryFile);
#else
    static constexpr uint8_t NORFLASH_SMALL_SECTORS_NUMBER = 8;
    static constexpr uint8_t RESERVED_BL_SMALL_SECTORS = 5;
    static constexpr uint8_t RESERVED_OBSW_SMALL_SECTORS =
    		NORFLASH_SMALL_SECTORS_NUMBER - RESERVED_BL_SMALL_SECTORS;
    static constexpr size_t NORFLASH_TOTAL_SMALL_SECTOR_MEM_OBSW =
    		RESERVED_OBSW_SMALL_SECTORS * NORFLASH_SMALL_SECTOR_SIZE;
    static constexpr uint8_t NORFLASH_MEDIUM_SECTORS_NUMBER = 15;
    static constexpr uint8_t NORFLASH_SECTORS_NUMBER = 23;
    static constexpr uint32_t NORFLASH_BASE_ADDRESS_READ = 0x10000000;
    static constexpr uint32_t NORFLASH_BL_SIZE_START = NORFLASH_SA5_ADDRESS - 6;
    static constexpr uint32_t NORFLASH_BL_CRC16_START = NORFLASH_SA5_ADDRESS - 2;
    static constexpr size_t COPYING_BUCKET_SIZE = 2048;
    ReturnValue_t copySdCardImageToNorFlash();
    /**
     * For the bootloader, 5 small sectors (8192 * 5 = 40960 bytes) will
     * be erased. For the primary image, all the remaining sectors will
     * be deleted.
     * @param bootloader
     * @return
     */
    ReturnValue_t handleNorflashErasure();
    ReturnValue_t handleObswErasure();
    uint32_t getBaseAddress(uint8_t stepCounter, size_t* offset);
    ReturnValue_t performNorCopyOperation(F_FILE** binaryFile);
    ReturnValue_t handleSdToNorCopyOperation();
    void writeBootloaderSizeAndCrc();
#endif
    ReturnValue_t prepareGenericFileInformation(VolumeId currentVolume,
            F_FILE** filePtr);

    /**
     * Generic function to read file which also simplfies error handling.
     * Plese note that this only works if the file already has been opened.
     * Also, the pointer from where to read inb the file
     * should be set beforehand.
     * @param buffer    Buffer where the read data will be stored
     * @param sizeToRead
     * @param sizeRead  Actual number of bytes read, can be smaller if close
     *                  to end of file.
     * @return
     * -@c SoftwareImageHandler::TASK_PERIOD_OVER_SOON Reading failed,
     *     new attempt should be made on next cycle
     * -@c RETURN_FAILED Reading failed 3 times
     * -@c RETURN_OK Read success
     */
    ReturnValue_t readFile(uint8_t* buffer, size_t sizeToRead,
            size_t* sizeRead, F_FILE** file);

    void handleInfoPrintout(int currentVolume);

};

#endif /* SAM9G20_CORE_IMAGECOPYINGENGINE_H_ */
