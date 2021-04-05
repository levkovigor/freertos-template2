#include "SDCardAccess.h"
#include "SDCAccessManager.h"
#include <OBSWConfig.h>

#include <fsfw/ipc/MutexFactory.h>
#include <fsfw/ipc/MutexGuard.h>
#include <fsfw/serviceinterface/ServiceInterface.h>

#include <hcc/api_fs_err.h>


SDCardAccess::SDCardAccess() {
    int result = 0;
    MutexGuard(SDCardAccessManager::instance()->mutex, MutexIF::TimeoutType::WAITING,
            config::SD_CARD_ACCESS_MUTEX_TIMEOUT);
#ifdef ISIS_OBC_G20
    /* we locked the mutex so we can access the internal states directly. */
    if(SDCardAccessManager::instance()->changingSdCard == true) {
        /* Deny the access, a SD card change is going on! */
        accessResult = SD_CARD_CHANGE_ONGOING;
        return;
    }
#endif
    currentVolumeId = SDCardAccessManager::instance()->activeSdCard;
    if(SDCardAccessManager::instance()->activeAccesses == 0) {
        result = open_filesystem();
        if(result != F_NO_ERROR) {
            /* This could be major problem, maybe reboot or change of SD card necessary! */
            accessResult = HasReturnvaluesIF::RETURN_FAILED;
        }
    }
    SDCardAccessManager::instance()->activeAccesses++;

    /* Register this task with filesystem */
    result = select_sd_card(currentVolumeId, true);
    if(result != F_NO_ERROR){
        sif::printWarning("open_filesystem: SD Card %d not present or defect.\n", currentVolumeId);
        accessResult = HasReturnvaluesIF::RETURN_FAILED;
    }
    /* Even for failed cases, we perform a full teardown of the file system, as long as
    no SD-Card change is going on */
    accessSuccess = true;
}

SDCardAccess::~SDCardAccess() {
    if(not accessSuccess) {
        return;
    }
    int result = f_delvolume(static_cast<uint8_t>(currentVolumeId));
    if(result != F_NO_ERROR) {
        sif::printWarning("SDCardAccess::~SDCardAccess: f_delvolume failed with code"
                " %d.\n", result);
    }
    f_releaseFS();
    MutexGuard(SDCardAccessManager::instance()->mutex, MutexIF::TimeoutType::WAITING,
            config::SD_CARD_ACCESS_MUTEX_TIMEOUT);
    SDCardAccessManager::instance()->activeAccesses--;
    if(SDCardAccessManager::instance()->activeAccesses == 0) {
        close_filesystem(false, false, VolumeId::SD_CARD_0);
    }
}

ReturnValue_t SDCardAccess::getAccessResult() const {
    return accessResult;
}

VolumeId SDCardAccess::getActiveVolume() const {
    return currentVolumeId;
}
