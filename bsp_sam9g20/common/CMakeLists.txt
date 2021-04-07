target_sources(${TARGET_NAME} PRIVATE
    lowlevel.c
    At91SpiDriver.c
	SDCardApi.c
	SRAMApi.c
)

if(NOT SIMPLE_AT91_BL)
    target_sources(${TARGET_NAME} PRIVATE
        norflashHook.c
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
add_subdirectory(fram)