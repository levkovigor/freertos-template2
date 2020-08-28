/**
 * @file RS232DeviceComIF.cpp
 *
 * @date 30.30.2020
 *
 */

#include <sam9g20/comIF/RS232DeviceComIF.h>

#include <fsfw/serialize/SerializeAdapter.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>

RS232DeviceComIF::RS232DeviceComIF(object_id_t object_id_): SystemObject(object_id_),len(0) {
}

RS232DeviceComIF::~RS232DeviceComIF() {
}

UARTconfig RS232DeviceComIF::setRS232Config(UARTconfig RS232Config) {
	RS232Config.mode = AT91C_US_USMODE_NORMAL;
	RS232Config.baudrate = 9600;
	RS232Config.busType = rs232_uart;
	// From SAM9G20 datasheet p.461: The timeguard feature enables the USART interface with slow remote devices. ...
	// When this field is programmed at zero, no timeguard is generated.
	RS232Config.timeGuard = 1;
	// From SAM9G20 datasheet p.462: The Receiver Time-out provides support in handling variable-length frames...
	// If the TO field is programmed at 0, the Receiver Time-out is disabled and no time-out is detected...
	RS232Config.rxtimeout = 0;
	return RS232Config;
}




