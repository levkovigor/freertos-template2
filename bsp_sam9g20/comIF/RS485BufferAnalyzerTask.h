#ifndef SAM9G20_COMIF_RS485BUFFERANALYZERTASK_H_
#define SAM9G20_COMIF_RS485BUFFERANALYZERTASK_H_

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/container/SharedRingBuffer.h>
#include <sam9g20/core/RingBufferAnalyzer.h>
#include <fsfwconfig/OBSWConfig.h>
#include <mission/utility/uslpDataLinkLayer/UslpDataLinkLayer.h>

/**
 * @brief       Is used to analyze the RS485 reception buffer
 * @details
 * @author      L. Rajer
 */
class RS485BufferAnalyzerTask: public ExecutableObjectIF,
        public SystemObject {
public:
    static constexpr size_t TMTC_FRAME_MAX_LEN = config::RS485_MAX_SERIAL_FRAME_SIZE;
    static constexpr uint8_t MAX_TC_PACKETS_HANDLED = 5;

    RS485BufferAnalyzerTask(object_id_t objectId, object_id_t sharedRingBufferId,
            object_id_t UslpDataLinkLayerId);
    virtual ~RS485BufferAnalyzerTask();

    /** SystemObject override */
    ReturnValue_t initialize() override;
    /** ExecutableObject override */
    ReturnValue_t performOperation(uint8_t opCode) override;

private:

    object_id_t sharedRingBufferId;
    RingBufferAnalyzer *bufferAnalyzer = nullptr;
    std::array<uint8_t, TMTC_FRAME_MAX_LEN + 5> receiveArray;

    object_id_t UslpDataLinkLayerId;
    UslpDataLinkLayer *linkLayer = nullptr;

    ReturnValue_t handleReceiveBuffer();

};

#endif /* SAM9G20_COMIF_RS485BUFFERANALYZERTASK_H_ */
