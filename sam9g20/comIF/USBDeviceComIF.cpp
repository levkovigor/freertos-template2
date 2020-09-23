#include "USBDeviceComIF.h"

#include <fsfw/ipc/MutexHelper.h>
#include <fsfw/tasks/TaskFactory.h>
#include <fsfw/timemanager/Countdown.h>

extern "C" {
#include <board.h>
#include <utility/trace.h>
#include <peripherals/pio/pio_it.h>
#include <usb/device/cdc/CDCDSerialDriver.h>
}

size_t USBDeviceComIF::sentDataLen = 0;
uint8_t USBDeviceComIF::errorStatus = 0;

//------------------------------------------------------------------------------
//         VBus monitoring (optional)
//------------------------------------------------------------------------------
#if defined(PIN_USB_VBUS)

#define VBUS_CONFIGURE()  VBus_Configure()
static void ISR_Vbus(const Pin *pPin);
static void VBus_Configure(void);

#else
    #define VBUS_CONFIGURE()    USBD_Connect()
#endif //#if defined(PIN_USB_VBUS)


USBDeviceComIF::USBDeviceComIF(object_id_t objectId,
		SharedRingBuffer* usbRingBuffer): SystemObject(objectId),
		usbRingBuffer(usbRingBuffer) {
	// If they are present, configure Vbus & Wake-up pins
	// we could assign a higher priority here maybe..
	PIO_InitializeInterrupts(AT91C_AIC_PRIOR_LOWEST);

	// BOT driver initialization
	CDCDSerialDriver_Initialize();

	// connect if needed
	VBUS_CONFIGURE();

}



ReturnValue_t USBDeviceComIF::initializeInterface(CookieIF *cookie) {
	// Device is not configured
	if (USBD_GetState() < USBD_STATE_CONFIGURED) {
		// Connect pull-up, wait for configuration
		USBD_Connect();
		// Wait for max 5 ms for now.
		Countdown countdown(5);
		while (USBD_GetState() < USBD_STATE_CONFIGURED) {
			if(countdown.hasTimedOut()) {
				sif::error << "USBDeviceComIF::sendMessage: USB configuration"
						"failed!" << std::endl;
				return HasReturnvaluesIF::RETURN_FAILED;
			}
			TaskFactory::delayTask(1);
		}
	}

	if(usbRingBuffer == nullptr) {
		sif::error << "USBPolling::initialize: Ring buffer is nullptr!"
				<< std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t USBDeviceComIF::sendMessage(CookieIF *cookie,
		const uint8_t *sendData, size_t sendLen) {
	if((writeState == TransferStates::WRITE_BUSY or
			writeState == TransferStates::WRITE_FAILURE) and not
			errorOccuredOnce) {
		// allow one failure.
		writeState = TransferStates::WRITE_SUCCESS;
		errorOccuredOnce = true;
	}
	else if(errorOccuredOnce) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}


	// I am not sure this should happen at this point..
	if (USBD_GetState() < USBD_STATE_CONFIGURED) {
		sif::info << "USBDeviceComIF::sendMessage: USB device not configured!"
				<< std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	// will be set by callback (maybe not needed?)
	sentDataLen = 0;
	writeState = TransferStates::WRITE_BUSY;
	uint8_t writeResult = CDCDSerialDriver_Write(const_cast<uint8_t*>(sendData),
			sendLen, reinterpret_cast<TransferCallback>(UsbDataSent),
			&writeState);
	if(writeResult == USBD_STATUS_LOCKED) {
		// configuration error.
	}

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t USBDeviceComIF::getSendSuccess(CookieIF *cookie) {
	if(writeState == TransferStates::WRITE_SUCCESS or
			writeState == TransferStates::WRITE_BUSY) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
		return HasReturnvaluesIF::RETURN_FAILED;
	}

}

ReturnValue_t USBDeviceComIF::requestReceiveMessage(CookieIF *cookie,
		size_t requestLen) {
	MutexHelper(usbRingBuffer->getMutexHandle(),
			MutexIF::TimeoutType::WAITING, 5);
	auto fifoHandle = usbRingBuffer->getReceiveSizesFIFO();
	if(fifoHandle->empty()) {
		return HasReturnvaluesIF::RETURN_OK;
	}

	if(not fifoHandle->empty()) {
		 fifoHandle->retrieve(&sizeRead);
	}

	usbRingBuffer->readData(usbBuffer.data(), sizeRead);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t USBDeviceComIF::readReceivedMessage(CookieIF *cookie,
		uint8_t **buffer, size_t *size) {
	if(sizeRead > 0) {
		*buffer = usbBuffer.data();
		*size = sizeRead;
		sizeRead = 0;
	}

	return HasReturnvaluesIF::RETURN_OK;
}


#if defined(PIN_USB_VBUS)

#define VBUS_CONFIGURE()  VBus_Configure()

/// VBus pin instance.
static const Pin pinVbus = PIN_USB_VBUS;

//------------------------------------------------------------------------------
/// Handles interrupts coming from PIO controllers.
//------------------------------------------------------------------------------
void ISR_Vbus(const Pin *pPin)
{
    // Check current level on VBus
    if (PIO_Get(&pinVbus)) {

        TRACE_INFO("VBUS conn\n\r");
        USBD_Connect();
    }
    else {

        TRACE_INFO("VBUS discon\n\r");
        USBD_Disconnect();
    }
}

//------------------------------------------------------------------------------
/// Configures the VBus pin to trigger an interrupt when the level on that pin
/// changes.
//------------------------------------------------------------------------------
void VBus_Configure( void )
{
    TRACE_INFO("VBus configuration\n\r");

    // Configure PIO
    PIO_Configure(&pinVbus, 1);
    PIO_ConfigureIt(&pinVbus, ISR_Vbus);
    PIO_EnableIt(&pinVbus);

    // Check current level on VBus
    if (PIO_Get(&pinVbus)) {

        // if VBUS present, force the connect
        TRACE_INFO("VBUS conn\n\r");
        USBD_Connect();
    }
    else {
        USBD_Disconnect();
    }
}

#else
    #define VBUS_CONFIGURE()    USBD_Connect()
#endif //#if defined(PIN_USB_VBUS)




void USBDeviceComIF::UsbDataSent(void *writeState, unsigned char status,
		unsigned int transferred, unsigned int remaining) {
	TransferStates* transferState = static_cast<TransferStates*>(writeState);
	if (status == USBD_STATUS_SUCCESS) {

		*transferState = TransferStates::WRITE_SUCCESS;
		TRACE_DEBUG("USBDeviceComIF::UsbDataSent: Data Sent");
	}
	else {
		// Should not happen, callback will occur when transfer is completed.
		TRACE_DEBUG("USBDeviceComIF::UsbDataSent: Data sent callback has"
				" status %d\r\n", status);
		errorStatus = status;
		*transferState = TransferStates::WRITE_FAILURE;
	}
	sentDataLen = transferred;
}

// remove this if it is not needed anymore, this was test code to
// test connection between PLOC and AT91

//------------------------------------------------------------------------------
/// Initializes drivers and start the USB <-> Serial bridge.
//------------------------------------------------------------------------------
//int serial_test()
//{
//
//    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
//    printf("-- USB Device CDC Serial Project %s --\n\r", SOFTPACK_VERSION);
//    printf("-- %s\n\r", BOARD_NAME);
//    printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
//
//
//    // If they are present, configure Vbus & Wake-up pins
//    PIO_InitializeInterrupts(0);
//
//
//    // Configure timer 0
//    AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_TC0);
//    AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS;
//    AT91C_BASE_TC0->TC_IDR = 0xFFFFFFFF;
//    AT91C_BASE_TC0->TC_CMR = AT91C_TC_CLKS_TIMER_DIV5_CLOCK
//                             | AT91C_TC_CPCSTOP
//                             | AT91C_TC_CPCDIS
//                             | AT91C_TC_WAVESEL_UP_AUTO
//                             | AT91C_TC_WAVE;
//    AT91C_BASE_TC0->TC_RC = 0x00FF;
//    AT91C_BASE_TC0->TC_IER = AT91C_TC_CPCS;
//    AIC_ConfigureIT(AT91C_ID_TC0, 0, ISR_Timer0);
//    AIC_EnableIT(AT91C_ID_TC0);
//
//    // BOT driver initialization
//    CDCDSerialDriver_Initialize();
//
//    // connect if needed
//    VBUS_CONFIGURE();
//
//    // Driver loop
//    while (1) {
//
//        // Device is not configured
//        if (USBD_GetState() < USBD_STATE_CONFIGURED) {
//
//            // Connect pull-up, wait for configuration
//            USBD_Connect();
//            while (USBD_GetState() < USBD_STATE_CONFIGURED);
//
//
//        }
//            else{
//            // Start receiving data on the USB
//            CDCDSerialDriver_Read(usbBuffer,
//                                  DATABUFFERSIZE,
//                                  (TransferCallback) UsbDataReceived,
//                                  0);
//
//            }
//
//
//
//        if( USBState == STATE_SUSPEND ) {
//            TRACE_DEBUG("suspend  !\n\r");
//            LowPowerMode();
//            USBState = STATE_IDLE;
//        }
//        if( USBState == STATE_RESUME ) {
//            // Return in normal MODE
//            TRACE_DEBUG("resume !\n\r");
//            NormalPowerMode();
//            USBState = STATE_IDLE;
//        }
//    }
//}
