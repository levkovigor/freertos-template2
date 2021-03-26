if(BOARD_IOBC)
    target_sources(${TARGET_NAME} PRIVATE
        FRAMApi.c
        FRAMNoOs.c
    )
else()
    if(NOT SIMPLE_AT91_BL)
        target_sources(${TARGET_NAME} PRIVATE
            VirtualFRAMApi.c
        )
    endif()
endif()

target_sources(${TARGET_NAME} PRIVATE
    lowlevel.c
)

if(NOT SIMPLE_AT91_BL)
    target_sources(${TARGET_NAME} PRIVATE
        SRAMApi.c
        norflashHook.c
        SDCardApi.c
    )

    if(BOOTLOADER)
        if(BL_USE_FREERTOS)
            target_sources(${TARGET_NAME} PRIVATE
                watchdog.c
            )
        endif()
    else()
        target_sources(${TARGET_NAME} PRIVATE
            watchdog.c
        )
    endif()
endif()

add_subdirectory(config)