if(BOARD_IOBC)
    target_sources(${TARGET_NAME} PRIVATE
        FRAMApi.c
        FRAMNoOs.c
    )
else()
    target_sources(${TARGET_NAME} PRIVATE
        VirtualFRAMApi.c
    )
endif()

target_sources(${TARGET_NAME} PRIVATE
    lowlevel.c
    SDCardApi.c
    SRAMApi.c
    watchdog.c
    norflashHook.c
)