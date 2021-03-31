#include "SDCAccessManager.h"

#ifdef ISIS_OBC_G20
#include <bsp_sam9g20/common/FRAMApi.h>
#endif

#include <fsfw/ipc/MutexIF.h>
#include <fsfw/ipc/MutexFactory.h>
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/ipc/MutexGuard.h>


SDCardAccessManager* SDCardAccessManager::factoryInstance = nullptr;

void SDCardAccessManager::create() {
    if(factoryInstance == nullptr) {
        factoryInstance = new SDCardAccessManager();
    }
}

SDCardAccessManager* SDCardAccessManager::instance() {
    SDCardAccessManager::create();
    return SDCardAccessManager::factoryInstance;
}

SDCardAccessManager::SDCardAccessManager() {
    /* On the iOBC, the active SD card is derived from a value in FRAM so the
    /SD card access manager should not be used before the FRAM is active! */
#ifdef ISIS_OBC_G20
    int result = fram_get_preferred_sd_card(&activeSdCard);
    uint8_t activeSdCardRaw = static_cast<int>(activeSdCard);
    if(activeSdCardRaw != 0 and activeSdCardRaw != 1) {
        /* Invalid volume IDs in FRAM, use SD Card 0 */
        activeSdCard = SD_CARD_0;
    }
    if(result != 0) {
        sif::printWarning("SDCardAccessManager::SDCardAccessManager: "
                "Could not get active SD Card\n");
    }
#endif
    mutex = MutexFactory::instance()->createMutex();
}

SDCardAccessManager::~SDCardAccessManager() {
    MutexFactory::instance()->deleteMutex(mutex);
}


bool SDCardAccessManager::getSdCardChangeOngoing() const {
#ifndef ISIS_OBC_G20
    return false;
#else
    MutexGuard(mutex, MutexIF::TimeoutType::WAITING, config::SD_CARD_ACCESS_MUTEX_TIMEOUT);
    return changingSdCard;
#endif
}

/* Only one manager class is supposed to call this! */
bool SDCardAccessManager::tryActiveSdCardChange() {
#ifndef ISIS_OBC_G20
    return true;
#else
    MutexGuard(mutex, MutexIF::TimeoutType::WAITING, config::SD_CARD_ACCESS_MUTEX_TIMEOUT);
    if(this->changingSdCard == false) {
        this->changingSdCard = true;
    }
    /* No task is using the SD cards anymore, so we can safely switch the active SD card. */
    if(this->activeAccesses == 0) {
        if(this->activeSdCard == SD_CARD_0) {
            this->activeSdCard = SD_CARD_1;
        }
        else {
            this->activeSdCard = SD_CARD_0;
        }
        this->changingSdCard = false;
        return true;
    }
    return false;
#endif
}

VolumeId SDCardAccessManager::getActiveSdCard() const {
    return activeSdCard;
}

