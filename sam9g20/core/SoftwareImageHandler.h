#ifndef SAM9G20_MEMORY_SOFTWAREUPDATEHANDLER_H_
#define SAM9G20_MEMORY_SOFTWAREUPDATEHANDLER_H_

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

    // Handler functions for the SD cards
    void copySdCardImageToNorFlash(SdCard sdCard, ImageSlot imageSlot);
    void copyNorFlashImageToSdCards(SdCard sdCard, ImageSlot imageSlot);

    // Scrubbing functions
    void checkNorFlashImage();
    void checkSdCardImage(SdCard sdCard, ImageSlot imageSlot);

    ActionHelper actionHelper;
};



#endif /* SAM9G20_MEMORY_SOFTWAREUPDATEHANDLER_H_ */
