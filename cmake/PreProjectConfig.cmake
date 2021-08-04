function(options_config)

option(ADD_ETL_LIB "Add ETL library" ON)
option(BOOTLOADER "Build bootloader" OFF)
option(USE_LTO "Enable link-time optimization" OFF)

option(BUILD_UNITTEST "Build Catch2 unit tests" OFF)

if(BUILD_UNITTEST)
    option(FSFW_CUSTOM_UNITTEST_RUNNER
        "Specify whether custom main or Catch2 main is used" OFF
    )
    option(FSFW_ADD_UNITTESTS "Specify whether FSFW tests are added" OFF)
    set(HOST_BUILD ON CACHE BOOL "Host build set to on for unit tests")
else()
    option(HOST_BUILD "Build for host" OFF)
endif()

# This option can also decrease binary size.
option(DISABLE_AGGRESSIVE_OPTIMIZATION 
    "Enable to use O2 instead of O3 optimization for the release build" OFF
)

if(NOT FSFW_OSAL)
    if(HOST_BUILD)
        set(FSFW_OSAL host CACHE STRING "OS for the FSFW.")
    else()
        set(FSFW_OSAL freertos CACHE STRING "OS for the FSFW.")
    endif()
endif()

if(NOT HOST_BUILD)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SCRIPT_PATH}/Arm926ejsCrossCompile.cmake PARENT_SCOPE)

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
             set(BL_USE_FREERTOS OFF PARENT_SCOPE)
        endif()
    endif()
endif()

if(BOOTLOADER)
    set(AT91_USE_AT91_STDIO ON PARENT_SCOPE)
endif()

if(BOARD_IOBC)
    set(BOARD_PRINTOUT "iOBC" PARENT_SCOPE)
    if(BOOTLOADER)
        add_definitions(-DISIS_BL)
        set(AT91_MEMORY norflash PARENT_SCOPE)
    else()
        set(AT91_MEMORY sdram PARENT_SCOPE)
        if(SAT_BUILD)
            add_definitions(-DSAT_BUILD)
        endif()
    endif()
else()
    set(BOARD_PRINTOUT "AT91SAM9G20-EK" PARENT_SCOPE)
    if(BOOTLOADER)
        add_definitions(-DBOOTLOADER)
        if(USE_LTO)
            set(AT91_USE_LINK_TIME_OPTIMIZATION ON PARENT_SCOPE)
        endif()
        set(AT91_ISOLATE_ERRONEOUS_PATHS OFF PARENT_SCOPE)
        if(NOT BL_STAGE_TWO)
            add_definitions(-DAT91_BL_ST1)
            set(AT91_MEMORY sram PARENT_SCOPE)
        else()
            add_definitions(-DAT91_BL_ST2)
            set(AT91_MEMORY sdram PARENT_SCOPE)
        endif()
    else()
        set(AT91_MEMORY sdram PARENT_SCOPE)
    endif()
endif()

endfunction()
