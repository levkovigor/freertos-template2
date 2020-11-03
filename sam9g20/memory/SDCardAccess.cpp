#include "SDCardAccess.h"

#include <fsfw/ipc/MutexFactory.h>
#include <fsfw/ipc/MutexHelper.h>

extern "C" {
#include <hcc/api_fs_err.h>
#include <at91/utility/trace.h>
#include <sam9g20/common/FRAMApi.h>
}

#include <OBSWConfig.h>

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
    // On the iOBC, the active SD card is derived from a value in FRAM so the
    // SD card access manager should not be used before the FRAM is active!
#ifdef ISIS_OBC_G20
    // will be tested and allowed later.
    //int result = get_prefered_sd_card(&activeSdCard);
#endif
    mutex = MutexFactory::instance()->createMutex();
}

SDCardAccessManager::~SDCardAccessManager() {
    MutexFactory::instance()->deleteMutex(mutex);
}


bool SDCardAccessManager::getSdCardChangeOngoing() const {
    MutexHelper(mutex, MutexIF::TimeoutType::WAITING,
            config::SD_CARD_ACCESS_MUTEX_TIMEOUT);
    return changingSdCard;
}

// Only one manager class is supposed to call this!
bool SDCardAccessManager::tryActiveSdCardChange() {
    if(this->changingSdCard == false) {
        this->changingSdCard = true;
    }
    MutexHelper(mutex, MutexIF::TimeoutType::WAITING,
            config::SD_CARD_ACCESS_MUTEX_TIMEOUT);
    // No task is using the SD cards anymore, so we can safely switch the
    // active SD card.
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
}

SDCardAccess::SDCardAccess() {
    int result = 0;
    MutexHelper(SDCardAccessManager::instance()->mutex,
            MutexIF::TimeoutType::WAITING,
            config::SD_CARD_ACCESS_MUTEX_TIMEOUT);
    // we locked the mutex so we can access the internal states directly.
    if(SDCardAccessManager::instance()->changingSdCard == true) {
        // deny the access, a SD card change is going on!
        accessResult = HasReturnvaluesIF::RETURN_FAILED;
        return;
    }
    currentVolumeId = SDCardAccessManager::instance()->activeSdCard;
    if(SDCardAccessManager::instance()->activeAccesses == 0) {
        result = open_filesystem(currentVolumeId);
        if(result != F_NO_ERROR) {
            // This could be major problem, maybe reboot or change of SD card
            // necessary!
            accessResult = HasReturnvaluesIF::RETURN_FAILED;
        }
    }
    SDCardAccessManager::instance()->activeAccesses++;

    /* Register this task with filesystem */
    result = f_enterFS();
    if(result != F_NO_ERROR){
        TRACE_ERROR("open_filesystem: fs_enterFS failed with "
                "code %d\n\r", result);
    }

    result = select_sd_card(currentVolumeId);
    if(result != F_NO_ERROR){
        TRACE_ERROR("open_filesystem: SD Card %d not present or "
                "defect.\n\r", currentVolumeId);
    }
}

SDCardAccess::~SDCardAccess() {
    int result = f_delvolume(currentVolumeId);
    if(result != F_NO_ERROR) {
        TRACE_ERROR("SDCardAccess::~SDCardAccess: f_delvolume failed with code"
                " %d.\n\r", result);
    }
    f_releaseFS();
    MutexHelper(SDCardAccessManager::instance()->mutex,
            MutexIF::TimeoutType::WAITING,
            config::SD_CARD_ACCESS_MUTEX_TIMEOUT);
    SDCardAccessManager::instance()->activeAccesses--;
    if(SDCardAccessManager::instance()->activeAccesses == 0) {
        close_filesystem(currentVolumeId);
    }
}
