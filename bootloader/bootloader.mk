CSRC += $(CURRENTPATH)/main.c

# AT91 sources
ifeq ($(BOARD), AT91SAM9G20_EK)
CSRC += $(wildcard $(CURRENTPATH)/at91/*.c)
else
# iOBC sources
CSRC += $(wildcard $(CURRENTPATH)/iobc/*.c)
CSRC += $(wildcard $(CURRENTPATH)/iobc/common/*.c)
CSRC += $(wildcard $(CURRENTPATH)/fat/fatfs/*.c)
CSRC += $(wildcard $(CURRENTPATH)/fat/memories/*.c)
CSRC += $(wildcard $(CURRENTPATH)/fat/memories/sdmmc/*.c)

ifeq ($(MEMORY_TYPE), norflash)
CSRC += $(wildcard $(CURRENTPATH)/iobc/norflash/*.c)
else
CSRC += $(wildcard $(CURRENTPATH)/iobc/sram/*.c)
endif

endif

CSRC += $(wildcard $(CURRENTPATH)/utility/*.c)

INCLUDES += $(CURRENTPATH)
INCLUDES += $(CURRENTPATH)/utility
INCLUDES += $(CURRENTPATH)/config