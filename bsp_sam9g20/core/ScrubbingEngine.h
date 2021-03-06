#ifndef SAM9G20_CORE_SCRUBBINGENGINE_H_
#define SAM9G20_CORE_SCRUBBINGENGINE_H_

#include <cstdint>

class SoftwareImageHandler;

class ScrubbingEngine {
public:
    ScrubbingEngine(SoftwareImageHandler* owner);

    /**
     * The hamming code can be stored on FRAM or on the SD card.
     * For the AT91 board, the bootloader will always be stored on the SD
     * card. For the iOBC, the bootloader can also be stored on the FRAM.
     * This will be the  default setting.
     */
    uint8_t hammingCodeOnSdCard = false;
private:
    SoftwareImageHandler* owner;

};



#endif /* SAM9G20_CORE_SCRUBBINGENGINE_H_ */
