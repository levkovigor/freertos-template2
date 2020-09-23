#include "SDCardAccess.h"
extern "C" {
#include <hcc/api_fs_err.h>
}

SDCardAccess::SDCardAccess(VolumeId volumeId):
		currentVolumeId(volumeId) {
    int result = open_filesystem(volumeId);
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
    currentVolumeId = volumeId;
}

SDCardAccess::~SDCardAccess() {
    close_filesystem(currentVolumeId);
}
