#include "SDCardAccess.h"

#include <fsfw/ipc/MutexFactory.h>
#include <fsfw/ipc/MutexHelper.h>
#include <OBSWConfig.h>

extern "C" {
#include <hcc/api_fs_err.h>
#include <at91/utility/trace.h>
}


// I think we need this manager because if we tear down a filesystem while
// another task is still busy in there, we might get into trouble..

SDCardAccessManager* SDCardAccessManager::factoryInstance =
        new SDCardAccessManager();

SDCardAccessManager::~SDCardAccessManager() {}

SDCardAccessManager::SDCardAccessManager() {
    mutex = MutexFactory::instance()->createMutex();
}

SDCardAccessManager* SDCardAccessManager::instance() {
    return SDCardAccessManager::factoryInstance;
}


SDCardAccess::SDCardAccess(VolumeId volumeId):
		currentVolumeId(volumeId) {
    int result = 0;
    MutexHelper(SDCardAccessManager::instance()->mutex,
            MutexIF::TimeoutType::WAITING,
            config::SD_CARD_ACCESS_MUTEX_TIMEOUT);
    if(SDCardAccessManager::instance()->activeAccesses == 0) {
        result = open_filesystem(volumeId);
        if(result != F_NO_ERROR) {
            // try to open other SD card.
            if(volumeId == SD_CARD_0) {
                volumeId = SD_CARD_1;
            }
            else {
                volumeId = SD_CARD_0;
            }
            accessResult = open_filesystem(volumeId);
            if(accessResult != F_NO_ERROR) {
                accessResult = OTHER_VOLUME_ACTIVE;
            }
            else {
                accessResult = HasReturnvaluesIF::RETURN_FAILED;
            }
        }
    }
    SDCardAccessManager::instance()->activeAccesses++;

    /* Register this task with filesystem */
    result = f_enterFS();
    if(result != F_NO_ERROR){
        TRACE_ERROR("open_filesystem: fs_enterFS failed with "
                "code %d\n\r", result);
    }

    result = select_sd_card(volumeId);
    if(result != F_NO_ERROR){
        TRACE_ERROR("open_filesystem: SD Card %d not present or "
                "defect.\n\r", volumeId);
    }
    currentVolumeId = volumeId;
}

SDCardAccess::~SDCardAccess() {
    int result = f_delvolume(currentVolumeId);
    if(result != F_NO_ERROR) {
        TRACE_ERROR("open_filesystem: f_devlvolume failed with code"
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
