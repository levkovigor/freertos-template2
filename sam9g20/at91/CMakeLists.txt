cmake_minimum_required(VERSION 3.13)

set(LIB_AT91_NAME at91)
add_library(${LIB_AT91_NAME})

add_subdirectory(src)

target_compile_definitions(${LIB_AT91_NAME} PRIVATE 
    at91sam9g20
)

target_compile_definitions(${LIB_AT91_NAME} INTERFACE
    at91sam9g20
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
