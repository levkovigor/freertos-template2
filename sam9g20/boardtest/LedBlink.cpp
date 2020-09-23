#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <sam9g20/boardtest/LedBlink.h>

extern "C" {
#include <sam9g20/utility/print.h>
#if defined(AT91SAM9G20_EK)
#include <led_ek.h>
#endif
}

LED_Blink::LED_Blink(const char * printName, object_id_t objectId):
		SystemObject(objectId), printName(printName) {
	print_uart("LED Blink object\n");
}

LED_Blink::~LED_Blink() {
}

ReturnValue_t LED_Blink::performOperation(uint8_t operationCode) {
	#if defined(AT91SAM9G20_EK)
		LED_Toggle(1);
	#endif

	return HasReturnvaluesIF::RETURN_OK;
}

