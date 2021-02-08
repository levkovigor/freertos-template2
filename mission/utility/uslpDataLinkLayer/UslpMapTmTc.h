#ifndef MISSION_UTILITY_USLPDATALINKLAYER_USLPMAPTMTC_H_
#define MISSION_UTILITY_USLPDATALINKLAYER_USLPMAPTMTC_H_

#include <fsfw/objectmanager/ObjectManagerIF.h>
#include <fsfw/ipc/MessageQueueSenderIF.h>
#include <fsfw/storagemanager/StorageManagerIF.h>
#include "UslpMapIF.h"
#include "USLPTransferFrame.h"

/**
 * @brief       Implementation for a USLP MAP that handles Tm and Tc
 * @details     This is basically an implementation of the USLP MAP Packet service
 *
 * @author      L. Rajer
 */


class UslpMapTmTc: public UslpMapIF {
public:
    /**
     * @brief Default constructor
     * @param mapId  The MAP ID of the instance.
     */
    UslpMapTmTc(uint8_t mapId, object_id_t tmSource, object_id_t tcDestination);

    ReturnValue_t initialize() override;

    ReturnValue_t extractPackets(USLPTransferFrame *frame) override;

    /**
     * @brief Packs a transfer frame with the Multiplexer Access Point Access Service
     * @details This means privately formated SDUs
     * @return
     */
    virtual ReturnValue_t packFrameMapa() override;

    /**
     * @brief Packs a transfer frame with the Multiplexer Access Point Packket Service
     * @details This means space packets
     * @return
     */
    virtual ReturnValue_t packFrameMapp() override;

    /**
     * Getter.
     * @return The MAP ID of this instance.
     */
    uint8_t getMapId() const;
private:
    static const uint32_t MAX_PACKET_SIZE = 4096;
    uint8_t mapId;  //!< MAP ID of this MAP Channel.
    uint32_t packetLength = 0;  //!< Complete length of the current Space Packet.
    uint8_t *bufferPosition;    //!< Position to write to in the internal Packet buffer.
    uint8_t packetBuffer[MAX_PACKET_SIZE];  //!< The internal Space Packet Buffer.
    object_id_t packetDestination;
    //!< Pointer to the store where full TC packets are stored.
    StorageManagerIF *packetStore = nullptr;
    MessageQueueId_t tcQueueId;     //!< QueueId to send found packets to the distributor.

    /**
     * Helper method to forward a complete packet to the OBSW.
     * @param data  Pointer to the data, either directly from the frame or from the packetBuffer.
     * @param size  Complete total size of the packet.
     * @return  Return Code of the Packet Store or the Message Queue.
     */
    ReturnValue_t sendCompletePacket(uint8_t *data, uint32_t size);

    /**
     * Helper method to reset the internal buffer.
     */
    void clearBuffers();

    /**
     * Debug method to print the packet Buffer's content.
     */
    void printPacketBuffer();

};

#endif /* MISSION_UTILITY_USLPDATALINKLAYER_USLPMAPTMTC_H_ */
