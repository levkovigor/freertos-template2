CSRC += $(CURRENTPATH)/main.c

ifeq ($(BOARD), AT91SAM9G20_EK)
CSRC += $(wildcard $(CURRENTPATH)/at91/*.c)
else
CSRC += $(wildcard $(CURRENTPATH)/iobc/*.c)
endif

CSRC += $(wildcard $(CURRENTPATH)/utility/*.c)

INCLUDES += $(CURRENTPATH)
INCLUDES += $(CURRENTPATH)/utility
INCLUDES += $(CURRENTPATH)/config