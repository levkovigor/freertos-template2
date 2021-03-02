#ifndef SAM9G20_MEMORY_SDCARDACCESS_H_
#define SAM9G20_MEMORY_SDCARDACCESS_H_

#include "sdcardDefinitions.h"

#include <sam9g20/common/SDCardApi.h>

#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <fsfw/serviceinterface/ServiceInterface.h>

/**
 * @brief   This access class can be created locally to initiate access to the
 *          file system.
 * @details
 * It will automatically close the access when being destroyed. Please make sure to
 * check the access result, as it can be used to find out whether a SD card change is going on.
 * If this is the case, the access token will be invalid, and any subsequent SD card operations
 * will fail.
 */
class SDCardAccess {
public:
	static constexpr uint8_t INTERFACE_ID = CLASS_ID::SD_CARD_HANDLER;
	static constexpr ReturnValue_t SD_CARD_CHANGE_ONGOING =
	        HasReturnvaluesIF::makeReturnCode(INTERFACE_ID, 1);

    SDCardAccess();
    ~SDCardAccess();

    /**
     * Can be used to check whether the SD card access was granted successfully.
     * Will be RETURN_OK if access was established and SD_CARD_CHANGE_ONGOING is case it
     * was denied if a SD card change is ongoing. Can also be set to RETURN_FAILED for
     * access issues but this should not happen.
     * @return
     */
    ReturnValue_t getAccessResult() const;
    /**
     * Get the currently active volume.
     * @return
     */
    VolumeId getActiveVolume() const;
private:
    bool accessSuccess = false;
    ReturnValue_t accessResult = HasReturnvaluesIF::RETURN_OK;
    VolumeId currentVolumeId;
};

/**
 * @brief   Helper class to facilitate RAII conformance when working with file handles.
 *          It will close a given file handle on destruction.
 * @details
 * Can be used by instantiating it on the stack. It also possible to not open a file and simply
 * bind the f_close call to the lifetime of the file helper.
 */
class HCCFileGuard {
public:
    /**
     * Constructor which will attempt to open given filename with given access parameter.
     * See p.70 of HCC API documentation
     * @param fileHandle
     * @param fileName
     * @param access
     * @param noOpen
     */
    HCCFileGuard(F_FILE** fileHandle, const char* fileName, const char* access);
    /**
     * This constructor can be used if the file was already opened to enable automatic
     * file close on destruction.
     * @param fileHandle
     */
    HCCFileGuard(F_FILE** fileHandle);
    ~HCCFileGuard();
private:
    F_FILE* fileHandle;
};


#endif /* SAM9G20_MEMORY_SDCARDACCESS_H_ */
