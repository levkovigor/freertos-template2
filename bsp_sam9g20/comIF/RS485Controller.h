#ifndef SAM9G20_COMIF_RS485CONTROLLER_H_
#define SAM9G20_COMIF_RS485CONTROLLER_H_

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <cstdint>

enum RS485Steps: uint8_t {
    SYRLINKS_ACTIVE,
    PCDU_VORAGO_ACTIVE,
    PL_VORAGO_ACTIVE,
    PL_PIC24_ACTIVE
};

class RS485Controller: public SystemObject,
        public ExecutableObjectIF {
public:
    static constexpr uint8_t RETRY_COUNTER = 10;

    RS485Controller(object_id_t objectId);

    virtual ReturnValue_t performOperation(uint8_t opCode) override;
    virtual ReturnValue_t initialize() override;
private:
    uint8_t retryCount = 0;

    ReturnValue_t checkDriverState(uint8_t* retryCount);

};



#endif /* SAM9G20_COMIF_RS485CONTROLLER_H_ */
