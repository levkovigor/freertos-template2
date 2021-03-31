add_subdirectory(common)
add_subdirectory(norflash)

target_sources(${TARGET_NAME} PRIVATE
    main_iobc.c
)