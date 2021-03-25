# Cross compile CMake file for the SOURCE project. Requires ARM cross compiler in path

if(NOT DEFINED ENV{CROSS_COMPILE})
    set(CROSS_COMPILE "arm-none-eabi")
    message(STATUS 
        "No CROSS_COMPILE environmental variable set, using default ARM linux "
        "cross compiler name ${CROSS_COMPILE}"
    )
else()
    set(CROSS_COMPILE "$ENV{CROSS_COMPILE}")
    message(STATUS 
        "Using environmental variable CROSS_COMPILE as cross-compiler: "
        "$ENV{CROSS_COMPILE}"
    )
endif()

set(CROSS_COMPILE_CC "${CROSS_COMPILE}-gcc")
set(CROSS_COMPILE_CXX "${CROSS_COMPILE}-g++")
set(CROSS_COMPILE_OBJCOPY "${CROSS_COMPILE}-objcopy")
set(CROSS_COMPILE_SIZE "${CROSS_COMPILE}-size")

# At the very least, cross compile gcc and g++ have to be set!
find_program (CROSS_COMPILE_CC_FOUND ${CROSS_COMPILE_CC} REQUIRED)
find_program (CROSS_COMPILE_CXX_FOUND ${CROSS_COMPILE_CXX} REQUIRED)

set(CMAKE_CROSSCOMPILING TRUE)

# Define the compiler
set(CMAKE_C_COMPILER ${CROSS_COMPILE_CC})
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE_CXX})

# No system root used for bare metal compilation