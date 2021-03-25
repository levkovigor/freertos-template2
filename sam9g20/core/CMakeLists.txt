target_sources(${TARGET_NAME} PRIVATE
    CoreController.cpp
    ImageCopyingEngine.cpp
    RingBufferAnalyzer.cpp
    ScrubbingEngine.cpp
    SoftwareImageHandler.cpp
    SystemStateTask.cpp
)

add_subdirectory(at91)