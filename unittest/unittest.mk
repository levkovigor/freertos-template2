CXXSRC += $(wildcard $(CURRENTPATH)/internal/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/*.cpp)

ifdef UNIT_TEST
ifndef NO_TEST_FW
CXXSRC += $(wildcard $(CURRENTPATH)/hosted/core/*.cpp)

CXXSRC += $(wildcard $(CURRENTPATH)/hosted/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/hosted/newtests/*.cpp)

INCLUDES += $(UNIT_TEST_PATH)/catch2
INCLUDES += $(UNIT_TEST_PATH)
INCLUDES += $(UNIT_TEST_PATH)/hosted/core
INCLUDES += $(UNIT_TEST_PATH)/hosted
endif 
endif

INCLUDES += $(CURRENTPATH)