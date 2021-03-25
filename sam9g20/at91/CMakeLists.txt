cmake_minimum_required(VERSION 3.13)

set(LIB_AT91_NAME at91)
add_library(${LIB_AT91_NAME})

add_subdirectory(src)

target_compile_definitions(${LIB_AT91_NAME} PRIVATE 
    at91sam9g20
)

option(AT91_DYN_TRACES "Dynamic traces for AT91 library" ON)
option(AT91_USE_AT91_STDIO "Use stdio provided by AT91" OFF)

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
)

target_compile_definitions(${LIB_AT91_NAME} PRIVATE
    at91sam9g20
    TRACE_LEVEL=${AT91_TRACE_LEVEL}
    DYN_TRACES=${DYN_TRACE_VALUE}
    USE_AT91LIB_STDIO_AND_STRING=${AT91_STDIO_VALUE}
)

# Required include paths to compile AT91
target_include_directories(${LIB_AT91_NAME} INTERFACE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/at91
    ${CMAKE_CURRENT_SOURCE_DIR}/include/at91/boards/at91sam9g20-ek
    ${CMAKE_CURRENT_SOURCE_DIR}/include/at91/boards/at91sam9g20-ek/at91sam9g20
    ${CMAKE_CURRENT_SOURCE_DIR}/include/at91/boards/common
)

target_include_directories(${LIB_AT91_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${AT91_ADDITIONAL_INC_PATHS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/at91
    ${CMAKE_CURRENT_SOURCE_DIR}/include/at91/boards/at91sam9g20-ek
    ${CMAKE_CURRENT_SOURCE_DIR}/include/at91/boards/at91sam9g20-ek/at91sam9g20
    ${CMAKE_CURRENT_SOURCE_DIR}/include/at91/boards/common
)
