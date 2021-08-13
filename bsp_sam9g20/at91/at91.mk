####################################################################################################
# Set up include paths first
####################################################################################################
AT91_INC_PATH = $(CURRENTPATH)/include
AT91_LIB = $(AT91_INC_PATH)/at91

INCLUDES += $(AT91_INC_PATH)
INCLUDES += $(AT91_LIB)
INCLUDES += $(AT91_LIB)/peripherals
INCLUDES += $(AT91_LIB)/boards/common

ifeq ($(BOARD), ISIS_OBC_G20)

INCLUDES += $(AT91_LIB)/boards/ISIS_OBC_G20
INCLUDES += $(AT91_LIB)/boards/ISIS_OBC_G20/at91sam9g20

else # AT91SAM9G20_EK
INCLUDES += $(AT91_LIB)/boards/at91sam9g20-ek
INCLUDES += $(AT91_LIB)/boards/at91sam9g20-ek/at91sam9g20

ifeq ($(ADD_ETHERNET), 1)
INCLUDES += $(AT91_LIB)/boards/at91sam9g20-ek/drivers/emac
INCLUDES += $(AT91_LIB)/boards/at91sam9g20-ek/drivers/macb
INCLUDES += $(AT91_LIB)/boards/at91sam9g20-ek/drivers
INCLUDES += $(AT91_LIB)/boards/at91sam9g20-ek/ethernet
INCLUDES += $(LWIP_PATH)/include
INCLUDES += $(LWIP_PATH)/include/lwip/prot
endif # ($(ADD_ETHERNET), 1)

endif # AT91SAM9G20_EK

ifeq ($(ADD_TINYFATFS), 1)
INCLUDES += $(CURRENTPATH)/tinyfatfs/include
endif

SRC_UTILITY = $(AT91_PATH)/src/utility
SRC_PERIPH = $(AT91_PATH)/src/peripherals
SRC_BOARDS = $(AT91_PATH)/src/boards
SRC_MEMORIES = $(AT91_PATH)/src/memories

####################################################################################################
# Set up paths to be searched for sources
####################################################################################################

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

VPATH += $(SRC_UTILITY)
VPATH += $(SRC_PERIPH)/dbgu
VPATH += $(SRC_PERIPH)/aic
VPATH += $(SRC_PERIPH)/pio
VPATH += $(SRC_PERIPH)/pit
VPATH += $(SRC_PERIPH)/tc
VPATH += $(SRC_PERIPH)/pmc
VPATH += $(SRC_PERIPH)/cp15
VPATH += $(SRC_PERIPH)/twi
VPATH += $(SRC_PERIPH)/usart
VPATH += $(SRC_PERIPH)/rstc
VPATH += $(SRC_PERIPH)/spi
VPATH += $(SRC_PERIPH)/twi
VPATH += $(SRC_MEMORIES)/sdmmc

LOAD_MCI = 0
ifeq ($(BOOTLOADER), 1)
LOAD_MCI = 1
endif
ifeq ($(ADD_MMC_DRIVER), 1)
LOAD_MCI = 1
endif
ifeq ($(LOAD_MCI), 1)
VPATH += $(SRC_PERIPH)/mci
endif

ifeq ($(ADD_TINYFATFS), 1)
VPATH += $(CURRENTPATH)/tinyfatfs/src
endif

ifeq ($(BOARD), ISIS_OBC_G20)

VPATH += $(SRC_BOARDS)/ISIS_OBC_G20
VPATH += $(SRC_BOARDS)/ISIS_OBC_G20/at91sam9g20
VPATH += $(SRC_MEMORIES)/norflash

else # AT91SAM9G20_EK

VPATH += $(SRC_BOARDS)/at91sam9g20-ek
VPATH += $(SRC_BOARDS)/at91sam9g20-ek/at91sam9g20
VPATH += $(SRC_MEMORIES)/nandflash

ifeq ($(ADD_ETHERNET), 1)
VPATH += $(SRC_BOARDS)/at91sam9g20-ek/drivers/emac
VPATH += $(SRC_BOARDS)/at91sam9g20-ek/drivers/macb
VPATH += $(SRC_BOARDS)/at91sam9g20-ek/ethernet
endif

endif # AT91SAM9G20_EK

####################################################################################################
# Add sources
####################################################################################################

####################################################################################################
# Both boards
####################################################################################################
ASRC += cp15_asm.S

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
ifeq ($(LOAD_MCI), 1)
AT91_SRC += mci.c
endif

####################################################################################################
# Board specific
####################################################################################################

ifeq ($(BOARD), ISIS_OBC_G20)

# NOR-Flash sources
AT91_SRC += NorFlashCFI.c
AT91_SRC += NorFlashApi.c
AT91_SRC += NorFlashCommon.c
AT91_SRC += NorFlashAmd.c

else # AT91SAM9G20_EK

AT91_SRC += led.c

# NAND-Flash sources
AT91_SRC += EccNandFlash.c
AT91_SRC += NandFlashModel.c
AT91_SRC += NandFlashModelList.c
AT91_SRC += NandSpareScheme.c
AT91_SRC += RawNandFlash.c
AT91_SRC += SkipBlockNandFlash.c

ifeq ($(ADD_ETHERNET), 1)
AT91_SRC += emac.c
AT91_SRC += macb.c
AT91_SRC += emacif.c
AT91_SRC += lwip_init.c
endif

endif # AT91SAM9G20_EK


####################################################################################################
# FreeRTOS dependant
####################################################################################################

ifeq ($(OS_APP), freeRTOS)

AT91_SRC += demo_sd.c
AT91_SRC += syscalls.c
ifeq ($(MEMORY_TYPE), norflash)
ASRC += board_cstartup_freeRTOS_norflash.S
else
ASRC += board_cstartup_freeRTOS.S
endif

else

ASRC += board_cstartup.S

endif # ($(OS_APP),freeRTOS)

####################################################################################################
# Optional
####################################################################################################

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

ifeq ($(ADD_TINYFATFS), 1)
AT91_SRC += $(CURRENTPATH)/tinyfatfs/src/diskio.c
AT91_SRC += $(CURRENTPATH)/tinyfatfs/src/tff.c
endif

