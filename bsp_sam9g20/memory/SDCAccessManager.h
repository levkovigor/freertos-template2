#ifndef SAM9G20_MEMORY_SDCACCESSMANAGER_H_
#define SAM9G20_MEMORY_SDCACCESSMANAGER_H_

#include "OBSWConfig.h"
#include <bsp_sam9g20/common/SDCardApi.h>

class MutexIF;

/**
 * @brief   This class manages selecting and switching the active SD card, therefore also
 *          managing the redundancy of the SD cards
 * @details
 * This class will cache the currently active SD card. It is always used by the SD card access
 * token automatically to get the currently active SD card in a  thread safe way and it is
 * also used to determine if a SD card switch is currently going on.
 */
class SDCardAccessManager {
    friend class SDCardHandler;
    friend class SDCardAccess;
    friend class SDCHStateMachine;
public:
    virtual ~SDCardAccessManager();

    static void create();

    /**
     * Returns the single instance of the SD card manager.
     */
    static SDCardAccessManager* instance();

    /**
     * Can be used by SD card users during a transition to check
     * whether SD card change is complete.
     * @return
     */
    bool getSdCardChangeOngoing() const;

    VolumeId getActiveSdCard() const;

private:
    SDCardAccessManager();
    MutexIF* mutex;

    /**
     * Called by the SD card manager class to change the active SD card.
     * @return
     */
    bool tryActiveSdCardChange();
    bool changingSdCard = false;

    uint8_t activeAccesses = 0;
    VolumeId activeSdCard = SD_CARD_0;

    static SDCardAccessManager* factoryInstance;
};


#endif /* SAM9G20_MEMORY_SDCACCESSMANAGER_H_ */
