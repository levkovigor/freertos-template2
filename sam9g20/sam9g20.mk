CXXSRC += $(wildcard $(CURRENTPATH)/*.cpp)

CXXSRC += $(wildcard $(CURRENTPATH)/utility/*.cpp)
CSRC += $(wildcard $(CURRENTPATH)/utility/*.c)

CXXSRC += $(wildcard $(CURRENTPATH)/boardconfig/*.cpp)
CSRC += $(wildcard $(CURRENTPATH)/boardconfig/*.c)

CXXSRC += $(wildcard $(CURRENTPATH)/boardtest/*.cpp)
CSRC += $(wildcard $(CURRENTPATH)/boardtest/*.c)

CXXSRC += $(wildcard $(CURRENTPATH)/core/*.cpp)
CSRC += $(wildcard $(CURRENTPATH)/core/*.c)

ifeq ($(BOARD), AT91SAM9G20_EK)
CXXSRC += $(wildcard $(CURRENTPATH)/core/at91/*.cpp)
else
CXXSRC += $(wildcard $(CURRENTPATH)/core/iobc/*.cpp)
endif

CXXSRC += $(wildcard $(CURRENTPATH)/pus/*.cpp)
CSRC += $(wildcard $(CURRENTPATH)/pus/*.c)

ifeq ($(BOARD), ISIS_OBC_G20)
CXXSRC += $(wildcard $(CURRENTPATH)/memory/FRAMHandler.cpp)
endif

CXXSRC += $(wildcard $(CURRENTPATH)/memory/SDCardAccess.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/memory/SDCardHandlerAux.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/memory/SDCardHandlerCore.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/memory/SDCardHandlerRead.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/memory/SDCardHandlerWrite.cpp)

CXXSRC += $(wildcard $(CURRENTPATH)/comIF/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/comIF/cookies/*.cpp)

INCLUDES += $(CURRENTPATH)
INCLUDES += $(CURRENTPATH)/boardconfig

# The utility folder has not been included to prevent name-clashes with
# C/C++ standard libaries.
ifdef ETHERNET
INCLUDES += $(LWIP_PATH)/include
INCLUDES += $(LWIP_PATH)/include/lwip/prot
endif



