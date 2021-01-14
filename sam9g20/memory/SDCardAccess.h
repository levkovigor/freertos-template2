#ifndef SAM9G20_MEMORY_SDCARDACCESS_H_
#define SAM9G20_MEMORY_SDCARDACCESS_H_

#include "SDCardDefinitions.h"

#include <fsfw/ipc/MutexIF.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <sam9g20/common/SDCardApi.h>

/**
 * This class will cache the currently active SD card. It is always
 * used by the SD card access token automatically to get the currently
 * active SD card in a  thread safe way.
 */
class SDCardAccessManager {
    friend class SDCardHandler;
    friend class SDCardAccess;
public:
    virtual ~SDCardAccessManager();

    static void create();

    /**
     * Returns the single instance of the SD card manager.
     */
    static SDCardAccessManager* instance();

#ifdef ISIS_OBC_G20
    /**
     * Can be used by SD card users during a transition to check
     * whether SD card change is complete.
     * @return
     */
    bool getSdCardChangeOngoing() const;
#endif

private:
    SDCardAccessManager();
    MutexIF* mutex;

#ifdef ISIS_OBC_G20
    bool tryActiveSdCardChange();
    bool changingSdCard = false;
#endif

    uint8_t activeAccesses = 0;
    VolumeId activeSdCard = SD_CARD_0;

    static SDCardAccessManager* factoryInstance;
};

/**
 * @brief   This access class can be created locally to initiate access to the
 *          file system.
 * @details
 * It will automatically close the access when being destroyed
 */
class SDCardAccess {
public:
	static constexpr uint8_t INTERFACE_ID = CLASS_ID::SD_CARD_HANDLER;
	static constexpr ReturnValue_t OTHER_VOLUME_ACTIVE = MAKE_RETURN_CODE(0x01);

    SDCardAccess();
    ~SDCardAccess();

    ReturnValue_t accessResult = HasReturnvaluesIF::RETURN_OK;
    VolumeId currentVolumeId;
};


#endif /* SAM9G20_MEMORY_SDCARDACCESS_H_ */
