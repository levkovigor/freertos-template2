#ifndef BSP_SAM9G20_DEVICES_PVCH_ADS1278_H_
#define BSP_SAM9G20_DEVICES_PVCH_ADS1278_H_

#include "PvchIF.h"
#include "devices/logicalAddresses.h"

#include "bsp_sam9g20/comIF/cookies/SpiCookie.h"
#include "bsp_sam9g20/devices/pvch/Pca9554.h"

class SpiDeviceComIF;

/**
 * @brief   ADS1278 24bit ADC control software for the PVCH experiment
 * @details
 * The Voltage and Current measurements in the PVCH experiment are fed and converted to digital
 * values by this ADC.
 *
 * Based on code provided by the DLR.
 * @author  R. Mueller
 */
class Ads1278 {
public:

    enum Modes {
        HIGH_SPEED = 0,
        HIGH_RESOLUTION = 1,
        LOW_POWER = 2,
        LOW_SPEED = 3
    };

    Ads1278(Pca9554& i2cMux, addresses::LogAddr drdyPin, uint32_t drdyTimeout = 1000);

    ReturnValue_t getData(int32_t& currentValue, int32_t& voltageValue);
    ReturnValue_t setMode(Modes mode);
    Modes getMode() const;
private:
    enum DrdyLevel: bool {
        LOW = false,
        HIGH = true
    };

    Pca9554& i2cMux;
    addresses::LogAddr drdyPin;
    uint32_t drdyTimeout;
    Modes mode = Modes::HIGH_RESOLUTION;
    SpiCookie spiCookie;
    BinarySemaphore* spiSemaph = nullptr;
    SpiDeviceComIF* spiComIF = nullptr;

    bool isDrdy(DrdyLevel level, uint16_t timeoutMs = 0);
};

#endif /* BSP_SAM9G20_DEVICES_PVCH_ADS1278_H_ */
