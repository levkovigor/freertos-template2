#ifndef SAM9G20_MEMORY_SDCARDACCESS_H_
#define SAM9G20_MEMORY_SDCARDACCESS_H_

#include <fsfw/osal/FreeRTOS/CountingSemaphore.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <sam9g20/memory/SDCardApi.h>

class CountingSemaphore;

class SDCardAccessManager {
public:
    virtual ~SDCardAccessManager();

    /**
     * Returns the single instance of TaskFactory.
     * The implementation of #instance is found in its subclasses.
     * Thus, we choose link-time variability of the  instance.
     */
    static SDCardAccessManager* instance();
    CountingSemaphore countingSemaphore;
private:
    SDCardAccessManager();

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

    SDCardAccess(VolumeId volumeId = SD_CARD_0);
    ~SDCardAccess();

    ReturnValue_t accessResult = HasReturnvaluesIF::RETURN_OK;
    VolumeId currentVolumeId;
};



#endif /* SAM9G20_MEMORY_SDCARDACCESS_H_ */
