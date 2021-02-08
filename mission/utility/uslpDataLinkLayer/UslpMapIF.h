#ifndef MISSION_UTILITY_USLPDATALINKLAYER_USLPMAPIF_H_
#define MISSION_UTILITY_USLPDATALINKLAYER_USLPMAPIF_H_
#include <fsfw/datalinklayer/CCSDSReturnValuesIF.h>
#include "USLPTransferFrame.h"
/**
 * @brief       Interface for a USLP Multiplexer Access Point
 * @details     Functions for packing and unpacking frames
 *
 * @author      L. Rajer
 */
class UslpMapIF : public CCSDSReturnValuesIF {
protected:

public:
    /**
     * Empty virtual destructor.
     */
    virtual ~UslpMapIF() {
    }
    /**
     * @brief Method to call to handle a single Transfer Frame.
     * @details The method tries to extract Packets from the frame as stated in the Standard and
     *          forwards them to their destination
     * @param frame
     * @return
     */
    virtual ReturnValue_t extractPackets( USLPTransferFrame* frame ) = 0;

    /**
     * @brief Packs a transfer frame with the Multiplexer Access Point Access Service
     * @details This means privately formated SDUs
     * @return
     */
    // TODO: Best parameter for this
    virtual ReturnValue_t packFrameMapa();

    /**
      * @brief Packs a transfer frame with the Multiplexer Access Point Packket Service
      * @details This means space packets
      * @return
      */
    // TODO: Best parameter for this
    virtual ReturnValue_t packFrameMapp();
    /**
     * Any post-instantiation initialization shall be done in this method.
     * @return
     */
    virtual ReturnValue_t initialize() = 0;

    /**
     * Getter.
     * @return The MAP ID
     */
    virtual ReturnValue_t getMapId() const;
};




#endif /* MISSION_UTILITY_USLPDATALINKLAYER_USLPMAPIF_H_ */
