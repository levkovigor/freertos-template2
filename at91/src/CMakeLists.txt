target_sources(${LIB_AT91_NAME} PRIVATE
    syscalls.c
)

add_subdirectory(peripherals)
add_subdirectory(utility)
add_subdirectory(startup)
add_subdirectory(boards)
add_subdirectory(memories)
add_subdirectory(usb)