CSRC += $(CURRENTPATH)/main.c

# AT91 sources
ifeq ($(BOARD), AT91SAM9G20_EK)

CSRC += $(wildcard $(CURRENTPATH)/at91/common/*.c)

ifeq ($(BL_STAGE), 1)
CSRC += $(wildcard $(CURRENTPATH)/at91/first_stage/*.c)
else ifeq ($(BL_STAGE), 2)
CSRC += $(wildcard $(CURRENTPATH)/at91/second_stage/*.c)
endif

else 
# iOBC sources
CSRC += $(wildcard $(CURRENTPATH)/iobc/*.c)
CSRC += $(wildcard $(CURRENTPATH)/iobc/common/*.c)
CSRC += $(wildcard $(CURRENTPATH)/tinyfatfs/src/*.c)
CSRC += $(wildcard $(CURRENTPATH)/tinyfatfs/memories/*.c)
CSRC += $(wildcard $(CURRENTPATH)/tinyfatfs/memories/sdmmc/*.c)

ifeq ($(MEMORY_TYPE), norflash)
CSRC += $(wildcard $(CURRENTPATH)/iobc/norflash/*.c)
else
CSRC += $(wildcard $(CURRENTPATH)/iobc/sram/*.c)
endif

endif # ($(BOARD), AT91SAM9G20_EK)

CSRC += $(wildcard $(CURRENTPATH)/utility/CRC.c)
CSRC += $(wildcard $(CURRENTPATH)/utility/faultHandler.c)

ifndef IOBC
ifeq ($(BL_STAGE), 2)
CSRC += $(wildcard $(CURRENTPATH)/utility/hooks.c)
endif
endif

INCLUDES += $(CURRENTPATH)
INCLUDES += $(CURRENTPATH)/tinyfatfs
INCLUDES += $(CURRENTPATH)/tinyfatfs/include
INCLUDES += $(CURRENTPATH)/utility
INCLUDES += $(CURRENTPATH)/config