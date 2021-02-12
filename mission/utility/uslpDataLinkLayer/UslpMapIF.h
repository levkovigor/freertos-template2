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
     * @brief This method shall pack a frame with the given data into the output Buffer
     * @details It is possible to only both input and output buffer or only one of the two.
     *          This can be helpful if the data source or destination is provided at construction
     *          (e.g. a TM queue). However, this has to be clearly specified in the implementation
     * @param inputBuffer where data is taken from (may be ignored in certain implementation cases)
     * @param inputSize length of the data (cannot exceed data zone size for VC)
     * @param outputBuffer Where the frame is placed
     * @param outputSize Maximum size of the  output buffer
     * @param tfdzSize Size of the frame data zone
     * @param returnFrame [out] this pointer is passed back so that the frame can be filled further
     * @return  @c RETURN_OK if a frame with data is written into the buffer
     *          @c RETURN_FAILED if no frame is written because of missing data (e.g. from a queue)
     *          @c Return codes from CCSDSReturnValuesIF for other problems
     */
    virtual ReturnValue_t packFrame(uint8_t *inputBuffer, size_t inputSize,
            uint8_t *outputBuffer, size_t outputSize, size_t tfdzSize, USLPTransferFrame * returnFrame) = 0;

    /**
     * Any post-instantiation initialization shall be done in this method.
     * @return
     */
    virtual ReturnValue_t initialize() = 0;

    /**
     * Getter.
     * @return The MAP ID
     */
    virtual uint8_t getMapId() const = 0;
};




#endif /* MISSION_UTILITY_USLPDATALINKLAYER_USLPMAPIF_H_ */
