#ifndef SAM9G20_TMTCBRIDGE_RS485POLLINGTASK_H_
#define SAM9G20_TMTCBRIDGE_RS485POLLINGTASK_H_

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <fsfw/tasks/ExecutableObjectIF.h>

extern "C" {
#include <AT91SAM9G20.h>
#include <hal/Drivers/UART.h>
}

#include <config/OBSWConfig.h>

/**
 * @brief   Separate task to poll RS485 with ISIS drivers. Reads all data,
 *          by keeping up two queue transfers which use DMA and callbacks.
 * @details
 * This polling task does not handle the data and simply reads all of it into
 * a shared ring buffer. Analyzing the ring buffer needs to be done by
 * another task.
 * @author  R. Mueller
 */
class RS485PollingTask: public SystemObject,
						public ExecutableObjectIF,
						public HasReturnvaluesIF{
public:
    static constexpr uint32_t RS485_REGULARD_BAUD = config::RS485_REGULAR_BAUD;
    static constexpr uint32_t RS485_FAST_BAUD = config::RS485_FAST_BAUD;


    RS485PollingTask(object_id_t objectId);

    ReturnValue_t performOperation(uint8_t opCode) override;
    ReturnValue_t initialize() override;

private:

};



#endif /* SAM9G20_TMTCBRIDGE_RS485POLLINGTASK_H_ */
