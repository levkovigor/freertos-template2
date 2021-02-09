/**
 * @file 	RS485Cookie.h
 *
 * @brief 	Cookie for the SOURCE RS485 Interface
 * @date 	30.12.2020
 * @author 	L. Rajer
 */

#ifndef SAM9G20_COMIF_COOKIES_RS485COOKIE_H_
#define SAM9G20_COMIF_COOKIES_RS485COOKIE_H_

#include <fsfw/devicehandlers/CookieIF.h>

#include <fsfw/osal/FreeRTOS/BinarySemaphore.h>



enum RS485Timeslot : uint8_t {
    COM_FPGA,	// Redundant FPGA not counted here, set in device handler
    PCDU_VORAGO,
    PL_VORAGO,
    PL_PIC24,
    TIMESLOT_COUNT_RS485
};

enum RS485BaudRates : uint32_t {
    FAST = 2000000, NORMAL = 115000,
};

enum class ComStatusRS485 : uint8_t {
    IDLE, TRANSFER_INIT_SUCCESS, TRANSFER_SUCCESS, FAULTY //!< Errors in communication
};

class RS485Cookie: public CookieIF {
public:
    RS485Cookie(RS485Timeslot timeslot, RS485BaudRates baudrate, uint8_t uslp_virtual_channel_id, size_t uslp_tfdz_size,
            uint8_t uslp_deviceCom_map_id, bool hasTmTc = false, uint8_t uslp_tmTc_map_id = 0xFF);
    virtual ~RS485Cookie();

    void setTimeslot(RS485Timeslot timeslot);
    RS485Timeslot getTimeslot() const;

    ComStatusRS485 getComStatus();
    void setComStatus(ComStatusRS485 status);

    int8_t getReturnValue() const;
    void setReturnValue(int8_t retval);

    uint32_t getBaudrate() const;

    uint8_t getVcId() const;

    uint8_t getDevicComMapId() const;

    uint8_t getTmTcMapId() const;

    size_t getTfdzSize() const;
private:
    // Device that is communicated with
    RS485Timeslot timeslot = PCDU_VORAGO;
    // Baudrate of device
    RS485BaudRates baudrate = NORMAL;
    // VCID for device
    uint8_t uslp_virtual_channel_id;
    // MAP ID for Device Communication
    uint8_t uslp_deviceCom_map_id;
    // If device can send and receive TmTc space packets
    bool hasTmTc;
    // Optional MAP ID for TmTc Communication
    uint8_t uslp_tmTc_map_id;
    // Fixed frame data zone size
    size_t uslp_tfdz_size;


    // Stores returnvalues from UART driver, can also be negative
    int8_t returnValue = 0;
    // Stores communication status
    ComStatusRS485 comStatus = ComStatusRS485::IDLE;

};

#endif /* SAM9G20_COMIF_COOKIES_RS485COOKIE_H_ */
