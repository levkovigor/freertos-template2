#ifndef MISSION_UTILITY_USLPDATALINKLAYER_USLPDATALINKLAYER_H_
#define MISSION_UTILITY_USLPDATALINKLAYER_USLPDATALINKLAYER_H_

#include <map>
#include "USLPTransferFrame.h"
#include "UslpVirtualChannelIF.h"
#include <fsfw/datalinklayer/CCSDSReturnValuesIF.h>
#include <fsfw/objectmanager/SystemObject.h>
/**
 * @brief   Class for receiving fixed length USLP Frames
 * @details Performs VC and MAP demultiplexing of USLP Frames
 *          with truncated primary header. Derived from the Fsfw data link layer
 *          by B. Baetz without its FARM and CLCW parts.
 * @author  L. Rajer
 */
class UslpDataLinkLayer: public CCSDSReturnValuesIF,
        public SystemObject {
public:

    /**
     * The Constructor sets the passed parameters and nothing else.
     * @param set_scid  The SCID to operate on.
     */
    UslpDataLinkLayer(object_id_t objectId, uint16_t set_scid);
    /**
     * Empty virtual destructor.
     */
    virtual ~UslpDataLinkLayer();

    ReturnValue_t initialize() override;
    /**
     * This method tries to process a frame that is placed in #frameBuffer.
     * The procedures described in the Standard are performed.
     * @param length    Length of the incoming frame candidate.
     * @return  @c RETURN_OK on successful handling, otherwise the return codes of the higher methods.
     */
    ReturnValue_t processFrame(uint16_t length);

    /**
     * @brief Packs a frame filled with PUS TM packets
     * @details This method takes PUS TM packets from the telemetry queue supplied to the specified
     *          MAP object at construction and fills the specified buffer with it.
     * @param buffer to store the frame in
     * @param bufferSize Maximum size of the buffer
     * @param vcId The virtual channel ID for the telemetry device
     * @ param mapId multiplexer access point ID, must implement the packMappFrame method
     * @return  @c RETURN_OK if a frame with data is written into the buffer
     *          @c RETURN_FAILED if there are no packets in queue
     *          @c Return codes from CCSDSReturnValuesIF for other problems
     */
    ReturnValue_t packTmFrame(uint8_t *buffer, size_t bufferSize, uint8_t vcId, uint8_t mapId);

    /**
     * @brief Packs a frame with a device command and forwards it to the transmit buffer
     * @details This method takes a supplied device command from the buffer, packs it into a Uslp
     *          frame and then writes the  frame to mutex protected transmit buffer where it is sent
     *          in the next communication slot for the vcid.
     * @param buffer holds the device command
     * @param bufferSize Size of the device command
     * @param vcId The virtual channel ID for the device
     * @ param mapId multiplexer access point ID, must implement the packMapaFrame method
     * @return  @c RETURN_OK if a frame with data is written into the send buff
     *          @c Return codes from CCSDSReturnValuesIF for other problems
     */
    ReturnValue_t sendDeviceCommandFrame(uint8_t *commandBuffer, size_t commandSize, uint8_t vcId,
            uint8_t mapId);
    /**
     * Configuration method to add a new USLP Virtual Channel.
     * Shall only be called during initialization. As soon as the method was called, the layer can
     * handle Frames directed to this VC.
     * @param virtualChannelId  Id of the VC. Shall be smaller than 64.
     * @param object    Reference to the object that handles the Frame.
     * @return  @c RETURN_OK on success, @c RETURN_FAILED otherwise.
     */
    ReturnValue_t addVirtualChannel(uint8_t virtualChannelId, UslpVirtualChannelIF *object);

    /**
     * The initialization method calls the @c initialize routine of all virtual channels.
     * @return The return code of the first failed VC initialization or @c RETURN_OK.
     */
    ReturnValue_t initializeBuffer(uint8_t *frameBuffer);
private:
    typedef std::map<uint8_t, UslpVirtualChannelIF*>::iterator virtualChannelIterator; //!< Typedef to simplify handling the #virtualChannels map.
    static const uint8_t FRAME_VERSION_NUMBER_DEFAULT = 0b1100; //!< Constant for the default value of Frame Version Numbers.
    static const uint8_t FRAME_PRIMARY_HEADER_LENGTH = 4; //!< Length of the frame's primary header.
    static const bool USE_CRC = true; //!< A global, so called "Managed Parameter" that identifies if incoming frames have CRC's or not.
    uint16_t spacecraftId;  //!< The Space Craft Identifier (SCID) configured.
    uint8_t *frameBuffer;   //!< A pointer to point to the current incoming frame.
    uint16_t receivedDataLength;    //!< Stores the length of the currently processed frame.
    USLPTransferFrame currentFrame;   //!< Stores a more convenient access to the current frame.
    std::map<uint8_t, UslpVirtualChannelIF*> virtualChannels; //!< Map of all virtual channels assigned.
    /**
     * Method that performs all possible frame validity checks (as specified).
     * @return  Various error codes or @c RETURN_OK on success.
     */
    ReturnValue_t frameValidationCheck();

    /**
     * Small helper method to check the CRC of the Frame.
     * @return @c RETURN_OK or @c CRC_FAILED.
     */
    ReturnValue_t frameCheckCRC();

    /**
     * Method to demultiplex the Frames to Virtual Channels (VC's).
     * Looks up the requested VC in #virtualChannels and forwards the Frame to its
     * #frameAcceptanceAndReportingMechanism method, if found.
     * @return The higher method codes or @c VC_NOT_FOUND.
     */
    ReturnValue_t virtualChannelDemultiplexing();

};

#endif /* MISSION_UTILITY_USLPDATALINKLAYER_USLPDATALINKLAYER_H_ */
