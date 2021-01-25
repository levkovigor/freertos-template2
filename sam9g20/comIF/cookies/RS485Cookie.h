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


enum RS485Devices: uint8_t {
    COM_FPGA,	//!< Redundant FPGA not counted here, set in device handler
    PCDU_VORAGO,
    PL_VORAGO,
    PL_PIC24,
	DEVICE_COUNT_RS485
};

enum RS485BaudRates: uint32_t {
    FAST = 2000000,
    NORMAL = 115000,
};

enum class ComStatusRS485: uint8_t {
	IDLE,
	TRANSFER_INIT_SUCCESS,
	TRANSFER_SUCCESS,
	FAULTY //!< Errors in communication
};

class RS485Cookie: public CookieIF {
public:
	RS485Cookie(RS485Devices device, RS485BaudRates baudrate);
	virtual ~RS485Cookie();

		void setDevice(RS485Devices device);
		RS485Devices getDevice() const;



		ComStatusRS485 getComStatus();
		void setComStatus(ComStatusRS485 status);

		int8_t getReturnValue();
		void setReturnValue(int8_t retval);

		uint32_t getBaudrate();
private:
		// Device that is communicated with
		RS485Devices device = PCDU_VORAGO;

		// Baudrate of device
		RS485BaudRates baudrate = NORMAL;

		// Stores returnvalues from UART driver, can also be negative
		int8_t returnValue = 0;
		// Stores communication status
		ComStatusRS485 comStatus = ComStatusRS485::IDLE;

};



#endif /* SAM9G20_COMIF_COOKIES_RS485COOKIE_H_ */
