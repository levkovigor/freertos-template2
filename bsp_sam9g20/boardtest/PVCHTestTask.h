#ifndef BSP_SAM9G20_BOARDTEST_PVCHTESTTASK_H_
#define BSP_SAM9G20_BOARDTEST_PVCHTESTTASK_H_

#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/tasks/ExecutableObjectIF.h"

#include <array>

class I2cDeviceComIF;
class I2cCookie;

class PVCHTestTask: public SystemObject,
        public ExecutableObjectIF {
public:
    PVCHTestTask(object_id_t objectId);
    virtual~ PVCHTestTask();

    ReturnValue_t initialize() override;
    ReturnValue_t performOperation(uint8_t opCode) override;
private:
    // Cookie required to use the I2C communication interface
    I2cCookie* i2cCookie = nullptr;
    I2cDeviceComIF* i2cComIF = nullptr;

    enum PCA9554Regs: uint8_t {
        INPUT_PORT = 0,
        OUTPUT_PORT = 1,
        POLARITY_INVERSION = 2,
        CONFIG = 3
    };

    std::array<uint8_t, 3> txBuf;

    void simplePca9554Init();
};

#endif /* BSP_SAM9G20_BOARDTEST_PVCHTESTTASK_H_ */
