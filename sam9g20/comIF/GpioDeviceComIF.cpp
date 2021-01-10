#include "GpioDeviceComIF.h"

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfwconfig/devices/logicalAddresses.h>

extern "C" {
#include <board.h>
}

GpioDeviceComIF::GpioDeviceComIF(object_id_t objectId_):
		SystemObject(objectId_) {
	// Configure all pins which were designated as GPIO pins by ISIS.
    // The pins are configured to be output pins by default.
	Pin pin[] = {PIN_GPIO00, PIN_GPIO01, PIN_GPIO02, PIN_GPIO03, PIN_GPIO04,
			PIN_GPIO05, PIN_GPIO06, PIN_GPIO07, PIN_GPIO08, PIN_GPIO09,
			PIN_GPIO10, PIN_GPIO11,PIN_GPIO12, PIN_GPIO13, PIN_GPIO14,
			PIN_GPIO15, PIN_GPIO16, PIN_GPIO17, PIN_GPIO18, PIN_GPIO19,
			PIN_GPIO20, PIN_GPIO21, PIN_GPIO22, PIN_GPIO23, PIN_GPIO24,
			PIN_GPIO25, PIN_GPIO26};
	unsigned int pinSize = PIO_LISTSIZE(pin);
	PIO_Configure(pin,pinSize);
}

GpioDeviceComIF::~GpioDeviceComIF() {
}

/* Hardware Abstraction */

void GpioDeviceComIF::setGPIO(uint32_t address) {
	Pin pin[1] = {pinSelect(address)};
	PIO_Set(pin);
}

void GpioDeviceComIF::clearGPIO(uint32_t address) {
	Pin pin[1] = {pinSelect(address)};
	PIO_Clear(pin);
}

bool GpioDeviceComIF::getGPIO(uint32_t address, uint8_t outputPin) {
    Pin pin[1] = {pinSelect(address)};
    if(outputPin) {
        pin[0].type = PIO_OUTPUT_0;
    }
    else {
        pin[0].type = PIO_INPUT;
    }
    return PIO_Get(pin);
}

/* Decoder Select. See truth table of Housekeeping Board. */
void GpioDeviceComIF::enableDecoder1() {
    // Low, Low, High configuration.
    clearGPIO(addresses::DEC_SELECT_0_GPIO00);
    clearGPIO(addresses::DEC_SELECT_1_GPIO01);
    setGPIO(addresses::DEC_SELECT_2_GPIO02);
}
void GpioDeviceComIF::enableDecoder2() {
    // High, High, Low configuration.
    setGPIO(addresses::DEC_SELECT_0_GPIO00);
    setGPIO(addresses::DEC_SELECT_1_GPIO01);
    clearGPIO(addresses::DEC_SELECT_2_GPIO02);
}
void GpioDeviceComIF::enableDecoder3() {
    // High, Low, High configuration.
    setGPIO(addresses::DEC_SELECT_0_GPIO00);
    clearGPIO(addresses::DEC_SELECT_1_GPIO01);
    setGPIO(addresses::DEC_SELECT_2_GPIO02);
}
void GpioDeviceComIF::enableDecoder4() {
    // Dec1 and Dec4 will be on simultaneously in current configuration.
    // shall be fixed soon hopefully.
    clearGPIO(addresses::DEC_SELECT_0_GPIO00);
    clearGPIO(addresses::DEC_SELECT_1_GPIO01);
    setGPIO(addresses::DEC_SELECT_2_GPIO02);
}

void GpioDeviceComIF::disableDecoders() {
    clearGPIO(addresses::DEC_SELECT_0_GPIO00);
    clearGPIO(addresses::DEC_SELECT_1_GPIO01);
    clearGPIO(addresses::DEC_SELECT_2_GPIO02);
}


/* Decoder output select, see SN74LVC138A-EP function table */
void GpioDeviceComIF::enableDecoderOutput1() {
    clearGPIO(addresses::DEC_OUTPUT_SELECT_0_GPIO03);
    clearGPIO(addresses::DEC_OUTPUT_SELECT_1_GPIO04);
    clearGPIO(addresses::DEC_OUTPUT_SELECT_2_GPIO05);
}

void GpioDeviceComIF::enableDecoderOutput2() {
    clearGPIO(addresses::DEC_OUTPUT_SELECT_0_GPIO03);
    clearGPIO(addresses::DEC_OUTPUT_SELECT_1_GPIO04);
    setGPIO(addresses::DEC_OUTPUT_SELECT_2_GPIO05);
}

void GpioDeviceComIF::enableDecoderOutput3() {
    clearGPIO(addresses::DEC_OUTPUT_SELECT_0_GPIO03);
    setGPIO(addresses::DEC_OUTPUT_SELECT_1_GPIO04);
    clearGPIO(addresses::DEC_OUTPUT_SELECT_2_GPIO05);
}

void GpioDeviceComIF::enableDecoderOutput4() {
    clearGPIO(addresses::DEC_OUTPUT_SELECT_0_GPIO03);
    setGPIO(addresses::DEC_OUTPUT_SELECT_1_GPIO04);
    setGPIO(addresses::DEC_OUTPUT_SELECT_2_GPIO05);
}

void GpioDeviceComIF::enableDecoderOutput5() {
    setGPIO(addresses::DEC_OUTPUT_SELECT_0_GPIO03);
    clearGPIO(addresses::DEC_OUTPUT_SELECT_1_GPIO04);
    clearGPIO(addresses::DEC_OUTPUT_SELECT_2_GPIO05);
}

void GpioDeviceComIF::enableDecoderOutput6() {
    setGPIO(addresses::DEC_OUTPUT_SELECT_0_GPIO03);
    clearGPIO(addresses::DEC_OUTPUT_SELECT_1_GPIO04);
    setGPIO(addresses::DEC_OUTPUT_SELECT_2_GPIO05);
}

void GpioDeviceComIF::enableDecoderOutput7() {
    setGPIO(addresses::DEC_OUTPUT_SELECT_0_GPIO03);
    setGPIO(addresses::DEC_OUTPUT_SELECT_1_GPIO04);
    clearGPIO(addresses::DEC_OUTPUT_SELECT_2_GPIO05);
}

void GpioDeviceComIF::enableDecoderOutput8() {
    setGPIO(addresses::DEC_OUTPUT_SELECT_0_GPIO03);
    setGPIO(addresses::DEC_OUTPUT_SELECT_1_GPIO04);
    setGPIO(addresses::DEC_OUTPUT_SELECT_2_GPIO05);
}

/* RS485 Transceiver Select */
void GpioDeviceComIF::enableTransceiverFPGA1() {
    clearGPIO(addresses::GPIO22);
    setGPIO(addresses::GPIO23);
    clearGPIO(addresses::GPIO24);
    clearGPIO(addresses::GPIO25);
    clearGPIO(addresses::GPIO26);
}

void GpioDeviceComIF::enableTransceiverFPGA2() {
    clearGPIO(addresses::GPIO22);
    clearGPIO(addresses::GPIO23);
    clearGPIO(addresses::GPIO24);
    clearGPIO(addresses::GPIO25);
    setGPIO(addresses::GPIO26);
}

void GpioDeviceComIF::enableTransceiverPCDU() {
    setGPIO(addresses::GPIO22);
    clearGPIO(addresses::GPIO23);
    clearGPIO(addresses::GPIO24);
    clearGPIO(addresses::GPIO25);
    clearGPIO(addresses::GPIO26);
}

void GpioDeviceComIF::enableTransceiverVorago() {
    clearGPIO(addresses::GPIO22);
    clearGPIO(addresses::GPIO23);
    setGPIO(addresses::GPIO24);
    clearGPIO(addresses::GPIO25);
    clearGPIO(addresses::GPIO26);
}

void GpioDeviceComIF::enableTransceiverPIC24() {
    clearGPIO(addresses::GPIO22);
    clearGPIO(addresses::GPIO23);
    clearGPIO(addresses::GPIO24);
    setGPIO(addresses::GPIO25);
    clearGPIO(addresses::GPIO26);
}
Pin GpioDeviceComIF::pinSelect(uint32_t address) {
	Pin pin;
	switch(address) {
	case(addresses::DEC_SELECT_0_GPIO00): pin = PIN_GPIO00; break;
	case(addresses::DEC_SELECT_1_GPIO01): pin = PIN_GPIO01; break;
	case(addresses::DEC_SELECT_2_GPIO02): pin = PIN_GPIO02; break;
	case(addresses::DEC_OUTPUT_SELECT_0_GPIO03): pin = PIN_GPIO03; break;
	case(addresses::DEC_OUTPUT_SELECT_1_GPIO04): pin = PIN_GPIO04; break;
	case(addresses::DEC_OUTPUT_SELECT_2_GPIO05): pin = PIN_GPIO05; break;
	case(addresses::GPIO06): pin = PIN_GPIO06; break;
	case(addresses::GPIO07): pin = PIN_GPIO07; break;
	case(addresses::GPIO08): pin = PIN_GPIO08; break;
	case(addresses::GPIO09): pin = PIN_GPIO09; break;
	case(addresses::GPIO10): pin = PIN_GPIO10; break;
	case(addresses::GPIO11): pin = PIN_GPIO11; break;
	case(addresses::GPIO12): pin = PIN_GPIO12; break;
	case(addresses::GPIO13): pin = PIN_GPIO13; break;
	case(addresses::GPIO14): pin = PIN_GPIO14; break;
	case(addresses::GPIO15): pin = PIN_GPIO15; break;
	case(addresses::GPIO16): pin = PIN_GPIO16; break;
	case(addresses::GPIO17): pin = PIN_GPIO17; break;
	case(addresses::GPIO18): pin = PIN_GPIO18; break;
	case(addresses::GPIO19): pin = PIN_GPIO19; break;
	case(addresses::GPIO20): pin = PIN_GPIO20; break;
	case(addresses::GPIO21): pin = PIN_GPIO21; break;
	case(addresses::GPIO22): pin = PIN_GPIO22; break;
	case(addresses::GPIO23): pin = PIN_GPIO23; break;
	case(addresses::GPIO24): pin = PIN_GPIO24; break;
	case(addresses::GPIO25): pin = PIN_GPIO25; break;
	case(addresses::GPIO26): pin = PIN_GPIO26; break;
	default:
		sif::error << "GPIO ComIF: Invalid GPIO Address!" << std::endl;
		return pin;
	}
	return pin;
}

/* Device ComIF Functions */


ReturnValue_t GpioDeviceComIF::initializeInterface(
		CookieIF *cookie) {
	return RETURN_OK;
}

ReturnValue_t GpioDeviceComIF::sendMessage(CookieIF *cookie,
		const uint8_t *sendData, size_t sendLen) {
	return RETURN_OK;
}

ReturnValue_t GpioDeviceComIF::getSendSuccess(
		CookieIF *cookie) {
	return RETURN_OK;
}

ReturnValue_t GpioDeviceComIF::requestReceiveMessage(
		CookieIF *cookie, size_t requestLen) {
	return RETURN_OK;
}

ReturnValue_t GpioDeviceComIF::readReceivedMessage(
		CookieIF *cookie, uint8_t **buffer, size_t *size) {
	return RETURN_OK;
}
