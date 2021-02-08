#ifndef MISSION_UTILITY_USLPDATALINKLAYER_USLPVIRTUALCHANNEL_H_
#define MISSION_UTILITY_USLPDATALINKLAYER_USLPVIRTUALCHANNEL_H_
#include <fsfw/datalinklayer/CCSDSReturnValuesIF.h>
#include <map>
#include "UslpVirtualChannelIF.h"
#include "UslpMapIF.h"


/**
 * @brief   Implementation of a USLP Virtual Channel
 * @details Each virtual channel has a fixed data zone size and arbitrary
 *          amount of MAPs (within the standards limits)
 *          Derived from the Fsfw data link layer
 *          by B. Baetz without its FARM and CLCW parts.
 * @author  L. Rajer
 */
class UslpVirtualChannel: public UslpVirtualChannelIF,
        public CCSDSReturnValuesIF {
public:
    /**
     * @brief Default constructor for a VC
     * Only sets the channelId of the channel. Setting the Sliding Window width is possible as well.
     * @param channelId  Virtual Channel Identifier (VCID) of the channel.
     * @param tfdzSize Data Zone size in bytes, necessary because of fixed frames
     */
    UslpVirtualChannel(uint8_t channelId, size_t tfdzSize);


    ReturnValue_t frameAcceptanceAndReportingMechanism(USLPTransferFrame* frame) override;

    ReturnValue_t initialize() override;

    uint8_t getChannelId () const override;

    /**
     * Helper method to simplify adding a mapChannel during construction.
     * @param mapId The mapId of the object to add.
     * @param object    Pointer to the UslpMap object itself.
     * @return  @c RETURN_OK if the channel was successfully inserted, @c RETURN_FAILED otherwise.
     */
    ReturnValue_t addMapChannel( uint8_t mapId, UslpMapIF* object );
private:
    uint8_t channelId;
    size_t tfdzSize;
    // Typedef to simplify handling of the mapChannels map.
    typedef std::map<uint8_t, UslpMapIF*>::iterator mapChannelIterator;
    // A map that maintains all map Channels. Channels must be configured on initialization.
    std::map<uint8_t, UslpMapIF*> mapChannels;

    /**
     * This method handles demultiplexing to different map channels.
     * It parses the entries of #mapChannels and forwards the Frame to a found MAP Channel.
     * @param frame The frame to forward.
     * @return  #VC_NOT_FOUND or the return value of the map channel extraction.
     */
    ReturnValue_t mapDemultiplexing( USLPTransferFrame* frame );

};

#endif /* MISSION_UTILITY_USLPDATALINKLAYER_USLPVIRTUALCHANNEL_H_ */
