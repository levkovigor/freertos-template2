#################
# AT91 Includes #
#################
AT91LIB = $(CURRENTPATH)/include/at91

INCLUDES += $(CURRENTPATH)/include
INCLUDES += $(AT91LIB)
INCLUDES += $(AT91LIB)/peripherals
INCLUDES += $(AT91LIB)/peripherals
INCLUDES += $(AT91LIB)/peripherals/twi
INCLUDES += $(AT91LIB)/peripherals/aic
INCLUDES += $(AT91LIB)/peripherals/spi
INCLUDES += $(AT91LIB)/peripherals/twi
INCLUDES += $(AT91LIB)/peripherals/cp15
INCLUDES += $(AT91LIB)/peripherals/pio
INCLUDES += $(AT91LIB)/peripherals/tc
INCLUDES += $(AT91LIB)/peripherals/pit
INCLUDES += $(AT91LIB)/peripherals/pmc
INCLUDES += $(AT91LIB)/peripherals/rstc
INCLUDES += $(AT91LIB)/usb/device/core
INCLUDES += $(AT91LIB)/usb/device/hid-mouse
INCLUDES += $(AT91LIB)/usb/common/core
INCLUDES += $(AT91LIB)/usb/common/hid

ifeq ($(BOARD), ISIS_OBC_G20)
INCLUDES += $(AT91LIB)/boards/ISIS_OBC_G20
INCLUDES += $(AT91LIB)/boards/ISIS_OBC_G20/at91sam9g20
else
INCLUDES += $(AT91LIB)/boards/at91sam9g20-ek
INCLUDES += $(AT91LIB)/boards/at91sam9g20-ek/at91sam9g20
ifdef ETHERNET
INCLUDES += $(AT91LIB)/boards/at91sam9g20-ek/drivers/emac
INCLUDES += $(AT91LIB)/boards/at91sam9g20-ek/drivers/macb
INCLUDES += $(AT91LIB)/boards/at91sam9g20-ek/drivers
INCLUDES += $(AT91LIB)/boards/at91sam9g20-ek/ethernet
INCLUDES += $(LWIP_PATH)/include
INCLUDES += $(LWIP_PATH)/include/lwip/prot
endif
endif

UTILITY = $(AT91_PATH)/src/utility
PERIPH = $(AT91_PATH)/src/peripherals
BOARDS = $(AT91_PATH)/src/boards
AT91_MEMORIES = $(AT91_PATH)/src/memories

# if a target is not listed in the current directory, 
# make searches in the directories specified with VPATH
VPATH += $(AT91_PATH)/startup
VPATH += $(AT91_PATH)/src
VPATH += $(AT91_PATH)/src/utility
# needed for USB device test
VPATH += $(AT91_PATH)/src/usb/common/core
VPATH += $(AT91_PATH)/src/usb/common/hid
VPATH += $(AT91_PATH)/src/usb/device/core
VPATH += $(AT91_PATH)/src/usb/device/hid-mouse
VPATH += $(DEMO_PATH)/src/Tests
# end needed for USB device test

VPATH += $(UTILITY)
VPATH += $(PERIPH)/dbgu
VPATH += $(PERIPH)/aic
VPATH += $(PERIPH)/pio
VPATH += $(PERIPH)/pit
VPATH += $(PERIPH)/tc
VPATH += $(PERIPH)/pmc
VPATH += $(PERIPH)/cp15
VPATH += $(PERIPH)/twi
VPATH += $(PERIPH)/usart
VPATH += $(PERIPH)/rstc
VPATH += $(PERIPH)/spi
VPATH += $(PERIPH)/twi


ifeq ($(BOARD), ISIS_OBC_G20)
VPATH += $(BOARDS)/ISIS_OBC_G20
VPATH += $(BOARDS)/ISIS_OBC_G20/at91sam9g20
VPATH += $(AT91_MEMORIES)/norflash
else
VPATH += $(BOARDS)/at91sam9g20-ek
VPATH += $(BOARDS)/at91sam9g20-ek/at91sam9g20
VPATH += $(AT91_MEMORIES)/nandflash
ifdef ETHERNET
VPATH += $(BOARDS)/at91sam9g20-ek/drivers/emac
VPATH += $(BOARDS)/at91sam9g20-ek/drivers/macb
VPATH += $(BOARDS)/at91sam9g20-ek/ethernet
endif
endif

ASRC += cp15_asm.S

# Objects built from C source files
ifeq ($(BOARD), AT91SAM9G20_EK)
CSRC += led.c
ifdef ETHERNET
CSRC += emac.c
CSRC += macb.c
CSRC += emacif.c
CSRC += lwip_init.c
endif
endif

ifeq ($(BOARD), ISIS_OBC_G20)
CSRC += NorFlashCFI.c
CSRC += NorFlashApi.c
CSRC += NorFlashCommon.c
CSRC += NorFlashAmd.c
else
CSRC += EccNandFlash.c
CSRC += NandFlashModel.c
CSRC += NandFlashModelList.c
CSRC += NandSpareScheme.c
CSRC += RawNandFlash.c
CSRC += SkipBlockNandFlash.c
endif
ifeq ($(USE_AT91LIB_STDIO_AND_STRING), 1)
CSRC += stdio.c
endif
CSRC += dbgu.c
CSRC += pio.c
CSRC += pio_it.c
CSRC += tc.c
CSRC += twi_at91.c
CSRC += pmc.c
CSRC += hamming.c
CSRC += board_lowlevel.c
CSRC += trace.c
CSRC += at91_math.c
CSRC += board_memories.c
CSRC += aic_iobc.c
CSRC += cp15.c
CSRC += pit.c
CSRC += ExitHandler.c
CSRC += spi_at91.c
CSRC += rstc.c
CSRC += usart_at91.c
CSRC += SDCardTest.c
ifeq ($(OS_APP),freeRTOS)
CSRC += demo_sd.c
CSRC += syscalls.c
ASRC += board_cstartup_freeRTOS.S
else
ASRC += board_cstartup.S
endif

# USB drivers
CSRC += USBD_UDP.c
CSRC += USBDDriver.c
CSRC += USBGenericDescriptor.c
CSRC += USBGenericRequest.c
CSRC += USBConfigurationDescriptor.c
CSRC += USBGetDescriptorRequest.c
CSRC += USBInterfaceRequest.c
CSRC += USBSetAddressRequest.c
CSRC += USBSetConfigurationRequest.c
CSRC += USBEndpointDescriptor.c
CSRC += USBFeatureRequest.c
CSRC += USBDCallbacks_Initialized.c
CSRC += USBDCallbacks_Reset.c
CSRC += USBDDriverCb_IfSettingChanged.c


#CSRC += USBdeviceTest.o
#CSRC += HIDDMouseDriver.o
#CSRC += HIDDMouseDriverDescriptors.o
#CSRC += HIDDMouseInputReport.o
#CSRC += HIDIdleRequest.o
#CSRC += HIDReportRequest.o
# needed for USB device test

#include <at91/usb/common/core/USBConfigurationDescriptor.h>
#include <at91/usb/common/core/USBDeviceDescriptor.h>
#include <at91/usb/common/core/USBDeviceQualifierDescriptor.h>
#include <at91/usb/common/core/USBEndpointDescriptor.h>
#include <at91/usb/common/core/USBFeatureRequest.h>
#include <at91/usb/common/core/USBGenericDescriptor.h>
#include <at91/usb/common/core/USBGetDescriptorRequest.h>
#include <at91/usb/common/core/USBInterfaceRequest.h>
#include <at91/usb/common/core/USBSetAddressRequest.h>
#include <at91/usb/common/core/USBSetConfigurationRequest.h>
#include <at91/usb/device/core/USBD.h>
#include <at91/usb/device/core/USBDDriver.h>
#include <at91/usb/device/core/USBDDriverCallbacks.h>