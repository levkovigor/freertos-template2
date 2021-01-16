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

INCLUDES += $(AT91LIB)/boards/common

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
VPATH += $(AT91_PATH)/src
VPATH += $(AT91_PATH)/src/startup
VPATH += $(AT91_PATH)/src/utility
# needed for USB device test
VPATH += $(AT91_PATH)/src/usb/common/core
VPATH += $(AT91_PATH)/src/usb/common/hid
VPATH += $(AT91_PATH)/src/usb/common/cdc
VPATH += $(AT91_PATH)/src/usb/device/core
VPATH += $(AT91_PATH)/src/usb/device/cdc
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
VPATH += $(AT91_MEMORIES)/sdmmc

ifeq ($(BOOTLOADER), 1)
VPATH += $(PERIPH)/mci
endif

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
AT91_SRC += led.c
ifdef ETHERNET
AT91_SRC += emac.c
AT91_SRC += macb.c
AT91_SRC += emacif.c
AT91_SRC += lwip_init.c
endif
endif

ifeq ($(BOARD), ISIS_OBC_G20)
AT91_SRC += NorFlashCFI.c
AT91_SRC += NorFlashApi.c
AT91_SRC += NorFlashCommon.c
AT91_SRC += NorFlashAmd.c
else
AT91_SRC += EccNandFlash.c
AT91_SRC += NandFlashModel.c
AT91_SRC += NandFlashModelList.c
AT91_SRC += NandSpareScheme.c
AT91_SRC += RawNandFlash.c
AT91_SRC += SkipBlockNandFlash.c
endif

ifeq ($(USE_AT91LIB_STDIO_AND_STRING), 1)
AT91_SRC += stdio.c
endif
AT91_SRC += dbgu.c
AT91_SRC += pio.c
AT91_SRC += pio_it.c
AT91_SRC += tc.c
AT91_SRC += twi_at91.c
AT91_SRC += pmc.c
AT91_SRC += hamming.c
AT91_SRC += board_lowlevel.c
AT91_SRC += trace.c
AT91_SRC += at91_math.c
AT91_SRC += board_memories.c
AT91_SRC += aic_iobc.c
AT91_SRC += cp15.c
AT91_SRC += pit.c
AT91_SRC += ExitHandler.c
AT91_SRC += spi_at91.c
AT91_SRC += rstc.c
AT91_SRC += usart_at91.c
AT91_SRC += SDCardTest.c
ifeq ($(BOOTLOADER), 1)
AT91_SRC += mci.c
endif

ifeq ($(OS_APP),freeRTOS)

AT91_SRC += demo_sd.c
AT91_SRC += syscalls.c
ifeq ($(MEMORY_TYPE), norflash)
ASRC += board_cstartup_freeRTOS_norflash.S
else
ASRC += board_cstartup_freeRTOS.S
endif

else

ASRC += board_cstartup.S

endif

ifeq ($(ADD_USB_DRIVER), 1)
# USB drivers
AT91_SRC += USBD_UDP.c
AT91_SRC += USBDDriver.c
AT91_SRC += USBGenericDescriptor.c
AT91_SRC += USBGenericRequest.c
AT91_SRC += USBConfigurationDescriptor.c
AT91_SRC += USBGetDescriptorRequest.c
AT91_SRC += USBInterfaceRequest.c
AT91_SRC += USBSetAddressRequest.c
AT91_SRC += USBSetConfigurationRequest.c
AT91_SRC += USBEndpointDescriptor.c
AT91_SRC += USBFeatureRequest.c
AT91_SRC += USBDCallbacks_Initialized.c
AT91_SRC += USBDCallbacks_Reset.c
AT91_SRC += USBDDriverCb_IfSettingChanged.c

# CDC device drivers
AT91_SRC += CDCDSerialDriver.c
AT91_SRC += CDCDSerialDriverDescriptors.c
AT91_SRC += CDCLineCoding.c
AT91_SRC += CDCSetControlLineStateRequest.c
endif

ifeq ($(ADD_MMC_DRIVER), 1)
AT91_SRC += Media.c
AT91_SRC += MEDSdcard.c
AT91_SRC += sdmmc_mci.c
endif

