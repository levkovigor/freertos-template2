#ifndef SAM9G20_CORE_SOFTWAREUPDATEHANDLER_H_
#define SAM9G20_CORE_SOFTWAREUPDATEHANDLER_H_

#include <fsfw/action/HasActionsIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>


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

#ifdef ISIS_OBC_G20
    // Special functions, use with care!
    // Overwrites the bootloader, which can either be stored in FRAM or in
    // the SD card.
    ReturnValue_t copyBootloaderToNorFlash(bool performHammingCheck);
#elif defined(AT91SAM9G20_EK)
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

    uint16_t stepCounter = 0;
    bool oneShot = true;
};



#endif /* SAM9G20_CORE_SOFTWAREUPDATEHANDLER_H_ */
