/**
 * @file    RS485TmTcTarget.h
 * @brief   This file defines the RS485TmTcTarget class.
 * @date    22.12.2020
 * @author  L. Rajer
 */

#ifndef SAM9G20_COMIF_RS485TMTCTARGET_H_
#define SAM9G20_COMIF_RS485TMTCTARGET_H_

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tmtcservices/AcceptsTelemetryIF.h>
#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/tmtcservices/TmTcMessage.h>
#include <fsfw/container/DynamicFIFO.h>

/**
 * @brief       Is used by RS485DeviceComIF to send TM
 * @details
 * Has access to frame buffer of RS485DeviceComIF and is called by it
 * @author      L. Rajer
 */
class RS485TmTcTarget : public AcceptsTelemetryIF,
        public SystemObject {
public:
    static constexpr uint8_t TMTC_RECEPTION_QUEUE_DEPTH = 20;
    static constexpr uint8_t LIMIT_STORED_DATA_SENT_PER_CYCLE = 15;
    static constexpr uint8_t LIMIT_DOWNLINK_PACKETS_STORED = 20;

    static constexpr uint8_t DEFAULT_STORED_DATA_SENT_PER_CYCLE = 5;
    static constexpr uint8_t DEFAULT_DOWNLINK_PACKETS_STORED = 10;

    RS485TmTcTarget(object_id_t objectId, object_id_t tmStoreId);
    virtual ~RS485TmTcTarget();

    /** SystemObject override */
    ReturnValue_t initialize() override;

    /** AcceptsTelemetryIF override */
    MessageQueueId_t getReportReceptionQueue(uint8_t virtualChannel = 0) override;

    ReturnValue_t fillSendFrameBuffer();

private:
    object_id_t tmStoreId = objects::NO_OBJECT;

    MessageQueueIF *tmTcReceptionQueue = nullptr;
    StorageManagerIF* tmStore = nullptr;
    /**
     * This fifo can be used to store downlink data
     * which can not be sent at the moment.
     */
    DynamicFIFO<store_address_t>* tmFifo = nullptr;
    uint8_t sentPacketsPerCycle = DEFAULT_STORED_DATA_SENT_PER_CYCLE;
    uint8_t maxNumberOfPacketsStored = DEFAULT_DOWNLINK_PACKETS_STORED;

};

#endif /* SAM9G20_COMIF_RS485TMTCTARGET_H_ */
