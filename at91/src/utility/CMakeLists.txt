target_sources(${LIB_AT91_NAME} PRIVATE
    trace.c
    ExitHandler.c
    hamming.c
)

if(AT91_USE_AT91_STDIO)
    target_sources(${LIB_AT91_NAME} PRIVATE
        stdio.c
        # string.c
    )
endif()