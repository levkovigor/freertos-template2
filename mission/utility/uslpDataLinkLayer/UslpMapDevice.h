#ifndef MISSION_UTILITY_USLPDATALINKLAYER_USLPMAPDEVICE_H_
#define MISSION_UTILITY_USLPDATALINKLAYER_USLPMAPDEVICE_H_

#include "UslpMapIF.h"
#include "USLPTransferFrame.h"

/**
 * @brief       Implementation for a USLP MAP that handles Device Command
 * @details     This is basically an implementation of the USLP MAP Access service
 *
 * @author      L. Rajer
 */

class UslpMapDevice: public UslpMapIF {
public:
    static constexpr uint8_t USLP_PROTOCOL_ID = 1;
    static constexpr uint8_t USLP_TFDZ_CONSTRUCTION_RULES = 1;
    /**
     * @brief Default constructor
     * @param mapId  The MAP ID of the instance.
     */
    UslpMapDevice(uint8_t mapId, uint8_t *receiveBuffer, size_t receiveBufferSize);

    ReturnValue_t initialize() override;

    ReturnValue_t extractPackets(USLPTransferFrame *frame) override;

    /**
     * @brief Packs a frame with data supplied from the input buffer into the output buffer
     * @param inputBuffer Data to put into frame
     * @param inputSize Size of data to put into frame
     * @param outputBuffer Where the frame is placed
     * @param outputSize Maximum size of the  output buffer
     * @param tfdzSize Size of the frame data zone
     * @param returnFrame [out] reference to a frame pointer, the pointer is a nullptr and is
     *        set to the MAP output frame buffer here, so that it can be filled further in
     *        the higher multiplexing levels
     * @return  @c RETURN_OK if a frame with data is written into the buffer
     *          @c RETURN_FAILED if no frame is written because of missing data (e.g. from a queue)
     *          @c Return codes from CCSDSReturnValuesIF for other problems
     */
    ReturnValue_t packFrame(const uint8_t *inputBuffer, size_t inputSize, uint8_t *outputBuffer,
            size_t outputSize, size_t tfdzSize, USLPTransferFrame *&returnFrame) override;

    /**
     * Getter.
     * @return The MAP ID of this instance.
     */
    uint8_t getMapId() const override;

private:
    uint8_t mapId;  //!< MAP ID of this MAP Channel.
    uint8_t *receiveBuffer;  //!< Receive Buffer where received data is stored
    size_t receiveBufferSize; //!< Maximum receive buffer size

    USLPTransferFrame *outputFrame;

    /**
     * Helper method to set the frame info relevant in this multiplexing stage
     */
    void setFrameInfo(USLPTransferFrame *frame);

}
;

#endif /* MISSION_UTILITY_USLPDATALINKLAYER_USLPMAPDEVICE_H_ */
