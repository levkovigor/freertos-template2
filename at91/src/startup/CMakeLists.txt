if(NOT BOOTLOADER)
    target_sources(${LIB_AT91_NAME} PRIVATE
        board_cstartup_freeRTOS.S
    )
else()
    if(BOARD_IOBC)
        if(NOT BL_USE_FREERTOS)
            target_sources(${LIB_AT91_NAME} PRIVATE
                board_cstartup.S
            )
        else()
            target_sources(${LIB_AT91_NAME} PRIVATE
                board_cstartup_freeRTOS_norflash.S
            )
        endif()   
    else()
        if(BL_STAGE_TWO)
            if(NOT BL_USE_FREERTOS)
                target_sources(${LIB_AT91_NAME} PRIVATE
                    board_cstartup.S
                )
            else()
                target_sources(${LIB_AT91_NAME} PRIVATE
                    board_cstartup_freeRTOS.S
                )
            endif()
        else()
            target_sources(${LIB_AT91_NAME} PRIVATE
                board_cstartup.S
            )
        endif()
    endif()
endif()