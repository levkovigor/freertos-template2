/**
 * @file    RS485TmTcTarget.h
 * @brief   This file defines the RS485TmTcTarget class.
 * @date    22.12.2020
 * @author  L. Rajer
 */

#ifndef SAM9G20_COMIF_RS485TMTCTARGET_H_
#define SAM9G20_COMIF_RS485TMTCTARGET_H_

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/tmtcservices/AcceptsTelemetryIF.h>
#include <fsfw/tmtcservices/AcceptsTelecommandsIF.h>
#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/tmtcservices/TmTcMessage.h>
#include <fsfw/container/DynamicFIFO.h>
#include <fsfw/container/SharedRingBuffer.h>
#include <sam9g20/core/RingBufferAnalyzer.h>
#include <fsfwconfig/OBSWConfig.h>
#include "../../mission/utility/uslpDataLinkLayer/USLPTransferFrame.h"

/**
 * @brief       Is used by RS485DeviceComIF to send TM
 * @details
 * Has access to frame buffer of RS485DeviceComIF and is called by it
 * @author      L. Rajer
 */
class RS485TmTcTarget: public AcceptsTelemetryIF,
        public AcceptsTelecommandsIF,
        public ExecutableObjectIF,
        public SystemObject {
public:
    static constexpr uint8_t TMTC_RECEPTION_QUEUE_DEPTH = 20;
    static constexpr uint8_t LIMIT_STORED_DATA_SENT_PER_CYCLE = 15;
    static constexpr uint8_t LIMIT_DOWNLINK_PACKETS_STORED = 20;

    static constexpr uint8_t DEFAULT_STORED_DATA_SENT_PER_CYCLE = 5;
    static constexpr uint8_t DEFAULT_DOWNLINK_PACKETS_STORED = 10;

    static constexpr size_t TMTC_FRAME_MAX_LEN = config::RS485_MAX_SERIAL_FRAME_SIZE;
    static constexpr uint8_t MAX_TC_PACKETS_HANDLED = 5;

    RS485TmTcTarget(object_id_t objectId_, object_id_t tcDestination, object_id_t tmStoreId,
            object_id_t tcStoreId, object_id_t sharedRingBufferId);
    virtual ~RS485TmTcTarget();

    /** SystemObject override */
    ReturnValue_t initialize() override;

    ReturnValue_t performOperation(uint8_t opCode) override;

    /** AcceptsTelemetryIF override */
    MessageQueueId_t getReportReceptionQueue(uint8_t virtualChannel = 0) override;

    /** AcceptsTelecommandsIF override */
    virtual uint16_t getIdentifier() override;
    virtual MessageQueueId_t getRequestQueue() override;

    /**
     * @brief   Provides this class with the VC/size map
     * @details Needs to be called before any frames can be received
     * @param virtualChannelFrameSizes  Pointer to Map of <VCID, Total Frame size>
     * @returns -@c RETURN_OK If valid
     *          -@c RETURN_FAILED If nullpointer or empty
     */
    ReturnValue_t setvirtualChannelFrameSizes(std::map<uint8_t, size_t>* virtualChannelFrameSizes);

    ReturnValue_t fillSendFrameBuffer(USLPTransferFrame *frame);

private:
    object_id_t tmStoreId = objects::NO_OBJECT;
    object_id_t tcStoreId = objects::NO_OBJECT;
    object_id_t tcDestination = objects::NO_OBJECT;

    MessageQueueIF *tmTcReceptionQueue = nullptr;
    StorageManagerIF *tmStore = nullptr;
    StorageManagerIF *tcStore = nullptr;
    // Used to split packets into different frames
    TmTcMessage *overhangMessage = nullptr;
    uint8_t overhangMessageSentBytes = 0;

    ReturnValue_t handleReceiveBuffer();
    ReturnValue_t handlePacketReception(size_t foundLen);

    object_id_t sharedRingBufferId = objects::NO_OBJECT;
    RingBufferAnalyzer *analyzerTask = nullptr;
    std::array<uint8_t, TMTC_FRAME_MAX_LEN + 5> receiveArray;

    // Stores VC Length map
    std::map<uint8_t, size_t>* virtualChannelFrameSizes = nullptr;
    /**
     * This fifo can be used to store downlink data
     * which can not be sent at the moment.
     */
    DynamicFIFO<store_address_t> *tmFifo = nullptr;
    uint8_t sentPacketsPerCycle = DEFAULT_STORED_DATA_SENT_PER_CYCLE;
    uint8_t maxNumberOfPacketsStored = DEFAULT_DOWNLINK_PACKETS_STORED;

};

#endif /* SAM9G20_COMIF_RS485TMTCTARGET_H_ */
