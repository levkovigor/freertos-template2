CXXSRC += $(wildcard $(CURRENTPATH)/*.cpp)

ifndef ETHERNET
CXXSRC := $(filter-out $(CURRENTPATH)/TmTcLwIpUdpBridge.cpp, $(CXXSRC))
CXXSRC := $(filter-out $(CURRENTPATH)/EmacPollingTask.cpp, $(CXXSRC))
endif

INCLUDES += $(CURRENTPATH)