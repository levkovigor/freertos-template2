target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(${TARGET_NAME} PRIVATE
    ipc/missionMessageTypes.cpp
    pollingsequence/PollingSequenceFactory.cpp
)