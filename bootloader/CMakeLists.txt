if(BOARD_IOBC)
    add_subdirectory(iobc)
else()
    add_subdirectory(at91)
endif()

add_subdirectory(utility)
add_subdirectory(config)
add_subdirectory(core)

target_sources(${TARGET_NAME} PRIVATE
    main.c
)