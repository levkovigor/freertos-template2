#ifndef MISSION_UTILITY_USLPDATALINKLAYER_USLPMAPTMTC_H_
#define MISSION_UTILITY_USLPDATALINKLAYER_USLPMAPTMTC_H_

#include <fsfw/objectmanager/ObjectManagerIF.h>
#include <fsfw/ipc/MessageQueueSenderIF.h>
#include <fsfw/tmtcservices/AcceptsTelemetryIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/storagemanager/StorageManagerIF.h>
#include <fsfw/tmtcservices/TmTcMessage.h>
#include "UslpMapIF.h"
#include "USLPTransferFrame.h"

/**
 * @brief       Implementation for a USLP MAP that handles Tm and Tc
 * @details     This is basically an implementation of the USLP MAP Packet service
 *              It needs to be a system object as it basically handles reading from
 *              the tm queue.
 *              Not thread safe.
 *
 * @author      L. Rajer
 */

class UslpMapTmTc: public UslpMapIF,
        public AcceptsTelemetryIF,
        public SystemObject {
public:
    static constexpr uint8_t USLP_PROTOCOL_ID = 0;
    static constexpr uint8_t USLP_TFDZ_CONSTRUCTION_RULES = 0;
    /**
     * @brief Default constructor
     * @param mapId  The MAP ID of the instance.
     */
    UslpMapTmTc(object_id_t objectId, uint8_t mapId, object_id_t tcDestination,
            object_id_t tmStoreId, object_id_t tcStoreId);

    ReturnValue_t initialize() override;

    ReturnValue_t extractPackets(USLPTransferFrame *frame) override;

    /**
     * @brief Packs a frame with tm from the supplied queue into the output Buffer
     * @param inputBuffer Not used, as data source is TM queue
     * @param inputSize Not used, as data source is TM queue
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

    /** AcceptsTelemetryIF override */
    MessageQueueId_t getReportReceptionQueue(uint8_t virtualChannel = 0) override;

private:
    static const uint32_t MAX_PACKET_SIZE = 4096;
    static constexpr uint8_t TMTC_RECEPTION_QUEUE_DEPTH = 20;
    uint8_t mapId;  //!< MAP ID of this MAP Channel.
    uint32_t packetLength = 0;  //!< Complete length of the current Space Packet.
    uint8_t *bufferPosition;  //!< Position to write to in the internal Packet buffer.
    uint8_t packetBuffer[MAX_PACKET_SIZE];  //!< The internal Space Packet Buffer.

    // TmTc Queues and stores
    object_id_t tmStoreId = objects::NO_OBJECT;
    object_id_t tcStoreId = objects::NO_OBJECT;
    object_id_t tcDestination = objects::NO_OBJECT;
    MessageQueueIF *tmTcReceptionQueue = nullptr;
    StorageManagerIF *tmStore = nullptr;
    StorageManagerIF *tcStore = nullptr;
    MessageQueueId_t tcQueueId = 0;  //!< QueueId to send found packets to the distributor.

    USLPTransferFrame *outputFrame;
    // Used to split packets into different frames
    TmTcMessage *overhangMessage = nullptr;
    uint8_t overhangMessageSentBytes = 0;

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
     * Helper method to set the frame info relevant in this multiplexing stage
     */
    void setFrameInfo(USLPTransferFrame *frame);
    /**
     * Helper method to reset the internal buffer.
     */
    void clearBuffers();

    /**
     * Debug method to print the packet Buffer's content.
     */
    void printPacketBuffer();

}
;

#endif /* MISSION_UTILITY_USLPDATALINKLAYER_USLPMAPTMTC_H_ */
