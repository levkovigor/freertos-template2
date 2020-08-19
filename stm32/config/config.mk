CXXSRC += $(wildcard $(CURRENTPATH)/cdatapool/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/ipc/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/hk/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/objects/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/pollingsequence/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/events/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/tmtc/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/osal/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/devices/*.cpp)

INCLUDES += $(CURRENTPATH)/objects
INCLUDES += $(CURRENTPATH)/cdatapool
INCLUDES += $(CURRENTPATH)/returnvalues
INCLUDES += $(CURRENTPATH)/tmtc
INCLUDES += $(CURRENTPATH)/devices
INCLUDES += $(CURRENTPATH)/hk