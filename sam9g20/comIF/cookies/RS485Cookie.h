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
    COM_FPGA,	// Redundant FPGA not counted here, set in device handler
    PCDU_VORAGO,
    PL_VORAGO,
    PL_PIC24,
	DEVICE_COUNT_RS485
};

enum class ComStatusRS485: uint8_t {
	IDLE,
	TRANSFER_INIT_SUCCESS,
	TRANSFER_SUCCESS,
	FAULTY //!< Errors in communication
};

class RS485Cookie: public CookieIF {
public:
	RS485Cookie(RS485Devices device);
	virtual ~RS485Cookie();

		void setDevice(RS485Devices device);
		RS485Devices getDevice() const;

		// Dont know why size_t does not work for me here
		unsigned int getSendLen();
		void setSendLen(unsigned int sendLen);

		unsigned char* getWriteData();
		void setWriteData(unsigned char* writeData);

		ComStatusRS485 getComStatus();
		void setComStatus(ComStatusRS485 status);

		int8_t getReturnValue();
		void setReturnValue(int8_t retval);
private:
		// Device that is communicated with
		RS485Devices device = COM_FPGA;
		// Cookie is used to transfer write information between different functions of RS485DeviceComIF
		unsigned char * writeData;
		unsigned int 	sendLen;
		// Stores returnvalues from UART driver, can also be negative
		int8_t returnValue = 0;
		// Stores communication status
		ComStatusRS485 comStatus = ComStatusRS485::IDLE;

};



#endif /* SAM9G20_COMIF_COOKIES_RS485COOKIE_H_ */
