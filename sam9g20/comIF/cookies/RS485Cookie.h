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
    FPGA_1,
	FPGA_2,
    PCDU_VORAGO,
    PL_VORAGO,
    PL_PIC24
};

class RS485Cookie: public CookieIF {
public:
	RS485Cookie();
	virtual ~RS485Cookie();

		void setDevice(RS485Devices device);
		RS485Devices getDevice() const;
private:
		RS485Devices device = FPGA_1;
};



#endif /* SAM9G20_COMIF_COOKIES_RS485COOKIE_H_ */
