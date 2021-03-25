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

option(ADD_ETL_LIB "Add ETL library" ON)

if(NOT OS_FSFW)
    set(OS_FSFW freertos CACHE STRING "OS for the FSFW.")
endif()

if(OS_FSFW MATCHES "freertos")
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SCRIPT_PATH}/Arm926ejsCrossCompile.cmake)
endif()

# Perform steps like loading toolchain files where applicable.

# Project Name
project(sourceobsw ASM C CXX)

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

# Set path names
set(FSFW_PATH fsfw)
set(MISSION_PATH mission)
set(TEST_PATH test)
set(ETL_PATH etl)
set(FREERTOS_PATH freertos)

if(OS_FSFW MATCHES "host")
    set(BSP_PATH bsp_hosted)
elseif(OS_FSFW MATCHES "freertos")
    set(BSP_PATH sampleapp)
endif()

# Analyse different OS and architecture/target options, determine BSP_PATH,
# display information about compiler etc.

set(FSFW_CONFIG_PATH "${BSP_PATH}/fsfwconfig")

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(WARNING_FLAGS
        -Wall
        -Wextra
        -Wimplicit-fallthrough=1
        -Wno-unused-parameter
        -Wno-psabi
    )

    if(NOT ${TGT_BSP} MATCHES "arm/raspberrypi")
        # The RPi compiler is older and emitts odd warnings if this is used.
        set(WARNING_FLAGS ${WARNING_FLAGS} -Wshadow=local)
    endif()

    set(FSFW_WARNING_FLAGS ${WARNING_FLAGS})
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/permissive- /d2SSAOptimizer-)
    # To avoid nameclashes with min and max macro
    add_compile_definitions(NOMINMAX)
endif()

################################################################################
# Executable and Sources
################################################################################

# Add executable
add_executable(${TARGET_NAME})

# Add subdirectories
add_subdirectory(${BSP_PATH})
add_subdirectory(${FSFW_PATH})
add_subdirectory(${MISSION_PATH})
add_subdirectory(${TEST_PATH})
if(ADD_ETL_LIB)
    add_subdirectory(${ETL_PATH})
endif()

if(OS_FSFW MATCHES "freertos")
    add_subdirectory(${LIB_OS_NAME})
endif()

################################################################################
# Post-Sources preparation
################################################################################

# Add libraries for all sources.
target_link_libraries(${TARGET_NAME} PRIVATE
    ${LIB_FSFW_NAME} 
)

if(ADD_ETL_LIB)
    target_link_libraries(${TARGET_NAME} PRIVATE
        ${LIB_ETL_NAME}
    )
endif()

# Add include paths for all sources.
target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${FSFW_CONFIG_PATH}
)    


if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(${TARGET_NAME} PRIVATE
        "-Wl,-Map=${TARGET_NAME}.map"
    )

    # Remove unused sections.
    target_compile_options(${TARGET_NAME} PRIVATE
        "-ffunction-sections"
        "-fdata-sections"
    )

    # Removed unused sections.
    if(NOT ${OS_FSFW} MATCHES rtems)
        target_link_options(${TARGET_NAME} PRIVATE
            "-Wl,--gc-sections"
        )
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
endif()

if(CMAKE_VERBOSE)
    message(STATUS "Warning flags: ${WARNING_FLAGS}")
endif()
         

# Compile options for all sources.
target_compile_options(${TARGET_NAME} PRIVATE
    ${WARNING_FLAGS}
)

if(NOT CMAKE_SIZE)
    set(CMAKE_SIZE size)
    if(WIN32)
        set(FILE_SUFFIX ".exe")
    endif()
endif()

if(TGT_BSP)
    set(TARGET_STRING "Target BSP: ${TGT_BSP}")
else()
    set(TARGET_STRING "Target BSP: Hosted")
endif()

string(CONCAT POST_BUILD_COMMENT
    "Build directory: ${CMAKE_BINARY_DIR}\n"
    "Target OSAL: ${OS_FSFW}\n"
    "Target Build Type: ${CMAKE_BUILD_TYPE}\n"
    "${TARGET_STRING}"
)

add_custom_command(
   TARGET ${TARGET_NAME}
   POST_BUILD
   COMMAND ${CMAKE_SIZE} ${TARGET_NAME}${FILE_SUFFIX}
   COMMENT ${POST_BUILD_COMMENT}
)

include (${CMAKE_SCRIPT_PATH}/BuildType.cmake)
set_build_type()





