#ifndef MISSION_DEVICES_PVCH_PCA9554_H_
#define MISSION_DEVICES_PVCH_PCA9554_H_

#include "fsfw/devicehandlers/CookieIF.h"
#include "fsfw/devicehandlers/DeviceCommunicationIF.h"
#include "fsfw/returnvalues/HasReturnvaluesIF.h"

#include "bsp_sam9g20/comIF/cookies/I2cCookie.h"

#include <cstdint>

class I2cDeviceComIF;

/**
 * @brief   PCA9554 I2C demultiplexer control software used on the PVCH board supplied by the DLR
 * @details
 * This expander is required because there are not enough GPIO pins available on the iOBC to
 * switch the other SPI devices on the PVCH board otherwise.
 * Based on code provided by the DLR.
 *
 * @author  R. Mueller
 */
class Pca9554 {
public:
    enum PCA9554Regs: uint8_t {
        INPUT_PORT = 0,
        OUTPUT_PORT = 1,
        POLARITY_INVERSION = 2,
        CONFIG = 3
    };

    Pca9554();

    ReturnValue_t intialize();

    ReturnValue_t setAdcModeLow();
    ReturnValue_t clearAdcModeLow();

    ReturnValue_t setAdcModeHigh();
    ReturnValue_t clearAdcModeHigh();

    ReturnValue_t setCsLoadSw();
    ReturnValue_t clearCsLoadSw();

    ReturnValue_t setMuxSync();
    ReturnValue_t clearMuxSync();

    ReturnValue_t setAdcSync();
    ReturnValue_t clearAdcSync();
private:

    void simplePca9554Init();
    ReturnValue_t blockingWriteWrapper(uint8_t reg, uint8_t val);
    ReturnValue_t blockingReadbackWrapper(uint8_t** buf, size_t* readSize);

    // Cookie required to use the I2C communication interface
    I2cCookie i2cCookie;
    I2cDeviceComIF* i2cComIF = nullptr;
    BinarySemaphore* semaphHandle = nullptr;
    // Initialize state as all high
    uint8_t outputState = 0xff;

    std::array<uint8_t, 3> txBuf;
};

#endif /* MISSION_DEVICES_PVCH_PCA9554_H_ */
