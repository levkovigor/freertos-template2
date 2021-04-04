if(BOARD_IOBC)
    target_sources(${TARGET_NAME} PRIVATE
        FRAMApi.c
        FRAMNoOs.c
        FRAMApiNoOs.c
    )
else()
    if(NOT SIMPLE_AT91_BL)
        target_sources(${TARGET_NAME} PRIVATE
            VirtualFRAMApi.c
        )
    endif()
endif()
