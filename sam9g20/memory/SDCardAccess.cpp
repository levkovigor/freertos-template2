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
    	accessSuccess = open_filesystem(volumeId);
    	if(accessSuccess != F_NO_ERROR) {
    		// maybe trigger event.
    		accessSuccess = OTHER_VOLUME_ACTIVE;
    	}
    	else {
    		// maybe trigger event.
    		accessSuccess = HasReturnvaluesIF::RETURN_FAILED;
    	}
    }
}

SDCardAccess::~SDCardAccess() {
    close_filesystem(currentVolumeId);
}
