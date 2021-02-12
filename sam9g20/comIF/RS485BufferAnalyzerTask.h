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

    ReturnValue_t performOperation(uint8_t opCode) override;

    /**
     * @brief   Provides this class with the VC/size map
     * @details Needs to be called before any frames can be received
     * @param virtualChannelFrameSizes  Pointer to Map of <VCID, Total Frame size>
     * @returns -@c RETURN_OK If valid
     *          -@c RETURN_FAILED If nullpointer or empty
     */
    ReturnValue_t setvirtualChannelFrameSizes(std::map<uint8_t, size_t> *virtualChannelFrameSizes);

private:

    object_id_t UslpDataLinkLayerId;
    UslpDataLinkLayer *linkLayer = nullptr;

    ReturnValue_t handleReceiveBuffer();

    object_id_t sharedRingBufferId;
    RingBufferAnalyzer *bufferAnalyzer = nullptr;
    std::array<uint8_t, TMTC_FRAME_MAX_LEN + 5> receiveArray;

    // Stores VC Length map
    std::map<uint8_t, size_t> *virtualChannelFrameSizes = nullptr;

};

#endif /* SAM9G20_COMIF_RS485BUFFERANALYZERTASK_H_ */
