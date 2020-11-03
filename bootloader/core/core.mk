CSRC += $(wildcard $(CURRENTPATH)/*.c)

ifeq ($(BOARD), AT91SAM9G20_EK)
CSRC += $(wildcard $(CURRENTPATH)/at91/*.c)
else
CSRC += $(wildcard $(CURRENTPATH)/iobc/*.c)
endif