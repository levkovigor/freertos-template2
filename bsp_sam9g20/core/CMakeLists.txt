target_sources(${TARGET_NAME} PRIVATE
    CoreController.cpp
    ImageCopyingEngine.cpp
    RingBufferAnalyzer.cpp
    ScrubbingEngine.cpp
    SoftwareImageHandler.cpp
    SystemStateTask.cpp
)

if(BOARD_IOBC)
    add_subdirectory(iobc)
else()
    add_subdirectory(at91)
endif()