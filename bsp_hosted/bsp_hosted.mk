CXXSRC += $(wildcard $(CURRENTPATH)/*.cpp)
CSRC += $(wildcard $(CURRENTPATH)/*.c)

CXXSRC += $(wildcard $(CURRENTPATH)/boardtest/*.cpp)
CSRC += $(wildcard $(CURRENTPATH)/boardtest/*.c)

CSRC += $(wildcard $(CURRENTPATH)/boardconfig/*.c)
CXXSRC += $(wildcard $(CURRENTPATH)/boardconfig/*.cpp)

INCLUDES += $(CURRENTPATH)
