#ifndef SAM9G20_CORE_SOFTWAREIMAGEHANDLER_H_
#define SAM9G20_CORE_SOFTWAREIMAGEHANDLER_H_

#include <fsfw/action/HasActionsIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>

#if defined(ISIS_OBC_G20) && defined(AT91SAM9G20_EK)
#error "Two board defined at once. Please check includes!"
#endif

class ImageCopyingEngine;
class PeriodicTaskIF;
class ScrubbingEngine;
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
 * There are 2 SD cards available.
 * In normal cases, SD card 0 will be preferred
 */
enum class SdCard {
    SD_CARD_0,
    SD_CARD_1
};

/** Image slots on SD cards */
enum class ImageSlot {
    IMAGE_0, //!< Primary Image
    IMAGE_1 //!< Secondary image (or software update)
};

/**
 * @brief   Commandable handler object to manage software updates and
 *          perform scrubbing.
 * @details
 * This class is used instead of a dedicated NORFlashHandler because
 * the NOR-Flash is only used for software images and only needs
 * to be written to for software updates.
 * Decoupled from core controller, has lower priority. Also, writing to
 * NOR-Flash takes ages. This handler also takes care of verifying
 * (and possibly correcting) the binary with the supplied hamming code.
 *
 * Run-time scrubbing / hamming code checking will also be performed by this
 * task as long as no software updates need to be performed.
 * If an error is detected, the image handler can take care of replacing
 * faulty binaries immediately.
 *
 * The NOR-Flash binary will be checked more often than the SD-Card image.
 *
 * @author  R. Mueller, J. Meier
 *
 */
class SoftwareImageHandler: public SystemObject,
        public ExecutableObjectIF,
        public HasActionsIF {
public:
	static constexpr uint8_t SUBSYSTEM_ID = CLASS_ID::SW_IMAGE_HANDLER;
	static constexpr ReturnValue_t OPERATION_FINISHED = MAKE_RETURN_CODE(0x00);
	static constexpr ReturnValue_t TASK_PERIOD_OVER_SOON = MAKE_RETURN_CODE(0x01);

	static constexpr uint8_t SW_IMG_HANDLER_MQ_DEPTH = 5;

	using ImageBuffer = std::array<uint8_t, 2048>;

    SoftwareImageHandler(object_id_t objectId);

    /** ExecutableObjectIF overrides */
    ReturnValue_t performOperation(uint8_t opCode) override;
    ReturnValue_t initializeAfterTaskCreation() override;
    void setTaskIF(PeriodicTaskIF* executingTask) override;

    /** SystemObject overridfes */
    ReturnValue_t initialize() override;

    /** HasActionsIF overrides */
    MessageQueueId_t getCommandQueue() const override;
    ReturnValue_t executeAction(ActionId_t actionId,
            MessageQueueId_t commandedBy, const uint8_t* data,
            size_t size) override;
private:

    /** These internal states are used for the primary state machine */
    enum class HandlerState {
        IDLE,
        //! Copy operations will have priority over scrubbing operations
        COPYING,
        //! Scrubbing will be performed at fixed priorities or on command
        //! if there is no copy operation going on.
        SCRUBBING
    };

    /** HasActionIF (Service 8) definitions */

    // TODO: make manual scrubbing work first..
    //! Configure whether periodic scrubbing will be performed and how often.
    //! TODO: This belongs in HasParametersIF
    //! The data field has the following form:
    //! First 4 bytes: Bucket size. Number of 256 byte buckets which will
    //!                be read in one scrubbing cycle.
    //! Second 4 bytes: Scrubbing interval for the flash bootloader in seconds.
    //! Third 4 bytes: Scrubbing interval for the flash OBSW in seconds
    //!
    //! All other memories need to be scrubbed manually for now!
    //! Only one scrubbing operation can be perfomed at once.
    //! If there are mutliple outstanding scrubbing operations, the scrubbing
    //! engine will keep an internal list of those operations. Up to three
    //! operations can be queue inside that list.
    //!
    //! Please note that the scrubbing engine will only scrub the active
    //! SD card. If the SD card is switched, any ongoing scrubbing operation
    //! on an SD card will be cancelled.
    //!
    static constexpr ActionId_t SET_PERIODIC_SCRUBBING = 1;

    //! Report the status of the scrubbing engine
    //! First byte: Number of scrubbing operations ongoing.
    //! Second, third and fourth byte: ID of any ongoing scrubbing operations.
    static constexpr ActionId_t REPORT_SCRUBBING_STATUS = 2;

    //! Specify whether a hamming code check will be performed during
    //! copy operations.
    static constexpr ActionId_t SET_HAMMING_CODE_CHECK_FOR_COPYING = 3;

    //! Instruct the image handler to copy a SD-card binary to the flash
    //! start-up memory. Two uint8_t data field should be supplied, which have
    //! the following meaning:
    //!
    //! First byte:     SD-Card Volume ID, 0 for SD-Card 0 and 1 for SD-Card 1
    //! Second byte:    Binary slot, 0 for slot 1, 1 for slot 2, 2 for the
    //!                 software update slot
    static constexpr ActionId_t COPY_OBSW_SDC_TO_FLASH = 4;
    //! Copies the image on the flash memory to the SD-card. Two uint8_t data
    //! fields should be supplied, which have the following meaning:
    //!
    //! First byte:     SD-Card Volume ID, 0 for SD-Card 0 and 1 for SD-Card 1
    //! Second byte:    Binary slot, 0 for slot 1, 1 for slot 2, 2 for the
    //!                 software update slot
    static constexpr ActionId_t COPY_OBSW_FLASH_TO_SDC = 5;
    //! Copies the image on the flash memory to the SD-card. Please note that
    //! SD-card files can only be copied inside the same SD-card for now.
    //! Two uint8_t data fields should be supplied, which have the following
    //! meaning:
    //!
    //! First byte:     Source data binary slot, 0 for slot 1, 1 for slot 2,
    //!                 2 for the software update slot
    //! Second byte:    Target binary slot, 0 for slot 1, 1 for slot 2,
    //!                 2 for the software update slot
    static constexpr ActionId_t COPY_OBSW_SDC_TO_SDC = 5;

    /** Scurbbing operation IDs */

    //! Scrub the backup bootloader. One uint8_t data field should be supplied
    //! which has the following meaning
    //! First byte:     0 for FRAM bootloader or 1 for SDC bootloader.
    //!                 On AT91, 0 will be ignored and the image handler will
    //!                 always try to scrub the SDC bootloader.
    static constexpr ActionId_t SCRUB_BACKUP_BOOTLOADER = 6;
    //! Scrub the OBSW on the boot flash memory manually.
    static constexpr ActionId_t SCRUB_OBSW_ON_FLASH = 7;
    //! Scrub the OBSW on the active SDC manually.
    //! One uint8_t field has to be provided which has the following meaning:
    //! First byte:    Target binary slot, 0 for slot 1, 1 for slot 2,
    //!                2 for the software update slot
    static constexpr ActionId_t SCRUB_OBSW_ON_SDC = 8;


    //! Scrub the primary bootloader
    static constexpr ActionId_t SCRUB_BOOTLOADER_ON_FLASH = 10;
    //! Copy bootloader backup to flash, will be performed in uninterruptible
    //! task with highest priority.
    static constexpr ActionId_t COPY_BOOTLOADER_TO_FLASH = 11;
    //! Might not be needed, FRAM image is a lot safer..
    static constexpr ActionId_t COPY_BOOTLOADER_FLASH_TO_BACKUP = 15;

    MessageQueueIF* receptionQueue = nullptr;
    PeriodicTaskIF* executingTask = nullptr;
    ActionHelper actionHelper;
    ImageBuffer imgBuffer;
    Countdown* countdown = nullptr;
    ImageCopyingEngine* imgCpHelper = nullptr;
    ScrubbingEngine* scrubbingEngine = nullptr;

    HandlerState handlerState = HandlerState::IDLE;

    bool displayInfo = false;
    bool performHammingCodeCheck = false;
    bool oneShot = false;
    bool blWritten = false;


#ifdef ISIS_OBC_G20
    /**
     * Special functions, use with care!
     * Overwrites the bootloader, which can either be stored in FRAM or in
     * the SD card.
     * @param sdCard
     * @param imageSlot
     */
    ReturnValue_t copySdBootloaderToNorFlash();
    ReturnValue_t copyFramBootloaderToNorFlash();

    // Handler functions for the SD cards
    ReturnValue_t copySdCardImageToNorFlash(SdCard sdCard, ImageSlot imageSlot);
    ReturnValue_t copyNorFlashImageToSdCards(SdCard sdCard,
            ImageSlot imageSlot);
    // Scrubbing functions
    ReturnValue_t checkNorFlashImage();
#else
    ReturnValue_t copySdBootloaderToNandFlash(SdCard sdCard,
            ImageSlot imageSlot);
    ReturnValue_t copySdImageToNandFlash(SdCard sdCard, ImageSlot imageSlot);
    ReturnValue_t checkNandFlashImage();
#endif

    void checkSdCardImage(SdCard sdCard, ImageSlot imageSlot);

};



#endif /* SAM9G20_CORE_SOFTWAREIMAGEHANDLER_H_ */
