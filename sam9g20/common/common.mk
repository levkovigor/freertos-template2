ifeq ($(BOARD), ISIS_OBC_G20)
# CSRC += $(wildcard $(CURRENTPATH)/FRAMApi.c)
CSRC += $(wildcard $(CURRENTPATH)/watchdog.c)
else
CSRC += $(wildcard $(CURRENTPATH)/VirtualFRAMApi.c)
endif

CSRC += $(wildcard $(CURRENTPATH)/SDCardApi.c)
CSRC += $(wildcard $(CURRENTPATH)/SRAMApi.c)

INCLUDES += $(CURRENTPATH)
