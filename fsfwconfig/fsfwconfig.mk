CXXSRC += $(wildcard $(CURRENTPATH)/ipc/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/hk/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/objects/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/objects/system/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/pollingsequence/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/events/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/tmtc/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/osal/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/devices/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/*.cpp)

INCLUDES += $(CURRENTPATH)