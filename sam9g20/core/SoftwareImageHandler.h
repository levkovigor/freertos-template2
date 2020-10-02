#ifndef SAM9G20_CORE_SOFTWAREUPDATEHANDLER_H_
#define SAM9G20_CORE_SOFTWAREUPDATEHANDLER_H_

#include <fsfw/action/HasActionsIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/timemanager/Countdown.h>

#if defined(ISIS_OBC_G20) && defined(AT91SAM9G20_EK)
#error "Two board defined at once. Please check includes!"
#endif

/**
 * @brief   Commandable handler object to manage software updates.
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
    SoftwareImageHandler(object_id_t objectId);

    ReturnValue_t performOperation(uint8_t opCode) override;

    ReturnValue_t initialize() override;

    ReturnValue_t initializeAfterTaskCreation() override;

    void setTaskIF(PeriodicTaskIF* executingTask) override;

    /** HasActionsIF overrides */
    MessageQueueId_t getCommandQueue() const override;
    ReturnValue_t executeAction(ActionId_t actionId,
            MessageQueueId_t commandedBy, const uint8_t* data,
            size_t size) override;
private:
    PeriodicTaskIF* executingTask = nullptr;

    std::array<uint8_t, 8192> readArray;
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

    enum class InternalState {
    	IDLE,
		// Usually reserved for preparation steps like clearing memory
    	STEP_1,
		// Usually reserved for the actual copy operation.
		STEP_2
    };

#ifdef ISIS_OBC_G20
    // Special functions, use with care!
    // Overwrites the bootloader, which can either be stored in FRAM or in
    // the SD card.
    ReturnValue_t copySdBootloaderToNorFlash(bool performHammingCheck);
#endif
#ifdef AT91SAM9G20_EK
    static constexpr size_t NAND_PAGE_SIZE = 2048;
    ReturnValue_t copyBootloaderToNandFlash(bool performHammingCheck,
            bool displayInfo = false);
    ReturnValue_t nandFlashInit(bool displayInfo = false);
#endif

    // Handler functions for the SD cards
    void copySdCardImageToNorFlash(SdCard sdCard, ImageSlot imageSlot,
            bool performHammingCheck);
    void copyNorFlashImageToSdCards(SdCard sdCard, ImageSlot imageSlot,
            bool performHammingCheck);

    // Scrubbing functions
    void checkNorFlashImage();
    void checkSdCardImage(SdCard sdCard, ImageSlot imageSlot);

    ActionHelper actionHelper;

    Countdown* countdown = nullptr;
    InternalState internalState = InternalState::IDLE;
    uint16_t stepCounter = 0;
    size_t currentIdx = 0;
    size_t currentFileSize = 0;
    uint8_t errorCount = 0;
    bool operationOngoing = false;
    bool dataRead = false;
    bool oneShot = true;
    bool miscFlag = false;
};



#endif /* SAM9G20_CORE_SOFTWAREUPDATEHANDLER_H_ */
