CXXSRC += $(wildcard $(CURRENTPATH)/*.cpp)

CXXSRC += $(wildcard $(CURRENTPATH)/utility/*.cpp)
CSRC += $(wildcard $(CURRENTPATH)/utility/*.c)

CXXSRC += $(wildcard $(CURRENTPATH)/boardconfig/*.cpp)
CSRC += $(wildcard $(CURRENTPATH)/boardconfig/*.c)

CXXSRC += $(wildcard $(CURRENTPATH)/boardtest/*.cpp)
CSRC += $(wildcard $(CURRENTPATH)/boardtest/*.c)

CXXSRC += $(wildcard $(CURRENTPATH)/core/*.cpp)
CSRC += $(wildcard $(CURRENTPATH)/core/*.c)

CXXSRC += $(wildcard $(CURRENTPATH)/pus/*.cpp)
CSRC += $(wildcard $(CURRENTPATH)/pus/*.c)

CXXSRC += $(wildcard $(CURRENTPATH)/memory/*.cpp)
CSRC += $(wildcard $(CURRENTPATH)/memory/*.c)

CXXSRC += $(wildcard $(CURRENTPATH)/comIF/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/comIF/cookies/*.cpp)

CSRC += $(wildcard $(CURRENTPATH)/common/*.c)

INCLUDES += $(CURRENTPATH)
INCLUDES += $(CURRENTPATH)/boardconfig


# The utility folder has not been included to prevent name-clashes with
# C/C++ standard libaries.
ifdef ETHERNET
INCLUDES += $(LWIP_PATH)/include
INCLUDES += $(LWIP_PATH)/include/lwip/prot
endif



