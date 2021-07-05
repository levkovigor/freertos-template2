target_sources(${TARGET_NAME} PRIVATE
    pollingsequence/PollingSequenceFactory.cpp
    ipc/MissionMessageTypes.cpp
)

# Add include paths for the executable
target_include_directories(${TARGET_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
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
