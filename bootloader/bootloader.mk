CSRC += $(CURRENTPATH)/main.c
CSRC += $(wildcard $(CURRENTPATH)/utility/*.c)
CSRC += $(wildcard $(CURRENTPATH)/core/*.c)

INCLUDES += $(CURRENTPATH)
INCLUDES += $(CURRENTPATH)/utility
INCLUDES += $(CURRENTPATH)/config