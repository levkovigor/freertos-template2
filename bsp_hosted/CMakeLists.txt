target_sources(${TARGET_NAME} PRIVATE
    ObjectFactory.cpp
    InitMission.cpp
    main.cpp
)

add_subdirectory(fsfwconfig)
add_subdirectory(boardtest)
add_subdirectory(boardconfig)

# Add include paths for the executable
target_include_directories(${TARGET_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)
