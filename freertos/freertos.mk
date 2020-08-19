CSRC += $(wildcard $(CURRENTPATH)/src/*.c)
CSRC += $(wildcard $(CURRENTPATH)/src/portable/*.c)
CSRC += $(wildcard $(CURRENTPATH)/src/portable/GCC/ARM9_AT91SAM9G20/*.c)
# Heap 4 memory management is used.
CSRC += $(wildcard $(CURRENTPATH)/src/portable/MemMang/heap_4.c)

INCLUDES += $(CURRENTPATH)/include
INCLUDES += $(CURRENTPATH)/include/freertos
INCLUDES += $(CURRENTPATH)/include/freertos/portable/GCC/ARM9_AT91SAM9G20
INCLUDES += $(CURRENTPATH)/include/freertos/portable/MemMang

###################################################################
# Old FreeRTOS version in case there are problems with new version.
###################################################################
#CSRC += $(wildcard $(CURRENTPATH)/freertosold/src/*.c)
#CSRC += $(wildcard $(CURRENTPATH)/freertosold/src/portable/*.c)
#CSRC += $(wildcard $(CURRENTPATH)/freertosold/src/portable/GCC/ARM9_AT91SAM9G20/*.c)
# Heap 4 memory management is used.
#CSRC += $(wildcard $(CURRENTPATH)/freertos/src/portable/MemMang/heap_4.c)
#INCLUDES += $(CURRENTPATH)/freertosold/include
#INCLUDES += $(CURRENTPATH)/freertosold/include/freertos
#INCLUDES += $(CURRENTPATH)/freertosold/include/freertos/portable/GCC/ARM9_AT91SAM9G20
#INCLUDES += $(CURRENTPATH)/freertosold/include/freertos/portable/MemMang