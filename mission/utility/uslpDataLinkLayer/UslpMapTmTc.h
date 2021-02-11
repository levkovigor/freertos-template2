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
    UslpMapTmTc(uint8_t mapId, object_id_t tcDestination, object_id_t tmStoreId,
            object_id_t tcStoreId);

    ReturnValue_t initialize() override;

    ReturnValue_t extractPackets(USLPTransferFrame *frame) override;

    /**
     * @brief Packs a frame with tm from the supplied queue into the output Buffer
     * @param inputBuffer Not used, as data source is TM queue
     * @param inputSize Not used, as data source is TM queue
     * @param outputBuffer Where the frame is placed
     * @param outputSize Maximum size of the  output buffer
     * @return  @c RETURN_OK if a frame with data is written into the buffer
     *          @c RETURN_FAILED if no frame is written because of missing data (e.g. from a queue)
     *          @c Return codes from CCSDSReturnValuesIF for other problems
     */
    ReturnValue_t packFrame(uint8_t *inputBuffer, size_t inputSize, uint8_t *outputBuffer,
            size_t outputSize) override;

    /**
     * Getter.
     * @return The MAP ID of this instance.
     */
    uint8_t getMapId() const override;
private:
    static const uint32_t MAX_PACKET_SIZE = 4096;
    uint8_t mapId;  //!< MAP ID of this MAP Channel.
    uint32_t packetLength = 0;  //!< Complete length of the current Space Packet.
    uint8_t *bufferPosition;    //!< Position to write to in the internal Packet buffer.
    uint8_t packetBuffer[MAX_PACKET_SIZE];  //!< The internal Space Packet Buffer.

    // TmTc Queues and stores
    object_id_t tmStoreId = objects::NO_OBJECT;
    object_id_t tcStoreId = objects::NO_OBJECT;
    object_id_t tcDestination = objects::NO_OBJECT;
    MessageQueueIF *tmTcReceptionQueue = nullptr;
    StorageManagerIF *tmStore = nullptr;
    StorageManagerIF *tcStore = nullptr;
    MessageQueueId_t tcQueueId;     //!< QueueId to send found packets to the distributor.

    /**
     * Helper method to forward a complete packet to the OBSW.
     * @param data  Pointer to the data, either directly from the frame or from the packetBuffer.
     * @param size  Complete total size of the packet.
     * @return  Return Code of the Packet Store or the Message Queue.
     */
    ReturnValue_t sendCompletePacket(uint8_t *data, uint32_t size);

    /**
     * Helper method to handle Packets until segmentation occurs
     */
    ReturnValue_t handleWholePackets(USLPTransferFrame *frame);
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
