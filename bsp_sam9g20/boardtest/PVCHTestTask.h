#ifndef BSP_SAM9G20_BOARDTEST_PVCHTESTTASK_H_
#define BSP_SAM9G20_BOARDTEST_PVCHTESTTASK_H_

#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/tasks/ExecutableObjectIF.h"

#include "bsp_sam9g20/devices/pvch/Max7301.h"
#include "bsp_sam9g20/devices/pvch/Pca9554.h"
#include "bsp_sam9g20/comIF/cookies/I2cCookie.h"

#include <array>

class PVCHTestTask: public SystemObject,
        public ExecutableObjectIF {
public:

    enum TestModes {
        MAX_7301
    };

    TestModes testMode = TestModes::MAX_7301;

    PVCHTestTask(object_id_t objectId);
    virtual~ PVCHTestTask();

    ReturnValue_t initialize() override;
    ReturnValue_t performOperation(uint8_t opCode) override;
private:
    Pca9554 i2cMux;
    Max7301 gpioExpander;

};

#endif /* BSP_SAM9G20_BOARDTEST_PVCHTESTTASK_H_ */
