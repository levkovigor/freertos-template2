#-------------------------------------------------------------------------------
# Makefile  for the SOURCE On-Board Software
#-------------------------------------------------------------------------------
# @manual	See README.md
# @details
# Makefiles are the primary tool used to build the flight software for various
# operating systems and build configurations, using the GNU make tool.
# 
# It takes care of including all the right paths, source files and performing
# other procedures used to build the software.
# I tried my best to make the arcane syntax of this old but established
# tool as understandable as possible by adding comments and good links to 
# help other students and software developers to understand how this tool
# works so they can make necessary adaptions and improvements to improve
# the software.
#
# @author	 R.Mueller, J. Meier
#-------------------------------------------------------------------------------

# Fundamentals on the build process of C/C++ Software:
# https://www3.ntu.edu.sg/home/ehchua/programming/cpp/gcc_make.html

# Make documentation: https://www.gnu.org/software/make/manual/make.pdf
# Online: https://www.gnu.org/software/make/manual/make.html
# General rules: http://make.mad-scientist.net/papers/rules-of-makefiles/#rule3
SHELL = /bin/sh

#-------------------------------------------------------------------------------
#		User-modifiable options
#-------------------------------------------------------------------------------
# Chip & board used for compilation
# (can be overriden by adding CHIP=chip and BOARD=board to the command-line)
CHIP  = at91sam9g20
CUSTOM_DEFINES += -D$(CHIP)
# select the target platform. Add IOBC=1 to build the binary for the iOBC.
ifdef IOBC
BOARD = ISIS_OBC_G20
BOARD_NAME = iOBC
else 
BOARD = AT91SAM9G20_EK
BOARD_NAME = at91ek
# This define is used by the AT91 to perform 
# reduced NAND-Flash operations
CUSTOM_DEFINES += -DOP_BOOTSTRAP_on
endif
CUSTOM_DEFINES += -D$(BOARD) -D$(CHIP)

ifdef ADD_CR
CUSTOM_DEFINES += -DADD_CR
endif

CHIP_NAME = sam9g20
BOARD_FILE_ROOT = bsp_$(CHIP_NAME)

ADD_USB_DRIVER = 1
ADD_MMC_DRIVER = 1
ADD_TINYFATFS = 1

ifdef IOBC
CHIP_PATH = iobc
else
CHIP_PATH = sam9g20ek
endif

ENABLE_FLTO = 0

OS_FSFW = freeRTOS
OS_APP = $(OS_FSFW)
CUSTOM_DEFINES += -D$(OS_APP)

# General folder paths
FRAMEWORK_PATH = fsfw
MISSION_PATH = mission
CONFIG_PATH = fsfwconfig
TEST_PATH = test
UNITTEST_PATH = $(FRAMEWORK_PATH)/unittest/internal
PRIVLIB_PATH = privlib
FREERTOS_PATH = freertos

# Board specific paths
COMIF_PATH = $(BOARD_FILE_ROOT)/comIF
TMTCBRIDGE_PATH = $(BOARD_FILE_ROOT)/tmtcbridge

LWIP_PATH = $(BOARD_FILE_ROOT)/lwip
BOARDTEST_PATH = $(BOARD_FILE_ROOT)/boardtest

LINKER_SCRIPT_PATH = at91/linker-scripts
GDB_PATH = $(BOARD_FILE_ROOT)/gdb
BSP_PATH = $(BOARD_FILE_ROOT)

# Private (non-public) libraries.
# If programming for AT91 development board or iOBC, add the libraries manually.
AT91_PATH = at91
SAM9G20_COMMON_PATH = $(BOARD_FILE_ROOT)/common
HCC_PATH = $(PRIVLIB_PATH)/hcc
HAL_PATH = $(PRIVLIB_PATH)/hal

# Output file basename
BASENAME = sourceobsw
BINARY_NAME = $(BASENAME)-$(BOARD_NAME)
# Output files will be put in this directory inside
OUTPUT_FOLDER = $(CHIP_PATH)
NO_TEST_FW = 1

OUTPUT_COMM = serial
COMIF_MESSAGE = Serial Communication \(RS232\)

ifdef ETHERNET
COMIF_MESSAGE = Ethernet Communication \(UDP Datagrams\)
OUTPUT_COMM = udp
COM_DEFINE = -DETHERNET
CUSTOM_DEFINES += $(COM_DEFINE)
endif

# Default debug output
DEBUG_LEVEL = -g3

# Trace level used for compilation
# (can be overriden by adding TRACE_LEVEL=#number to the command-line)
# TRACE_LEVEL_DEBUG      5
# TRACE_LEVEL_INFO       4
# TRACE_LEVEL_WARNING    3
# TRACE_LEVEL_ERROR      2
# TRACE_LEVEL_FATAL      1
# TRACE_LEVEL_NO_TRACE   0
TRACE_LEVEL = 5

USE_AT91LIB_STDIO_AND_STRING = 0

# Dynamic Traces to allow change of trace level at run-time by 
# setting traceLevel global variable
DYN_TRACES = 1

# Optimization level. -O0 for debugging, -Os for size. -O0 is default.
OPTIMIZATION = -O0

# Output directories
BUILDPATH = _bin
DEPENDPATH = _dep
OBJECTPATH = _obj
ifeq ($(MAKECMDGOALS),mission)
BUILD_FOLDER = mission
else 
BUILD_FOLDER = devel
endif

DEPENDDIR = $(DEPENDPATH)/$(OUTPUT_FOLDER)/$(BUILD_FOLDER)
OBJDIR = $(OBJECTPATH)/$(OUTPUT_FOLDER)/$(BUILD_FOLDER)
BINDIR = $(BUILDPATH)/$(OUTPUT_FOLDER)/$(BUILD_FOLDER)

CLEANDEP = $(DEPENDPATH)/$(OUTPUT_FOLDER)
CLEANOBJ = $(OBJECTPATH)/$(OUTPUT_FOLDER)
CLEANBIN = $(BUILDPATH)

ifeq ($(BOARD),at91sam9g20_ek)
# Compile with chip specific features
include $(AT91_PATH)/include/at91/boards/at91sam9g20-ek/$(CHIP)/chip.mak
MEMORIES = sdram
# Compile for all memories available on the board (this sets $(MEMORIES))
#include $(AT91_PATH)/include/at91/boards/at91sam9g20-ek/board.mak
else
MEMORIES = sdram
endif

#-------------------------------------------------------------------------------
#		Tools and Includes
#-------------------------------------------------------------------------------

# Tool suffix when cross-compiling
CROSS_COMPILE = arm-none-eabi-

ifdef WINDOWS
CC = $(CROSS_COMPILE)gcc.exe

# C++ compiler
CXX = $(CROSS_COMPILE)g++.exe

# Additional Tools
SIZE = $(CROSS_COMPILE)size.exe
STRIP = $(CROSS_COMPILE)strip.exe
CP = $(CROSS_COMPILE)objcopy.exe
else
# C Compiler
CC = $(CROSS_COMPILE)gcc

# C++ compiler
CXX = $(CROSS_COMPILE)g++

# Additional Tools
SIZE = $(CROSS_COMPILE)size
STRIP = $(CROSS_COMPILE)strip
CP = $(CROSS_COMPILE)objcopy
endif

HEXCOPY = $(CP) -O ihex
BINCOPY = $(CP) -O binary

# files to be compiled, will be filled in by include makefiles
# := assignment here is neccessary as initialization so that the +=
# operator can be used in the submakefiles to achieve immediate evaluation. 
# See: http://make.mad-scientist.net/evaluation-and-expansion/
CSRC := 
FREERTOS_SRC :=
FREERTOS_PORT :=
AT91_SRC :=
CXXSRC := 
ASRC := 
INCLUDES := 

# Directories where $(directoryname).mk files should be included from.
# Source files and includes can be added in those submakefiles.
SUBDIRS := $(CONFIG_PATH) $(FRAMEWORK_PATH) $(MISSION_PATH) $(BSP_PATH) \
		$(AT91_PATH) $(TEST_PATH) $(TMTCBRIDGE_PATH) $(PRIVLIB_PATH) \
		$(FREERTOS_PATH) $(UNITTEST_PATH) $(SAM9G20_COMMON_PATH)
		
# $(info $${SUBDIRS} is [${SUBDIRS}])	
# to include the lwip source files
ifeq ($(BOARD),at91sam9g20_ek)
ifdef ETHERNET
SUBDIRS += $(LWIP_PATH)/src/LwIP
endif
endif

# This is a hack from http://make.mad-scientist.net/the-eval-function/
#
# The problem is, that included makefiles should be aware of their relative path
# but not need to guess or hardcode it. So we set $(CURRENTPATH) for them. If
# we do this globally and the included makefiles want to include other makefiles as
# well, they would overwrite $(CURRENTPATH), screwing the include after them.
#
# By using a for-loop with an eval'd macro, we can generate the code to include all
# sub-makefiles (with the correct $(CURRENTPATH) set) before actually evaluating
# (and by this possibly changing $(CURRENTPATH)) them.
#
# This works recursively, if an included makefile wants to include, it can safely set 
# $(SUBDIRS) (which has already been evaluated here) and do
# "$(foreach S,$(SUBDIRS),$(eval $(INCLUDE_FILE)))"
# $(SUBDIRS) must be relative to the project root, so to include subdir foo, set
# $(SUBDIRS) = $(CURRENTPATH)/foo.
define INCLUDE_FILE
CURRENTPATH := $S
include $(S)/$(notdir $S).mk
endef
$(foreach S,$(SUBDIRS),$(eval $(INCLUDE_FILE)))

# ETL library include.
ETL_PATH = etl/include
I_INCLUDES += -I$(ETL_PATH)

# Include all includes defined as INCLUDES=...
I_INCLUDES += $(addprefix -I, $(INCLUDES))

# Debug Info
#$(info $${I_INCLUDES} is [${I_INCLUDES}])

#-------------------------------------------------------------------------------
#		Source Files
#-------------------------------------------------------------------------------
# Additional source files which were not includes by other .mk
# files are added here.
# To help Makefile find source files, the source location paths
# can be added by using the VPATH variable
# See: https://www.gnu.org/software/make/manual/html_node/General-Search.html
# It is recommended to only use VPATH to add source locations
# See: http://make.mad-scientist.net/papers/how-not-to-use-vpath/

# Please ensure that no files are included by both .mk file and here !

# Directories where C source files can be found.
VPATH +=

# Manual source file includes (VPATH directories will be searched)
CSRC +=
CXXSRC +=
ASRC +=

# All C Sources included by .mk files are assigned here
# Add the objects to sources so dependency handling works.

# Objects built from C source files.
C_OBJECTS += $(CSRC:.c=.o)
FREERTOS_OBJECTS += $(FREERTOS_SRC:.c=.o)
AT91_OBJECTS += $(AT91_SRC:.c=.o)
# Objects built from assembler source files.
ASM_OBJECTS += $(ASRC:.S=.o)
# ASM_OBJECTS += $(ASRC:.s=.o)
# Objects built from C++ source files.
CXX_OBJECTS +=  $(CXXSRC:.cpp=.o)

#-------------------------------------------------------------------------------
#		Build Configuration + Output
#-------------------------------------------------------------------------------

TARGET = Debug
DEBUG_MESSAGE = Off
DYN_TRACE_MESSAGE = Off
ifeq (1, $(DYN_TRACES))
DYN_TRACE_MESSAGE = On
endif
OPTIMIZATION_MESSAGE = Off

# Define Messages
ifdef IOBC
MSG_INFO = Software: Primary mission software for iOBC
else
MSG_INFO = Software: Primary mission software for AT91SAM9G20-EK
endif
MSG_LINKING = Linking:
MSG_COMPILING = Compiling:
MSG_ASSEMBLING = Assembling:
MSG_DEPENDENCY = Collecting dependencies for:
MSG_BINARY = Generate binary: 
MSG_OPTIMIZATION = Optimization: $(OPTIMIZATION), $(OPTIMIZATION_MESSAGE)
MSG_TARGET = Target Build: $(TARGET)
MSG_DEBUG = Debug level: $(DEBUG_LEVEL), FSFW Debugging: $(DEBUG_MESSAGE), \
		Trace Level: $(TRACE_LEVEL), Dynamic Traces: $(DYN_TRACE_MESSAGE)
MSG_COMIF = TMTC Communication Interface: $(COMIF_MESSAGE)

# See: https://stackoverflow.com/questions/6687630/how-to-remove-unused-c-c-symbols-with-gcc-and-ld
# Used to throw away unused code. Reduces code size significantly !
# -Wl,--gc-sections: needs to be passed to the linker to discard unused sections
ifdef KEEP_UNUSED_CODE
PROTOTYPE_OPTIMIZATION = 
UNUSED_CODE_REMOVAL = 
else
PROTOTYPE_OPTIMIZATION = -ffunction-sections -fdata-sections
UNUSED_CODE_REMOVAL = -Wl,--gc-sections
# Link time optimization
# See https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html for reference
# Link time is larger and size of object files can not be retrieved
# but resulting binary is smaller. Could be used in mission/deployment build
# Requires -ffunction-section in linker call
LINK_TIME_OPTIMIZATION =
FREERTOS_OPTIMIZATION = -fno-lto
AT91_OPTIMIZATION = -fno-lto
OPTIMIZATION += $(PROTOTYPE_OPTIMIZATION) 
endif 
# CPU specification
CPU_FLAG = -mcpu=arm926ej-s
# Newlib Nano
NEWLIB_NANO = -specs=nosys.specs -specs=nano.specs

# Dependency Flags
# These flags tell the compiler to build dependencies
# See: https://www.gnu.org/software/make/manual/html_node/Automatic-Prerequisites.html
# Using following guide: 
# http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPENDDIR)/$*.d

# Flags for the compiler call. CFLAGS C only, CXXFLAGS C++ and C, CPPFLAGS C++.
# - std: Which C++ version to use. Common versions: c++11, c++14 and c++17
# - Wall: enable all warnings
# - Wextra: enable extra warnings
# - g: defines debug level
# - fmessage-length: to control the formatting algorithm for diagnostic messages;
#   =0 means  no line-wrapping is done; each error message appears on a single line
# - fno-exceptions: stops generating extra code needed to propagate exceptions, 
#   which can produce significant data size overhead
# - USE_AT91LIB_STDIO_AND_STRING: Set this to 1 to use AT91 implementation. 
#   Does not support float I/O !
# - Set to 0 so newlib nano is used.
# - NEWLIB_NO_C99_IO=1: Right now, newlib does not support 
#   C99 scanf/printf formats (e.g. uint8)
CUSTOM_DEFINES += -DUSE_AT91LIB_STDIO_AND_STRING=$(USE_AT91LIB_STDIO_AND_STRING)
CUSTOM_DEFINES += -DNEWLIB_NANO_NO_C99_IO
WARNING_FLAGS = -Wall -Wshadow=local -Wextra -Wimplicit-fallthrough=1 \
		-Wno-unused-parameter -Wno-psabi
		
CXXDEFINES := -DTRACE_LEVEL=$(TRACE_LEVEL) -DDYN_TRACES=$(DYN_TRACES) \
		$(CUSTOM_DEFINES) -D$(MEMORIES)
CFLAGS +=  
CXXFLAGS += -I.  $(DEBUG_LEVEL) $(DEPFLAGS) $(WARNING_FLAGS) \
		-fmessage-length=0 $(OPTIMIZATION) $(I_INCLUDES) $(CXXDEFINES) \
		$(NEWLIB_NANO) $(CPU_FLAG)
CPPFLAGS += -std=c++17 -fno-exceptions
ASFLAGS =  -Wall -g $(OPTIMIZATION) $(I_INCLUDES) -D$(MEMORIES) -D$(CHIP) -D__ASSEMBLY__ \
           $(NEWLIB_NANO) $(CPU_FLAG)

# Flags for the linker call
# - specs=nosys.specs: this library contains system calls such as _sbrk(); 
#   without this library system calls are not enabled an functions like malloc
#   will not work	 
# - specs=nano.specs: links the reduced-size variant of the libc, called newlib-nano
# - LINK_INCLUDES: Specify the path to used libraries and the linker script
# - LINK_LIBRARIES: Link HCC and HAL library and enable float support

LDFLAGS = $(DEBUG_LEVEL) $(CPU_FLAG) $(UNUSED_CODE_REMOVAL) $(OPTIMIZATION) \
		$(LINK_TIME_OPTIMIZATION) $(NEWLIB_NANO) 
LINK_INCLUDES = -L"$(HAL_PATH)/lib" -L"$(HCC_PATH)/lib" \
		-T"$(LINKER_SCRIPT_PATH)/$(MEMORIES).lds" -Wl,-Map=$(BINDIR)/$(BINARY_NAME)-$(MEMORIES).map
LINK_LIBRARIES = -lc -u _printf_float -u _scanf_float -lHCC -lHCCD -lHAL -lHALD 

# $(info $${I_INCLUDES} is [${I_INCLUDES}])

#-------------------------------------------------------------------------------
#		Rules
#-------------------------------------------------------------------------------

# Makefile rules: https://www.gnu.org/software/make/manual/html_node/Rules.html
# This is the primary section which defines the ruleset to build
# the executable from the sources.

# Default build (first target found)
default: all

# The call function assigns parameters to temporary variables
# https://www.gnu.org/software/make/manual/make.html#Call-Function
# $(1) = Memory names
# Rules are called for each memory type
# Two Expansion Symbols $$ are to escape the dollar sign for eval.
# See: http://make.mad-scientist.net/the-eval-function/

all: executable 

# Build target configuration, which also includes information output.
# See: https://www.gnu.org/software/make/manual/html_node/Target_002dspecific.html
# Link time optimization is added in the rules to disable it for certain source files
mission: OPTIMIZATION = -Os $(PROTOTYPE_OPTIMIZATION)
# This detects nullptr deferences and turns them into traps. We don't want that,
# we actually want the core to reboot immediately, so we suppress this 
# optimization.
mission: OPTIMIZATION += -fno-isolate-erroneous-paths-dereference 
# I don't understand why this is necessary, but it (sometimes?) is.
# mission: FREERTOS_OPTIMIZATION += -fno-omit-frame-pointer
# Was necessary but the culprit function was marked ((noinline)) explicitely in the source code.
# mission: FREERTOS_OPTIMIZATION += -fno-inline-small-functions

# Link time optimization can lead to issues, so  if there is an issue with the
# mission binary, try to disable it to see if that fixes the problem.
ifeq ($(ENABLE_FLTO), 1)
mission: LINK_TIME_OPTIMIZATION = -flto
endif

mission: TARGET = Mission binary
ifeq ($(ENABLE_FLTO), 1)
mission: OPTIMIZATION_MESSAGE = On with Link Time Optimization.
else
mission: OPTIMIZATION_MESSAGE = On without Link Time Optimization.
endif
mission: DEBUG_LEVEL = -g0

debug: CXXDEFINES += -DDEBUG
debug: DEBUG_MESSAGE = On
debug: DEBUG_LEVEL = -g3

virtual: CXXDEFINES += -DVIRTUAL_BUILD -DDEBUG
virtual: TARGET = Debug binary with virtualized interfaces
virtual: DEBUG_MESSAGE = On
virtual: DEBUG_LEVEL = -g3

ifdef KEEP_UNUSED_CODE
debug virtual mission: OPTIMIZATION_MESSAGE += , no unused code removal
endif

debug virtual mission: executable

# Cleans all files 
hardclean: 
	-rm -rf $(BUILDPATH)
	-rm -rf $(OBJECTPATH)
	-rm -rf $(DEPENDPATH)

# Only clean files for current build
clean:
	-rm -rf $(CLEANOBJ)
	-rm -rf $(CLEANBIN)
	-rm -rf $(CLEANDEP)

# Only clean binaries. Useful for changing the binary type when object 
# files are already compiled so complete rebuild is not necessary
cleanbin:
	-rm -rf $(BUILDPATH)/$(OUTPUT_FOLDER)

# In this section, the binaries are built for all selected memories.
# The section defined between define and endef will be evaluated
# for each memory type.
# See: http://make.mad-scientist.net/the-eval-function/
# define MEMORY_BUILDS

executable: $(BINDIR)/$(BINARY_NAME)-$(MEMORIES).bin

C_OBJECTS := $(addprefix $(OBJDIR)/, $(C_OBJECTS))
FREERTOS_OBJECTS := $(addprefix $(OBJDIR)/$(OS_FSFW)/, $(FREERTOS_OBJECTS))
AT91_OBJECTS := $(addprefix $(OBJDIR)/at91/, $(AT91_OBJECTS))
ASM_OBJECTS := $(addprefix $(OBJDIR)/, $(ASM_OBJECTS))
CXX_OBJECTS := $(addprefix $(OBJDIR)/, $(CXX_OBJECTS))

ALL_OBJECTS =  $(ASM_OBJECTS) $(C_OBJECTS) $(CXX_OBJECTS) $(FREERTOS_OBJECTS) $(AT91_OBJECTS)

# Useful for debugging the Makefile
# Also see: https://www.oreilly.com/openbook/make3/book/ch12.pdf
# $$(info $${ALL_OBJECTS} is [${ALL_OBJECTS}])
# $$(info {$$(OBJDIR)/$(C_OBJECTS)} is [$(OBJDIR)/$(1)/$(C_OBJECTS)}])
# $$(info {$$(CSRC)} is [${CSRC}])
# $$(info $${CXXSRC} is [${CXXSRC}])

# Automatic variables are used here extensively. Some of them
# are escaped($$) to suppress immediate evaluation. The most important ones are:
# $@: Name of Target (left side of rule)
# $<: Name of the first prerequisite (right side of rule)
# @^: List of all prerequisite, omitting duplicates
# @D: Directory and file-within-directory part of $@

# Generates binary and displays all build properties
# -p with mkdir ignores error and creates directory when needed.

# SHOW_DETAILS = 1

$(BINDIR)/$(BINARY_NAME)-$(MEMORIES).bin: $(BINDIR)/$(BINARY_NAME)-$(MEMORIES).elf
	@echo
	@echo $(MSG_INFO)
	@echo $(MSG_TARGET)
	@echo $(MSG_OPTIMIZATION)
	@echo $(MSG_DEBUG)
	@echo $(MSG_COMIF)
	@echo $(MSG_BINARY) $@
	@mkdir -p $(@D)
	@$(BINCOPY) $< $@ 
	
ifeq ($(OS),Windows_NT)
# Check whether stat is available.
# Otherwise, try to display size with busybox
ifeq (, $(shell where stat))
	@echo Binary Size: `busybox stat -c %s $@` bytes
else
	@stat --printf='Binary Size: %s bytes' $@
endif

else
	@stat --printf='Binary Size: %s bytes' $@
endif
	@echo

$(BINDIR)/$(BINARY_NAME)-$(MEMORIES).hex: $(BINDIR)/$(BINARY_NAME)-$(MEMORIES).elf
	@echo
	@echo $(MSG_BINARY)
	@mkdir -p $(@D)
	$(HEXCOPY) $< $@

# Link with required libraries: HAL (Hardware Abstraction Layer) and 
# HCC (File System Library)
$(BINDIR)/$(BINARY_NAME)-$(MEMORIES).elf: $(ALL_OBJECTS) 
	@echo
	@echo $(MSG_LINKING) Target $@
	@mkdir -p $(@D)
ifdef SHOW_DETAILS
	$(CXX) $(LDFLAGS) $(LINK_INCLUDES) -o $@ $^ $(LINK_LIBRARIES)
else
	@$(CXX) $(LDFLAGS) $(LINK_INCLUDES) -o $@ $^ $(LINK_LIBRARIES)
endif
ifeq ($(BUILD_FOLDER), mission)
# With Link Time Optimization, section size is not available
	$(SIZE) $@
else
	$(SIZE) $^ $@
endif

# Build new objects for changed dependencies. 
$(OBJDIR)/%.o: %.cpp
$(OBJDIR)/%.o: %.cpp $(DEPENDDIR)/%.d | $(DEPENDDIR)
	@echo 
	@echo $(MSG_COMPILING) $<
	@mkdir -p $(@D)
ifdef SHOW_DETAILS
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LINK_TIME_OPTIMIZATION) -c -o $@ $<
else
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LINK_TIME_OPTIMIZATION) -c -o $@ $<
endif

$(OBJDIR)/%.o: %.c
$(OBJDIR)/%.o: %.c $(DEPENDDIR)/%.d | $(DEPENDDIR) 
	@echo 
	@echo $(MSG_COMPILING) $<
	@mkdir -p $(@D)
ifdef SHOW_DETAILS
	$(CC) $(CXXFLAGS) $(CFLAGS) $(LINK_TIME_OPTIMIZATION) -c -o $@ $<
else
	@$(CC) $(CXXFLAGS) $(CFLAGS) $(LINK_TIME_OPTIMIZATION) -c -o $@ $<
endif

# Seperate rules for FreeRTOS to tweak build settings.
$(OBJDIR)/$(OS_FSFW)/%.o: %.c
$(OBJDIR)/$(OS_FSFW)/%.o: %.c $(DEPENDDIR)/%.d | $(DEPENDDIR) 
	@echo 
	@echo Compiling FreeRTOS source: $<
	@mkdir -p $(@D)
ifdef SHOW_DETAILS
	$(CC) $(CXXFLAGS) $(CFLAGS) $(FREERTOS_OPTIMIZATION) -c -o $@ $<
else
	@$(CC) $(CXXFLAGS) $(CFLAGS) $(FREERTOS_OPTIMIZATION) -c -o $@ $<
endif

# Seperate rules for AT91 to tweak build settings.
$(OBJDIR)/at91/%.o: %.c
$(OBJDIR)/at91/%.o: %.c $(DEPENDDIR)/%.d | $(DEPENDDIR) 
	@echo 
	@echo Compiling AT91 library source: $<
	@mkdir -p $(@D)
ifdef SHOW_DETAILS
	$(CC) $(CXXFLAGS) $(CFLAGS) $(AT91_OPTIMIZATION) -c -o $@ $<
else
	@$(CC) $(CXXFLAGS) $(CFLAGS) $(AT91_OPTIMIZATION) -c -o $@ $<
endif

$(OBJDIR)/%.o: %.S Makefile
	@echo
	@echo $(MSG_ASSEMBLING) $<
	@mkdir -p $(@D)
ifdef SHOW_DETAILS
	$(CC) $(ASFLAGS) -c -o $@ $<
else
	@$(CC) $(ASFLAGS) -c -o $@ $<
endif

#endef
# $(foreach MEMORY, $(MEMORIES), $(eval $(call MEMORY_BUILDS,$(MEMORY))))

#-------------------------------------------------------------------------------
#		Dependency Handling
#-------------------------------------------------------------------------------

# Dependency Handling according to following guide:
# http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
$(DEPENDDIR):
	@mkdir -p $(@D)
DEPENDENCY_RELATIVE = $(CSRC:.c=.d) $(CXXSRC:.cpp=.d) $(FREERTOS_SRC:.c=.d) $(AT91_SRC:.c=.d)
# This is the list of all dependencies
DEPFILES = $(addprefix $(DEPENDDIR)/, $(DEPENDENCY_RELATIVE))
# Create subdirectories for dependencies
$(DEPFILES):
	@mkdir -p $(@D)
# Include all dependencies
include $(wildcard $(DEPFILES))

# configure sdram before writing the image to it
# needs only to be done once after powering up the board
# if there are problems running JLinkGDBServerCL.exe like this
# you can use the JLink GDB Server App by starting it, setting ARM9 as 
# target architecture and running sdramCfg again. If using eclipse, check
# whether the JLink binaries path is in the environment variables of eclipse.
sdramCfg: 
	@echo "Starting a J-Link connection."
ifdef WINDOWS
	JLinkGDBServerCL.exe -USB -device AT91SAM9G20 -endian little -if JTAG \
	-speed auto -noLocalhostOnly &
	# @sleep 1
	arm-none-eabi-gdb.exe -nx --batch -ex 'target remote localhost:2331' -ex \
	'source $(GDB_PATH)/at91sam9g20-ek-sdram.gdb'
	@fortune | cowsay | lolcat || true
else
	JLinkGDBServerCLExe -USB -device AT91SAM9G20 -endian little -if JTAG \
	-speed adaptive -noLocalhostOnly &
	@sleep 1
	arm-none-eabi-gdb -nx --batch -ex 'target remote localhost:2331' -ex \
	'source $(GDB_PATH)/at91sam9g20-ek-sdram.gdb'
	@fortune | cowsay | lolcat || true
endif

IOBC_REMOTE_IP = 192.168.199.228

# Configre SDRAM of IOBC
iobcCfg:
	arm-none-eabi-gdb -nx --batch -ex 'target remote $(IOBC_REMOTE_IP):2331' -ex \
	'source $(GDB_PATH)/at91sam9g20-ek-sdram.gdb'

# .PHONY tells make that these targets aren't files
.PHONY: clean sdramCfg mission debug virtual all hardclean

