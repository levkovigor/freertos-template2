CXXSRC += $(wildcard $(CURRENTPATH)/*.cpp)

CSRC += $(wildcard $(CURRENTPATH)/freertos/src/*.c)
CSRC += $(wildcard $(CURRENTPATH)/freertos/src/portable/*.c)
CSRC += $(wildcard $(CURRENTPATH)/freertos/src/portable/MemMang/heap_4.c)

INCLUDES += $(CURRENTPATH)
INCLUDES += $(CURRENTPATH)/freertos/include/
INCLUDES += $(CURRENTPATH)/freertos/include/freertos
INCLUDES += $(CURRENTPATH)/freertos/include/freertos/portable
INCLUDES += $(CURRENTPATH)/freertos/include/freertos/portable/GCC/ARM_CM7/r0p1