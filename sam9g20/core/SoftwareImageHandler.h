#ifndef SAM9G20_CORE_SOFTWAREIMAGEHANDLER_H_
#define SAM9G20_CORE_SOFTWAREIMAGEHANDLER_H_

#include "imageHandlerDefintions.h"

#include <fsfw/action/HasActionsIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/parameters/ParameterHelper.h>
#include <fsfw/parameters/ReceivesParameterMessagesIF.h>
#include <fsfw/tasks/ExecutableObjectIF.h>

#ifdef ISIS_OBC_G20
#include <hal/Storage/NORflash.h>
#endif

#include <array>


#if defined(ISIS_OBC_G20) && defined(AT91SAM9G20_EK)
#error "Two board defined at once. Please check includes!"
#endif

class ImageCopyingEngine;
class PeriodicTaskIF;
class ScrubbingEngine;
class Countdown;

/**
 * There are 2 SD cards available.
 * In normal cases, SD card 0 will be preferred
 */
enum class SdCard {
    SD_CARD_0,
    SD_CARD_1
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
class SoftwareImageHandler:
        public SystemObject,
        public ExecutableObjectIF,
        public HasActionsIF,
        public ReceivesParameterMessagesIF {
public:
    //static constexpr uint8_t SUBSYSTEM_ID = CLASS_ID::SW_IMAGE_HANDLER;

    static constexpr uint8_t SW_IMG_HANDLER_MQ_DEPTH = 5;
    static constexpr uint8_t MAX_MESSAGES_HANDLED  = 5;

    SoftwareImageHandler(object_id_t objectId);

    /** ExecutableObjectIF overrides */
    ReturnValue_t performOperation(uint8_t opCode) override;
    ReturnValue_t initializeAfterTaskCreation() override;
    void setTaskIF(PeriodicTaskIF* executingTask) override;

    /** SystemObject overrides */
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

    enum ParameterIds {
        HAMMING_CODE_FROM_SDC = 0,
    };

    /* HasActionIF (Service 8) definitions */

    /**
     * Specify whether a hamming code check will be performed during copy operations.
     */
    static constexpr ActionId_t ENABLE_HAMMING_CODE_CHECK_FOR_COPYING = 0;

    /**
     * Specify whether a hamming code check will be performed during copy operations.
     */
    static constexpr ActionId_t DISABLE_HAMMING_CODE_CHECK_FOR_COPYING = 1;

    /**
     * Retrieve whether hamming code checks are enabled.
     */
    static constexpr ActionId_t GET_HAMMING_CODE_CHECK_FOR_COPYING = 2;

    /**
     * Instruct the image handler to copy a SD-card binary to the flash
     * start-up memory. Two uint8_t data field should be supplied, which have
     * the following meaning:
     *
     * First byte:     Binary slot, 0 for slot 0, 1 for slot 1
     */
    static constexpr ActionId_t COPY_OBSW_SDC_TO_FLASH = 3;
    /**
     * Copies the image on the flash memory to the SD-card. Two uint8_t data
     * fields should be supplied, which have the following meaning:
     *
     * First byte:     SD-Card Volume ID, 0 for SD-Card 0 and 1 for SD-Card 1
     * Second byte:    Binary slot, 0 for slot 1, 1 for slot 2, 2 for the
     *                 software update slot
     */
    static constexpr ActionId_t COPY_OBSW_FLASH_TO_SDC = 4;
    static constexpr ActionId_t COPY_OBSW_SDC_TO_SDC = 5;
    /**
     * Copies the hamming code from SD card to storage. The storage can either be the
     * NAND-Flash module or the NOR-Flash module. Two uint8_t fields should be supplied
     * which have the following meaning:
     *
     * First byte:  Slot the hamming code belongs to. 1 for NOR-Flash, 2 for SD slot 0 and 3 for SD
     *              slot 2.
     */
    static constexpr ActionId_t COPY_HAMMING_SDC_TO_STORAGE = 7;
    /**
     * Copy bootloader backup to flash, will be performed in uninterruptible
     * task with highest priority. One byte of data should be provided which
     * has the following meaning:
     *  First byte:     1 for FRAM bootloader or 0 for SDC bootloader.
     */
    static constexpr ActionId_t COPY_BOOTLOADER_SDC_TO_FLASH = 16;

    /* Scrubbing operation IDs */

    //! TODO: make manual scrubbing work first.
    //static constexpr ActionId_t SET_PERIODIC_SCRUBBING = 33;
    /**
     * Report the status of the scrubbing engine
     * First byte: Number of scrubbing operations ongoing.
     * Second, third and fourth byte: ID of any ongoing scrubbing operations.
     */
    //static constexpr ActionId_t REPORT_SCRUBBING_STATUS = 2;

    /**
     * Scrub the OBSW on the active SDC manually.
     * One uint8_t field has to be provided which has the following meaning:
     * First byte:    Target binary slot, 0 for slot 1, 1 for slot 2,
     *                2 for the software update slot
     */
    static constexpr ActionId_t SCRUB_OBSW_ON_SDC = 33;
    //! Scrub the OBSW on the boot flash memory manually.
    static constexpr ActionId_t SCRUB_OBSW_ON_FLASH = 34;
    /**
     * Scrub the backup bootloader. One uint8_t data field should be supplied
     *
     * which has the following meaning
     * First byte:     0 for FRAM bootloader or 1 for SDC bootloader.
     *                 On AT91, 0 will be ignored and the image handler will
     *                 always try to scrub the SDC bootloader.
     */
    static constexpr ActionId_t SCRUB_BACKUP_BOOTLOADER = 35;
    //! Scrub the primary bootloader
    static constexpr ActionId_t SCRUB_BOOTLOADER_ON_FLASH = 36;


    MessageQueueIF* receptionQueue = nullptr;
    PeriodicTaskIF* executingTask = nullptr;
    ActionHelper actionHelper;
    ParameterHelper parameterHelper;
    image::ImageBuffer imgBuffer;
    Countdown* countdown = nullptr;
    ImageCopyingEngine* imgCpHelper = nullptr;
    ScrubbingEngine* scrubbingEngine = nullptr;

    HandlerState handlerState = HandlerState::IDLE;

    MessageQueueId_t recipient = MessageQueueIF::NO_QUEUE;
    ActionId_t currentAction = 0xffffffff;
    bool displayInfo = false;
    bool performHammingCodeCheck = false;
    bool oneShot = false;

    ReturnValue_t getParameter(uint8_t domainId,
            uint8_t uniqueIdentifier, ParameterWrapper *parameterWrapper,
            const ParameterWrapper *newValues, uint16_t startAtIndex) override;

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
    ReturnValue_t copySdCardImageToNorFlash(SdCard sdCard, image::ImageSlot sourceSlot);
    ReturnValue_t copyNorFlashImageToSdCards(SdCard sdCard,
            image::ImageSlot sourceSlot);
    // Scrubbing functions
    ReturnValue_t checkNorFlashImage();
#else
    ReturnValue_t copySdBootloaderToNandFlash(SdCard sdCard,
            image::ImageSlot imageSlot);
    ReturnValue_t copySdImageToNandFlash(SdCard sdCard, image::ImageSlot imageSlot);
    ReturnValue_t checkNandFlashImage();
#endif

    void checkSdCardImage(SdCard sdCard, image::ImageSlot imageSlot);

};



#endif /* SAM9G20_CORE_SOFTWAREIMAGEHANDLER_H_ */
