#ifndef SAM9G20_CORE_SOFTWAREUPDATEHANDLER_H_
#define SAM9G20_CORE_SOFTWAREUPDATEHANDLER_H_

#include <fsfw/action/HasActionsIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>

#if defined(ISIS_OBC_G20) && defined(AT91SAM9G20_EK)
#error "Two board defined at once. Please check includes!"
#endif

class ImageCopyingHelper;
class PeriodicTaskIF;
class ScrubbingEngine;
class Countdown;

/** These generic states can be used inside the primary state machine */
enum class GenericInternalState {
    IDLE,
    // Usually reserved for preparation steps like clearing memory
    STEP_1,
    // Usually reserved for the actual copy operation.
    STEP_2
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
	static constexpr ReturnValue_t TASK_PERIOD_OVER_SOON = MAKE_RETURN_CODE(0x00);

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
    /**
     * There are 2 SD cards available.
     * In normal cases, SD card 1 will be preferred
     */
    enum class SdCard {
        SD_CARD_1,
        SD_CARD_2
    };

    /** Image slots on SD cards */
    enum class ImageSlot {
        IMAGE_1, //!< Primary Image
        IMAGE_2 //!< Secondary image (or software update)
    };

    /** These internal states are used for the primary state machine */
    enum class HandlerState {
        IDLE,
        COPY_BL,
        COPY_OBSW,
        SCRUB_BL,
        SCRUB_OBSW_NOR_FLASH,
        SCRUB_OBSW_SDC1_SL1,
        SCRUB_OBSW_SDC1_SL2,
        SCRUB_OBSW_SDC2_SL1,
        SCRUB_OBSW_SDC2_SL2
    };

    PeriodicTaskIF* executingTask = nullptr;
    ActionHelper actionHelper;
    ImageBuffer imgBuffer;
    Countdown* countdown = nullptr;
    ImageCopyingHelper* imgCpHelper = nullptr;
    ScrubbingEngine* scrubbingEngine = nullptr;

    HandlerState handlerState = HandlerState::IDLE;

    bool displayInfo = false;
    bool operationOngoing = false;
    bool performHammingCodeCheck = false;
    bool oneShot = true;


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
    ReturnValue_t copySdCardImageToNorFlash(SdCard sdCard, ImageSlot imageSlot,
            bool performHammingCheck);
    ReturnValue_t copyNorFlashImageToSdCards(SdCard sdCard, ImageSlot imageSlot,
            bool performHammingCheck);
    // Scrubbing functions
    ReturnValue_t checkNorFlashImage();
#else
    ReturnValue_t copySdBootloaderToNandFlash();
    ReturnValue_t copySdImageToNandFlash();
    ReturnValue_t checkNandFlashImage();
#endif

    void checkSdCardImage(SdCard sdCard, ImageSlot imageSlot);

};



#endif /* SAM9G20_CORE_SOFTWAREUPDATEHANDLER_H_ */
