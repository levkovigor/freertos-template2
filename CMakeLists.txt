################################################################################
# CMake support for the SOURCE OBSW.
# 
# Developed in an effort to replace Make with a modern build system.
# Currently only works for the hosted build.
# 
# Author: R. Mueller
################################################################################

################################################################################
# Pre-Project preparation
################################################################################
cmake_minimum_required(VERSION 3.13)

# set(CMAKE_VERBOSE TRUE)
# set(RTEMS_VERBOSE TRUE)

set(CMAKE_SCRIPT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

################################################################################
# Options
################################################################################

option(ADD_ETL_LIB "Add ETL library" ON)
option(BOOTLOADER "Build bootloader" OFF)
option(HOST_BUILD "Build for host" OFF)
option(USE_LTO "Enable link-time optimization" OFF)
# This option can also decrease binary size.
option(DISABLE_AGGRESSIVE_OPTIMIZATION 
    "Enable to use O2 instead of O3 optimization for the release build" OFF
)

if(NOT OS_FSFW)
    if(HOST_BUILD)
        set(OS_FSFW host CACHE STRING "OS for the FSFW.")
    else()
        set(OS_FSFW freertos CACHE STRING "OS for the FSFW.")
    endif()
endif()

if(NOT HOST_BUILD)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SCRIPT_PATH}/Arm926ejsCrossCompile.cmake)
    
    option(BOARD_IOBC "iOBC target hardware instead of AT91SAM9G20-EK development board" OFF)
    option(SAT_BUILD
        "If the iOBC is not connected to the flatsat or integrated into the satellite" ON
    )
    
    if(BOOTLOADER)
        if(NOT BOARD_IOBC)
            option(BL_STAGE_TWO 
                "Build second stage bootloader instead of first stage bootloader" OFF
             )
        endif()
        option(BL_USE_FREERTOS
            "Use FreeRTOS for iOBC or AT91 second-stage bootloader" ON
        )
        
        if(NOT BOARD_IOBC AND NOT BL_STAGE_TWO)
             set(SIMPLE_AT91_BL ON CACHE INTERNAL "Simple flag to detect simple bootloader")
             set(BL_USE_FREERTOS OFF)
        endif() 
    endif()
endif()

if(BOARD_IOBC)
    add_definitions(-DISIS_OBC_G20)
    set(BOARD_PRINTOUT "iOBC")
    if(BOOTLOADER)
        add_definitions(-DISIS_BL)
        set(AT91_MEMORY norflash)
    else()
        set(AT91_MEMORY sdram)
        if(SAT_BUILD)
            add_definitions(-DSAT_BUILD)
        endif()
    endif()
else()
    add_definitions(-DAT91SAM9G20_EK)
    set(BOARD_PRINTOUT "AT91SAM9G20-EK")
    if(BOOTLOADER)
        add_definitions(-DBOOTLOADER)
        if(USE_LTO)
            set(AT91_USE_LINK_TIME_OPTIMIZATION ON)
        endif()
        set(AT91_USE_AT91_STDIO ON)
        if(NOT BL_STAGE_TWO)
            add_definitions(-DAT91_BL_ST1)

            set(AT91_MEMORY sram)
        else()
            add_definitions(-DAT91_BL_ST2)
            set(AT91_MEMORY sdram)
        endif()
    else()
        set(AT91_MEMORY sdram)
    endif()
endif()

# Perform steps like loading toolchain files where applicable.

# Project Name
if(BOOTLOADER)
    project(sourcebl ASM C)
else()
    project(sourceobsw ASM C CXX)
endif()

################################################################################
# Pre-Sources preparation
################################################################################

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set names and variables
set(TARGET_NAME ${CMAKE_PROJECT_NAME}) 
set(LIB_FSFW_NAME fsfw)
set(LIB_ETL_NAME etl)
set(LIB_OS_NAME freertos)
set(LIB_AT91_NAME at91)
set(LIB_PRIVAT_LIB_NAME privlib)

# Set path names
set(FSFW_PATH fsfw)
set(MISSION_PATH mission)
set(TEST_PATH test)
set(ETL_PATH etl)
set(SAM9G20_PATH bsp_sam9g20)
set(HOST_BSP_PATH bsp_hosted)
set(BOOTLOADER_PATH bootloader)

set(LIB_FREERTOS_PATH ${LIB_OS_NAME})
set(LIB_AT91_PATH ${LIB_AT91_NAME})
set(LIB_PRIVLIB_PATH ${LIB_PRIVAT_LIB_NAME})

if(OS_FSFW MATCHES "host")
    set(BSP_PATH ${HOST_BSP_PATH})
elseif(OS_FSFW MATCHES "freertos")
    set(BSP_PATH ${SAM9G20_PATH})
endif()

# Analyse different OS and architecture/target options, determine BSP_PATH,
# display information about compiler etc.
set(COMMON_AT91_CONFIG_PATH "${CMAKE_SOURCE_DIR}/${SAM9G20_PATH}/common/config")

if(NOT HOST_BUILD)
    if(BOOTLOADER)
        # Only required because the AT91 library loads a specific header.
        set(FSFW_CONFIG_PATH "${BOOTLOADER_PATH}/config")
    else()
        set(FSFW_CONFIG_PATH "fsfwconfig")
    endif()
    set(FSFW_CONFIG_PATH_FULL "${CMAKE_SOURCE_DIR}/${FSFW_CONFIG_PATH}")
    
    set(FSFW_ADDITIONAL_LINK_LIBS ${LIB_AT91_NAME})
    if(BOOTLOADER)
        set(BOARD_CONFIG_PATH "${CMAKE_SOURCE_DIR}/${BOOTLOADER_PATH}/config")
    else()
        set(BOARD_CONFIG_PATH "${CMAKE_SOURCE_DIR}/${SAM9G20_PATH}/boardconfig")
    endif()
    set(FREERTOS_PATH "${CMAKE_SOURCE_DIR}/freertos")
    
    set(FREERTOS_ADDITIONAL_INCLUDE_PATHS
        "${CMAKE_SOURCE_DIR}"
        "${FREERTOS_PATH}/include/freertos/portable/GCC/ARM9_AT91SAM9G20"
        "${FREERTOS_PATH}/include/freertos/portable/GCC/MemMang"
        ${BOARD_CONFIG_PATH}
    )

    set(FREERTOS_ADDITIONAL_SOURCES
        "${FREERTOS_PATH}/src/portable/GCC/ARM9_AT91SAM9G20/port.c"
        "${FREERTOS_PATH}/src/portable/MemMang/heap_4.c"
        "${FREERTOS_PATH}/isisAdditions.c"
    )
    
    set(FREERTOS_PORT_MACRO_PATH 
        ${FREERTOS_PATH}/include/freertos/portable/GCC/ARM9_AT91SAM9G20
    )
    
    set(FREERTOS_ADDITIONAL_LIBS ${LIB_AT91_NAME})
    
    set(FREERTOS_KERNEL_SOURCE_FOLDER src)

    set(AT91_ADDITIONAL_INC_PATHS 
        ${FSFW_CONFIG_PATH_FULL}
        ${COMMON_AT91_CONFIG_PATH}
        ${FREERTOS_PORT_MACRO_PATH}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${BOARD_CONFIG_PATH}
    )

    # Select correct linker script
    if(NOT BOOTLOADER)
        set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/${LIB_AT91_PATH}/linker-scripts/sdram.lds) 
    else()
        if(BOARD_IOBC)
            set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/${BOOTLOADER_PATH}/norflash.lds)
        else()
            if(BL_STAGE_TWO)
                set(LINKER_SCRIPT 
                    ${CMAKE_CURRENT_SOURCE_DIR}/${BOOTLOADER_PATH}/sdram_second_stage_bl.lds
                )
            else()
                set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/${BOOTLOADER_PATH}/sram.lds)
            endif()
        endif()
    endif()
else()
    set(FSFW_CONFIG_PATH "${BSP_PATH}/fsfwconfig")
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(WARNING_FLAGS
        -Wall
        -Wextra
        -Wimplicit-fallthrough=1
        -Wno-unused-parameter
        -Wshadow=local
        -Wno-psabi
    )

    set(FSFW_WARNING_FLAGS ${WARNING_FLAGS})
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/permissive- /d2SSAOptimizer-)
    # To avoid nameclashes with min and max macro
    add_compile_definitions(NOMINMAX)
endif()

set(LINK_HCC ON)
set(LINK_HAL ON)

################################################################################
# Executable and Sources
################################################################################

# Add executable
add_executable(${TARGET_NAME})

# Add subdirectories
add_subdirectory(${BSP_PATH})

if(NOT HOST_BUILD)
    if(NOT BOOTLOADER)
        add_subdirectory(${LIB_FREERTOS_PATH})
    elseif(BL_USE_FREERTOS)
        add_subdirectory(${LIB_FREERTOS_PATH})
    endif()
    add_subdirectory(${LIB_AT91_PATH})
    add_subdirectory(${LIB_PRIVLIB_PATH})
endif()
    
if(BOOTLOADER)
    add_subdirectory(${BOOTLOADER_PATH})
else()
    add_subdirectory(${MISSION_PATH})
    add_subdirectory(${TEST_PATH})
    add_subdirectory(${FSFW_CONFIG_PATH})
    add_subdirectory(${FSFW_PATH})
    if(ADD_ETL_LIB)
        add_subdirectory(${ETL_PATH})
    endif()
endif()

################################################################################
# Post-Sources preparation
################################################################################

# Add libraries for all sources.
if(NOT BOOTLOADER)
    target_link_libraries(${TARGET_NAME} PRIVATE
        ${LIB_FSFW_NAME} 
    )
    
    if(ADD_ETL_LIB)
        target_link_libraries(${TARGET_NAME} PRIVATE
            ${LIB_ETL_NAME}
        )
    endif()
endif()

if(NOT HOST_BUILD)
    target_link_libraries(${TARGET_NAME} PRIVATE
        ${LIB_AT91_NAME}
    )
    
    set(LINK_FREERTOS OFF)
    if(BOOTLOADER)
        if(BL_USE_FREERTOS)
            set(LINK_FREERTOS ON)
        endif()
    else()
        set(LINK_FREERTOS ON)
    endif()
    
    if(LINK_FREERTOS)
        target_link_libraries(${TARGET_NAME} PRIVATE
             ${LIB_OS_NAME}
        )
    else()
        add_definitions(-DNO_RTOS)
    endif()
    
    if(LINKER_SCRIPT)
        target_link_options(${TARGET_NAME} PRIVATE
            -T${LINKER_SCRIPT}
        )
        if(NOT BOOTLOADER)
            target_link_options(${TARGET_NAME} PRIVATE
                -Wl,--undefined,_printf_float -Wl,--undefined,_scanf_float
            )
        endif()
    endif()
    
    
    if(NOT HOST_BUILD)
        target_include_directories(${TARGET_NAME} PRIVATE
            ${COMMON_AT91_CONFIG_PATH}
        )
    endif()
endif()
    
# Add include paths for all sources.
target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${FSFW_CONFIG_PATH}
)

# Compile options for all sources.
target_compile_options(${TARGET_NAME} PRIVATE
    ${WARNING_FLAGS}
)

if(USE_LTO)
    set_property(TARGET ${TARGET_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    # Remove unused sections.
    target_compile_options(${TARGET_NAME} PRIVATE
        "-ffunction-sections"
        "-fdata-sections"
    )
    target_link_options(${TARGET_NAME} PRIVATE
        "-Wl,--gc-sections"
    )
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
endif()

if(CMAKE_VERBOSE)
    message(STATUS "Warning flags: ${WARNING_FLAGS}")
endif()

if(NOT CMAKE_SIZE)
    set(CMAKE_SIZE size)
    if(WIN32)
        set(FILE_SUFFIX ".exe")
    endif()
endif()

if(NOT HOST_BUILD)
    if(BOOTLOADER)
        set(TYPE_PRINTOUT "Bootloader")
        if(NOT BOARD_IOBC)
            if(BL_STAGE_TWO)
                set(TYPE_PRINTOUT "${TYPE_PRINTOUT} (second stage)")
            else()
                set(TYPE_PRINTOUT "${TYPE_PRINTOUT} (first stage)")
            endif()
        endif()
    else()
        set(TYPE_PRINTOUT "OBSW")
    endif()
    set(TARGET_STRING "${BOARD_PRINTOUT} ${TYPE_PRINTOUT}")
    set(BUILD_TYPE_STR "debug")
    if(CMAKE_BUILD_TYPE MATCHES "MinSizeRel" OR CMAKE_BUILD_TYPE MATCHES "Release")
        set(BUILD_TYPE_STR "mission")
    endif()
    if(BOOTLOADER)
        if(NOT BOARD_IOBC)
            if(SIMPLE_AT91_BL)
                set(BUILD_TYPE_STR "stage1")
            else()
                set(BUILD_TYPE_STR "stage2")
            endif()
        endif()
    endif()
    if(BOARD_IOBC)
        set_target_properties(${TARGET_NAME} PROPERTIES 
            OUTPUT_NAME "${TARGET_NAME}-iobc-${BUILD_TYPE_STR}"
        )
    else()
        set_target_properties(${TARGET_NAME} PROPERTIES 
            OUTPUT_NAME "${TARGET_NAME}-at91-${BUILD_TYPE_STR}"
        )
    endif()
else()
    set(TARGET_STRING "Hosted")
endif()

get_target_property(OUTPUT_NAME_VAR ${TARGET_NAME} OUTPUT_NAME)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_link_options(${TARGET_NAME} PRIVATE
        "-Wl,-Map=${OUTPUT_NAME_VAR}.map"
    )
endif()

string(CONCAT POST_BUILD_COMMENT
    "Build directory: ${CMAKE_BINARY_DIR}\n"
    "Target OSAL: ${OS_FSFW}\n"
    "Target Build Type: ${CMAKE_BUILD_TYPE}\n"
    "Target Build: ${TARGET_STRING}"
)

add_custom_command(
   TARGET ${TARGET_NAME}
   POST_BUILD
   COMMAND ${CMAKE_SIZE} ${OUTPUT_NAME_VAR}
   COMMENT ${POST_BUILD_COMMENT}
)

if(NOT HOST_BUILD)
    add_custom_command(
        TARGET ${TARGET_NAME}
        POST_BUILD
        COMMAND echo Generating binary file ${OUTPUT_NAME_VAR}.bin..
        COMMAND ${CMAKE_OBJCOPY} -O binary ${OUTPUT_NAME_VAR} ${OUTPUT_NAME_VAR}.bin
    )
endif()

include (${CMAKE_SCRIPT_PATH}/BuildType.cmake)
set_build_type()





