CXXSRC += $(wildcard $(CURRENTPATH)/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/pus/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/devices/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/comIF/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/comIF/Cookies/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/fdir/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/controller/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/memory/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/utility/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/assembly/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/sensors/*.cpp)

ifneq ($(CHIP_NAME), sam9g20)
CXXSRC := $(filter-out $(CURRENTPATH)/pus/Service3HousekeepingPSB.cpp, $(CXXSRC))
endif

INCLUDES += $(CURRENTPATH)