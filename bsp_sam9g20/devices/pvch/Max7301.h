#ifndef BSP_SAM9G20_DEVICES_MAX7301_H_
#define BSP_SAM9G20_DEVICES_MAX7301_H_

#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <bsp_sam9g20/comIF/cookies/SpiCookie.h>
#include <bsp_sam9g20/devices/pvch/Pca9554.h>

#include <array>

class SpiDeviceComIF;

/**
 * @brief   MAX7301 GPIO expander control software used on the PVCH board supplied by the DLR
 * @details
 * This GPIO expander is used to control the Switchable Resistor Network (SRN) of the PVCH board.
 * 8 Resistors are located on ports 4 to 11.
 */
class Max7301 {
public:
    enum RegisterData: uint8_t {
        OUTPUT = 0x01,
        INPUT = 0x02,
        INPUT_PULLUP = 0x03,
        OUTPUT_ALL = 0x55,
        SET_OUTPUT_LOW = 0x00,
        SET_OUTPUT_HIGH = 0x01,
        NORMAL_OP = 0x01
    };

    enum CmdAddr: uint8_t {
        NO_OP = 0x00,

        CFG = 0x04,

        P_CONF_7_TO_4 = 0x09,
        P_CONF_11_TO_8 = 0x0A,
        P_CONF_15_TO_12 = 0x0B,
        P_CONF_19_TO_16 = 0x0C,
        P_CONF_23_TO_20 = 0x0D,
        P_CONF_27_TO_24 = 0x0E,
        P_CONF_31_TO_28 = 0x0F,

        P4 = 0x24,
        P5 = 0x25,
        P6 = 0x26,
        P7 = 0x27,
        P8 = 0x28,
        P9 = 0x29,
        P10 = 0x2A,
        P11 = 0x2B,
        P12 = 0x2C,
        P13 = 0x2D,
        P14 = 0x2E,
        P15 = 0x2F,
        P16 = 0x30,
        P17 = 0x31,
        P18 = 0x32,
        P19 = 0x33,
        P20 = 0x34,
        P21 = 0x35,
        P22 = 0x36,
        P23 = 0x37,
        P24 = 0x38,
        P25 = 0x39,
        P26 = 0x3A,
        P27 = 0x3B,
        P28 = 0x3C,
        P29 = 0x3D,
        P30 = 0x3E,
        P31 = 0x3F,

        // 8 ports set-up
        P4_TO_11 = 0x44,
        P12_TO_19 = 0x4C,
        P20_TO_27 = 0x54,
        P24_TO_31 = 0x58,

    };

    Max7301(Pca9554& i2cMux);

    ReturnValue_t initialize();

    ReturnValue_t set(CmdAddr cmdAddr, RegisterData data);

    ReturnValue_t read(CmdAddr cmdAddr, uint8_t& readByte);

private:
    static constexpr uint8_t READ_MSK = 0b10000000;

    std::array <uint8_t, 3> txBuf = {};
    Pca9554& i2cMux;
    SpiCookie spiCookie;
    BinarySemaphore* spiSemaph = nullptr;
    SpiDeviceComIF* spiComIF;
};



#endif /* BSP_SAM9G20_DEVICES_MAX7301_H_ */
