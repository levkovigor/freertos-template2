#ifndef BSP_SAM9G20_DEVICES_PVCH_ADS1278_H_
#define BSP_SAM9G20_DEVICES_PVCH_ADS1278_H_

#include "devices/logicalAddresses.h"
#include "PvchIF.h"

/**
 * @brief   ADS1278 24bit ADC control software for the PVCH experiment
 * @details
 * The Voltage and Current measurements in the PVCH experiment are fed and converted to digital
 * values by this ADC.
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

    Ads1278(addresses::LogAddr drdyPin);

    ReturnValue_t getData(int32_t& currentValue, int32_t& voltageValue);
    ReturnValue_t setMode(Modes mode);
    Modes getMode() const;
private:
    enum DrdyLevel {
        LOW = false,
        HIGH = true
    };

    addresses::LogAddr drdyPin;
    bool isDrdy(bool level, uint16_t timeoutValue = 0);
    Modes mode = Modes::HIGH_RESOLUTION;
};

#endif /* BSP_SAM9G20_DEVICES_PVCH_ADS1278_H_ */
