target_sources(${TARGET_NAME} PRIVATE
    ObjectFactory.cpp
    InitMission.cpp
    main.cpp
)

add_subdirectory(fsfwconfig)
add_subdirectory(boardtest)
add_subdirectory(boardconfig)
