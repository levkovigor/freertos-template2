add_subdirectory(pvch)

target_sources(${TARGET_NAME} PRIVATE
    PVCHHandler.cpp
)