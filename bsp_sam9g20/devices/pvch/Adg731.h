#ifndef BSP_SAM9G20_DEVICES_PVCH_ADG731_H_
#define BSP_SAM9G20_DEVICES_PVCH_ADG731_H_

#include <bsp_sam9g20/comIF/cookies/SpiCookie.h>
#include "Pca9554.h"
#include "fsfw/returnvalues/HasReturnvaluesIF.h"

#include <cstdint>

class SpiDeviceComIF;

/**
 * @brief   ADG731 channel multiplexer control software for the PVCH experiment
 * @details
 * This device switches the Current and Voltage measurement channels for the characterization
 * experiment.
 *
 * Based on code provided by the DLR.
 * @author  R. Mueller
 */
class Adg731 {
public:

    enum AdgChannels: uint8_t {
        CH_1,
        CH_2,
        CH_3,
        CH_4,
        CH_5,
        ALL_OFF_CMD
    };

    Adg731(Pca9554& spiMux);

    ReturnValue_t initialize();

    ReturnValue_t set(AdgChannels channel);
private:
    uint8_t cmd = 0;
    Pca9554& spiMux;
    SpiCookie spiCookie;
    BinarySemaphore* spiSemaph = nullptr;
    SpiDeviceComIF* spiComIF = nullptr;
};



#endif /* BSP_SAM9G20_DEVICES_PVCH_ADG731_H_ */
