CXXSRC += $(wildcard $(CURRENTPATH)/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/pus/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/devices/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/fdir/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/controller/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/controller/acs/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/controller/acs/helpfunctions/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/memory/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/utility/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/assembly/*.cpp)


INCLUDES += $(CURRENTPATH)