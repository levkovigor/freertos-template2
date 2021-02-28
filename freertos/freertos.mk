FREERTOS_SRC += $(wildcard $(CURRENTPATH)/*.c)
FREERTOS_SRC += $(wildcard $(CURRENTPATH)/src/*.c)
FREERTOS_SRC += $(wildcard $(CURRENTPATH)/src/portable/*.c)
FREERTOS_SRC += $(wildcard $(CURRENTPATH)/src/portable/GCC/ARM9_AT91SAM9G20/*.c)
# FREERTOS_PORT = $(CURRENTPATH)/src/portable/GCC/ARM9_AT91SAM9G20/port.c
# Heap 4 memory management is used.
FREERTOS_SRC += $(wildcard $(CURRENTPATH)/src/portable/MemMang/heap_4.c)
# FREERTOS_SRC += $(wildcard $(CURRENTPATH)/src/portable/MemMang/standardMemMang.c)

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