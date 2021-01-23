#ifndef SAM9G20_CORE_IMAGECOPYINGENGINE_H_
#define SAM9G20_CORE_IMAGECOPYINGENGINE_H_

#include "imageHandlerDefintions.h"
#include <OBSWConfig.h>

#include <hcc/api_fat.h>
#include <sam9g20/common/SDCardApi.h>

#ifdef AT91SAM9G20_EK
#include <sam9g20/common/config/commonAt91Config.h>
#else /* iOBC */
#include <sam9g20/common/config/commonIOBCConfig.h>
#endif

class SoftwareImageHandler;
class Countdown;

/**
 * These generic states can be used inside the primary state machine.
 * They will also be used by the action helper to generate step replies.
 */
enum class GenericInternalState {
    IDLE,
    STEP_1,
    STEP_2,
    STEP_3
};

/**
 * @brief   This class encapsulates all image copying operations required by
 *          the SoftwareImageHandler
 * @author  R. Mueller
 */
class ImageCopyingEngine {
public:

    enum class ImageHandlerStates {
        IDLE,
        //! Copy SDC image either to NOR-Flash (iOBC) or NAND-Flash (AT91 EK)
        COPY_IMG_SDC_TO_FLASH,
        //! Copy image on NOR-Flash or NAND-Flash to SD-Card
        COPY_IMG_FLASH_TO_SDC,
        //! Replace image on SD-Card by another image on SD-Card
        COPY_IMG_SDC_TO_SDC,
        //! Copy image hamming code (of NOR-Flash or SDC image) to FRAM
        COPY_IMG_HAMMING_SDC_TO_FRAM,

        //! Copy bootloader in FRAM to NOR-Flash
        COPY_BL_FRAM_TO_FLASH,
        //! Copy bootloader hamming code to FRAM
        COPY_BL_HAMMING_SDC_TO_FRAM,
        //! Copy bootloader on SD-Card to NOR-Flash or NAND-Flash
        COPY_BL_SDC_TO_FLASH,
        //! Copy bootloader on SD-Card to FRAM
        COPY_BL_SDC_TO_FRAM
    };

    ImageCopyingEngine(SoftwareImageHandler* owner, Countdown* countdown,
            image::ImageBuffer* imgBuffer);

    // void setActiveSdCard(SdCard sdCard);

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
     * memory. Don't forget to configure the NAND-Flash once when calling this on the AT91 board.
     * @param sdCard        Select the SD card
     * @param imageSlot     Select the image slot (if OBSW is copied)
     * @return
     */
    ReturnValue_t startSdcToFlashOperation(image::ImageSlot sourceSlot);

    /**
     * Starts to copy the bootloader to the flash. Use with care! Parts of this operation might
     * be performed in a special high priority task which can not be preempted and interrupted
     * to ensure nothing goes wrong.
     * @param fromFRAM  Specify whether to copy the bootloader from the FRAM instead of the SD-Card.
     *                  Only works on the iOBC.
     * @return
     */
    ReturnValue_t startBootloaderToFlashOperation(image::ImageSlot bootloaderType, bool fromFram);

    /**
     * Copy the image on flash to the SD card. Don't forget to configure the
     * NAND-Flash once when calling this on the AT91 board.
     * @param sdCard
     * @param imageSlot
     * @return
     */
    ReturnValue_t startFlashToSdcOperation(image::ImageSlot targetSlot);

#ifdef ISIS_OBC_G20
    /**
     * Copy the hamming code belonging to a certain image to the FRAM.
     * @param respectiveSlot    Hamming code belongs to this image
     * @param bootloader        Specify to true to copy the hamming code of the bootloader
     * @return
     */
    ReturnValue_t startHammingCodeToFramOperation(image::ImageSlot respectiveSlot);
#endif

    /**
     * Continue the current operation.
     * @return
     *      -@c RETURN_OK if the operation was finished
     *      -@c TASK_PERIOD_OVER_SOON if the operation was continued but not finished
     *          and the task period is over soon.
     *      -@c RETURN_FAILED if the operation has failed.
     */
    ReturnValue_t continueCurrentOperation();

    /**
     * Reset the state of the helper class.
     */
    void reset();

private:
    SoftwareImageHandler* owner = nullptr;
    Countdown* countdown = nullptr;
    image::ImageBuffer* imgBuffer = nullptr;

    ImageHandlerStates imageHandlerState = ImageHandlerStates::IDLE;
    GenericInternalState internalState = GenericInternalState::IDLE;

    image::ImageSlot sourceSlot = image::ImageSlot::NONE;
    image::ImageSlot targetSlot = image::ImageSlot::NONE;

    bool hammingCode = false;
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
    /** Please note that the algorithms are not currently written in a way to support
    variable bootloader small sectors numbers depending on these configuration constants!
    The algorithms assumes 8 small sectors are reserved for the bootloader! */
    static constexpr size_t NORFLASH_TOTAL_SMALL_SECTOR_MEM_OBSW =
    		RESERVED_OBSW_SMALL_SECTORS * NORFLASH_SMALL_SECTOR_SIZE;
    static constexpr size_t COPYING_BUCKET_SIZE = NORFLASH_SMALL_SECTOR_SIZE;
    ReturnValue_t copyImgHammingSdcToFram();
    ReturnValue_t copySdCardImageToNorFlash();

    /**
     * For the bootloader, all small sectors (8192 * 8 = 65536 bytes) will
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
#endif /* AT91SAM9G20_EK */

    /**
     * Generic handler to open files and read important information like size.
     * @param currentVolume
     * @param filePtr
     * @return
     *  - F_ERR_NOTFOUND if file was not found.
     */
    ReturnValue_t prepareGenericFileInformation(VolumeId currentVolume,
            F_FILE** filePtr);

    ReturnValue_t copySdcImgToSdc();

    /**
     * Generic function to read file which also simplfies error handling.
     * Plese note that this only works if the file already has been opened.
     * Also, the pointer from where to read in the file should be set beforehand.
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
    ReturnValue_t readFile(uint8_t* buffer, size_t sizeToRead, size_t* sizeRead, F_FILE** file);

    void handleInfoPrintout(VolumeId currentVolume);
    void handleGenericInfoPrintout(char const* board, char const* typePrint,
            char const* sourcePrint, char const* targetPrint);
    void handleFinishPrintout();

};

#endif /* SAM9G20_CORE_IMAGECOPYINGENGINE_H_ */
