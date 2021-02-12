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
    virtual ReturnValue_t addMapChannel(uint8_t mapId, UslpMapIF *object) = 0;
    /**
     * @brief This method shall accept frames and do VC demultiplexing
     * @details Handling the Frame includes forwarding to higher-level procedures.
     * @param frame The USLP Transfer Frame that was received and checked.
     * @return The return Value shall indicate successful processing with @c RETURN_OK.
     */
    virtual ReturnValue_t frameAcceptanceAndReportingMechanism(USLPTransferFrame *frame) = 0;

    /**
     * @brief This method shall forward the source and origin buffer to the specified MAP
     * @param inputBuffer where data is taken from (may be ignored in certain implementation cases)
     * @param inputSize length of the data (cannot exceed data zone size for VC)
     * @param outputBuffer Where the frame is placed
     * @param outputSize Maximum size of the  output buffer
     * @param mapId multiplexer access point ID
     * @param returnFrame [out] reference to a frame pointer, the pointer is a nullptr and is filled
     *        set in the MAP routine to the MAP output frame buffer
     * @return  @c RETURN_OK if a frame with data is written into the buffer
     *          @c RETURN_FAILED if no frame is written because of missing data (e.g. from a queue)
     *          @c Return codes from CCSDSReturnValuesIF for other problems
     */
    virtual ReturnValue_t multiplexFrameMap(uint8_t *inputBuffer, size_t inputSize,
            uint8_t *outputBuffer, size_t outputSize, uint8_t mapId, USLPTransferFrame *&returnFrame) = 0;

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
