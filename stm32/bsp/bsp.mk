CXXSRC += $(wildcard $(CURRENTPATH)/*.cpp)
CSRC += $(wildcard $(CURRENTPATH)/*.c)
ASRC += $(wildcard $(CURRENTPATH)/*.s)

CSRC += $(wildcard $(CURRENTPATH)/utility/*.c)
CXXSRC += $(wildcard $(CURRENTPATH)/utility/*.cpp)

CXXSRC += $(wildcard $(CURRENTPATH)/objects/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/pollingsequence/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/TmTcBridge/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/comIF/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/comIF/Cookies/*.cpp)

INCLUDES += $(CURRENTPATH)