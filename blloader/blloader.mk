CSRC += $(wildcard $(CURRENTPATH)/*.c)
CXXSRC += $(wildcard $(CURRENTPATH)/*.cpp)

CSRC += $(wildcard $(CURRENTPATH)/tmtcpacket/*.c)
CSRC += $(wildcard $(CURRENTPATH)/utility/*.c)
CXXSRC += $(wildcard $(CURRENTPATH)/utility/*.cpp)
CSRC += $(wildcard $(CURRENTPATH)/core/*.c)
CXXSRC += $(wildcard $(CURRENTPATH)/core/*.cpp)

ifeq ($(OS_APP), None)
CSRC := $(filter-out $(CURRENTPATH)/utility/hooks.c, $(CSRC))
endif

INCLUDES += $(CURRENTPATH)

SUBDIRS = $(CURRENTPATH)/config
define INCLUDE_FILE
CURRENTPATH := $S
include $(S)/$(notdir $S).mk
endef
$(foreach S,$(SUBDIRS),$(eval $(INCLUDE_FILE)))