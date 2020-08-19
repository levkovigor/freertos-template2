CXXSRC += $(wildcard $(CURRENTPATH)/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/testdevices/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/testinterfaces/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/testtasks/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/prototypes/*.cpp)

ifeq ($(BOARD), stm32)
CXXSRC := $(filter-out test/testdevices/ArduinoDeviceHandler.cpp, $(CXXSRC))
endif

INCLUDES += $(CURRENTPATH)