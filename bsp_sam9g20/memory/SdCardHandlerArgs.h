#include "fsfw/memory/FileSystemArgsIF.h"
#include <cstdint>

class SdCardHandlerArgs: public FileSystemArgsIF {
public:
    SdCardHandlerArgs(uint16_t* packetSeqIfMissing): packetSeqIfMissing(packetSeqIfMissing) {

    }

    uint16_t* packetSeqIfMissing;
private:
};
