target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(${TARGET_NAME} PRIVATE
    ipc/missionMessageTypes.cpp
    pollingsequence/PollingSequenceFactory.cpp
)

# If a special translation file for object IDs exists, compile it.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/objects/translateObjects.cpp")
    target_sources(${TARGET_NAME} PRIVATE
        objects/translateObjects.cpp
    )
endif()

# If a special translation file for events exists, compile it.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/objects/translateObjects.cpp")
    target_sources(${TARGET_NAME} PRIVATE
        events/translateEvents.cpp
    )
endif()
