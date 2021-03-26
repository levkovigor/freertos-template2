if(BL_USE_FREERTOS)
    target_sources(${TARGET_NAME} PRIVATE
        hooks.c
    )
endif()

target_sources(${TARGET_NAME} PRIVATE
    faultHandler.c
    CRC.c
)
