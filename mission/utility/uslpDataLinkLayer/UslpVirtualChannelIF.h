#ifndef MISSION_UTILITY_USLPDATALINKLAYER_USLPVIRTUALCHANNELIF_H_
#define MISSION_UTILITY_USLPDATALINKLAYER_USLPVIRTUALCHANNELIF_H_

#include "USLPTransferFrame.h"
#include "UslpMapIF.h"
#include <fsfw/returnvalues/HasReturnvaluesIF.h>

/**
 * @brief   Interface for a virtual channel class for USLp
 * @details Derived from the Fsfw data link layer
 *          by B. Baetz without its FARM and CLCW parts.
 * @author  L. Rajer
 */
class UslpVirtualChannelIF {
public:
    /**
     * Empty virtual destructor.
     */
    virtual ~UslpVirtualChannelIF() {

    }


    /**
     * @brief   This method shall add a Map to this VC
     * @param mapId The mapId of the object to add.
     * @param object    Pointer to the UslpMap object itself.
     * @return  @c RETURN_OK if adding to map works
     */
    virtual ReturnValue_t addMapChannel( uint8_t mapId, UslpMapIF* object ) = 0;
    /**
     * This method shall accept frames and do VC demultiplexing
     * Handling the Frame includes forwarding to higher-level procedures.
     * @param frame The USLP Transfer Frame that was received and checked.
     * @return The return Value shall indicate successful processing with @c RETURN_OK.
     */
    virtual ReturnValue_t frameAcceptanceAndReportingMechanism(USLPTransferFrame *frame) = 0;
    /**
     * If any other System Objects are required for operation they shall be initialized here.
     * @return  @c RETURN_OK for successful initialization.
     */
    virtual ReturnValue_t initialize() = 0;
    /**
     * Getter for the VCID.
     * @return  The #channelId.
     */
    virtual uint8_t getChannelId() const = 0;
};

#endif /* MISSION_UTILITY_USLPDATALINKLAYER_USLPVIRTUALCHANNELIF_H_ */
