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

/**
 * @brief       Is used by RS485DeviceComIF to send TM
 * @details
 * Has access to frame buffer of RS485DeviceComIF and is called by it
 * @author      L. Rajer
 */
class RS485TmTcTarget : public AcceptsTelemetryIF,
        public SystemObject {
public:
    RS485TmTcTarget(object_id_t objectId);
    virtual ~RS485TmTcTarget();

    /** AcceptsTelemetryIF override */
    virtual MessageQueueId_t getReportReceptionQueue(uint8_t virtualChannel = 0) override;

    ReturnValue_t fillSendFrameBuffer();

private:
    MessageQueueIF *tmTcReceptionQueue = nullptr;
};

#endif /* SAM9G20_COMIF_RS485TMTCTARGET_H_ */
