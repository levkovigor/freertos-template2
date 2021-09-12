#include "GpioDeviceComIF.h"
#include <OBSWConfig.h>
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <devices/logicalAddresses.h>

extern "C" {
#include <board.h>
}

std::vector<Pin> GpioDeviceComIF::pins = {
        PIN_GPIO00, PIN_GPIO01, PIN_GPIO02, PIN_GPIO03, PIN_GPIO04, PIN_GPIO05, PIN_GPIO06,
        PIN_GPIO07, PIN_GPIO08, PIN_GPIO09, PIN_GPIO10, PIN_GPIO11,PIN_GPIO12, PIN_GPIO13,
        PIN_GPIO14, PIN_GPIO15, PIN_GPIO16, PIN_GPIO17, PIN_GPIO18, PIN_GPIO19, PIN_GPIO20,
        PIN_GPIO21, PIN_GPIO22, PIN_GPIO23, PIN_GPIO24, PIN_GPIO25, PIN_GPIO26
};

Pin GpioDeviceComIF::invalidPin = {0, nullptr, 0, 0, 0};

GpioDeviceComIF::GpioDeviceComIF(object_id_t objectId_):
		SystemObject(objectId_) {
    // I think configuring these GPIO pins disables the ethernet capabilities. This
    // might be due to a multiplexed pin because the iOBC does not use have an ethernet port
#if OBSW_ENABLE_ETHERNET == 0
	// Configure all pins which were designated as GPIO pins by ISIS
    // The pins are configured to be output pins by default
	PIO_Configure(pins.data(), pins.size());
#endif /* OBSW_ENABLE_ETHERNET == 0 */
}

GpioDeviceComIF::~GpioDeviceComIF() {
}

/* Hardware Abstraction */

void GpioDeviceComIF::setGpio(addresses::LogAddr address) {
	auto pin = pinSelect(address);
	PIO_Set(&pin);
}

void GpioDeviceComIF::clearGpio(addresses::LogAddr address) {
	auto pin = pinSelect(address);
	PIO_Clear(&pin);
}

bool GpioDeviceComIF::getGpio(addresses::LogAddr address) {
    auto pin = pinSelect(address);
    return PIO_Get(&pin);
}

bool GpioDeviceComIF::configurePin(addresses::LogAddr address, PinType pinType, uint8_t pinCfg) {
    auto pin = pinSelect(address);
    pin.type = pinType;
    pin.attribute = pinCfg;
    return PIO_Configure(&pin, 1);
}

/* Decoder Select. See truth table of Housekeeping Board. */
void GpioDeviceComIF::enableDecoder1() {
    // Low, Low, High configuration.
    clearGpio(addresses::DEC_SELECT_0_GPIO00);
    clearGpio(addresses::DEC_SELECT_1_GPIO01);
    setGpio(addresses::DEC_SELECT_2_GPIO02);
}
void GpioDeviceComIF::enableDecoder2() {
    // High, High, Low configuration.
    setGpio(addresses::DEC_SELECT_0_GPIO00);
    setGpio(addresses::DEC_SELECT_1_GPIO01);
    clearGpio(addresses::DEC_SELECT_2_GPIO02);
}
void GpioDeviceComIF::enableDecoder3() {
    // High, Low, High configuration.
    setGpio(addresses::DEC_SELECT_0_GPIO00);
    clearGpio(addresses::DEC_SELECT_1_GPIO01);
    setGpio(addresses::DEC_SELECT_2_GPIO02);
}
void GpioDeviceComIF::enableDecoder4() {
    // Dec1 and Dec4 will be on simultaneously in current configuration.
    // shall be fixed soon hopefully.
    clearGpio(addresses::DEC_SELECT_0_GPIO00);
    clearGpio(addresses::DEC_SELECT_1_GPIO01);
    setGpio(addresses::DEC_SELECT_2_GPIO02);
}

void GpioDeviceComIF::disableDecoders() {
    clearGpio(addresses::DEC_SELECT_0_GPIO00);
    clearGpio(addresses::DEC_SELECT_1_GPIO01);
    clearGpio(addresses::DEC_SELECT_2_GPIO02);
}


/* Decoder output select, see SN74LVC138A-EP function table */
void GpioDeviceComIF::enableDecoderOutput1() {
    clearGpio(addresses::DEC_OUTPUT_SELECT_0_GPIO03);
    clearGpio(addresses::DEC_OUTPUT_SELECT_1_GPIO04);
    clearGpio(addresses::DEC_OUTPUT_SELECT_2_GPIO05);
}

void GpioDeviceComIF::enableDecoderOutput2() {
    clearGpio(addresses::DEC_OUTPUT_SELECT_0_GPIO03);
    clearGpio(addresses::DEC_OUTPUT_SELECT_1_GPIO04);
    setGpio(addresses::DEC_OUTPUT_SELECT_2_GPIO05);
}

void GpioDeviceComIF::enableDecoderOutput3() {
    clearGpio(addresses::DEC_OUTPUT_SELECT_0_GPIO03);
    setGpio(addresses::DEC_OUTPUT_SELECT_1_GPIO04);
    clearGpio(addresses::DEC_OUTPUT_SELECT_2_GPIO05);
}

void GpioDeviceComIF::enableDecoderOutput4() {
    clearGpio(addresses::DEC_OUTPUT_SELECT_0_GPIO03);
    setGpio(addresses::DEC_OUTPUT_SELECT_1_GPIO04);
    setGpio(addresses::DEC_OUTPUT_SELECT_2_GPIO05);
}

void GpioDeviceComIF::enableDecoderOutput5() {
    setGpio(addresses::DEC_OUTPUT_SELECT_0_GPIO03);
    clearGpio(addresses::DEC_OUTPUT_SELECT_1_GPIO04);
    clearGpio(addresses::DEC_OUTPUT_SELECT_2_GPIO05);
}

void GpioDeviceComIF::enableDecoderOutput6() {
    setGpio(addresses::DEC_OUTPUT_SELECT_0_GPIO03);
    clearGpio(addresses::DEC_OUTPUT_SELECT_1_GPIO04);
    setGpio(addresses::DEC_OUTPUT_SELECT_2_GPIO05);
}

void GpioDeviceComIF::enableDecoderOutput7() {
    setGpio(addresses::DEC_OUTPUT_SELECT_0_GPIO03);
    setGpio(addresses::DEC_OUTPUT_SELECT_1_GPIO04);
    clearGpio(addresses::DEC_OUTPUT_SELECT_2_GPIO05);
}

void GpioDeviceComIF::enableDecoderOutput8() {
    setGpio(addresses::DEC_OUTPUT_SELECT_0_GPIO03);
    setGpio(addresses::DEC_OUTPUT_SELECT_1_GPIO04);
    setGpio(addresses::DEC_OUTPUT_SELECT_2_GPIO05);
}

Pin& GpioDeviceComIF::pinSelect(addresses::LogAddr address) {
	switch(address) {
	case(addresses::DEC_SELECT_0_GPIO00): return pins[0];
	case(addresses::DEC_SELECT_1_GPIO01): return pins[1];
	case(addresses::DEC_SELECT_2_GPIO02): return pins[2];
	case(addresses::DEC_OUTPUT_SELECT_0_GPIO03): return pins[3];
	case(addresses::DEC_OUTPUT_SELECT_1_GPIO04): return pins[4];
	case(addresses::DEC_OUTPUT_SELECT_2_GPIO05): return pins[5];
	case(addresses::GPIO06): return pins[6];
	case(addresses::GPIO07): return pins[7];
	case(addresses::GPIO08): return pins[8];
	case(addresses::GPIO09): return pins[9];
	case(addresses::GPIO10): return pins[10];
	case(addresses::GPIO11): return pins[11];
	case(addresses::GPIO12): return pins[12];
	case(addresses::GPIO13): return pins[13];
	case(addresses::GPIO14): return pins[14];
	case(addresses::GPIO15): return pins[15];
	case(addresses::GPIO16): return pins[16];
	case(addresses::GPIO17): return pins[17];
	case(addresses::GPIO18): return pins[18];
	case(addresses::GPIO19): return pins[19];
	case(addresses::GPIO20): return pins[20];
	case(addresses::GPIO21): return pins[21];
	case(addresses::GPIO22): return pins[22];
	case(addresses::GPIO23): return pins[23];
	case(addresses::GPIO24): return pins[24];
	case(addresses::GPIO25): return pins[25];
	case(addresses::GPIO26): return pins[26];
	default:
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "GpioDeviceComIF: Invalid GPIO Address!" << std::endl;
#else
		sif::printError("GpioDeviceComIF: Invalid GPIO Address!\n");
#endif
		return invalidPin;
	}
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
