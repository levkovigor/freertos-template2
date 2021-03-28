cmake_minimum_required(VERSION 3.13)

####################################################################################################
# User configurable options
####################################################################################################

option(AT91_CREATE_GCC_SECTIONS 
    "Generate sections for functions and data. Required to remove unused code" ON
)
if(AT91_CREATE_GCC_SECTIONS)
    option(AT91_REMOVE_UNUSED_CODE "Remove unused code." ON)
endif()

option(AT91_USE_LINK_TIME_OPTIMIZATION "Enable link time optimization" OFF)
option(AT91_DYN_TRACES "Dynamic traces for AT91 library" ON)
option(AT91_USE_AT91_STDIO "Use stdio provided by AT91" OFF)
option(AT91_ISOLATE_ERRONEOUS_PATHS "Can be disabled to prevent trap of erroneous paths" ON)
option(AT91_NO_FREERTOS_STARTER_FILE "Use regular startup file even if FreeRTOS is linked" OFF)

if(NOT AT91_MEMORY)
    message(WARNING "AT91_MEMORY type not set, setting sdram")
    set(AT91_MEMORY sdram CACHE STRING "AT91 memory type")
else()
    set(AT91_MEMORY ${AT91_MEMORY} CACHE STRING "AT91 memory type")
endif()

set_property(CACHE AT91_MEMORY PROPERTY STRINGS sdram norflash sram)

set(LIB_AT91_NAME at91)
add_library(${LIB_AT91_NAME})

add_subdirectory(src)

target_compile_definitions(${LIB_AT91_NAME} PRIVATE 
    at91sam9g20
)


if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    if(NOT AT91_WARNING_FLAGS)
        set(AT91_WARNING_FLAGS
            -Wall
            -Wextra
            -Wimplicit-fallthrough=1
            -Wno-unused-parameter
            -Wshadow=local
        )
    endif()
endif()


if(AT91_USE_AT91_STDIO)
    set(AT91_STDIO_VALUE 1)
else()
    set(AT91_STDIO_VALUE 0)
endif()

if(NOT AT91_TRACE_LEVEL) 
    set(AT91_TRACE_LEVEL 5)
endif()

if(AT91_DYN_TRACES)
    set(DYN_TRACE_VALUE 1)
else()
    set(DYN_TRACE_VALUE 0)
endif()

target_compile_definitions(${LIB_AT91_NAME} INTERFACE
    at91sam9g20
    DYN_TRACES=${DYN_TRACE_VALUE}
    ${AT91_MEMORY}
)

target_compile_definitions(${LIB_AT91_NAME} PRIVATE
    at91sam9g20
    TRACE_LEVEL=${AT91_TRACE_LEVEL}
    DYN_TRACES=${DYN_TRACE_VALUE}
    USE_AT91LIB_STDIO_AND_STRING=${AT91_STDIO_VALUE}
    OP_BOOTSTRAP_on
    ${AT91_MEMORY}
)

if(NOT AT91_ISOLATE_ERRONEOUS_PATHS)
target_compile_options(${LIB_AT91_NAME} PRIVATE
    "-fno-isolate-erroneous-paths-dereference"
)
endif()

if(BOARD_IOBC)
    set(BOARD_INCLUDE_PACKAGE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/at91/boards/ISIS_OBC_G20
        ${CMAKE_CURRENT_SOURCE_DIR}/include/at91/boards/ISIS_OBC_G20/at91sam9g20
    )
else()
    set(BOARD_INCLUDE_PACKAGE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/at91/boards/at91sam9g20-ek
        ${CMAKE_CURRENT_SOURCE_DIR}/include/at91/boards/at91sam9g20-ek/at91sam9g20
    )
endif()

# Required include paths to compile AT91
target_include_directories(${LIB_AT91_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/at91
    ${CMAKE_CURRENT_SOURCE_DIR}/include/at91/boards/common
    ${BOARD_INCLUDE_PACKAGE}
)

target_include_directories(${LIB_AT91_NAME} PRIVATE
    ${AT91_ADDITIONAL_INC_PATHS}
    ${CMAKE_SOURCE_DIR}/privlib/hal/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/at91
    ${CMAKE_CURRENT_SOURCE_DIR}/include/at91/boards/common
    ${BOARD_INCLUDE_PACKAGE}
)

if(AT91_CREATE_GCC_SECTIONS)
    target_compile_options(${LIB_AT91_NAME} PRIVATE
        ${AT91_WARNING_FLAGS}
    )
        
    # Create sections for a GCC compiler
    target_compile_options(${LIB_AT91_NAME} PRIVATE
        "-ffunction-sections"
        "-fdata-sections"
    )
        
    if(AT91_REMOVE_UNUSED_CODE)
        target_link_options(${LIB_AT91_NAME} PRIVATE
            "-Wl,--gc-sections"
        )
    endif()
    
    if(AT91_USE_LINK_TIME_OPTIMIZATION)
        set_property(TARGET ${LIB_AT91_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    endif()
endif()